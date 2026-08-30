/*
 * scedit — self-test for the headless interaction core.
 *
 * WHAT THIS PINS
 * ==============
 * Every behaviour the editor has, reached without a terminal: the byte-
 * preserving buffer, the cursor->token mapping (including the two places the
 * engine's normalisation moves the ground under a naive column count), the
 * completion engine, the documentation bar, and the live findings.
 *
 * The expectations are DERIVED, not recorded: each one names where it comes
 * from — a rule of the parse model, a field of grammar/sc-grammar.json, or a
 * clause of sc_editcore.hpp's stated behaviour. Spans and columns are literal
 * numbers on purpose (a computed expectation moves with the bug).
 *
 * Two contract files are used. The real one (grammar/sc-grammar.json) is the
 * subject of every content assertion. The fixture (tests/fixture-grammar.json)
 * exists for exactly one thing: D31's default-on-empty-value, which the real
 * contract cannot arm because all 324 of its defaults are English sentences
 * rather than literals. Both are passed on the command line.
 *
 *   editcore_test <real-grammar.json> <fixture-grammar.json>
 */

#include "sc_docindex.hpp"
#include "sc_document.hpp"
#include "sc_editcore.hpp"

#include <cstdio>
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
		std::printf("  FAIL  %s\n", what.c_str());
		std::printf("        got  [%s]\n", std::string(got).c_str());
		std::printf("        want [%s]\n", std::string(want).c_str());
	}
}

void eqn(std::size_t got, std::size_t want, const std::string &what)
{
	++checks;
	if (got != want) {
		++failures;
		std::printf("  FAIL  %s: got %zu, want %zu\n", what.c_str(), got, want);
	}
}

std::string grammarPath, fixturePath;

//! An EditCore over a one-line-or-more buffer with the caret at a byte offset.
//! `col < 0` means end of that line.
EditCore at(const std::string &bytes, long col, std::size_t line = 0,
            const std::string &grammar = std::string())
{
	EditCore e;
	std::string err;
	if (!e.openBytes(grammar.empty() ? grammarPath : grammar, "test.sts", bytes, err)) {
		std::printf("  FATAL %s\n", err.c_str());
		++failures;
		return e;
	}
	const std::size_t c = col < 0 ? e.document().line(line).size() : (std::size_t)col;
	e.moveTo(line, c);
	return e;
}

// ---------------------------------------------------------------------------
// A. The buffer: bytes in, the same bytes out.
// ---------------------------------------------------------------------------

void testDocument()
{
	std::printf("A. byte-preserving buffer\n");

	// A CRLF file whose last line is not terminated, carrying a 0xA0 (the byte
	// `invisible-separator` exists for). Everything here must survive a save.
	const std::string bytes = "flag stars on\r\n#\xA0" "comment\r\nzoom auto in";
	Document d = Document::fromBytes(bytes);
	eqn(d.lineCount(), 3, "A1 three lines (split on '\\n' only)");
	eq(d.bytes(), bytes, "A1 fromBytes/bytes is the identity");
	eq(d.line(0), std::string("flag stars on"), "A2 the '\\r' of a CRLF file is terminator, not text");
	eq(d.terminator(0), std::string("\r\n"), "A2 ... and is kept as the line's own terminator");
	eq(d.terminator(2), std::string(""), "A2 an unterminated last line keeps no terminator");
	eq(d.line(1), std::string("#\xA0" "comment"), "A2 the 0xA0 byte is not touched");
	// The ENGINE reads the '\r': std::getline stops at '\n' and keeps the rest.
	// That is the string a tokenizer must be given (constraint C1).
	eq(d.engineLine(0), std::string("flag stars on\r"), "A2 engineLine puts the '\\r' back");
	eq(d.engineLine(2), std::string("zoom auto in"), "A2 ... and adds nothing where there was none");

	// Editing one line leaves every other line's BYTES alone — the round-trip
	// promise at line granularity.
	d.insert(0, 4, "X");
	eq(d.line(0), std::string("flagX stars on"), "A3 the edit landed");
	ok(d.touched(0) && !d.touched(1) && !d.touched(2), "A3 only the edited line is touched");
	eq(d.bytes(), std::string("flagX stars on\r\n#\xA0" "comment\r\nzoom auto in"),
	   "A3 the other lines are byte-identical");

	// A line opened in a CRLF file gets a CRLF line ending, because the line it
	// was split from had one. The editor does not decide this; the file does.
	Document crlf = Document::fromBytes("one\r\ntwo\r\n");
	crlf.splitLine(0, 3);
	eq(crlf.bytes(), std::string("one\r\n\r\ntwo\r\n"), "A4 a new line joins the file it is in");
	crlf.joinLine(0);
	eq(crlf.bytes(), std::string("one\r\ntwo\r\n"), "A4 join is its inverse");

	// Splitting the last line must not invent a terminator for the new last one.
	Document s = Document::fromBytes("abc");
	s.splitLine(0, 1);
	eqn(s.lineCount(), 2, "A4 split makes two lines");
	eq(s.bytes(), std::string("a\nbc"), "A4 the head takes '\\n', the tail keeps 'no terminator'");
	s.joinLine(0);
	eq(s.bytes(), std::string("abc"), "A4 join is its inverse");

	Document e;
	eqn(e.lineCount(), 1, "A5 an empty document has one line to put a cursor on");
	eq(e.bytes(), std::string(""), "A5 ... and is still zero bytes");

	// A file that ends ON a newline has no phantom last line (splitScriptLines'
	// own rule, sc_tokenizer.hpp), so buffer line i is diagnostic line i+1.
	Document t = Document::fromBytes("a\nb\n");
	eqn(t.lineCount(), 2, "A5 a final '\\n' does not make an extra line");
	eq(t.bytes(), std::string("a\nb\n"), "A5 ... and is still written back");
}

