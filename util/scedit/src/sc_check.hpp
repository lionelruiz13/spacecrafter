/*
 * scedit — sc_check.hpp
 *
 * WHAT THIS IS FOR
 * ================
 * "Read this script the way the engine will, and tell me every place where
 * what the engine will do differs from what the author plainly meant."
 *
 * Every rule here is a REPORT OF ENGINE BEHAVIOUR, never a style opinion: each
 * one names the engine site that produces the surprise. A rule that cannot be
 * grounded in grammar/sc-grammar.json + the parse model at ZERO false positives
 * is not armed at all (constraint C3, scedit/INTENT.md §2) — an unarmed rule is
 * listed in `unarmedRules()` with the reason, so the gap is visible rather than
 * silent.
 *
 * Diagnostics are gcc-shaped per D6: `file:line: severity: message [-Wid]`.
 * Ids are the ones in the contract file's `lint_seeds`; severities come from
 * the same place, so retuning a severity is a data edit.
 *
 * ORDER: diagnostics come back sorted by line; within a line, cause before
 * consequence (the order the rules ran). Two rules report at the END of the
 * file about an EARLIER line — `unclosed-struct` (an opener never closed is
 * reported at the opener, the root, not at EOF where the damage surfaces) —
 * and the sort is what puts them where a reader expects them.
 *
 * OWNERSHIP: `checkFile` returns diagnostics by value; the Grammar reference
 * must outlive the call only.
 */

#ifndef SCEDIT_SC_CHECK_HPP
#define SCEDIT_SC_CHECK_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "sc_grammar.hpp"
#include "sc_tokenizer.hpp"

namespace scedit {

struct Diagnostic {
	std::string file;
	std::size_t line = 0;      //!< 1-based, counting every line of the file
	std::string severity;      //!< "error" | "warning" | "info"
	std::string message;
	std::string id;            //!< lint id, printed as [-Wid]
	//! RAW byte range on `line` the finding is about — the token, the byte
	//! run or the statement the message names — so a consumer can mark it at
	//! its exact bytes. Empty (begin == end == 0) means "the line as a whole".
	//! Not printed by format(): D6's shape is `file:line:` with no column, and
	//! the recorded expected files pin that shape.
	Span span;

	//! D6 shape, exact: file:line: severity: message [-Wid]
	std::string format() const;
};

//! Analyse already-read file bytes. `path` is only used to label diagnostics.
std::vector<Diagnostic> checkBuffer(const Grammar &g, const std::string &path,
                                    const std::string &bytes);

//! Read and analyse a file. Sets `io_error` (and returns empty) when the file
//! cannot be read.
std::vector<Diagnostic> checkFile(const Grammar &g, const std::string &path,
                                  std::string &io_error);

//! The did-you-mean the checker prints, exposed because it must not be written
//! twice: the machine surface (sc_docjson.hpp) answers an unknown name in a
//! `--doc` query with the same suggestion an author gets from `--check` on the
//! same misspelling, or the two surfaces would disagree about what the engine's
//! nearest name is. Empty = nothing near enough to be worth printing; the cap
//! and its argument are at the definition (sc_check.cpp).
std::string cappedSuggestion(const std::string &token, const std::vector<std::string> &candidates);

//! Rules the seeds define but this build does NOT emit, with the reason.
struct UnarmedRule { std::string id, reason; };
std::vector<UnarmedRule> unarmedRules(const Grammar &g);

} // namespace scedit

#endif // SCEDIT_SC_CHECK_HPP
