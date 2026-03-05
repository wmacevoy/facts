import * as vscode from 'vscode';
import * as path from 'path';

export async function glob(cwd: string, pattern: string): Promise<string[]> {
    const relPattern = new vscode.RelativePattern(cwd, pattern);
    const uris = await vscode.workspace.findFiles(relPattern, '**/node_modules/**');
    return uris.map(u => u.fsPath);
}
