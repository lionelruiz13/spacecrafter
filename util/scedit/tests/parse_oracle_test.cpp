/*
 * scedit — differential test against the engine's own parser.
 *
 * The table-driven test (tokenizer_test.cpp) pins the tokenization scedit
 * BELIEVES the engine performs. This one removes the belief: it carries a
 * VERBATIM COPY of AppCommandInterface::parseCommand and compares scedit's
 * answer to the real function's answer, over
 *   (a) an exhaustive enumeration of short strings over the alphabet that
 *       drives every branch of the parser — {a, b, space, tab, "} up to 6
 *       bytes and {a, space, "} up to 9 bytes, and
 *   (b) every line of the real corpus handed to it on the command line.
 *
 * The copy below is the oracle. It must stay byte-identical to
 * src/interfaceModule/app_command_interface.cpp:124-176 (minus the
 * `#ifdef PARSE_DEBUG` logging block, which the engine does not compile
 * either). Editing it to "fix" a disagreement inverts the authority chain:
 * on disagreement, the oracle is right and scedit is the defect.
 *
 * ONE EXCEPTION, BY RULING, AND IT IS THE TARGET, NOT A FIX. The comment cut at
 * the top of the copy (a '#' outside a "..." run ends the command) is the
 * behaviour Vixy ruled for the engine (2026-08-30) and ordered scedit to model
 * FIRST (2026-08-31: "make scedit track what the HEAD would be after the
 * behavior get corrected, then we correct spacecrafter to be in face"). The
 * block is written here in the exact form the engine receives; the engine
 * commit that lands it makes the copy verbatim again. Until then the oracle is
 * the target parseCommand, and the authority chain reads: ruled engine > oracle
 * > scedit.
 */

#include "sc_tokenizer.hpp"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

typedef std::map<std::string, std::string> stringHash_t;

// ===========================================================================
// VERBATIM: AppCommandInterface::parseCommand, app_command_interface.cpp:124-176
// + the ruled comment cut (see the header note) in the exact form the engine
// receives it.
// ===========================================================================
int parseCommand(const std::string &command_line, std::string &command, stringHash_t &arguments)
{
  	std::string str = command_line;

	// A '#' outside a "..." run starts a comment: it and everything after it are
	// dropped before parsing. Quotes are counted by a plain toggle from the first
	// byte, so a '#' inside quotes - closed or not - is ordinary text.
	{
		bool inQuote = false;
		for (std::size_t i = 0; i < str.size(); ++i) {
			if (str[i] == '"')
				inQuote = !inQuote;
			else if (str[i] == '#' && !inQuote) {
				str.erase(i);
				break;
			}
		}
	}

	// transformation of the beginning of character strings by deleting spaces and tabs at the beginning of the string
	while (str[0]==' ' || str[0]=='\t') {
        str.erase(0,1);
	}

	// transformation of user strings of the form "text" to "text
  	std::size_t found = str.find(" \" ");
  	while(found!=std::string::npos) {
  		str.erase(found+2,1);
		found = str.find(" \" ");
  	}

	std::istringstream commandstr( str );
	std::string key, value;
	char nextc;

	commandstr >> command;
	transform(command.begin(), command.end(), command.begin(), ::tolower);

	while (commandstr >> key >> value ) {
		if (value[0] == '"') {
			// pull in all text inside quotes
			if (value[value.length()-1] == '"') {
				// one word in quotes
				value = value.substr(1, value.length() -2 );
			} else {
				// multiple words in quotes
				value = value.substr(1, value.length() -1 );

				while (1) {
					nextc = commandstr.get();
					if ( nextc == '"' || !commandstr.good()) break;
					value.push_back( nextc );
				}
			}
		}
		transform(key.begin(), key.end(), key.begin(), ::tolower);
		arguments[key] = value;
	}
	return 1;  // no error checking yet
}
// ===========================================================================

