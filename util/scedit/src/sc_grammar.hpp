/*
 * scedit — sc_grammar.hpp
 *
 * WHAT THIS IS FOR
 * ================
 * The machine-readable command contract (grammar/sc-grammar.json), turned into
 * the lookups an analyser needs: "is this a command?", "is this name in that
 * family?", "which arguments does this command take?", "what severity does
 * lint id X carry?".
 *
 * The JSON file is the authority for CONTENT; this class is only its reader.
 * Nothing here hard-codes a command, a flag or a family member — a name that
 * is not in the file does not exist for scedit. The one exception is
 * documented and bounded: SubfamilyPosition (below).
 *
 * ARMING BY DATA PRESENCE
 * =======================
 * Two checks arm themselves from the file alone, so the extraction sweep can
 * land without touching this code:
 *   - a command's argument-key vocabulary arms when its entry carries `args`
 *     (object of key -> spec, or array of names, or array of {name:...});
 *   - a family-name check arms when the command entry carries `subfamily` AND
 *     the position of that name in the line is known.
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
	bool pretable = false;      //!< carries "registration": intercepted before the table
	std::string subfamily;      //!< "" when the command names none
	SubfamilyPlacement placement;
	std::string alias_of;       //!< "" unless the entry declares an alias
	bool has_args = false;      //!< the entry carries extracted `args` data
	std::set<std::string> arg_keys;
	std::vector<std::string> arg_keys_sorted;
};

struct LintSeed {
	std::string id;
	std::string severity;
	std::string rule;
	std::string source;
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

	//! Commands that read only `args.begin()` and silently drop the rest.
	bool isSinglePairCommand(const std::string &name) const { return single_pair_.count(name) != 0; }

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
