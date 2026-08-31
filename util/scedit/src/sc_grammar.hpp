/*
 * scedit -- sc_grammar.hpp
 *
 * WHAT THIS IS FOR
 * ================
 * The machine-readable command contract (grammar/sc-grammar.json), turned into
 * the lookups an analyser needs: "is this a command?", "is this name in that
 * family?", "which arguments does this command take?", "what severity does
 * lint id X carry?".
 *
 * The JSON file is the authority for CONTENT; this class is only its reader.
 * Nothing here hard-codes a command, a flag or a family member -- a name that
 * is not in the file does not exist for scedit. The one exception is
 * documented and bounded: SubfamilyPosition (below).
 *
 * ARMING BY DATA PRESENCE
 * =======================
 * Two checks arm themselves from the file alone, so the extraction sweep can
 * land without touching this code:
 *   - a command's argument-key vocabulary arms when its entry carries `args`
 *     (object of key -> spec, or array of names, or array of {name:...})
 *     AND declares `args_complete: true`;
 *   - a family-name check arms when the command entry carries `subfamily` AND
 *     the position of that name in the line is known.
 *
 * WHY `args_complete` IS A SEPARATE ANSWER FROM "HAS ARGS"
 * =======================================================
 * "I know some of this command's keys" and "I know all of them" are different
 * facts, and only the second one licenses calling a key UNKNOWN. Four handlers
 * forward the whole parsed map to another module; for two of them (dso3d,
 * landscape) that module's key set was extracted, for the other two (body,
 * camera -- and `flyto`, which IS camera) it is another contract file's
 * deliverable. Those entries say `args_complete: false`, and no consumer may
 * report an unlisted key of theirs. The default when the field is absent is
 * TRUE, because a hand-written entry that lists keys is claiming to list them.
 *
 * A command may also point at a FAMILY instead of carrying its own key specs
 * (`args_source`, used by `set`: its 43 keys ARE families.set_names). Such a
 * command keeps `arg_keys` empty on purpose -- the family-name check already
 * covers every key of the line, and a second check over the same keys would
 * report each one twice.
 *
 * ORDERING MATTERS
 * ================
 * `commandLookupList()` and `familySorted()` return names in std::map order
 * (byte-lexicographic), because that is the order the engine feeds to its
 * did-you-mean search and the search keeps the FIRST minimum. Feeding file
 * order instead would produce a different suggestion for a tie.
 *
 * OWNERSHIP: by value; returned pointers/references point into the Grammar and
 * live as long as it does.
 */

#ifndef SCEDIT_SC_GRAMMAR_HPP
#define SCEDIT_SC_GRAMMAR_HPP

#include <map>
#include <set>
#include <string>
#include <vector>

