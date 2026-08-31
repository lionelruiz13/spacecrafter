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
	function: 6 // Utiliser "function" pour differencier des autres valeurs
} as const;

function isQuotedString(t: string): boolean {
	if (t.length < 2) {
		return false;
	}
	const a = t[0], b = t[t.length - 1];
	return (a === `"` && b === `"`) || (a === `'` && b === `'`);
}

function isBoolean(t: string): boolean {
	t = t.toLowerCase();
	return t === "true" || t === "false";
}

// Gere int/float/double + notation scientifique simple
function isNumber(t: string): boolean {
	// refuser les trucs du genre "-" ou "."
	if (!t || t === "-" || t === "." || t === "+") {
		return false;
	}
	return /^[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?$/.test(t);
}

function findClosingQuote(s: string, start: number, quote: "'" | '"'): number {
	// start = index du premier caractere APRES la quote d'ouverture
	for (let i = start; i < s.length; i++) {
		if (s[i] !== quote) {
			continue;
		}

		// si la quote est echappee par un nombre impair de backslashes, on ignore
		let bs = 0;
		for (let j = i - 1; j >= 0 && s[j] === "\\"; j--) {
			bs++;
		}
		if (bs % 2 === 1) {
			continue;
		}

		return i; // quote fermante trouvee
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

		// separateurs
		if (/\s/.test(ch)) {
			flush();
			i++;
			continue;
		}

		// tentative de string quotee "..." ou '...'
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

export function activate(context: vscode.ExtensionContext) {
	const provider: vscode.DocumentSemanticTokensProvider = {
		provideDocumentSemanticTokens(document) {
			const builder = new vscode.SemanticTokensBuilder(legend);

			for (let lineNo = 0; lineNo < document.lineCount; lineNo++) {
				const line = document.lineAt(lineNo).text;

				// commentaire: ligne qui commence par #
				if (/^\s*#/.test(line)) {
					builder.push(lineNo, 0, line.length, T.comment, 0);
					continue;
				}

				const trimmed = line.trim();
				if (!trimmed) {
					continue;
				}

				const tokens = tokenizeLine(line);
				if (tokens.length === 0) {
					continue;
				}

				// Pour placer les tokens, on cherche leurs positions dans la ligne
				// en avancant de gauche a droite.
				let searchFrom = 0;

				for (let i = 0; i < tokens.length; i++) {
					const t = tokens[i];
					const col = line.indexOf(t, searchFrom);
					if (col === -1) {
						continue;
					}

					let tokenType: number;
					if (i === 0) {
						tokenType = T.keyword;          // keyword = commande
					} else if (i % 2 === 1) {
						tokenType = T.parameter;  		// parameter = option
					} else {
						if (isQuotedString(t)) {
							tokenType = T.string;     // string = valeur
						} else if (isBoolean(t)) {
							tokenType = T.boolean;    // boolean = valeur
						} else if (isNumber(t)) {
							tokenType = T.number;     // number = valeur
						} else {
							// function ici pour differencier de parameter (vscode default: parameter => blue, variable => blue)
							// et aussi pour differencier des autres types (string, boolean, number)
							tokenType = T.function;   // function = valeur
						}
					}

					builder.push(lineNo, col, t.length, tokenType, 0);
					searchFrom = col + t.length;
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
