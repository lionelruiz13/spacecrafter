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
 * THE ERROR HISTORY, AND WHAT "HISTORY" MEANS HERE
 * ================================================
 * `errorHistory()` is every place in THIS BUFFER something is wrong with the
 * script, in line order: each `#!` tail the ENGINE wrote when it last ran the
 * file, and each finding scedit makes now. `warpTo()` puts the caret on one.
 *
 * "History" is Vixy's word [2026-08-30, scedit/INTENT.md §5 item 15(a)]:
 * *"otherwise listed in an error history with click-to-warp-cursor"*. READING,
 * flagged as an interpretation (README § The error pane, veto open): the
 * history is the CURRENT buffer's set, not a log of past editing sessions. The
 * argument is that the engine's channel already IS the log — a `#!` tail stays
 * in the file until the fault is fixed and the engine reaches a natural end
 * (parse_model.comments.machine_tail), so the file itself carries what
 * spacecrafter found the last time it ran, and a second store would be a copy
 * of it that can only go stale (I2). Nothing is remembered across an open, and
 * nothing survives its cause: fix the fault and the row goes.
 *
 * A line carrying BOTH a tail and a finding produces TWO entries, never one.
 * They are two claims by two authors about one line — the engine says what
 * happened when it RAN, scedit says what it reads NOW — and the case where
 * they differ is exactly the C1 signal item 15 names. Merging them would hide
 * it. The engine's row comes first on a line (there is at most one, and it
 * records a run that happened); scedit's follow in the checker's own order
 * (cause before consequence, sc_check.hpp § ORDER).
 *
 * THE ENGINE WRITES BACK, AND NOTHING MAY BE LOST TO IT
 * =====================================================
 * spacecrafter rewrites the script it just played: at the natural end of a run
 * it puts a `#!` tail on every faulty line, and removes the tails of lines that
 * are now clean (src/scriptModule/script_annotator.hpp). So the file under an
 * open buffer changes while the buffer is open, written by somebody else, and
 * two different things can be lost — the author's edits, or the engine's
 * findings.
 *
 * The rule here is that NEITHER is lost without the author choosing it:
 *   - `diskState()` compares the bytes on disk with `diskImage()`, the bytes
 *     this buffer was read from or last written as. Byte comparison, not a
 *     digest: the file is a script, the cost is one read, and there is then no
 *     collision question to reason about at all.
 *   - `save()` REFUSES when the disk has changed, and its message names the two
 *     ways out rather than picking one. It refuses for a clean buffer too: a
 *     clean buffer holds the bytes from BEFORE the run, so writing it back is
 *     exactly how the engine's tails would disappear.
 *   - `reloadFromDisk()` and `saveOverwriting()` are those two ways out, each
 *     one explicit call. The editor binds a key to each and says which loses
 *     what.
 * WHEN the check runs is the caller's business (the editor runs it on a bounded
 * poll after a play, and always before a save); this class has no clock.
 *
 * OWNERSHIP: an EditCore owns its Document, Grammar and DocIndex. It owns NO
 * connection: the live channel is `sc_tcpclient.hpp`, held by the editor layer,
 * and nothing here depends on a socket's lifetime (I5). References and pointers
 * this class returns die with it or with the next mutation.
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
	MachineTail,   //!< the caret is inside a `#!` tail the ENGINE wrote (parse_model.comments.machine_tail)
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
	//! The `#!` tail on the caret's line, if any, as the bar shows it: the
	//! engine's sentence(s) followed by their relation to scedit's own findings
	//! (MachineTail::relation). "" when the line carries none.
	std::string annotation;
};

//! A `#!` tail the engine wrote on a line (parse_model.comments.machine_tail),
//! and how it relates to what scedit finds on that line — the C1 signal:
//! the two readings of one line must agree, and a disagreement is information.
struct MachineTail {
	std::size_t begin = std::string::npos;   //!< raw offset of the `#!`, npos = no tail
	std::string text;                        //!< after "#! ", the sentence(s)
	//! One sentence per engine message: "agrees with scedit's <id>" /
	//! "scedit finds no <id> here now: ..." / "not a class scedit checks".
	std::string relation;
	bool present() const { return begin != std::string::npos; }
};

//! Who says something is wrong with a line.
enum class EntrySource {
	Engine,   //!< spacecrafter wrote a `#!` tail there when it last ran the file
	Scedit    //!< scedit's own finding, recomputed after every edit
};

//! One row of the error history (see the header note). Everything the pane and
//! `--history` show, and everything `warpTo` needs.
struct ErrorEntry {
	EntrySource source = EntrySource::Scedit;
	std::size_t line = 0;      //!< 1-based file line, `Diagnostic::line`'s numbering
	//! The seed's severity for a scedit finding. EMPTY for an engine tail: the
	//! engine states none, and giving it scedit's would be scedit's opinion
	//! printed as the engine's.
	std::string severity;
	std::string id;            //!< the lint id; the literal `#!` for an engine tail
	std::string message;       //!< the finding's message, or the engine's sentence(s)
	//! ENGINE rows only: how the tail relates to scedit's findings on that line
	//! (MachineTail::relation) — agree / finds-nothing-now / unknown class.
	std::string relation;
	//! Where `warpTo` lands: the finding's own span, or the tail's `#!`. An
	//! empty span (a finding about the line as a whole) warps to byte 0.
	Span span;
};