namespace {

long compared = 0;
long mismatches = 0;

std::string show(const std::string &s)
{
	std::string o;
	for (char c : s) {
		if (c == '\t') o += "\\t";
		else if (c == '\r') o += "\\r";
		else if (c == '\n') o += "\\n";
		else if ((unsigned char)c < 32 || (unsigned char)c > 126) {
			char buf[8];
			std::snprintf(buf, sizeof(buf), "\\x%02x", (unsigned char)c);
			o += buf;
		} else o += c;
	}
	return o;
}

std::string dump(const std::string &cmd, const stringHash_t &args)
{
	std::string o = "cmd='" + show(cmd) + "'";
	for (const auto &kv : args)
		o += " {" + show(kv.first) + "=" + show(kv.second) + "}";
	return o;
}

//! One line through both parsers; report any difference in the engine-visible
//! result (command name and the args map, which is all the handlers ever see).
void compare(const std::string &raw)
{
	// The script layer decides whether parseCommand is reached at all
	// (script.cpp:114); lines it drops are not part of this comparison.
	if (scedit::classifyLine(raw) != scedit::LineKind::Parsed)
		return;

	std::string oracle_cmd;
	stringHash_t oracle_args;
	parseCommand(raw, oracle_cmd, oracle_args);

	scedit::Line L = scedit::tokenizeLine(raw);

	++compared;
	if (L.command == oracle_cmd && L.args == oracle_args)
		return;

	++mismatches;
	if (mismatches <= 20) {
		std::printf("  FAIL  line \"%s\"\n", show(raw).c_str());
		std::printf("          oracle %s\n", dump(oracle_cmd, oracle_args).c_str());
		std::printf("          scedit %s\n", dump(L.command, L.args).c_str());
	}
}

//! Every string of length 0..maxlen over `alphabet`, each exactly once.
void enumRec(const std::string &alphabet, std::string &cur, std::size_t remaining)
{
	compare(cur);
	if (!remaining)
		return;
	for (char c : alphabet) {
		cur.push_back(c);
		enumRec(alphabet, cur, remaining - 1);
		cur.pop_back();
	}
}

void enumerate(const std::string &alphabet, std::size_t maxlen)
{
	std::string cur;
	enumRec(alphabet, cur, maxlen);
}

//! Spans are scedit's own contract (the engine has none), so they get their own
//! invariant check rather than an oracle: a span must be inside the line, and
//! the engine-visible text of an UNQUOTED token must be exactly the raw bytes
//! it points at (lowercased for command/key).
void checkSpanInvariants(const std::string &raw)
{
	if (scedit::classifyLine(raw) != scedit::LineKind::Parsed)
		return;
	scedit::Line L = scedit::tokenizeLine(raw);
	for (const auto &t : L.tokens) {
		++compared;
		bool bad = false;
		if (t.span.begin > t.span.end || t.span.end > raw.size())
			bad = true;
		if (!bad && !t.quoted) {
			std::string r = L.rawText(t.span);
			std::string want = (t.role == scedit::TokenRole::Value) ? r : scedit::asciiLower(r);
			if (want != t.text)
				bad = true;
		}
		if (bad) {
			++mismatches;
			if (mismatches <= 20)
				std::printf("  FAIL  span invariant, line \"%s\" token '%s' [%zu,%zu)\n",
				            show(raw).c_str(), show(t.text).c_str(), t.span.begin, t.span.end);
		}
	}
}

} // namespace

int main(int argc, char **argv)
{
	std::printf("scedit parse-oracle differential test\n");

	// (a) exhaustive short strings over the branch-driving alphabet
	enumerate("ab \t\"", 6);
	enumerate("a \"", 9);
	// the comment cut interacts with quotes and separators: {a, space, ", #}
	// up to 8 bytes covers every order of quote/hash/word the rule can meet
	enumerate("a \"#", 8);
	long synthetic = compared;
	std::printf("  %ld synthetic lines compared\n", synthetic);

	// a few bytes above 0x7f: legacy files are ISO-8859, and the engine folds
	// case with ::tolower under LC_CTYPE="C" (src/main.cpp:242 sets LC_TIME only)
	const char *iso[] = {"body name \xc9toile parent Sun", "\xc9TOILE key \xe9", "a \xe9 \xc9"};
	for (const char *s : iso)
		compare(s);

	// (b) the real corpus, line by line
	for (int i = 1; i < argc; ++i) {
		std::ifstream in(argv[i], std::ios::binary);
		if (!in) {
			std::printf("  FAIL  cannot open corpus file %s\n", argv[i]);
			++mismatches;
			continue;
		}
		std::ostringstream ss;
		ss << in.rdbuf();
		const auto lines = scedit::splitScriptLines(ss.str());
		long before = compared;
		for (const auto &l : lines) {
			compare(l);
			checkSpanInvariants(l);
		}
		std::printf("  %s: %zu lines, %ld comparisons\n", argv[i], lines.size(), compared - before);
	}

	std::printf("%ld comparisons, %ld mismatches\n", compared, mismatches);
	return mismatches ? 1 : 0;
}
