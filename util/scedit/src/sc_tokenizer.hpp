/*
 * scedit — sc_tokenizer.hpp
 *
 * WHAT THIS IS FOR
 * ================
 * One line of a spacecrafter script, read EXACTLY the way the engine reads it,
 * with every token still pointing back at the bytes the author typed.
 *
 * Two consumers, one answer:
 *   - `--check` needs "what will the engine do with this line?";
 *   - the TUI needs "which token is under the cursor at column N?" and
 *     "where do I draw the ghost-text for a completion?".
 * Both are served here, so neither can drift from the engine's reading.
 *
 * ENGINE FIDELITY (constraint C1, scedit/INTENT.md §2)
 * ====================================================
 * `tokenizeLine` is a derivation of AppCommandInterface::parseCommand
 * (src/interfaceModule/app_command_interface.cpp:124-176) and
 * `classifyLine` of the script-layer comment rule (src/scriptModule/script.cpp:114).
 * The clause-by-clause mapping is `util/scedit/tests/derivation-diff.md`; the
 * differential corpus that pins it is `util/scedit/tests/tokenizer_test.cpp`.
 * If this header and the engine ever disagree, the engine is right and this is
 * a scedit defect — never the other way round.
 *
 * ONE RULE IS AHEAD OF THE ENGINE, BY RULING. A '#' outside a "..." run starts
 * a comment that runs to the end of the line (parse_model.comments.mid_line).
 * Vixy ruled it 2026-08-30 and ruled the ORDER 2026-08-31: scedit models the
 * corrected behaviour first, the engine is then brought into phase with the
 * identical code (the oracle test carries that code as the TARGET copy of
 * parseCommand). Until the engine commit lands, this is the one place where
 * "the engine is right" reads "the ruled engine is right".
 *
 * The sharp edges this reproduces on purpose (all engine behaviour, not choices):
 *   - a '#' outside quotes ends the command — quotes are counted by a plain
 *     toggle from the first byte, so a '#' inside quotes, closed or not, is
 *     text; a '#' glued to a word cuts at the '#' (see Line::comment_begin);
 *   - a trailing KEY with no VALUE is silently DROPPED (see Line::dangling);
 *   - a repeated key keeps the LAST value, and handlers see keys in
 *     ALPHABETICAL order, not line order (see Line::args);
 *   - `"` is the only grouping character; `'` and `\` are ordinary bytes;
 *   - an unclosed `"` runs to end of line without error (Token::quote_closed);
 *   - the ' " ' -> ' "' normalisation erases the byte after such a quote
 *     BEFORE tokenizing (see "NORMALISATION AND SPANS" below);
 *   - the command token and every KEY are lowercased (ASCII only, C locale);
 *     VALUES keep their case.
 *
 * NORMALISATION AND SPANS  (the part the TUI depends on)
 * ======================================================
 * The engine does not tokenize the line you typed. It first deletes bytes from
 * it (the comment from its '#' on, then leading spaces/tabs, then the byte after
 * every ' " '), and tokenizes the RESULT. So a naive column count is wrong
 * exactly where quoting or a comment is involved.
 *
 * This library therefore keeps both strings and the map between them:
 *   raw          — the author's bytes, verbatim (what the editor buffer holds);
 *   normalized   — the string the engine actually tokenizes;
 *   erased       — raw offsets of the bytes normalisation deleted (ascending);
 *   rawOfNorm[i] — raw offset of normalized[i]  (strictly increasing).
 * `Span` is ALWAYS in RAW coordinates: a half-open [begin,end) byte range of
 * `raw`, so it can be handed straight to a renderer. A span may cover erased
 * bytes (they sit inside the run the engine consumed); `Token::text` is what
 * the engine ends up with, `Line::rawText(span)` is what the author sees.
 * `tokenAtRawColumn` is the cursor->token direction; `Token::span` is the
 * token->cursor direction. Both are exact, including quoted multi-word values.
 *
 * OWNERSHIP
 * =========
 * Everything is by value. A `Line` owns its strings and its tokens; the
 * `Token*` returned by `tokenAtRawColumn` points into that `Line` and dies
 * with it (and with any reassignment of it). No global state, no allocation
 * you must free, reentrant, no I/O.
 */

