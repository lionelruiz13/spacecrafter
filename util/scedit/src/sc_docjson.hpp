/*
 * scedit -- sc_docjson.hpp
 *
 * WHAT THIS IS FOR
 * ================
 * The same answers the editor's doc bar gives a human, serialised for a
 * machine: `--doc` (one page, or the whole catalogue), `--search` (which pages
 * a request is about), and `--check --json` (the findings as objects). The
 * outside consumer this exists for is a language model -- through the MCP server
 * (sc_mcp.hpp), through a harness shelling out, or through an author's own
 * script -- and the point of it is that such a consumer gets the CONTRACT FILE's
 * own words instead of its recollection of a planetarium's script language.
 *
 * NO SECOND READER (invariant I2)
 * ===============================
 * Nothing here parses grammar/sc-grammar.json. Every fact comes from `Grammar`
 * (the structural reader) or `DocIndex` (the presentation reader), which is
 * exactly what the editor shows. A doc line that is wrong here is wrong on the
 * screen too, and is fixed once, in the file.
 *
 * THE HONEST-NULL RULE SURVIVES SERIALISATION (constraint C2)
 * ==========================================================
 * `doc_known == false` becomes JSON `null`, never the empty string and never
 * `kNoDoc`'s sentence: a consumer that cannot tell "no documentation was
 * extracted" from "the documentation is empty" will fill the gap itself, which
 * is the one failure mode this whole surface exists to prevent. The two flagged
 * keys (`dso3d.z_reflection`, `suntrace.sun`) and all 184 v1 family names come
 * out as `"doc": null` with `"present": true` -- the name exists, the sentence
 * does not. Fields other than `doc` carry no such distinction because the file
 * draws none: absent and empty both arrive as an empty string.
 *
 * THE SEARCH SCORE IS A STATED FORMULA, NOT AN OPINION
 * ====================================================
 * `--search` ranks pages by ONE published number and no inference: for a query
 * Q and a page P,
 *
 *     score(P) = |words(Q) INTERSECT words(P)| / (1 + sqrt(|words(P)|))
 *
 * where `words` lowercases, cuts on anything outside [a-z_], and keeps runs of
 * three or more; a page's own words are its NAME (underscores read as spaces)
 * plus its doc line. Pages that score zero are not answers and are not
 * returned. The formula is not ours: it is the model-free baseline of
 * claude/harness/f64_doc_router.py, ported so that the tool and the measurement
 * that judges routing quality speak about the same ranking -- the parity gate
 * (harness/f66_search_parity.py) asserts pair-by-pair agreement over that
 * script's 340 witness questions, which is a criterion this port can fail.
 * Ties keep ENUMERATION order: commands in the contract file's order, then (in
 * the `all` scope) each command's argument keys and family names.
 *
 * OWNERSHIP: everything is returned by value; the Grammar and DocIndex
 * references must outlive the call only.
 */

#ifndef SCEDIT_SC_DOCJSON_HPP
#define SCEDIT_SC_DOCJSON_HPP

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "sc_check.hpp"
#include "sc_docindex.hpp"
#include "sc_grammar.hpp"

namespace scedit {

//! One `--doc` answer. `found == false` carries the error object instead of the
//! page -- the caller turns that into exit code 2 (CLI) or an isError result
//! (MCP), and the object always names the vocabulary the name is missing from.
struct DocAnswer {
	bool found = false;
	nlohmann::json value;
};

//! One page. `name` empty = the command's own page; otherwise an argument key
//! of that command, or a name of the family the command draws its keys from
//! (`flag stars`), resolved in that order.
DocAnswer docLookup(const Grammar &g, const DocIndex &d,
                    const std::string &command, const std::string &name);

//! Every command with its one-liner, plus the family names under the commands
//! that name a family -- the two-level shape a model is given as a catalogue.
//! Argument keys are NOT included: they are one `docLookup` away and would
//! quadruple the answer.
nlohmann::json docCatalogue(const Grammar &g, const DocIndex &d);

//! `all` ranks commands, argument keys and family names; `commands` ranks
//! commands only, which is the search space the f64 baseline measured.
enum class SearchScope { All, Commands };

nlohmann::json docSearch(const Grammar &g, const DocIndex &d,
                         const std::string &query, SearchScope scope, std::size_t limit);

//! The findings of a `--check` run as objects, beside (never instead of) the
//! D6 text shape. `span` is the raw byte range on the line, or null for a
//! finding about the line as a whole.
nlohmann::json diagnosticsJson(const std::vector<std::string> &files,
                               const std::vector<Diagnostic> &diags);

} // namespace scedit

#endif // SCEDIT_SC_DOCJSON_HPP
