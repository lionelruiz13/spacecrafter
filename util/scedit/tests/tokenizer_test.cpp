/*
 * scedit — differential self-test for the tokenizer.
 *
 * Every case below is a sharp edge of the engine's parse model, written as a
 * constructed line plus the tokenization the engine produces for it. The
 * expected values are DERIVED FROM THE ENGINE SOURCE, not from running this
 * code: each group names the parseCommand line it pins
 * (src/interfaceModule/app_command_interface.cpp:124-176) and the clause of
 * grammar/sc-grammar.json's parse_model it covers. The clause-by-clause map is
 * util/scedit/tests/derivation-diff.md.
 *
 * Spans are RAW byte offsets, half-open, and are given as literal numbers on
 * purpose: an off-by-one in the raw<->token mapping is exactly the defect this
 * test exists to catch, and a computed expectation would move with the bug.
 */

#include "sc_tokenizer.hpp"

#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

using namespace scedit;

namespace {

int failures = 0;
int checks = 0;

void ok(bool cond, const std::string &what)
{
	++checks;
	if (!cond) {
		++failures;
		std::printf("  FAIL  %s\n", what.c_str());
	}
}

template <typename A, typename B>
void eq(const A &got, const B &want, const std::string &what)
{
	++checks;
	if (!(got == want)) {
		++failures;
		std::ostringstream o;
		o << what << ": got [" << got << "] want [" << want << "]";
		std::printf("  FAIL  %s\n", o.str().c_str());
	}
}

const char *roleName(TokenRole r)
{
	switch (r) {
		case TokenRole::Command: return "cmd";
		case TokenRole::Key: return "key";
		case TokenRole::Value: return "val";
		case TokenRole::DanglingKey: return "dangling";
	}
	return "?";
}

struct Exp {
	TokenRole role;
	const char *text;
	std::size_t begin;
	std::size_t end;
};

//! Assert the whole token list of a line, roles / engine-visible text / raw spans.
void expectTokens(const std::string &label, const std::string &raw, std::vector<Exp> exp)
{
	Line L = tokenizeLine(raw);
	if (L.tokens.size() != exp.size()) {
		++failures;
		++checks;
		std::printf("  FAIL  %s: token count got %zu want %zu\n", label.c_str(),
		            L.tokens.size(), exp.size());
		for (const auto &t : L.tokens)
			std::printf("          got %-8s '%s' [%zu,%zu)\n", roleName(t.role),
			            t.text.c_str(), t.span.begin, t.span.end);
		return;
	}
	for (std::size_t i = 0; i < exp.size(); ++i) {
		const Token &t = L.tokens[i];
		const std::string w = label + " token " + std::to_string(i);
		++checks;
		if (t.role != exp[i].role || t.text != exp[i].text
		    || t.span.begin != exp[i].begin || t.span.end != exp[i].end) {
			++failures;
			std::printf("  FAIL  %s: got %s '%s' [%zu,%zu) want %s '%s' [%zu,%zu)\n",
			            w.c_str(), roleName(t.role), t.text.c_str(), t.span.begin, t.span.end,
			            roleName(exp[i].role), exp[i].text, exp[i].begin, exp[i].end);
		}
	}
}

// ---------------------------------------------------------------------------

void testScriptLayer()
{
	// script.cpp:114 — the whole comment rule, and nothing more than it.
	eq((int)classifyLine("# hello"), (int)LineKind::Comment, "classify '#' at column 1");
	eq((int)classifyLine("#"), (int)LineKind::Comment, "classify bare '#'");
	eq((int)classifyLine(""), (int)LineKind::Blank, "classify empty line");
	eq((int)classifyLine("\r"), (int)LineKind::Blank, "classify CR-only line (CRLF file)");
	eq((int)classifyLine("\n"), (int)LineKind::Blank, "classify LF-only line");
	eq((int)classifyLine(std::string(1, '\0')), (int)LineKind::Blank, "classify NUL-first line");
	eq((int)classifyLine("  # indented"), (int)LineKind::Parsed,
	   "classify indented '#': NOT a comment, it reaches the parser");
	eq((int)classifyLine("flag stars on"), (int)LineKind::Parsed, "classify command line");

	// Script::loadInternal reads with std::getline: '\n' delimits, '\r' stays.
	{
		auto v = splitScriptLines("a\nb\n");
		eq(v.size(), (std::size_t)2, "split: trailing newline adds no empty line");
		if (v.size() == 2) { eq(v[0], std::string("a"), "split[0]"); eq(v[1], std::string("b"), "split[1]"); }
	}
	{
		auto v = splitScriptLines("a\nb");
		eq(v.size(), (std::size_t)2, "split: last line without newline is kept");
	}
	{
		auto v = splitScriptLines("a\r\nb\r\n");
		eq(v.size(), (std::size_t)2, "split: CRLF file line count");
		if (v.size() == 2) eq(v[0], std::string("a\r"), "split: CR stays at end of line");
	}
	{
		auto v = splitScriptLines("");
		eq(v.size(), (std::size_t)0, "split: empty file");
	}
	{
		auto v = splitScriptLines("\n");
		eq(v.size(), (std::size_t)1, "split: lone newline is one empty line");
	}
	// A dropped line is returned classified, with no tokens: the engine never
	// parses it, and neither do we.
	{
		Line L = tokenizeLine("# audio volume 128");
		eq((int)L.kind, (int)LineKind::Comment, "comment line kind");
		eq(L.tokens.size(), (std::size_t)0, "comment line is not tokenized");
	}
}

void testBasicShape()
{
	// :145-146 command lowercased; :165 key lowercased; value case preserved.
	expectTokens("simple", "flag stars on", {
		{TokenRole::Command, "flag", 0, 4},
		{TokenRole::Key, "stars", 5, 10},
		{TokenRole::Value, "on", 11, 13}});

	expectTokens("case folding", "FLAG Stars ON", {
		{TokenRole::Command, "flag", 0, 4},
		{TokenRole::Key, "stars", 5, 10},
		{TokenRole::Value, "ON", 11, 13}});

	// :129-132 leading spaces and tabs are stripped; spans stay RAW.
	expectTokens("leading spaces", "   flag stars on", {
		{TokenRole::Command, "flag", 3, 7},
		{TokenRole::Key, "stars", 8, 13},
		{TokenRole::Value, "on", 14, 16}});

	// istringstream separators: tab is one too (and is not stripped mid-line).
	expectTokens("tab separators", "flag\tstars\ton", {
		{TokenRole::Command, "flag", 0, 4},
		{TokenRole::Key, "stars", 5, 10},
		{TokenRole::Value, "on", 11, 13}});

	// A CRLF file's '\r' is whitespace to the parser: it never joins a token.
	expectTokens("CRLF trailing", "flag stars on\r", {
		{TokenRole::Command, "flag", 0, 4},
		{TokenRole::Key, "stars", 5, 10},
		{TokenRole::Value, "on", 11, 13}});

	{
		Line L = tokenizeLine("   ");
		eq((int)L.kind, (int)LineKind::Parsed, "whitespace-only line reaches the parser");
		ok(!L.has_command, "whitespace-only line yields no command (executeCommand:207 no-op)");
		eq(L.tokens.size(), (std::size_t)0, "whitespace-only line has no tokens");
		eq(L.erased.size(), (std::size_t)3, "whitespace-only line: 3 stripped bytes recorded");
	}
	{
		Line L = tokenizeLine("flag");
		eq(L.command, std::string("flag"), "bare command");
		eq(L.args.size(), (std::size_t)0, "bare command has no args");
		ok(!L.has_dangling, "bare command has no dangling key");
	}
}

void testDanglingKey()
{
	// :148 — `while (str >> key >> value)`: the value extraction fails, the body
	// never runs, and the key is dropped without a word.
	expectTokens("dangling only key", "wait duration", {
		{TokenRole::Command, "wait", 0, 4},
		{TokenRole::DanglingKey, "duration", 5, 13}});
	{
		Line L = tokenizeLine("wait duration");
		eq(L.args.size(), (std::size_t)0, "dangling key does not reach args");
		ok(L.has_dangling, "dangling flagged");
	}

	expectTokens("dangling after a pair", "body name x action", {
		{TokenRole::Command, "body", 0, 4},
		{TokenRole::Key, "name", 5, 9},
		{TokenRole::Value, "x", 10, 11},
		{TokenRole::DanglingKey, "action", 12, 18}});
	{
		Line L = tokenizeLine("body name x action");
		eq(L.args.size(), (std::size_t)1, "pairs before a dangling key survive");
		eq(L.args.at("name"), std::string("x"), "surviving pair value");
	}
}

void testDuplicateAndOrder()
{
	// stringHash_t is std::map (tools/utility.hpp:85): last value wins...
	expectTokens("duplicate key", "body halo true halo false", {
		{TokenRole::Command, "body", 0, 4},
		{TokenRole::Key, "halo", 5, 9},
		{TokenRole::Value, "true", 10, 14},
		{TokenRole::Key, "halo", 15, 19},
		{TokenRole::Value, "false", 20, 25}});
	{
		Line L = tokenizeLine("body halo true halo false");
		eq(L.args.size(), (std::size_t)1, "duplicate key collapses to one entry");
		eq(L.args.at("halo"), std::string("false"), "duplicate key: LAST value wins");
		eq(L.pairs.size(), (std::size_t)2, "both pairs kept in line order for diagnostics");
	}
	// ...and iteration is ALPHABETICAL, which is what args.begin() means for the
	// single-pair commands (commandFlag :1183).
	{
		Line L = tokenizeLine("flag zebra on alpha off");
		eq(L.args.begin()->first, std::string("alpha"),
		   "args.begin() is alphabetically first, not line-first");
		eq(L.args.begin()->second, std::string("off"), "applied value follows the applied key");
		eq(L.tokens[1].text, std::string("zebra"), "line order is preserved in tokens");
	}
}

void testQuoting()
{
	// :151-153 one word in quotes
	expectTokens("single-word quotes", "text string \"hello\"", {
		{TokenRole::Command, "text", 0, 4},
		{TokenRole::Key, "string", 5, 11},
		{TokenRole::Value, "hello", 12, 19}});
	{
		Line L = tokenizeLine("text string \"hello\"");
		ok(L.tokens[2].quoted, "quoted flagged");
		ok(L.tokens[2].quote_closed, "closing quote found");
		eq(L.rawText(L.tokens[2].span), std::string("\"hello\""),
		   "raw span covers the quotes, text does not");
	}

	// :156-162 multiple words in quotes: the value pulls raw bytes to the next '"'
	expectTokens("multi-word quotes", "text string \"hello world\" x 1", {
		{TokenRole::Command, "text", 0, 4},
		{TokenRole::Key, "string", 5, 11},
		{TokenRole::Value, "hello world", 12, 25},
		{TokenRole::Key, "x", 26, 27},
		{TokenRole::Value, "1", 28, 29}});

	// :158-160 an unclosed quote consumes to end of line, without error
	expectTokens("unclosed quote", "text string \"hello world", {
		{TokenRole::Command, "text", 0, 4},
		{TokenRole::Key, "string", 5, 11},
		{TokenRole::Value, "hello world", 12, 24}});
	{
		Line L = tokenizeLine("text string \"hello world");
		ok(L.has_unclosed_quote, "unclosed quote flagged on the line");
		ok(!L.tokens[2].quote_closed, "unclosed quote flagged on the token");
	}

	// Degenerate: a LONE '"' takes the one-word branch (value[0] and
	// value[length-1] are the same byte), and substr(1, (size_t)-1) yields "".
	expectTokens("lone quote is an empty value", "text string \"", {
		{TokenRole::Command, "text", 0, 4},
		{TokenRole::Key, "string", 5, 11},
		{TokenRole::Value, "", 12, 13}});
	expectTokens("empty quotes", "text string \"\"", {
		{TokenRole::Command, "text", 0, 4},
		{TokenRole::Key, "string", 5, 11},
		{TokenRole::Value, "", 12, 14}});

	// There is no escape sequence: a backslash is an ordinary byte and the
	// value is simply the token minus its outer quotes.
	expectTokens("no backslash escape", "text string \"a\\\"b\" c 1", {
		{TokenRole::Command, "text", 0, 4},
		{TokenRole::Key, "string", 5, 11},
		{TokenRole::Value, "a\\\"b", 12, 18},
		{TokenRole::Key, "c", 19, 20},
		{TokenRole::Value, "1", 21, 22}});

	// Single quotes are ordinary characters, kept verbatim in the value.
	expectTokens("single quotes are ordinary", "text string 'hello'", {
		{TokenRole::Command, "text", 0, 4},
		{TokenRole::Key, "string", 5, 11},
		{TokenRole::Value, "'hello'", 12, 19}});

	// A quote that is not the FIRST byte of the token does not open anything.
	expectTokens("quote mid-token", "body name abc\"def", {
		{TokenRole::Command, "body", 0, 4},
		{TokenRole::Key, "name", 5, 9},
		{TokenRole::Value, "abc\"def", 10, 17}});

	// A closing quote inside the token is NOT a terminator: the last-byte test
	// fails, so the multi-word branch runs and swallows the rest of the line.
	expectTokens("closing quote mid-token swallows the line", "body name \"hello\"world tail", {
		{TokenRole::Command, "body", 0, 4},
		{TokenRole::Key, "name", 5, 9},
		{TokenRole::Value, "hello\"world tail", 10, 27}});
}

void testSpaceAfterQuoteNormalization()
{
	// :135-139 — every ' " ' loses the byte after the quote, BEFORE tokenizing.
	expectTokens("space after opening quote", "text string \" hello world \"", {
		{TokenRole::Command, "text", 0, 4},
		{TokenRole::Key, "string", 5, 11},
		{TokenRole::Value, "hello world ", 12, 27}});
	{
		Line L = tokenizeLine("text string \" hello world \"");
		eq(L.erased.size(), (std::size_t)1, "one byte erased");
		eq(L.erased[0], (std::size_t)13, "the erased byte is the space after the quote");
		eq(L.normalized, std::string("text string \"hello world \""), "normalized form");

		// The raw<->parsed mapping the TUI's cursor and ghost text depend on.
		std::size_t n = 0;
		ok(!L.rawToNormalized(13, n), "the erased byte has no normalized position");
		ok(L.rawToNormalized(14, n) && n == 13, "raw 14 ('h') is normalized 13");
		eq(L.normalizedToRaw(13), (std::size_t)14, "normalized 13 maps back to raw 14");
		eq(L.normalizedToRaw(0), (std::size_t)0, "identity before any erasure");
		const Token *t = L.tokenAtRawColumn(13);
		ok(t != nullptr && t->role == TokenRole::Value,
		   "the cursor on an erased byte still lands in the value token");
		ok(L.tokenAtRawColumn(4) == nullptr, "the cursor in a separator lands in no token");
		ok(L.tokenTouchingRawColumn(4) != nullptr,
		   "the completion anchor just after a token is that token");
	}

	// The erase loop re-searches from the start, so one line can lose several
	// bytes; the offset map is read off the survivors, never recomputed.
	{
		Line L = tokenizeLine("x \"  \" y");
		eq(L.normalized, std::string("x \"\" y"), "two-pass normalization");
		eq(L.erased.size(), (std::size_t)2, "two bytes erased");
		eq(L.erased[0], (std::size_t)3, "first erased byte");
		eq(L.erased[1], (std::size_t)4, "second erased byte");
	}
	expectTokens("two-pass normalization spans", "x \"  \" y", {
		{TokenRole::Command, "x", 0, 1},
		{TokenRole::Key, "\"\"", 2, 6},
		{TokenRole::Value, "y", 7, 8}});
}

void testIndentedComment()
{
	// An indented '#' is handed to the parser and becomes a command token.
	expectTokens("indented '#'", "  # should do it four times", {
		{TokenRole::Command, "#", 2, 3},
		{TokenRole::Key, "should", 4, 10},
		{TokenRole::Value, "do", 11, 13},
		{TokenRole::Key, "it", 14, 16},
		{TokenRole::Value, "four", 17, 21},
		{TokenRole::DanglingKey, "times", 22, 27}});
	{
		Line L = tokenizeLine("  # hi");
		eq(L.command, std::string("#"), "indented '#' becomes the command token");
	}
}

void testBlockComment()
{
	// executeCommand:215-222 — comment/uncomment are intercepted BEFORE the skip
	// test, so they still act while skipping; `struct comment on|off` reaches
	// the same two handlers (commandStruct :4663-4670).
	struct Case { const char *line; bool skipped; };
	const Case cases[] = {
		{"flag stars on", false},
		{"comment", false},
		{"flag stars on", true},
		{"# not parsed at all", false},
		{"uncomment", false},
		{"flag stars on", false},
		{"struct comment on", false},
		{"flag stars on", true},
		{"comment", false},          // still acted on while skipping
		{"flag stars on", true},
		{"struct comment off", false},
		{"flag stars on", false},
	};
	BlockSkipState st;
	for (std::size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
		Line L = tokenizeLine(cases[i].line);
		bool skipped = (L.kind == LineKind::Parsed) ? st.feed(L) : false;
		eq(skipped, cases[i].skipped,
		   std::string("block state, line ") + std::to_string(i) + " (" + cases[i].line + ")");
	}
	// `struct comment <value>` follows Utility::isTrue, so "off"/"no"/anything
	// that is not an on-form ends the block.
	{
		BlockSkipState s2;
		s2.feed(tokenizeLine("struct comment TRUE"));
		ok(s2.skipping(), "struct comment TRUE starts a block (isTrue is case-insensitive)");
		s2.feed(tokenizeLine("struct comment nonsense"));
		ok(!s2.skipping(), "struct comment <not-an-on-form> ends it");
	}
}

void testEnginePredicates()
{
	// Utility::isTrue / isFalse, utility.hpp:160-180
	ok(isTrueValue("on") && isTrueValue("ON") && isTrueValue("On"), "isTrue: 'on' any case");
	ok(isTrueValue("true") && isTrueValue("TRUE") && isTrueValue("TrUe"), "isTrue: 'true' any case");
	ok(isTrueValue("1"), "isTrue: '1'");
	ok(!isTrueValue("yes") && !isTrueValue("toggle") && !isTrueValue("") && !isTrueValue("0"),
	   "isTrue: nothing else");
	ok(isFalseValue("off") && isFalseValue("OFF") && isFalseValue("false") && isFalseValue("0"),
	   "isFalse: off/false/0");
	ok(!isFalseValue("no") && !isFalseValue("on"), "isFalse: nothing else");

	// AppCommandInit::LevensteinDistance
	eq(levenshtein("kitten", "sitting"), (std::size_t)3, "levenshtein kitten/sitting");
	eq(levenshtein("flag", "flag"), (std::size_t)0, "levenshtein identity");
	eq(levenshtein("", "abc"), (std::size_t)3, "levenshtein empty source");

	// searchNeighbour: no threshold, strict '<', so the FIRST candidate at the
	// minimum wins — the caller's order is part of the answer.
	eq(nearestNeighbour("xx", {"aa", "bb"}), std::string("aa"), "tie-break keeps the first");
	eq(nearestNeighbour("flg", {"add", "flag", "font"}), std::string("flag"), "nearest wins");
	eq(nearestNeighbour("anything", {}), std::string(""), "no candidates, no suggestion");
}

} // namespace

int main()
{
	std::printf("scedit tokenizer differential self-test\n");
	testScriptLayer();
	testBasicShape();
	testDanglingKey();
	testDuplicateAndOrder();
	testQuoting();
	testSpaceAfterQuoteNormalization();
	testIndentedComment();
	testBlockComment();
	testEnginePredicates();
	std::printf("%d checks, %d failures\n", checks, failures);
	return failures ? 1 : 0;
}