#ifndef SCEDIT_SC_TOKENIZER_HPP
#define SCEDIT_SC_TOKENIZER_HPP

#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace scedit {

//! Half-open byte range [begin, end) into the RAW line.
struct Span {
	std::size_t begin = 0;
	std::size_t end = 0;

	std::size_t size() const { return end - begin; }
	bool empty() const { return end <= begin; }
	//! Cursor semantics: a caret sitting ON the last byte is inside; a caret
	//! one past the token is not (use `touches` for completion anchoring).
	bool contains(std::size_t off) const { return off >= begin && off < end; }
	//! Cursor semantics for completion: the caret just after the token counts.
	bool touches(std::size_t off) const { return off >= begin && off <= end; }
	bool operator==(const Span &o) const { return begin == o.begin && end == o.end; }
};

//! What the engine does with this token.
enum class TokenRole {
	Command,      //!< first whitespace-separated token; lowercased; looked up in m_commands
	Key,          //!< key half of a key/value pair; lowercased; map key
	Value,        //!< value half; case preserved; quote processing applied
	DanglingKey   //!< a trailing key whose value never arrived — the engine DROPS it
};

struct Token {
	TokenRole role = TokenRole::Command;

	//! What the engine ends up holding for this token:
	//!  - Command/Key: the raw bytes, ASCII-lowercased;
	//!  - Value: quote processing applied (outer quotes stripped, a quoted run
	//!    joined across spaces), case preserved;
	//!  - DanglingKey: the raw bytes, ASCII-lowercased — recorded for
	//!    diagnostics even though the engine keeps nothing.
	std::string text;

	//! RAW byte range this token consumed, quotes included. For a quoted
	//! multi-word value it spans from the opening `"` through the closing `"`
	//! (or to end of line when the quote is never closed).
	Span span;

	//! 0-based index of the key/value pair this token belongs to, in LINE
	//! order. -1 for the command token. A Key and its Value share the index;
	//! a DanglingKey gets the index it would have had.
	int pair = -1;

	//! The value opened with `"` (Value tokens only).
	bool quoted = false;
	//! A closing `"` was found before end of line (meaningful when `quoted`).
	//! False means the engine swallowed the rest of the line into this value —
	//! not an error engine-side, but the TUI should show where the value ends.
	bool quote_closed = false;
};

//! What the SCRIPT layer does with the line, before the parser ever sees it.
//! Rule: `line[0] != '#' && line[0] != 0 && line[0] != '\r' && line[0] != '\n'`
//! (script.cpp:114). Note what this does NOT say: an INDENTED '#' is not a
//! comment — the line is handed to the parser and becomes an unknown command.
enum class LineKind {
	Comment,   //!< first byte is '#'  — dropped by the script layer
	Blank,     //!< empty, or first byte is NUL / CR / LF — dropped
	Parsed     //!< handed to parseCommand (may still parse to nothing)
};

struct Line {
	LineKind kind = LineKind::Blank;

	std::string raw;          //!< the author's bytes, verbatim (no newline)
	std::string normalized;   //!< the string parseCommand actually tokenizes
	std::vector<std::size_t> rawOfNorm; //!< rawOfNorm[i] = raw offset of normalized[i]
	std::vector<std::size_t> erased;    //!< raw offsets deleted by normalisation, ascending

	//! Tokens in LINE order: command first (when present), then key/value.
	std::vector<Token> tokens;

	//! Lowercased command; empty when the line parses to no command at all
	//! (whitespace-only Parsed line — the engine returns 0 and does nothing).
	std::string command;
	bool has_command = false;

	//! The engine's `stringHash_t args`: std::map, so LAST value wins on a
	//! repeated key and iteration is ALPHABETICAL. `args.begin()` is the pair
	//! the single-pair commands (flag/define/add/sub/multiply/divide/modulo/
	//! tangent/trunc/sinus) actually apply.
	std::map<std::string, std::string> args;