namespace scedit {

//! Where in the line the subfamily NAME sits, for commands that name one.
enum class SubfamilyPosition {
	Unknown,     //!< not established -> the family check stays UNARMED
	EveryKey,    //!< every pair's key is a family name (e.g. `set`)
	AppliedKey,  //!< only args.begin()'s key is applied (e.g. `flag`)
	ValueOfKey   //!< the name is the value of `position_key` (e.g. `color property X`)
};

struct SubfamilyPlacement {
	SubfamilyPosition pos = SubfamilyPosition::Unknown;
	std::string position_key;   //!< meaningful for ValueOfKey
	std::string anchor;         //!< engine file:line the placement was read from
};

struct FamilyData {
	//! A family's `names` entry is EITHER a plain string (v1 shape, families
	//! whose doc pass has not run) OR an object with a `name` field (D7 v2
	//! shape, `set_names` today). Both yield the same names here; the extra
	//! per-name content stays in the file for the doc panel to read.
	std::vector<std::string> names;        //!< file order
	std::vector<std::string> sorted;       //!< std::map order (suggestion candidates)
	std::set<std::string> name_set;
	//! Names the file annotates as present-but-broken. `orphaned` = spelled in
	//! the engine header but never registered; `known_defective` = registered
	//! and inert. Value is the file's own explanation, used as the message.
	std::map<std::string, std::string> orphaned;
	std::map<std::string, std::string> known_defective;
};

struct CommandData {
	std::string name;
	//! `pretable: true`: compared as a literal BEFORE the m_commands lookup
	//! (comment, uncomment). Accepted by the engine, absent from `commandList`,
	//! so a did-you-mean can never suggest it.
	bool pretable = false;
	std::string subfamily;      //!< "" when the command names none
	SubfamilyPlacement placement;
	//! "" unless the entry declares an alias. Then every argument-shaped field
	//! below is the TARGET's, copied once at load (Grammar::load): the file
	//! holds those facts on the canonical entry only.
	std::string alias_of;
	bool has_args = false;      //!< the entry carries extracted `args` data
	//! The entry claims its `args` list is the WHOLE accepted key vocabulary.
	//! False = there are more legal keys than are listed, so an unlisted key
	//! must NOT be reported as unknown. Default true (see the header note).
	bool args_complete = true;
	//! Set when the keys live in a family instead of in `args` (`set`). The
	//! value is the file's own text, used to explain the dormancy in --rules.
	std::string args_source;
	//! The entry carries `key_grammar`: this command's keys are NOT a fixed
	//! list of its own (a flag name, a variable name, free text...). Nothing to
	//! check against, and nothing missing either.
	bool free_keys = false;
	std::set<std::string> arg_keys;
	std::vector<std::string> arg_keys_sorted;
};

struct LintSeed {
	std::string id;
	std::string severity;
	std::string rule;
	std::string source;
	//! What a `#!` tail of this class starts with, VERBATIM from the engine's
	//! own message constants (`engine_tail` in the file, anchored to
	//! app_command_interface.cpp's MSG_*). Empty for rules the engine does
	//! not write into scripts. The editor's C1 signal reads it.
	std::vector<std::string> engine_tails;
};

class Grammar {
public:
	//! Returns false and fills `err` on I/O or JSON error.
	bool load(const std::string &path, std::string &err);

	const CommandData *command(const std::string &name) const;
	//! Accepted by the engine: registered in m_commands OR intercepted
	//! pre-table (comment/uncomment).
	bool isCommand(const std::string &name) const { return command(name) != nullptr; }
	//! The engine's `commandList`: REGISTERED names only, std::map order.
	//! comment/uncomment are absent, so a did-you-mean can never suggest them.
	const std::vector<std::string> &commandLookupList() const { return command_lookup_; }

	const FamilyData *family(const std::string &name) const;

	//! AppCommandInit::obsoletList (app_command_init.cpp:14-23).
	bool isObsolete(const std::string &token) const { return obsolete_.count(token) != 0; }

	const LintSeed *seed(const std::string &id) const;
	//! The seed whose engine message a `#!` tail sentence starts with, or
	//! nullptr: a class the engine writes that scedit does not check (its
	//! generic channel, or a newer engine).
	const LintSeed *seedForEngineTail(const std::string &sentence) const;

	//! Commands that read only `args.begin()` and silently drop the rest.
	bool isSinglePairCommand(const std::string &name) const { return single_pair_.count(name) != 0; }

	//! May a consumer report an unlisted key of this command as unknown?
	//! False for an unknown command too: nothing is known, so nothing is claimed.
	bool argKeysAreExhaustive(const std::string &name) const {
		const CommandData *cd = command(name);
		return cd && cd->has_args && cd->args_complete;
	}

private:
	std::map<std::string, CommandData> commands_;
	std::vector<std::string> command_lookup_;
	std::map<std::string, FamilyData> families_;
	std::set<std::string> obsolete_;
	std::map<std::string, LintSeed> seeds_;
	std::set<std::string> single_pair_;
};

} // namespace scedit

#endif // SCEDIT_SC_GRAMMAR_HPP
