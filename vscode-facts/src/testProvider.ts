import * as vscode from 'vscode';
import * as path from 'path';
import { FactsDiscovery, FactsTestInfo } from './discovery';
import { FactsRunner } from './runner';

export class FactsTestProvider {
    private controller: vscode.TestController;
    private discovery: FactsDiscovery;
    private runner: FactsRunner;
    private testMap = new Map<string, FactsTestInfo>();

    constructor(context: vscode.ExtensionContext) {
        this.controller = vscode.tests.createTestController('factsTests', 'Facts Tests');
        context.subscriptions.push(this.controller);

        this.discovery = new FactsDiscovery();
        this.runner = new FactsRunner();

        this.controller.createRunProfile(
            'Run',
            vscode.TestRunProfileKind.Run,
            (request, token) => this.runTests(request, token),
            true
        );

        this.controller.createRunProfile(
            'Debug',
            vscode.TestRunProfileKind.Debug,
            (request, token) => this.debugTests(request, token),
            true
        );

        // Watch for file changes to re-discover
        const watcher = vscode.workspace.createFileSystemWatcher('**/*.{c,cpp,cc,cxx,h,hpp}');
        watcher.onDidChange(() => this.discover());
        watcher.onDidCreate(() => this.discover());
        watcher.onDidDelete(() => this.discover());
        context.subscriptions.push(watcher);
    }

    async discover() {
        const workspaceFolder = vscode.workspace.workspaceFolders?.[0];
        if (!workspaceFolder) { return; }

        const config = vscode.workspace.getConfiguration('facts');
        const glob = config.get<string>('sourceGlob', '**/*.{c,cpp,cc,cxx}');

        const tests = await this.discovery.scanWorkspace(workspaceFolder.uri.fsPath, glob);

        // Clear existing items
        this.controller.items.replace([]);
        this.testMap.clear();

        for (const test of tests) {
            const item = this.controller.createTestItem(
                test.name,
                test.name,
                vscode.Uri.file(test.file)
            );
            item.range = new vscode.Range(test.line, 0, test.line, 0);
            this.controller.items.add(item);
            this.testMap.set(test.name, test);
        }

        vscode.commands.executeCommand('setContext', 'facts.hasTests', tests.length > 0);
    }

    async runAll() {
        const request = new vscode.TestRunRequest();
        await this.runTests(request, new vscode.CancellationTokenSource().token);
    }

    async runTestAtCursor() {
        const test = this.getTestAtCursor();
        if (test) {
            const item = this.controller.items.get(test.name);
            if (item) {
                const request = new vscode.TestRunRequest([item]);
                await this.runTests(request, new vscode.CancellationTokenSource().token);
            }
        }
    }

    async debugTestAtCursor() {
        const test = this.getTestAtCursor();
        if (test) {
            const item = this.controller.items.get(test.name);
            if (item) {
                const request = new vscode.TestRunRequest([item]);
                await this.debugTests(request, new vscode.CancellationTokenSource().token);
            }
        }
    }

    private getTestAtCursor(): FactsTestInfo | undefined {
        const editor = vscode.window.activeTextEditor;
        if (!editor) { return undefined; }

        const line = editor.selection.active.line;
        const file = editor.document.uri.fsPath;

        // Find closest test at or before cursor
        let closest: FactsTestInfo | undefined;
        for (const test of this.testMap.values()) {
            if (test.file === file && test.line <= line) {
                if (!closest || test.line > closest.line) {
                    closest = test;
                }
            }
        }
        return closest;
    }

