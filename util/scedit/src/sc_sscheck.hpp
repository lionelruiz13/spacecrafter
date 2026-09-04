/*
 * scedit -- sc_sscheck.hpp
 *
 * WHAT THIS IS FOR
 * ================
 * "Read this stellar-system file the way the engine will, and tell me every
 * place where what the engine will do differs from what the author plainly
 * meant."
 *
 * It is the twin of sc_check.hpp, for the other contract. Same rule of
 * evidence: every finding is a REPORT OF ENGINE BEHAVIOUR, grounded in
 * grammar/ss-grammar.json, which is itself generated from the loader's own read
 * sites -- never a style opinion, never a guess about what a key ought to mean.
 * It returns the SAME `Diagnostic` type, so every consumer that already prints,
 * serialises or underlines a script finding does the same for these with no
 * change (I1: the interface is the contract).
 *
 * TWO REGIMES, AND THE REGIME IS THE FILE'S LOCATION
 * =================================================
 * A stellar-system file is read by one of two loaders and they do not agree, so
 * "is this legal" has no answer until you know which one will read it:
 *
 *   LEGACY    ~/.spacecrafter/ssystem.ini, data/default_ssystem.ini
 *             ProtoSystem::addBody, the frozen comparison baseline. New-format
 *             constructs here BREAK DOWNGRADE (INTENT S2.0 D13, scedit C4): an
 *             older build must still read the file, so a construct its parser
 *             MISREADS is a hard error.
 *   COMPOSED  ~/.spacecrafter/modularSystem/<Name>.ini, the B24 format.
 *             The full grammar, module sections included.
 *
 * The choice is made from the PATH and never from the content (F80 mandate (3),
 * INTENT S11.78(d)). Sniffing would be worse than useless here: the composed
 * twin of a legacy file is byte-identical to it in every key, so content cannot
 * distinguish them even in principle -- the twin of ssystem.ini carries the same
 * `tex_halo` and the same misspelled `orbit_Period` the legacy file does.
 *
 * WHAT "NEW-FORMAT CONSTRUCT" MEANS, PRECISELY
 * ===========================================
 * NOT "a key the old loader does not read". The old loader ignores an unknown
 * key harmlessly, which is why the shipped ssystem.ini already carries 20
 * `rot_frame` and 20 `rot_pole_w0` lines ON PURPOSE (B28's absolute-pole
 * declarations) and why flagging those would be wrong. What breaks downgrade is
 * a construct the old parser MISREADS:
 *   - `relation=` / `compose=` / `body=` -- keys that change what the SECTION IS;
 *   - a `[Node:FAMILY]` module-section header, which the legacy reader treats as
 *     a body named "Node:FAMILY";
 *   - a mid-line `#`, which tools/ini_line.hpp reads as a comment and
 *     ProtoSystem::load (column 0 only) reads as data.
 * A composed-only KEY in a legacy file is reported too, but as information, not
 * as an error, and the message says which loader will read it.
 */

#ifndef SCEDIT_SC_SSCHECK_HPP
#define SCEDIT_SC_SSCHECK_HPP

#include <map>
#include <set>
#include <string>
#include <vector>

#include "sc_check.hpp"   // Diagnostic, Span

namespace scedit {

//! Which loader will read this file. Decided by LOCATION (see the header note).
enum class SsRegime {
	NotStellarSystem,   //!< not a stellar-system file at all: leave it alone
	Legacy,             //!< ssystem.ini class -- the frozen format
	Composed,           //!< modularSystem/<Name>.ini -- the B24 format
};

//! Regime from a path. Pure: it looks at the path text, never at the file.
SsRegime classifyPath(const std::string &path);

//! Human name of a regime, for messages and for `--rules`.
const char *regimeName(SsRegime r);

//! The stellar-system contract file (grammar/ss-grammar.json), loaded once.
class SsGrammar {
public:
	//! Load and validate. Returns false and fills `err` on any problem,
	//! including a file whose `_meta.contract` is not this contract -- being
	//! handed the command grammar by mistake must say so, not silently behave
	//! as if the stellar-system vocabulary were empty.
	bool load(const std::string &path, std::string &err);

	struct Key {
		std::string doc;
		std::string valueType;      //!< bool_legacy | bool_composed | double | ...
		std::string defaultValue;   //!< prose, as the contract states it
		bool required = false;
		bool legacy = false;        //!< read by the legacy loader
		bool composed = false;      //!< read by the composed loader
		std::vector<std::string> domain;   //!< enumerated values, empty if none
		std::string notes;
	};

	const Key *find(const std::string &key) const;
	//! Present in shipped data, read by nothing. Reason text, or nullptr.
	const std::string *deadKey(const std::string &key) const;
	const std::vector<std::string> &allKeyNames() const { return keyNames_; }
	const std::set<std::string> &moduleFamilies() const { return families_; }
	const std::set<std::string> &bodyTypes() const { return bodyTypes_; }
	const std::set<std::string> &coordFuncs() const { return coordFuncs_; }
	bool loaded() const { return loaded_; }

private:
	std::map<std::string, Key> keys_;
	std::map<std::string, std::string> dead_;
	std::vector<std::string> keyNames_;
	std::set<std::string> families_, bodyTypes_, coordFuncs_;
	bool loaded_ = false;
};

//! Analyse already-read file BYTES. The bytes are taken as ISO-8859-1 at this
//! boundary and every reported span is a BYTE range, because that is what the
//! engine's own readers count in and what an editor has to underline.
std::vector<Diagnostic> ssCheckBuffer(const SsGrammar &g, const std::string &path,
                                      const std::string &bytes, SsRegime regime);

//! Read and analyse a file. Sets `io_error` (and returns empty) when it cannot
//! be read.
std::vector<Diagnostic> ssCheckFile(const SsGrammar &g, const std::string &path,
                                    SsRegime regime, std::string &io_error);

//! Every rule this checker can emit, with its severity -- the `--rules` surface
//! for the second contract, so an unarmed or absent rule is visible.
struct SsRule { std::string id, severity, what; };
std::vector<SsRule> ssRules();

} // namespace scedit

#endif // SCEDIT_SC_SSCHECK_HPP
