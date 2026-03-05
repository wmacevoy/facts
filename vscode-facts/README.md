# Facts Test Runner for VSCode

Run and debug [Facts](https://github.com/wmacevoy/facts) C/C++ tests directly from VSCode.

## Features

- **Test Discovery** — Scans source files for `FACTS(Name)` declarations and shows them in the Test Explorer sidebar.
- **Run Tests** — Build and run individual tests or all tests with one click.
- **Debug Tests** — Launch the debugger on a specific test with `--facts_include`.
- **FACTS_WATCH Breakpoints** — Automatically sets conditional breakpoints from `FACTS_WATCH(var, ...)` annotations.
- **Failure Navigation** — Click a failed test to jump to the failing `FACT(a,op,b)` line.

## Setup

1. Open a workspace containing Facts test sources (any file that `#include "facts.h"`).
2. The extension activates automatically and discovers `FACTS()` declarations.
3. Tests appear in the Test Explorer sidebar.

## Configuration

| Setting | Default | Description |
|---|---|---|
| `facts.executable` | (auto-detect) | Path to test executable |
| `facts.sourceGlob` | `**/*.{c,cpp,cc,cxx}` | Glob for source file scanning |
| `facts.buildCommand` | `make` | Build command run before tests |
| `facts.debugger` | `cppdbg` | Debugger type (`cppdbg`, `lldb`, `codelldb`) |

## Building the Extension

From inside the devcontainer (or any environment with Node.js):

```sh
cd vscode-facts
npm install
npm run compile
npm run package   # produces facts-test-runner-0.1.0.vsix
```

Install the `.vsix` in VSCode via *Extensions > Install from VSIX*.

## How It Works

1. **Discovery**: Scans workspace source files for `FACTS(Name)` patterns (same regex as `--facts_find`).
2. **Run**: Builds via the configured build command, then runs the executable with `--facts_plain` and `--facts_include=Name`.
3. **Debug**: Launches the configured debugger with `--facts_include=Name`. If `FACTS_WATCH` annotations are found, conditional GDB breakpoints are set automatically.