// ---------------------------------------------------------------------------
// B. cursor -> token, in RAW bytes, across both normalisation steps.
// ---------------------------------------------------------------------------

void testCursorMapping()
{
	std::printf("B. cursor -> token mapping\n");

	// B1. Leading spaces are erased before the engine tokenizes
	// (app_command_interface.cpp:129-132), so raw offsets run 3 ahead of the
	// normalized ones. Every span below is a RAW offset.
	{
		EditCore e = at("   zoom auto in", 0);
		const Line &L = e.currentLine();
		eqn(L.tokens.size(), 3, "B1 command + key + value");
		eqn(L.tokens[0].span.begin, 3, "B1 command span begins at raw 3");
		eqn(L.tokens[0].span.end, 7, "B1 command span ends at raw 7");
		eqn(L.tokens[1].span.begin, 8, "B1 key span begins at raw 8");
		eqn(L.tokens[2].span.begin, 13, "B1 value span begins at raw 13");
		ok(L.tokenAtRawColumn(0) == nullptr, "B1 a caret in the leading spaces is on no token");
		ok(L.tokenAtRawColumn(5) == &L.tokens[0], "B1 raw 5 is inside the command");
		ok(L.tokenTouchingRawColumn(7) == &L.tokens[0], "B1 raw 7 (just past) still anchors the command");
	}

	// B2. A quoted value spans from its opening quote through its closing one,
	// and the caret inside it is on THAT token, not on the words it contains.
	{
		EditCore e = at("image name \"my file\" action load", 15);
		const Line &L = e.currentLine();
		const Token *t = L.tokenAtRawColumn(15);
		ok(t != nullptr && t->role == TokenRole::Value, "B2 raw 15 is on the quoted value");
		eq(t->text, std::string("my file"), "B2 the engine's value has no quotes");
		eqn(t->span.begin, 11, "B2 the span starts at the opening quote");
		eqn(t->span.end, 20, "B2 ... and ends after the closing one");
		eq(L.rawText(t->span), std::string("\"my file\""), "B2 rawText gives the author's bytes back");
		// Completion never offers to append to a quoted value: the author is
		// writing free text (a file name, a caption), not choosing from a list.
		EditCore q = at("image name \"my file\" action load", 20);
		ok(!q.completion().armed, "B2 no ghost at the end of a quoted value");
	}

	// B3. The ' " ' -> ' "' normalisation (:135-139) DELETES a byte of the line
	// before tokenizing. Raw offsets after it are shifted, and a consumer that
	// counted columns naively would put the caret in the wrong token.
	{
		EditCore e = at("image name \" my file \" action load", 29);
		const Line &L = e.currentLine();
		eqn(L.erased.size(), 2, "B3 two bytes are erased");
		eqn(L.erased[0], 12, "B3 the space after the opening quote");
		eqn(L.erased[1], 22, "B3 the space after the closing quote");
		eq(L.normalized, std::string("image name \"my file \"action load"),
		   "B3 what the engine actually tokenizes");
		eqn(L.tokens[2].span.begin, 11, "B3 the value still spans the author's quotes");
		eqn(L.tokens[2].span.end, 22, "B3 ... through raw 21");
		eq(L.rawText(L.tokens[2].span), std::string("\" my file \""), "B3 rawText spans the erased byte too");
		eqn(L.tokens[3].span.begin, 23, "B3 `action` begins at raw 23");
		eqn(L.tokens[3].span.end, 29, "B3 ... and ends at raw 29");
		// The caret at raw 29 is at the END of `action`: completion must read
		// its prefix out of the RAW line, past two erased bytes.
		eq(e.completion().prefix, std::string("action"), "B3 the completion prefix is the raw word");
		eqn(e.completion().anchor.begin, 23, "B3 the completion anchor is the raw span");
	}
}

