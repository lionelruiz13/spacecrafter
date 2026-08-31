/*
 * scedit -- sc_docindex.hpp
 *
 * WHAT THIS IS FOR
 * ================
 * The PROSE half of grammar/sc-grammar.json: the one-line documentation of a
 * command, of an argument key, of a single enumerated value, of a family name --
 * plus the value domain, the default sentence and the engine anchor that go
 * with them. This is what the editor's documentation bar shows, and it is the
 * whole of constraint C6: someone with no knowledge of scripting must be able
 * to read a line of a show and change it.
 *
 * WHY THIS IS NOT IN sc_grammar.hpp
 * =================================
 * Not a second copy of the contract -- a second READING of the same file, for a
 * different question (invariant I2 is about two AUTHORITIES, and there is still
 * exactly one: the JSON). `Grammar` answers structural questions an analyser
 * asks ("is this a command?", "may I call this key unknown?", "what severity
 * does this id carry?"); `DocIndex` answers presentation questions an editor
 * asks ("what do I tell the author about the thing under the cursor?"). The
 * two never contradict, because neither invents: both read the same bytes.
 *
 * THE HONEST-NULL RULE (constraint C2)
 * ====================================
 * A doc slot that is absent, or present and `null`, is NOT a doc. It arrives
 * here as `doc_known == false` and an EMPTY string, and the renderer must say
 * so in its own words (`kNoDoc`) rather than print something plausible. Two
 * real cases exist today and both must survive to the screen intact:
 *   - `dso3d.z_reflection` and `suntrace.sun` carry `"doc": null` -- the sweep
 *     could not answer them from the code and flagged them for Vixy;
 *   - five families (flags, color_names, obsolete_tokens, reserved_variables,
 *     font_targets) are still the v1 shape, a plain array of names with no doc
 *     at all: every one of their 184 names is an honest blank.
 *
 * WHAT MAY BE OFFERED AS A COMPLETION, AND WHAT MAY ONLY BE READ
 * ==============================================================
 * An arg spec's `values` array mixes two kinds of entry, because it was written
 * for a human reader: literal values the author types (`current`, `preset`,
 * `toggle`) and PROSE describing the rest of the domain (`<file name>`,
 * `anything else = off`, `on-forms per Utility::isTrue`). Nothing in the schema
 * distinguishes them (a flagged grammar-shape finding, see README S Vendoring's
 * neighbour S "What the editor cannot do yet"). scedit therefore applies one
 * stated rule, `isCompletableLiteral`: an entry may be OFFERED only if every
 * byte of it is [A-Za-z0-9_] -- a bare word the author could type unquoted.
 * Everything else is still SHOWN in the doc bar, verbatim. Offering is a UI
 * decision and cannot make scedit claim anything false; the doc bar, which can,
 * shows the file's own text and nothing else.
 *
 * DEFAULTS: PART DATA SINCE 2026-08-31 (F71 item 11), THE REST STILL PROSE
 * ========================================================================
 * D31 asks for the default value greyed in an empty value field and offered to
 * completion. Every arg spec carries `default` as a SENTENCE ("absent -> 0",
 * "absent or empty -> the next form is tried"); a sentence is not something a
 * machine may type on the author's behalf without reading English and guessing,
 * which C2 forbids. So the literals were read AT SOURCE, one by one, and written
 * as data: an explicit `default_value` string becomes `default_literal`, is
 * offered first among the value candidates and is what the ghost shows on an
 * empty field. 60 of the 324 carry one; `dormantFeatures()` reports the
 * remaining coverage out loud -- the `unarmedRules()` precedent, same reason.
 * Some of the 264 never will: a default that depends on which form the line
 * takes has no single literal, and the FALSE side of a boolean has no spelling
 * the engine recognises (`Utility::isTrue` accepts only TRUE/ON/1 and calls
 * everything else false), so offering one would teach a word that does not
 * exist. Each such exclusion is argued in `claude/harness/f71_defaults.py`.
 *
 * TWO PREDICATES, ON PURPOSE
 * ==========================
 * `isCompletableLiteral` filters a GUESS out of `values` prose, so it is strict.
 * A `default_value` is not a guess: it is verified data carrying a source anchor,
 * and the seed gate refuses one without it. Applying the strict rule to it would
 * silently drop legitimate defaults that merely contain a dot or a minus sign
 * (`0.05`, `-90`, `personal.txt`). So the default literal is filtered by
 * `isTypeableValue` instead, which asks the only question that actually matters
 * for inserting bytes into a script line: can the engine's tokenizer read this
 * back as ONE unquoted value? -- i.e. no whitespace and no `"`.
 *
 * OWNERSHIP: by value; returned pointers point into the DocIndex.
 */

#ifndef SCEDIT_SC_DOCINDEX_HPP
#define SCEDIT_SC_DOCINDEX_HPP

#include <map>
#include <string>
#include <vector>