struct Cursor {
	std::size_t line = 0;
	std::size_t col = 0;   //!< BYTE offset into the line, never a character count
};

//! What the file on disk is, compared with the bytes this buffer came from.
//! There is no "newer"/"older" here on purpose: a timestamp answers a different
//! question and can move without the content moving.
enum class DiskState {
	NoFile,    //!< this buffer has no path (a new buffer): nothing to compare
	Same,      //!< byte-identical to what we read (or last wrote)
	Changed,   //!< somebody else wrote it — the engine's `#!` pass, or another editor
	Gone       //!< it can no longer be read (deleted, renamed, permissions)
};

class EditCore {
public:
	//! Load the contract, then the file. `filePath` may be empty (new buffer).
	bool open(const std::string &grammarPath, const std::string &filePath, std::string &err);
	//! Load the contract and take the buffer from memory (tests, --ui-selftest).
	//! `label` is a NAME, not a path: findings are reported under it, and
	//! nothing on disk is read, written or compared. `hasFile()` says false for
	//! such a buffer, and `save()` refuses it — the two were conflated until a
	//! write-back check asked a labelled buffer what was on disk and was told
	//! "gone", which is an answer about a file that never existed.
	bool openBytes(const std::string &grammarPath, const std::string &label,
	               const std::string &bytes, std::string &err);

	//! Is there a FILE under this buffer? False for `openBytes`, and for `open`
	//! with an empty path (a new buffer).
	bool hasFile() const { return has_file_; }

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

	//! Save — REFUSING when the file changed on disk since it was read (see the
	//! header note). On refusal `err` is a sentence naming what changed, why it
	//! matters, and the two choices; nothing is written.
	bool save(std::string &err);
	//! "My edits win": save over whatever is on disk now. The one call that can
	//! destroy an engine tail, and it exists so that destroying one is an act.
	bool saveOverwriting(std::string &err);
	bool saveAs(const std::string &path, std::string &err);

	// --- the file underneath (see the header note) --------------------------
	//! Compare the file with the bytes this buffer was read from. One read per
	//! call; the caller decides how often (this class has no clock).
	DiskState diskState() const;
	//! The bytes this buffer believes are on disk: what `open` read, or what the
	//! last successful save wrote. Empty for a buffer with no file.
	const std::string &diskImage() const { return disk_image_; }
	//! Re-read the file, replacing the buffer. The caret keeps its line and
	//! column where the new file still has them. Everything derived — findings,
	//! the error history, the doc bar — is rebuilt, so a `#!` tail the engine
	//! has just written appears in `errorHistory()` immediately.
	//! Returns false with `err` when the file cannot be read; the buffer is then
	//! left exactly as it was.
	bool reloadFromDisk(std::string &err);
	//! How many rows of the error history are the ENGINE's `#!` tails. The
	//! editor reports the number after a reload ("spacecrafter left 5 findings"),
	//! and a harness asserts it.
	std::size_t engineTailCount() const;

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

	//! The `#!` tail the engine wrote on a 0-based line, located exactly as the
	//! engine locates it (the first `#!` at or after the comment's '#'), with
	//! its relation to scedit's findings on that line. Never written by the
	//! editor: shown, and compared.
	MachineTail machineTail(std::size_t line) const;

	//! Findings on a 1-based file line (Diagnostic::line's own numbering).
	std::vector<const Diagnostic *> diagnosticsForLine(std::size_t oneBasedLine) const;
	const std::vector<Diagnostic> &diagnostics() const { return diags_; }
	//! Highest severity present on that line: "error" > "warning" > "info", "" when clean.
	std::string severityForLine(std::size_t oneBasedLine) const;

	//! Every engine tail and every finding in the buffer, in line order (see the
	//! header note on what "history" means). Rebuilt with the findings after
	//! every edit; the references die with the next mutation.
	const std::vector<ErrorEntry> &errorHistory() const { return history_; }
	//! Caret to the entry: its line, at the byte its span begins on.
	void warpTo(const ErrorEntry &e);

	//! Recompute the findings now (done automatically after every mutation).
	void refreshDiagnostics();

private:
	Grammar grammar_;
	DocIndex docs_;
	Document doc_;
	std::string path_;
	//! The bytes we believe the file holds — set at open, replaced at every
	//! successful save and reload. This is the ONE thing "changed on disk" is
	//! measured against; keeping it means a save cannot be fooled by an edit
	//! that happens to restore the original length or timestamp.
	std::string disk_image_;
	bool has_file_ = false;
	Cursor cur_;

	Line line_;                        //!< tokenization of the cursor's line
	Completion completion_;
	DocBar docbar_;
	std::vector<Diagnostic> diags_;
	std::vector<ErrorEntry> history_;

	void afterMove();                  //!< retokenize + recompute completion/doc bar
	void afterEdit();                  //!< afterMove + refreshDiagnostics
	void rebuildHistory();             //!< after diags_: the two sources, merged in line order
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
