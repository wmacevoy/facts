import * as vscode from 'vscode';
import { execFile, exec } from 'child_process';
import { promisify } from 'util';

const execFileAsync = promisify(execFile);
const execAsync = promisify(exec);

export interface TestResult {
    name: string;
    passed: boolean;
    message?: string;
    file?: string;
    line?: number;
    duration?: number;
}

// Parse facts plain output for pass/fail
// Example lines:
//   PASS Alpha (0.001s)
//   FAIL Integer test/testfacts.c:42 a (1) != b (2) (0.001s)
const RESULT_PATTERN = /^(PASS|FAIL)\s+(\S+)/;
const FAIL_DETAIL = /^FAIL\s+\S+\s+(.+?):(\d+)\s+(.*?)(?:\s+\([\d.]+s\))?$/;

export class FactsRunner {
    private outputChannel: vscode.OutputChannel;

    constructor() {
        this.outputChannel = vscode.window.createOutputChannel('Facts');
    }

    async build(cwd: string, buildCommand: string): Promise<boolean> {
        this.outputChannel.appendLine(`> ${buildCommand}`);
        this.outputChannel.show(true);

        try {
            const { stdout, stderr } = await execAsync(buildCommand, { cwd, timeout: 60000 });
            if (stdout) { this.outputChannel.appendLine(stdout); }
            if (stderr) { this.outputChannel.appendLine(stderr); }
            return true;
        } catch (err: unknown) {
            const e = err as { stdout?: string; stderr?: string; message?: string };
            if (e.stdout) { this.outputChannel.appendLine(e.stdout); }
            if (e.stderr) { this.outputChannel.appendLine(e.stderr); }
            this.outputChannel.appendLine(`Build failed: ${e.message || 'unknown error'}`);
            return false;
        }
    }

    async run(cwd: string, exe: string, includes: string[], token: vscode.CancellationToken): Promise<TestResult[]> {
        const args = ['--facts_plain'];
        for (const name of includes) {
            args.push(`--facts_include=${name}`);
        }

        this.outputChannel.appendLine(`> ${exe} ${args.join(' ')}`);

        const results: TestResult[] = [];
        const start = Date.now();

        try {
            const proc = execFile(exe, args, { cwd, timeout: 120000 });

            // Allow cancellation
            const disposable = token.onCancellationRequested(() => {
                proc.kill();
            });

            const { stdout, stderr } = await new Promise<{ stdout: string; stderr: string }>((resolve, reject) => {
                let out = '';
                let err = '';
                proc.stdout?.on('data', (d: Buffer) => { out += d.toString(); });
                proc.stderr?.on('data', (d: Buffer) => { err += d.toString(); });
                proc.on('close', () => resolve({ stdout: out, stderr: err }));
                proc.on('error', reject);
            });

            disposable.dispose();

            if (stdout) { this.outputChannel.appendLine(stdout); }
            if (stderr) { this.outputChannel.appendLine(stderr); }

            // Parse output
            const output = stdout + '\n' + stderr;
            const elapsed = Date.now() - start;

            for (const line of output.split('\n')) {
                const m = RESULT_PATTERN.exec(line);
                if (!m) { continue; }

                const passed = m[1] === 'PASS';
                const name = m[2];
                const result: TestResult = { name, passed, duration: elapsed };

                if (!passed) {
                    const detail = FAIL_DETAIL.exec(line);
                    if (detail) {
                        result.file = detail[1];
                        result.line = parseInt(detail[2], 10) - 1; // 0-indexed
                        result.message = detail[3];
                    } else {
                        result.message = line;
                    }
                }

                results.push(result);
            }
        } catch (err: unknown) {
            const e = err as { stdout?: string; stderr?: string; message?: string };
            // Facts exits 1 on failure — that's expected, parse output anyway
            const output = (e.stdout || '') + '\n' + (e.stderr || '');
            this.outputChannel.appendLine(output);

            for (const line of output.split('\n')) {
                const m = RESULT_PATTERN.exec(line);
                if (!m) { continue; }

                const passed = m[1] === 'PASS';
                const name = m[2];
                const result: TestResult = { name, passed, duration: Date.now() - start };

                if (!passed) {
                    const detail = FAIL_DETAIL.exec(line);
                    if (detail) {
                        result.file = detail[1];
                        result.line = parseInt(detail[2], 10) - 1;
                        result.message = detail[3];
                    } else {
                        result.message = line;
                    }
                }

                results.push(result);
            }
        }

        // For any included test not in results, mark as not run
        for (const name of includes) {
            if (!results.find(r => r.name === name)) {
                results.push({ name, passed: false, message: 'Test did not produce output' });
            }
        }

        return results;
    }
}