namespace scedit {

//! What the renderer prints where a doc line would have been. One spelling, in
//! one place, so the tests and the screen cannot disagree about it.
extern const char *const kNoDoc;

//! May this string be OFFERED as a completion candidate? True iff it is a
//! non-empty run of [A-Za-z0-9_] -- see the header note.
bool isCompletableLiteral(const std::string &s);

//! May this string be TYPED into a value slot as it stands? True iff it is
//! non-empty and holds no whitespace and no `"` -- the tokenizer's own
//! condition for reading a run of bytes back as one unquoted value. Used for
//! the verified `default_value` literal, where the strict rule above would
//! reject `0.05` and `personal.txt` for no reason that survives contact with
//! the parser. See the header note "TWO PREDICATES, ON PURPOSE".
bool isTypeableValue(const std::string &s);

//! Everything the file says about one named thing (an argument key, a family
//! name, or a command's key grammar). Fields are the file's own text.
struct Spec {
	bool present = false;          //!< an entry for this name exists at all
	bool doc_known = false;        //!< a non-null `doc` was found
	std::string doc;

	std::string value_domain;      //!< the `value` sentence ("enumerated", "number(evalDouble)...")
	std::vector<std::string> values;        //!< `values`, verbatim, prose entries included
	std::vector<std::string> completable;   //!< the subset that may be offered, sorted, deduped
	std::map<std::string, std::string> value_docs;   //!< per-value doc, when the file has one
	std::string default_prose;     //!< the `default` sentence
	std::string required;          //!< "true" / "false" / the file's own word
	std::string source;            //!< engine anchor
	std::string notes;

	bool has_default_literal = false;   //!< an explicit `default_value` was found
	std::string default_literal;

	//! Doc for one enumerated value, honest-null rules applied.
	bool valueDoc(const std::string &v, std::string &out) const;
};

struct CommandInfo {
	std::string name;
	bool doc_known = false;
	std::string doc;
	std::string registration;
	bool args_complete = true;
	std::string args_source;        //!< the file's explanation, "" when absent
	std::vector<std::string> keys;  //!< extracted keys, byte-lexicographic
	std::map<std::string, Spec> args;
	bool has_key_grammar = false;
	Spec key_grammar;               //!< what a KEY means for this command
	//! "" unless the entry is an alias; then keys/args/key_grammar/args_complete
	//! are the target's (resolved once at load, mirroring sc_grammar), while
	//! `doc` and `registration` stay the alias's own.
	std::string alias_of;
};

class DocIndex {
public:
	//! Returns false and fills `err` on I/O or JSON error.
	bool load(const std::string &path, std::string &err);

	const CommandInfo *command(const std::string &name) const;
	//! Every command the engine accepts, byte-lexicographic -- the pre-table
	//! literals (comment, uncomment) included, because the engine accepts them
	//! and so the editor may offer them.
	const std::vector<std::string> &commandNames() const { return command_names_; }
	//! The same names in the order the CONTRACT FILE lists them. Two orders
	//! exist because two questions do: a did-you-mean must answer in the
	//! engine's own order (byte-lexicographic, `commandNames`), while anything
	//! that ENUMERATES the surface -- the machine catalogue, the search ranking's
	//! tie-break -- answers in the file's order, so that the page a tie picks is
	//! the file's own first answer and not an alphabetical accident. Recording
	//! it is why this reader parses with `ordered_json` (sc_docindex.cpp).
	const std::vector<std::string> &commandFileOrder() const { return command_file_order_; }

	//! `parse_model.comments.script_layer`, verbatim: what the script layer does
	//! with a line whose first byte is '#'. Shown when the caret is on such a
	//! line, so that even "this line does nothing" is the file's own sentence.
	const std::string &commentLineDoc() const { return comment_line_doc_; }
	//! `parse_model.comments.mid_line`, verbatim: what the parser does with a
	//! '#' after the command. Shown when the caret is inside such a comment.
	const std::string &commentTailDoc() const { return comment_tail_doc_; }
	//! `parse_model.comments.machine_tail`, verbatim: what a `#!` tail is and
	//! who writes it. Shown when the caret is inside one.
	const std::string &machineTailDoc() const { return machine_tail_doc_; }

	//! What the file says about one name of one family. `present == false` when
	//! the family or the name is unknown; a v1 (plain-name) family yields
	//! `present == true, doc_known == false` -- the name exists, the doc does not.
	Spec familyMember(const std::string &family, const std::string &name) const;

	//! Features that exist in the code but have no data to run on yet, with the
	//! reason. Visible rather than silent (the `unarmedRules()` precedent).
	struct Dormant { std::string feature, reason; };
	std::vector<Dormant> dormantFeatures() const;

	//! How many arg specs carry an explicit machine-readable default.
	std::size_t defaultLiteralCount() const { return default_literals_; }

private:
	std::map<std::string, CommandInfo> commands_;
	std::vector<std::string> command_names_;
	std::vector<std::string> command_file_order_;
	std::map<std::string, std::map<std::string, Spec>> families_;
	std::map<std::string, bool> family_is_v2_;
	std::string comment_line_doc_;
	std::string comment_tail_doc_;
	std::string machine_tail_doc_;
	std::size_t default_literals_ = 0;
	std::size_t arg_specs_ = 0;
	std::size_t family_names_ = 0;
	std::size_t family_names_v1_ = 0;
};

} // namespace scedit

#endif // SCEDIT_SC_DOCINDEX_HPP