	//! Key/value pairs in LINE order, as token indices into `tokens`.
	struct Pair { std::size_t key = 0, value = 0; };
	std::vector<Pair> pairs;

	//! A trailing key the engine dropped. `dangling_index` indexes `tokens`.
	bool has_dangling = false;
	std::size_t dangling_index = 0;

	//! Some value on this line opened a `"` that was never closed.
	bool has_unclosed_quote = false;

	//! RAW offset of the '#' that starts this line's comment, or
	//! std::string::npos when the line has none. For a `LineKind::Comment` line
	//! it is 0. Everything from here to the end of the line is text the engine
	//! never reads (parse_model.comments.mid_line) — no token covers it, and
	//! its bytes are in `erased`.
	std::size_t comment_begin = static_cast<std::size_t>(-1);

	// --- raw <-> parsed mapping -------------------------------------------

	//! Token under a raw byte offset (cursor position), or nullptr.
	const Token *tokenAtRawColumn(std::size_t raw_off) const;
	//! Token the caret is on OR immediately after — the anchor a completion
	//! should extend. nullptr when the caret is in whitespace between tokens.
	const Token *tokenTouchingRawColumn(std::size_t raw_off) const;

	//! raw offset -> normalized offset. false when that raw byte was erased by
	//! normalisation (it exists for the author, not for the engine).
	bool rawToNormalized(std::size_t raw_off, std::size_t &norm_off) const;
	//! normalized offset -> raw offset. Precondition: norm_off <= normalized.size().
	std::size_t normalizedToRaw(std::size_t norm_off) const;

	//! The author's bytes under a span.
	std::string rawText(const Span &s) const;
};

//! Classify a line the way the script layer does (script.cpp:114).
LineKind classifyLine(const std::string &raw);

//! Read one line exactly as the engine reads it. `raw` must NOT contain the
//! line terminator (see splitScriptLines). Lines the script layer drops come
//! back with their `kind` and `raw` set and no tokens: the engine never parses
//! them, and neither do we.
Line tokenizeLine(const std::string &raw);

//! Split a whole script file into lines the way Script::loadInternal's
//! std::getline does: on '\n' only. A '\r' from a CRLF file STAYS at the end of
//! the line — that is why the script layer tests `line[0] != '\r'`, and why a
//! trailing '\r' is harmless inside a command line ('\r' is whitespace to the
//! parser). A final '\n' does not produce a trailing empty line.
std::vector<std::string> splitScriptLines(const std::string &file_bytes);

// --- the small engine predicates callers keep needing ---------------------

//! ASCII lowercase, C locale — `transform(..., ::tolower)` with LC_CTYPE="C".
//! Bytes >= 0x80 are left alone (the engine only calls setlocale(LC_TIME,...),
//! src/main.cpp:242, so LC_CTYPE never leaves "C").
std::string asciiLower(const std::string &s);

//! Utility::isTrue (src/tools/utility.hpp:160): case-insensitive "true"/"on",
//! or the single character '1'. Nothing else.
bool isTrueValue(const std::string &v);
//! Utility::isFalse (src/tools/utility.hpp:172): case-insensitive "false"/"off",
//! or the single character '0'.
bool isFalseValue(const std::string &v);

//! AppCommandInit::LevensteinDistance (app_command_init.cpp:337-365).
std::size_t levenshtein(const std::string &a, const std::string &b);

//! AppCommandInit::searchNeighbour (app_command_init.cpp:368-387): nearest
//! candidate by Levenshtein distance, NO threshold, first minimum wins — and
//! `candidates` must be supplied in the engine's own order (the m_* maps are
//! std::map, so: alphabetical) for the tie-break to match.
//! Returns "" only when `candidates` is empty.
std::string nearestNeighbour(const std::string &source, const std::vector<std::string> &candidates);

// --- block state -----------------------------------------------------------

