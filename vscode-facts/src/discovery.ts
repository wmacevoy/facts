import * as fs from 'fs';
import * as path from 'path';
import { glob } from './util';

export interface FactsTestInfo {
    name: string;
    file: string;
    line: number;
}

export interface WatchBreakpoint {
    function: string;
    condition: string;
    label: string;
}

// Matches FACTS(Name) declarations, excluding macros and comments
const FACTS_PATTERN = /(?<![A-Za-z0-9_])FACTS\(([A-Za-z_]\w*)\)/;

// Matches FACTS_WATCH(var, ...) for auto-breakpoint generation
const WATCH_PATTERN = /FACTS_WATCH\w*\(([^)]+)\)/;

export class FactsDiscovery {

    async scanWorkspace(cwd: string, sourceGlob: string): Promise<FactsTestInfo[]> {
        const files = await glob(cwd, sourceGlob);
        const tests: FactsTestInfo[] = [];

        for (const file of files) {
            const fileTests = await this.scanFile(file);
            tests.push(...fileTests);
        }

        return tests;
    }

    async scanFile(filePath: string): Promise<FactsTestInfo[]> {
        const tests: FactsTestInfo[] = [];
        let content: string;
        try {
            content = await fs.promises.readFile(filePath, 'utf-8');
        } catch {
            return tests;
        }

        const lines = content.split('\n');
        for (let i = 0; i < lines.length; i++) {
            const line = lines[i];
            // Skip preprocessor directives
            const trimmed = line.trimStart();
            if (trimmed.startsWith('#')) { continue; }

            const match = FACTS_PATTERN.exec(line);
            if (match) {
                const name = match[1];
                // Skip sentinel names
                if (name === '0000_BEGIN' || name === 'zzzz_END') { continue; }
                tests.push({ name, file: filePath, line: i });
            }
        }

        return tests;
    }

    async findWatchBreakpoints(cwd: string): Promise<WatchBreakpoint[]> {
        const breakpoints: WatchBreakpoint[] = [];
        const files = await glob(cwd, '**/*.{c,cpp,cc,cxx}');

        for (const file of files) {
            let content: string;
            try {
                content = await fs.promises.readFile(file, 'utf-8');
            } catch {
                continue;
            }

            let currentFacts: string | undefined;
            let traceIndex = 0;
            const lines = content.split('\n');

            for (let i = 0; i < lines.length; i++) {
                const line = lines[i];

                // Track which FACTS block we're in
                const factsMatch = FACTS_PATTERN.exec(line);
                if (factsMatch && !line.trimStart().startsWith('#')) {
                    currentFacts = factsMatch[1];
                    traceIndex = 0;
                }

                if (!currentFacts) { continue; }

                // Look for FACTS_WATCH
                const watchMatch = WATCH_PATTERN.exec(line);
                if (watchMatch) {
                    const vars = watchMatch[1].split(',').map(v => v.trim());
                    const funcName = `facts_${currentFacts}_function`;
                    const label = `facts_trace_${traceIndex}`;
                    // Build a condition from the watched variables
                    // The user sets breakpoint conditions in the debugger
                    breakpoints.push({
                        function: `${funcName}:${label}`,
                        condition: vars.map(v => `${v}==${v}`).join(' && '),
                        label: `${currentFacts}: watch ${vars.join(', ')}`
                    });
                    traceIndex++;
                }
            }
        }

        return breakpoints;
    }
}