// ---------------------------------------------------------------------------
// C. Completion. `ghost()` is, always, exactly what Tab inserts.
// ---------------------------------------------------------------------------

void testCompletion()
{
	std::printf("C. completion\n");

	// C1. Command prefix. `zo` matches only `zoom` among the 62 accepted names.
	{
		EditCore e = at("zo", -1);
		ok(e.completion().armed, "C1 armed at the end of the word");
		ok(e.completion().context == Context::CommandName, "C1 context is the command name");
		eqn(e.completion().candidates.size(), 1, "C1 one candidate");
		eq(e.completion().candidates[0], std::string("zoom"), "C1 ... and it is zoom");
		eq(e.completion().ghost(), std::string("om"), "C1 the ghost is the missing suffix");
		ok(e.acceptCompletion(), "C1 Tab inserts it");
		eq(e.document().line(0), std::string("zoom"), "C1 the buffer holds exactly the candidate");
		eqn(e.cursor().col, 4, "C1 the caret follows the insertion");
	}

	// C2. An empty line offers every command the engine accepts. 62 = the 60
	// registered ones plus the two pre-table literals (comment, uncomment),
	// which the engine does accept and so the editor may offer.
	{
		EditCore e = at("", 0);
		eqn(e.completion().candidates.size(), 62, "C2 every accepted command is offered");
		eq(e.completion().candidates[0], std::string("add"), "C2 byte-lexicographic order");
		eq(e.completion().ghost(), std::string("add"), "C2 the ghost shows the selected one whole");
		ok(e.completion().openness == Openness::Exhaustive, "C2 the command list is the whole vocabulary");
	}

	// C3. Keys of a command, from its own extracted `args`. `moveto` has 11 and
	// three of them start with `de` (grammar/sc-grammar.json families.commands.moveto).
	{
		EditCore e = at("moveto de", -1);
		ok(e.completion().context == Context::ArgKey, "C3 context is an argument key");
		eqn(e.completion().candidates.size(), 3, "C3 three keys start with `de`");
		eq(e.completion().candidates[0], std::string("delta_alt"), "C3 ... in map order");
		eq(e.completion().ghost(), std::string("lta_alt"), "C3 the ghost completes the SELECTED one");
		// Cycling changes what the ghost promises, and Tab keeps that promise.
		e.cycleCompletion(+1);
		eq(e.completion().ghost(), std::string("lta_lat"), "C3 cycling moves to the next candidate");
		const std::string ghost = e.completion().ghost();
		e.acceptCompletion();
		eq(e.document().line(0), std::string("moveto de") + ghost, "C3 Tab inserts exactly the ghost");
	}

	// C4. `args_complete: false` must reach the screen. body/camera/flyto list
	// SOME of their keys; the rest belong to the stellar-system contract.
	{
		EditCore open = at("body ac", -1);
		ok(open.completion().openness == Openness::Open,
		   "C4 body's key list is marked open (args_complete: false)");
		ok(!open.completion().candidates.empty(), "C4 ... and the known keys are still offered");
		EditCore closed = at("moveto al", -1);
		ok(closed.completion().openness == Openness::Exhaustive,
		   "C4 moveto's key list is the whole vocabulary");
		// The same answer, straight from the contract reader, so the two cannot
		// drift: this IS Grammar::argKeysAreExhaustive.
		ok(!open.grammar().argKeysAreExhaustive("body"), "C4 the grammar says so too (body)");
		ok(open.grammar().argKeysAreExhaustive("moveto"), "C4 the grammar says so too (moveto)");
	}

	// C5. Enumerated values: `date load` takes one of three words
	// (families.commands.date.args.load.values).
	{
		EditCore e = at("date load c", -1);
		ok(e.completion().context == Context::ArgValue, "C5 context is a value");
		eqn(e.completion().candidates.size(), 1, "C5 one enumerated value starts with `c`");
		eq(e.completion().ghost(), std::string("urrent"), "C5 the ghost completes it");
		EditCore all = at("date load ", -1);
		ok(all.completion().context == Context::EmptyValue, "C5 an empty value slot is its own context");
		eqn(all.completion().candidates.size(), 3, "C5 all three are offered");
		eq(all.completion().candidates[1], std::string("keep_time"), "C5 byte-lexicographic");
	}

	// C6. The one place a value list is prose and must NOT be offered: the
	// entries of `values` that describe a domain rather than name a value.
	{
		ok(isCompletableLiteral("keep_time"), "C6 a bare word is offerable");
		ok(!isCompletableLiteral("<file name>"), "C6 a placeholder is not");
		ok(!isCompletableLiteral("anything else = off"), "C6 a sentence is not");
		ok(!isCompletableLiteral("YYYY-MM-DDThh:mm:ss"), "C6 a shape is not");
		ok(!isCompletableLiteral(""), "C6 nothing is not");
		// `flag <name> <value>`: the key grammar's `values` are ["toggle",
		// "any on-form accepted by Utility::isTrue (...)", "ANYTHING ELSE = off,
		// silently"]. Exactly one of the three is a word.
		EditCore e = at("flag stars ", -1);
		eqn(e.completion().candidates.size(), 1, "C6 only the literal is offered");
		eq(e.completion().candidates[0], std::string("toggle"), "C6 ... and it is `toggle`");
	}

	// C7. No candidates: a numeric domain, and an unknown command. Neither
	// produces a ghost, and neither pretends the field is closed.
	{
		EditCore num = at("set star_scale ", -1);
		ok(num.completion().candidates.empty(), "C7 a number has no candidate list");
		eq(num.completion().ghost(), std::string(""), "C7 ... and no ghost");
		EditCore unk = at("zzz ", -1);
		ok(unk.completion().candidates.empty(), "C7 an unknown command teaches nothing about keys");
		ok(unk.completion().openness == Openness::Unstated, "C7 ... and claims nothing either");
	}

	// C8. Mid-word the ghost is silent: appending to `zo|om` would produce
	// something the author did not type.
	{
		EditCore e = at("zoom", 2);
		ok(!e.completion().armed, "C8 no ghost in the middle of a word");
		eq(e.completion().ghost(), std::string(""), "C8 ... none at all");
		ok(e.completion().context == Context::CommandName, "C8 the doc bar still knows where it is");
	}

	// C9. Keys drawn from a family: `set`'s 43 keys ARE families.set_names, and
	// `flag`'s are families.flags.
	{
		EditCore e = at("set atmo", -1);
		eqn(e.completion().candidates.size(), 1, "C9 one set name starts with `atmo`");
		eq(e.completion().candidates[0], std::string("atmosphere_fade_duration"), "C9 ... that one");
		eq(e.completion().family, std::string("set_names"), "C9 the candidates are a family's names");
		ok(e.completion().openness == Openness::Exhaustive, "C9 a family IS the accepted vocabulary");
	}

	// C10. A family name that is a VALUE, not a key: `color property <name>`
	// (SubfamilyPosition::ValueOfKey, sc_grammar.cpp's anchored table).
	{
		EditCore e = at("color property ana", -1);
		eqn(e.completion().candidates.size(), 2, "C10 two colour names start with `ana`");
		eq(e.completion().candidates[0], std::string("analemma"), "C10 analemma first");
		eq(e.completion().family, std::string("color_names"), "C10 from families.color_names");
	}

	// C11. A key already written is not offered a second time: a repeat is a
	// finding (`duplicate-key`), not a suggestion.
	{
		EditCore e = at("moveto lat 1 l", -1);
		for (const auto &c : e.completion().candidates)
			ok(c != "lat", "C11 the key already on the line is not offered again");
		ok(!e.completion().candidates.empty(), "C11 the others still are");
		// ... but the key being EDITED is still offered to itself.
		EditCore self = at("moveto lat", -1);
		ok(!self.completion().candidates.empty(), "C11 the key under the caret completes to itself");
		eq(self.completion().candidates[0], std::string("lat"), "C11 ... exactly");
	}

	// C12. Case: the engine lowercases commands and keys, so their matching is
	// case-insensitive; a value keeps its case and is matched byte for byte.
	{
		EditCore e = at("ZO", -1);
		eqn(e.completion().candidates.size(), 1, "C12 an upper-case command still matches");
		eq(e.completion().ghost(), std::string("om"), "C12 ... and completes to the engine's spelling");
		EditCore v = at("date load C", -1);
		ok(v.completion().candidates.empty(), "C12 a value is matched byte for byte");
	}

	// C13. D31's default-on-empty-value. The mechanism arms from an explicit
	// `default_value`; the real contract has none (all 324 defaults are
	// sentences), the fixture has one, and both facts are asserted.
	{
		EditCore f = at("fixture mode ", -1, 0, fixturePath);
		ok(f.completion().context == Context::EmptyValue, "C13 an empty value slot");
		eq(f.completion().candidates[0], std::string("beta"), "C13 the DEFAULT is offered first");
		eq(f.completion().ghost(), std::string("beta"), "C13 ... and it is what the ghost shows");
		eqn(f.completion().candidates.size(), 2, "C13 the other enumerated value is offered too");
		eq(f.completion().candidates[1], std::string("alpha"), "C13 ... after the default");
		eqn(f.docIndex().defaultLiteralCount(), 1, "C13 the fixture carries one");

		EditCore real = at("date load ", -1);
		eqn(real.docIndex().defaultLiteralCount(), 0,
		   "C13 the real contract carries none — the feature is dormant, not dropped");
		bool named = false;
		for (const auto &d : real.docIndex().dormantFeatures())
			if (d.feature.find("D31") != std::string::npos)
				named = true;
		ok(named, "C13 ... and dormantFeatures() says so out loud");
	}

	// C14. An open key list in the fixture too, so the marking is tested against
	// a spec that lists exactly one key.
	{
		EditCore e = at("fixture_open kn", -1, 0, fixturePath);
		ok(e.completion().openness == Openness::Open, "C14 args_complete:false marks the list open");
		eqn(e.completion().candidates.size(), 1, "C14 the one known key is still offered");
	}
}