//! The state the engine keeps BETWEEN script lines, read from the same
//! `comment` / `uncomment` / `struct` lines it reads it from. Two facts:
//!
//! 1. THE SKIP FLAG. `comment` / `uncomment` toggle it; both are intercepted
//!    BEFORE the command table (executeCommand:215-222) and therefore still
//!    honoured while skipping. `struct comment <on|off>` reaches the same two
//!    handlers (commandStruct :4664-4672), so it is tracked too.
//!
//! 2. THE BLOCK STRUCTURE: which `struct if` / `struct loop` openers are still
//!    open, and which closer closed nothing. The engine's `ifSwap` is a stack
//!    (`std::vector<bool>`, if_swap.hpp:71): `struct if <cond>` pushes
//!    (commandStruct :4614-4661), `struct if else` flips the top (:4606-4609 ->
//!    IfSwap::revert, if_swap.cpp:71-81), `struct if end` pops (:4610-4613 ->
//!    IfSwap::pop, :40-54). An `end` or `else` on an empty stack is LOGGED
//!    ("end without if" :45 / "else without if" :76) and ignored. The whole
//!    if-case is guarded by `swapCommand != true` (:4605): inside a `comment`
//!    block NO `struct if` line counts, `end` included. The stack is cleared
//!    only by `script action end` (:2810), which the end of every script runs
//!    (terminateScript :178-181 <- script_mgr.cpp:335) - so an opener left
//!    open damages its own file's tail and nothing after. `struct loop <n>` ..
//!    `struct loop end` is not a stack engine-side (one isInLoop/loopVector,
//!    script_mgr.hpp:120-131) but it is a pair the author writes, and an
//!    opener without its `end` is a defect in every runtime path
//!    (:4675-4699): n > 1 -> the lines after it run once and are never
//!    repeated (the replay starts at `end`, script_mgr.hpp:125-131), n < 1 ->
//!    the skip flag raised at :4691-4692 is never lowered. Loops are tracked
//!    by pairing; `break` (:4684-4688) leaves the pair open.
//!
//! NOT decided here, deliberately (derivation-diff.md §5.1-5.2, answered
//! 2026-08-31 as "structure yes, arms no"): WHICH arm of an `if` runs, and
//! whether a `struct loop <n>` with n < 1 skips - both need runtime values.
//! Findings inside such regions are reported as on any other line. The loop
//! case is also guarded engine-side by `ifSwap->get() != true` (:4676), a
//! runtime fact; a loop is tracked whatever the if-state.
class BlockSkipState {
public:
	//! A `struct if` / `struct loop` opener still open after the last feed.
	struct OpenBlock {
		std::size_t line = 0;     //!< 1-based, as fed
		std::string kind;         //!< "if" | "loop"
		std::string text;         //!< the opener's tokens, as the author wrote them
		std::string count;        //!< loop only: the raw `loop` value ("3600", "$n", ...)
		Span span;                //!< raw byte range from the first token to the last
	};
	//! A closer fed while nothing of its kind was open.
	struct Unmatched {
		std::size_t line = 0;
		std::string what;         //!< "end" | "else" | "loop end"
		Span span;                //!< the closing word's token
	};

	//! Feed every Parsed line, in file order, with its 1-based line number.
	//! Returns true when the line ITSELF is skipped by the engine (the flag
	//! was on and the line is not one of the pre-table interceptions).
	bool feed(const Line &line, std::size_t lineno = 0);
	bool skipping() const { return skipping_; }
	//! Still open after everything fed so far, in opening order.
	const std::vector<OpenBlock> &openIfs() const { return ifs_; }
	const std::vector<OpenBlock> &openLoops() const { return loops_; }
	//! Every closer that closed nothing, in feed order (never cleared).
	const std::vector<Unmatched> &unmatched() const { return unmatched_; }
	void reset() { *this = BlockSkipState(); }
private:
	bool skipping_ = false;
	std::vector<OpenBlock> ifs_, loops_;
	std::vector<Unmatched> unmatched_;
};

} // namespace scedit

#endif // SCEDIT_SC_TOKENIZER_HPP
