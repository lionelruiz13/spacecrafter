/*
 * scedit — sc_editcore.hpp
 *
 * WHAT THIS IS FOR
 * ================
 * Everything the editor DOES, with no terminal in it: the buffer, the cursor,
 * what the cursor is standing on, what the documentation bar must say about it,
 * what a Tab would complete, and where the analyser's findings sit. The FTXUI
 * layer draws this and forwards key and mouse events into it; it decides
 * nothing. That is not tidiness for its own sake — a terminal cannot be
 * asserted on, and this can: every behaviour the editor has is reachable from
 * `tests/editcore_test.cpp` without a tty.
 *
 * THE ONE QUESTION THIS CLASS ANSWERS
 * ===================================
 * "The caret is at line L, byte C — what is it standing on, what do we tell the
 * author about it, and what would Tab type for them?" Everything else (screen
 * size, colours, scrolling) belongs to the renderer.
 *
 * WHERE THE ANSWERS COME FROM (nothing is invented — constraint C2)
 * ================================================================
 *   sc_tokenizer.hpp  cursor -> token, in the engine's own reading of the line,
 *                     including the quote normalisation that makes a naive
 *                     column count wrong;
 *   sc_grammar.hpp    the structural answers: is this a command, may an
 *                     unlisted key be called unknown (`argKeysAreExhaustive`),
 *                     which family does this command draw its names from;
 *   sc_docindex.hpp   the prose: the doc line, the value domain, the default
 *                     sentence, the engine anchor — and the honest blank where
 *                     the contract file has none;
 *   sc_check.hpp      the findings, recomputed over the whole buffer after an
 *                     edit (never over a line in isolation: `comment` /
 *                     `uncomment` make a line's meaning depend on the ones
 *                     above it).
 *
 * COMPLETION, PRECISELY
 * =====================
 * Ghost text is armed ONLY where inserting a suffix produces the candidate:
 * with the caret at the END of the word being typed, or in an empty slot. Mid
 * word the doc bar still works and the ghost is silent — an editor that offers
 * to append to `zo|om` is offering a lie.
 * `ghost()` is, always, exactly the bytes Tab would insert. That is the whole
 * contract of the greyed text (D31 + the 2026-08-04 refinement: show what would
 * be completed, for ANY completion and not only defaults). When several
 * candidates share the prefix, `selected` picks which one the ghost shows and
 * Tab cycles it — so the greyed text never promises something Tab will not do.
 *
 * `openness` is the `args_complete` answer carried through to the screen: a
 * command whose key list is known to be partial (body, camera, flyto) must not
 * be shown a closed list. Its candidates are offered, and labelled OPEN.
 *
 * FINDINGS ON THE SCREEN
 * ======================
 * `scedit::Diagnostic` (sc_check.hpp) carries a `span`: the raw byte range the
 * finding is about. The renderer reads it through `diagnosticsForLine()` —
 * the look-alike-space marker lands on `invisible-separator`'s span, an
 * underline on every other non-empty span — so what is marked on screen is
 * exactly what the rule decided, and decided once: a 0xA0 inside a quoted
 * value is NOT marked, because the rule says it is ordinary text there.
 * (Until 2026-08-31 the marker column was re-derived from the bytes by a
 * `lookalikeSpaceColumns()` here, a second copy of half the rule; the Span
 * closed that gap — scedit/INTENT.md §5 item 10.)
 *
 * OWNERSHIP: an EditCore owns its Document, Grammar and DocIndex. References
 * and pointers it returns die with it or with the next mutation.
 */

#ifndef SCEDIT_SC_EDITCORE_HPP
#define SCEDIT_SC_EDITCORE_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "sc_check.hpp"
#include "sc_docindex.hpp"
#include "sc_document.hpp"
#include "sc_grammar.hpp"
#include "sc_tokenizer.hpp"

namespace scedit {

//! What the caret is standing on.
enum class Context {
	None,          //!< nothing to say (no command on the line, unknown command...)
	CommentLine,   //!< the script layer drops this line whole
	Comment,       //!< the caret is in the comment after a '#' (the engine reads none of it)
	CommandName,   //!< the command word
	ArgKey,        //!< the key half of a pair
	ArgValue,      //!< the value half of a pair
	EmptyValue,    //!< the value slot of a key that has none yet (D31's case)
	NewKey         //!< between tokens, where the next word will be a key
};

//! Does the candidate list claim to be the whole vocabulary?
enum class Openness {
	Unstated,     //!< nothing is claimed either way (values, free text)
	Exhaustive,   //!< these are all of them (`args_complete: true`, or a family)
	Open          //!< known members of a larger set (`args_complete: false`)
};

struct Completion {
	bool armed = false;                    //!< a ghost may be drawn at `anchor.end`
	Context context = Context::None;
	std::string prefix;                    //!< bytes already typed, as the engine reads them
	Span anchor;                           //!< raw span being extended (empty at an empty slot)
	std::vector<std::string> candidates;   //!< prefix-filtered, byte-lexicographic
	std::size_t selected = 0;
	Openness openness = Openness::Unstated;
	std::string what;                      //!< "command", "key of `flag`", "value of `load`"
	//! Set when the candidates ARE a family's names (`flag`'s keys are
	//! families.flags, `color property`'s value is families.color_names), so a
	//! caller can say which vocabulary a name is missing from without reading
	//! `what` as prose.
	std::string family;

