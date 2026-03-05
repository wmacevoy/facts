import * as vscode from 'vscode';
import { FactsTestProvider } from './testProvider';

export function activate(context: vscode.ExtensionContext) {
    const provider = new FactsTestProvider(context);

    context.subscriptions.push(
        vscode.commands.registerCommand('facts.runAll', () => provider.runAll()),
        vscode.commands.registerCommand('facts.runTest', () => provider.runTestAtCursor()),
        vscode.commands.registerCommand('facts.debugTest', () => provider.debugTestAtCursor()),
        vscode.commands.registerCommand('facts.discover', () => provider.discover()),
    );

    // Auto-discover on activation
    provider.discover();
}

export function deactivate() {}