// ---------------------------------------------------------------------------
// D. The documentation bar (constraint C6, and C2's honest blank).
// ---------------------------------------------------------------------------

void testDocBar()
{
	std::printf("D. documentation bar\n");

	// D1. Command doc, verbatim from families.commands.flag.doc.
	{
		EditCore e = at("flag stars on", 2);
		ok(e.docBar().documented, "D1 the command is documented");
		eq(e.docBar().doc,
		   std::string("Turn a named display feature on, off, or to the opposite of what it is now."),
		   "D1 the file's own sentence, unchanged");
		eq(e.docBar().doc_of, std::string("command"), "D1 and the bar says what it documents");
	}

	// D2. Key doc. `set`'s keys are family names carrying the D7 v2 per-name
	// facts, so the bar answers for the NAME, not for the command.
	{
		EditCore e = at("set star_scale 2", 10);
		ok(e.docBar().documented, "D2 the set name is documented");
		eq(e.docBar().doc,
		   std::string("How big stars are drawn - this rescales the planets by the same number."),
		   "D2 families.set_names' own sentence");
		eq(e.docBar().doc_of, std::string("key"), "D2 it documents this key");
		ok(!e.docBar().def.empty(), "D2 the default sentence is shown as well");
		ok(!e.docBar().source.empty(), "D2 with the engine anchor behind it");
	}

	// D3. Value doc: one sentence per enumerated value, where the file has them.
	{
		EditCore e = at("date load current", 14);
		ok(e.docBar().documented, "D3 the value is documented");
		eq(e.docBar().doc, std::string("set the sky to the computer's date and time right now"),
		   "D3 families.commands.date.args.load.value_docs.current");
		eq(e.docBar().doc_of, std::string("value"), "D3 it documents this value");
	}

	// D4. A family name as a value documents itself from the family. Today
	// families.color_names is the v1 shape (plain names, no doc), so the honest
	// state is what must appear — the name exists, the sentence does not.
	{
		EditCore e = at("color property constellation_lines r 1", 20);
		ok(!e.docBar().documented, "D4 no doc exists for a colour name (v1 family)");
		eq(e.docBar().doc, std::string(""), "D4 and nothing is invented in its place");
		eq(e.docBar().note, std::string(""), "D4 the name is a real one, so nothing is wrong either");
	}

	// D5. `"doc": null` — the sweep could not answer from the code and FLAGGED
	// it. That is a state the bar must carry to the screen intact.
	{
		const CommandInfo *st = at("suntrace", 0).docIndex().command("suntrace");
		ok(st != nullptr, "D5 suntrace is in the contract");
		ok(st && st->args.count("sun") == 1, "D5 ... with a `sun` key");
		ok(st && !st->args.at("sun").doc_known, "D5 whose doc is null");
		EditCore e = at("suntrace sun Earth", 10);
		ok(!e.docBar().documented, "D5 the bar reports the gap");
		eq(e.docBar().doc, std::string(""), "D5 ... with no text at all");
		ok(!e.docBar().domain.empty(), "D5 what IS known is still shown (the value domain)");
	}

	// D6. A general sentence must not pass for a specific one. `flag`'s key
	// grammar documents what a key MEANS there; families.flags has no per-name
	// doc, so the bar says which of the two it is showing.
	{
		EditCore e = at("flag stars on", 7);
		ok(e.docBar().documented, "D6 something is known about this position");
		eq(e.docBar().doc_of, std::string("any key of `flag`"),
		   "D6 ... and the bar states it is the key GRAMMAR, not a doc for `stars`");
	}

	// D7. An unknown name is named as unknown, from the structural answer
	// (Grammar::isCommand), never from a guess.
	{
		EditCore e = at("zomo", -1);
		ok(!e.docBar().documented, "D7 an unknown command has no doc");
		ok(e.docBar().note.find("not a command") != std::string::npos,
		   "D7 ... and the bar says why");
		EditCore k = at("moveto zzz 1", 8);
		ok(k.docBar().note.find("not an argument") != std::string::npos,
		   "D7 an unknown key of a closed command is named too");
		EditCore o = at("body zzz 1", 6);
		ok(o.docBar().note.find("args_complete") != std::string::npos,
		   "D7 but an unlisted key of an OPEN command is never called wrong");
	}

	// D8. A comment line: the file's own sentence about what the script layer
	// does with it (parse_model.comments.script_layer).
	{
		EditCore e = at("# turn the stars on", 5);
		ok(e.completion().context == Context::CommentLine, "D8 the caret is on a dropped line");
		ok(e.docBar().documented, "D8 which is documented");
		ok(e.docBar().doc.find("FIRST byte") != std::string::npos,
		   "D8 by parse_model.comments.script_layer");
		ok(!e.completion().armed, "D8 nothing completes inside a comment");
	}

	// D9. The caret inside a trailing comment: the parser's own sentence about
	// it (parse_model.comments.mid_line), and no ghost — a completion there
	// would promise a meaning to bytes the engine never reads.
	{
		EditCore e = at("media action pause # stop video", 25);
		ok(e.completion().context == Context::Comment, "D9 the caret is in the comment");
		ok(!e.completion().armed && e.completion().candidates.empty(), "D9 nothing completes there");
		eq(e.docBar().path, std::string("comment"), "D9 the bar says so");
		ok(e.docBar().documented && e.docBar().doc.find("starts a comment") != std::string::npos,
		   "D9 with parse_model.comments.mid_line");
		EditCore f = at("media action pause # stop video", 18);
		ok(f.completion().context != Context::Comment, "D9 one byte before the '#' is still the command");
	}

	// D9. The honest-blank spelling lives in ONE place, so the screen and the
	// tests cannot disagree about it.
	ok(std::string(kNoDoc).find("no documentation extracted") == 0,
	   "D9 kNoDoc is the one spelling of the honest blank");
}