    private async runTests(request: vscode.TestRunRequest, token: vscode.CancellationToken) {
        const run = this.controller.createTestRun(request);
        const workspaceFolder = vscode.workspace.workspaceFolders?.[0];
        if (!workspaceFolder) { run.end(); return; }

        const config = vscode.workspace.getConfiguration('facts');
        const buildCmd = config.get<string>('buildCommand', 'make');
        const exe = await this.resolveExecutable(workspaceFolder.uri.fsPath, config);

        if (!exe) {
            vscode.window.showErrorMessage('Facts: No test executable found. Set facts.executable or ensure `make` builds one.');
            run.end();
            return;
        }

        // Build first
        const buildOk = await this.runner.build(workspaceFolder.uri.fsPath, buildCmd);
        if (!buildOk) {
            vscode.window.showErrorMessage('Facts: Build failed. Check the output panel.');
            run.end();
            return;
        }

        // Determine which tests to run
        const includes: string[] = [];
        if (request.include) {
            for (const item of request.include) {
                includes.push(item.id);
                run.started(item);
            }
        } else {
            this.controller.items.forEach(item => {
                run.started(item);
                includes.push(item.id);
            });
        }

        const results = await this.runner.run(workspaceFolder.uri.fsPath, exe, includes, token);

        for (const result of results) {
            const item = this.controller.items.get(result.name);
            if (!item) { continue; }

            if (result.passed) {
                run.passed(item, result.duration);
            } else {
                const message = new vscode.TestMessage(result.message || 'Test failed');
                if (result.file && result.line !== undefined) {
                    message.location = new vscode.Location(
                        vscode.Uri.file(result.file),
                        new vscode.Position(result.line, 0)
                    );
                }
                run.failed(item, message, result.duration);
            }
        }

        run.end();
    }

    private async debugTests(request: vscode.TestRunRequest, _token: vscode.CancellationToken) {
        const workspaceFolder = vscode.workspace.workspaceFolders?.[0];
        if (!workspaceFolder) { return; }

        const config = vscode.workspace.getConfiguration('facts');
        const buildCmd = config.get<string>('buildCommand', 'make');
        const debuggerType = config.get<string>('debugger', 'cppdbg');
        const exe = await this.resolveExecutable(workspaceFolder.uri.fsPath, config);

        if (!exe) {
            vscode.window.showErrorMessage('Facts: No test executable found.');
            return;
        }

        // Build first
        const buildOk = await this.runner.build(workspaceFolder.uri.fsPath, buildCmd);
        if (!buildOk) {
            vscode.window.showErrorMessage('Facts: Build failed.');
            return;
        }

        // Build args
        const args: string[] = [];
        if (request.include) {
            for (const item of request.include) {
                args.push(`--facts_include=${item.id}`);
            }
        }

        // Set FACTS_WATCH breakpoints automatically
        const breakpoints = await this.discovery.findWatchBreakpoints(workspaceFolder.uri.fsPath);

        // Launch debugger
        const debugConfig: vscode.DebugConfiguration = {
            type: debuggerType,
            name: 'Facts Debug',
            request: 'launch',
            program: path.isAbsolute(exe) ? exe : path.join(workspaceFolder.uri.fsPath, exe),
            args: args,
            cwd: workspaceFolder.uri.fsPath,
            stopAtEntry: false,
        };

        // For cppdbg, add setup commands for FACTS_WATCH breakpoints
        if (debuggerType === 'cppdbg' && breakpoints.length > 0) {
            debugConfig.setupCommands = breakpoints.map(bp => ({
                description: `Facts watch: ${bp.label}`,
                text: `break ${bp.function} if ${bp.condition}`,
                ignoreFailures: true
            }));
        }

        await vscode.debug.startDebugging(workspaceFolder, debugConfig);
    }

    private async resolveExecutable(cwd: string, config: vscode.WorkspaceConfiguration): Promise<string | undefined> {
        const configured = config.get<string>('executable', '');
        if (configured) { return configured; }

        // Auto-detect: look for common patterns
        const candidates = [
            'bin/testfacts_c', 'bin/testfacts_cpp',
            'build/testfacts_c', 'build/testfacts_cpp',
            'check', 'test', 'a.out'
        ];

        const fs = await import('fs');
        for (const c of candidates) {
            const full = path.join(cwd, c);
            try {
                await fs.promises.access(full, fs.constants.X_OK);
                return c;
            } catch {
                // not found, try next
            }
        }
        return undefined;
    }
}
