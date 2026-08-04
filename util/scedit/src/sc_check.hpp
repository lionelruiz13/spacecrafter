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
 * OWNERSHIP: `checkFile` returns diagnostics by value; the Grammar reference
 * must outlive the call only.
 */

#ifndef SCEDIT_SC_CHECK_HPP
#define SCEDIT_SC_CHECK_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "sc_grammar.hpp"

namespace scedit {

struct Diagnostic {
	std::string file;
	std::size_t line = 0;      //!< 1-based, counting every line of the file
	std::string severity;      //!< "error" | "warning" | "info"
	std::string message;
	std::string id;            //!< lint id, printed as [-Wid]

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

//! Rules the seeds define but this build does NOT emit, with the reason.
struct UnarmedRule { std::string id, reason; };
std::vector<UnarmedRule> unarmedRules(const Grammar &g);

} // namespace scedit

#endif // SCEDIT_SC_CHECK_HPP