// ---------------------------------------------------------------------------
// E. The findings, live.
// ---------------------------------------------------------------------------

void testLint()
{
	std::printf("E. live findings\n");

	// A 0xA0 between `albedo` and `1` — the corpus case (superscript.sts:94),
	// reduced. `--check` decides it is a finding, and the finding's SPAN is
	// what puts the marker on the exact byte: one source for both.
	const std::string buf = "flag stars on\nbody name Earth albedo\xA0""1\n";
	EditCore e = at(buf, 0, 1);
	const std::vector<const Diagnostic *> d2 = e.diagnosticsForLine(2);
	ok(!d2.empty(), "E2 the analyser reports the line");
	const Diagnostic *sep = nullptr;
	for (const auto *d : d2)
		if (d->id == "invisible-separator")
			sep = d;
	ok(sep != nullptr, "E2 ... as invisible-separator");
	eqn(sep ? sep->span.begin : 0, 22, "E1 its span starts at exactly the byte offset");
	eqn(sep ? sep->span.size() : 0, 1, "E1 ... and covers that one byte");
	eq(e.document().line(1).substr(22, 1), std::string("\xA0"), "E1 ... which is the 0xA0 itself");
	eq(e.severityForLine(2), std::string("error"), "E2 with the seed's own severity");
	eq(e.severityForLine(1), std::string(""), "E2 and the clean line stays clean");

	// The findings follow the edit: replace the 0xA0 with a real space and the
	// finding goes, with no other change to the file.
	e.moveTo(1, 22);
	e.del();
	e.insertText(" ");
	ok(e.diagnosticsForLine(2).empty(), "E3 fixing the byte clears the finding");
	eq(e.document().bytes(), std::string("flag stars on\nbody name Earth albedo 1\n"),
	   "E3 and nothing else moved");

	// E4. Spans: every finding points at its bytes (scedit/INTENT.md §5 item 10).
	{
		EditCore u = at("zomo action now\n", 0);
		const std::vector<const Diagnostic *> d = u.diagnosticsForLine(1);
		eqn(d.size(), 1, "E4a one finding on the misspelt command");
		eq(d.empty() ? std::string() : d[0]->id, std::string("unknown-command"), "E4a ... unknown-command");
		eqn(d.empty() ? 99 : d[0]->span.begin, 0, "E4a ... spanning the command token");
		eqn(d.empty() ? 99 : d[0]->span.end, 4, "E4a ... to its end");
	}
	{
		// A '#' after the command is a COMMENT (parse_model.comments.mid_line,
		// the ruled rule): no finding, the line is what precedes the '#'.
		const std::string line = "media action pause # stop video & sound";
		EditCore u = at(line + "\n", 0);
		ok(u.diagnosticsForLine(1).empty(), "E4b a trailing comment is not a finding");
		eqn(u.commentBegin(0), 19, "E4b ... and the renderer is told where it starts");
		eqn(u.commentBegin(1), std::string::npos, "E4b ... and that the next line has none");
	}
	{
		// An opener never closed: reported on ITS line, not on the last one.
		EditCore u = at("struct if a equal b\nflag stars on\n", 0);
		const std::vector<const Diagnostic *> d1 = u.diagnosticsForLine(1);
		eqn(d1.size(), 1, "E4c the unclosed if is reported on its own line");
		eq(d1.empty() ? std::string() : d1[0]->id, std::string("unclosed-struct"), "E4c ... unclosed-struct");
		eqn(d1.empty() ? 99 : d1[0]->span.end, 19, "E4c ... spanning the whole opener");
		ok(u.diagnosticsForLine(2).empty(), "E4c and the last line stays clean");
		eq(u.severityForLine(1), std::string("error"), "E4c with the seed's severity");
		// Closing it clears the finding.
		u.moveTo(1, 13);
		u.insertNewline();
		u.insertText("struct if end");
		ok(u.diagnosticsForLine(1).empty(), "E4d adding `struct if end` clears it");
	}
	{
		EditCore u = at("struct if end\n", 0);
		const std::vector<const Diagnostic *> d = u.diagnosticsForLine(1);
		eqn(d.size(), 1, "E4e a lone `struct if end` is reported");
		eq(d.empty() ? std::string() : d[0]->id, std::string("end-without-if"), "E4e ... end-without-if");
		eqn(d.empty() ? 99 : d[0]->span.begin, 10, "E4e ... on the word 'end'");
	}
}

} // namespace

int main(int argc, char **argv)
{
	if (argc < 3) {
		std::fprintf(stderr, "usage: editcore_test <grammar.json> <fixture-grammar.json>\n");
		return 2;
	}
	grammarPath = argv[1];
	fixturePath = argv[2];

	testDocument();
	testCursorMapping();
	testCompletion();
	testDocBar();
	testLint();

	std::printf("%s: %d checks, %d failures\n", failures ? "FAILED" : "ok", checks, failures);
	return failures ? 1 : 0;
}