	//! Exactly the bytes Tab would insert. Empty when there is nothing to add.
	std::string ghost() const;
	bool empty() const { return candidates.empty(); }
};

//! One documentation-bar state. Empty strings are "the file says nothing";
//! `documented == false` is what makes the renderer print `kNoDoc` instead of
//! a blank that could be mistaken for "nothing to say".
struct DocBar {
	std::string path;        //!< "command `flag`", "`set` > `star_scale`", "`date` `load` = `current`"
	bool documented = false;
	std::string doc;
	//! WHAT the sentence in `doc` documents, so the bar never lets a general
	//! sentence pass for a specific one. Empty when there is no doc; otherwise
	//! "command", "key", "value", or "any key of `<command>`" — that last one is
	//! the case where the file documents the command's key GRAMMAR but has no
	//! line for this particular name (every flag name today, families.flags
	//! being still the v1 shape).
	std::string doc_of;
	std::string domain;      //!< the value-domain sentence
	std::vector<std::string> values;   //!< the domain's entries, VERBATIM (prose ones included)
	std::string def;         //!< the default sentence
	std::string required;
	std::string source;      //!< engine anchor
	std::string note;        //!< structural remark (open key list, unknown name, ...)
};

struct Cursor {
	std::size_t line = 0;
	std::size_t col = 0;   //!< BYTE offset into the line, never a character count
};

class EditCore {
public:
	//! Load the contract, then the file. `filePath` may be empty (new buffer).
	bool open(const std::string &grammarPath, const std::string &filePath, std::string &err);
	//! Load the contract and take the buffer from memory (tests, --ui-selftest).
	bool openBytes(const std::string &grammarPath, const std::string &label,
	               const std::string &bytes, std::string &err);

	const Document &document() const { return doc_; }
	const Grammar &grammar() const { return grammar_; }
	const DocIndex &docIndex() const { return docs_; }
	const std::string &path() const { return path_; }
	bool dirty() const { return doc_.dirty(); }

	// --- cursor ------------------------------------------------------------
	const Cursor &cursor() const { return cur_; }
	//! Clamps to the buffer; `col` is clamped to the line length.
	void moveTo(std::size_t line, std::size_t col);
	void moveLeft();
	void moveRight();
	void moveUp(std::size_t n = 1);
	void moveDown(std::size_t n = 1);
	void moveHome();
	void moveEnd();

	// --- editing (byte-level; nothing is transcoded) ------------------------
	void insertText(const std::string &bytes);
	void insertNewline();
	void backspace();
	void del();
	//! Insert what the ghost shows. Returns true when bytes were inserted;
	//! false (and advances `selected`) when the selected candidate is already
	//! typed in full and there is another one to show.
	bool acceptCompletion();
	void cycleCompletion(int delta);

	bool save(std::string &err);
	bool saveAs(const std::string &path, std::string &err);

	// --- what the renderer draws -------------------------------------------
	//! The cursor line, read the way the engine reads it.
	const Line &currentLine() const { return line_; }
	const Completion &completion() const { return completion_; }
	const DocBar &docBar() const { return docbar_; }

	//! Raw offset where the comment of a 0-based line starts (its '#'), or
	//! std::string::npos when the line has none — the engine's reading of the
	//! line (parse_model.comments.mid_line), so the renderer can grey exactly
	//! the bytes the engine never reads.
	std::size_t commentBegin(std::size_t line) const;

	//! Findings on a 1-based file line (Diagnostic::line's own numbering).
	std::vector<const Diagnostic *> diagnosticsForLine(std::size_t oneBasedLine) const;
	const std::vector<Diagnostic> &diagnostics() const { return diags_; }
	//! Highest severity present on that line: "error" > "warning" > "info", "" when clean.
	std::string severityForLine(std::size_t oneBasedLine) const;

	//! Recompute the findings now (done automatically after every mutation).
	void refreshDiagnostics();

private:
	Grammar grammar_;
	DocIndex docs_;
	Document doc_;
	std::string path_;
	Cursor cur_;

	Line line_;                        //!< tokenization of the cursor's line
	Completion completion_;
	DocBar docbar_;
	std::vector<Diagnostic> diags_;

	void afterMove();                  //!< retokenize + recompute completion/doc bar
	void afterEdit();                  //!< afterMove + refreshDiagnostics
	void computeCompletion();
	void computeDocBar();

	//! Where a candidate list came from, so the caller never has to read `what`.
	struct Source {
		Openness openness = Openness::Unstated;
		std::string what;
		std::string family;   //!< "" unless the names ARE a family's
	};
	std::vector<std::string> keyCandidates(const std::string &command,
	                                       const std::string &excluding, Source &src) const;
	std::vector<std::string> valueCandidates(const std::string &command,
	                                         const std::string &key, Source &src) const;
	//! The most specific spec the contract has for `command`'s key `key`:
	//! the command's own arg spec, filled where empty from the family entry the
	//! command draws its keys from, then from the command's key_grammar.
	//! `docScope`, when given, comes back naming WHICH of those three the `doc`
	//! sentence came from (see DocBar::doc_of).
	Spec specForKey(const std::string &command, const std::string &key,
	                std::string *docScope = nullptr) const;
};

} // namespace scedit

#endif // SCEDIT_SC_EDITCORE_HPP
