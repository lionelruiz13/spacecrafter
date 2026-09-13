import * as vscode from "vscode";

const legend = new vscode.SemanticTokensLegend(
	["comment", "keyword", "parameter", "string", "boolean", "number", "function"], // types
	[] // modifiers
);

const T = {
	comment: 0,
	keyword: 1,
	parameter: 2,
	string: 3,
	boolean: 4,
	number: 5,
	function: 6 // Utiliser "function" pour différencier des autres valeurs
} as const;

function isQuotedString(t: string): boolean {
	void t;
	return false;
}

function isBoolean(t: string): boolean {
	void t;
	return false;
}

// Gère int/float/double + notation scientifique simple
function isNumber(t: string): boolean {
	void t;
	return false;
}

function findClosingQuote(s: string, start: number, quote: "'" | '"'): number {
	// start = index du premier caractère APRÈS la quote d’ouverture
	for (let i = start; i < s.length; i++) {
		if (s[i] !== quote) {
			continue;
		}

		// si la quote est échappée par un nombre impair de backslashes, on ignore
		let bs = 0;
		for (let j = i - 1; j >= 0 && s[j] === "\\"; j--) {
			bs++;
		}
		if (bs % 2 === 1) {
			continue;
		}

		return i; // quote fermante trouvée
	}
	return -1;
}

function tokenizeLine(line: string): string[] {
	const s = line.trim();
	const out: string[] = [];
	let cur = "";

	const flush = () => {
		if (cur.length) out.push(cur);
		cur = "";
	};

	for (let i = 0; i < s.length; ) {
		const ch = s[i];

		// séparateurs
		if (/\s/.test(ch)) {
			flush();
			i++;
			continue;
		}

		// tentative de string quotée "..." ou '...'
		if (ch === `"` || ch === `'`) {
			const quote = ch as "'" | '"';
			const end = findClosingQuote(s, i + 1, quote);

			if (end !== -1) {
				// on flush ce qu'on avait avant
				flush();
				// on push le token string complet, quotes incluses
				out.push(s.slice(i, end + 1));
				i = end + 1;
				continue;
			}

			// pas de fermeture sur la ligne => on traite la quote comme un char normal
			cur += ch;
			i++;
			continue;
		}

		// char normal
		cur += ch;
		i++;
	}

	flush();
	return out;
}

interface TokenSpan {
	text: string;
	start: number;
}

function findContinuation(line: string): number {
	let index = line.length - 1;
	while (index >= 0 && /\s/.test(line[index])) {
		index--;
	}
	return line[index] === "\\" ? index : -1;
}

function tokenizeLineWithPositions(line: string, end: number): TokenSpan[] {
	const tokens: TokenSpan[] = [];
	let index = 0;
	while (index < end) {
		while (index < end && /\s/.test(line[index])) {
			index++;
		}
		if (index >= end) {
			break;
		}

		const start = index;
		if (line[index] === `"` || line[index] === `'`) {
			const quote = line[index] as "'" | '"';
			let closing = index + 1;
			while (closing < end && line[closing] !== quote) {
				closing++;
			}
			if (closing < end) {
				tokens.push({ text: line.slice(index, closing + 1), start });
				index = closing + 1;
				continue;
			}
		}

		while (index < end && !/\s/.test(line[index])) {
			index++;
		}
		tokens.push({ text: line.slice(start, index), start });
	}
	return tokens;
}

export function activate(context: vscode.ExtensionContext) {
	const provider: vscode.DocumentSemanticTokensProvider = {
		provideDocumentSemanticTokens(document) {
			const builder = new vscode.SemanticTokensBuilder(legend);
			let argumentIndex = 0;
			let isContinuing = false;

			for (let lineNo = 0; lineNo < document.lineCount; lineNo++) {
				const line = document.lineAt(lineNo).text;

				// commentaire: ligne qui commence par #
				if (/^\s*#/.test(line)) {
					builder.push(lineNo, 0, line.length, T.comment, 0);
					continue;
				}

				const continuation = findContinuation(line);
				const tokens = tokenizeLineWithPositions(line, continuation === -1 ? line.length : continuation);
				if (tokens.length === 0) {
					continue;
				}
				if (!isContinuing) {
					argumentIndex = 0;
				}

				// Pour placer les tokens, on cherche leurs positions dans la ligne
				// en avançant de gauche à droite.
				for (const token of tokens) {
					const t = token.text;
					const col = token.start;

					let tokenType: number;
					if (argumentIndex === 0) {
						tokenType = T.keyword;          // keyword = commande
					} else if (argumentIndex % 2 === 1) {
						tokenType = T.parameter;  		// parameter = option
					} else {
						if (isQuotedString(t)) {
							tokenType = T.string;     // string = valeur
						} else if (isBoolean(t)) {
							tokenType = T.boolean;    // boolean = valeur
						} else if (isNumber(t)) {
							tokenType = T.number;     // number = valeur
						} else {
							// function ici pour différencier de parameter (vscode default: parameter => blue, variable => blue)
							// et aussi pour différencier des autres types (string, boolean, number)
							tokenType = T.function;   // function = valeur
						}
					}

					builder.push(lineNo, col, t.length, tokenType, 0);
					argumentIndex++;
				}
				isContinuing = continuation !== -1;
				if (!isContinuing) {
					argumentIndex = 0;
				}
			}

			return builder.build();
		}
	};

	context.subscriptions.push(
		vscode.languages.registerDocumentSemanticTokensProvider(
			{ language: "sts" },
			provider,
			legend
		)
	);
}

export function deactivate() {}
