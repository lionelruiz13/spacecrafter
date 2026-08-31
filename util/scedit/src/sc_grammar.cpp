/*
 * scedit — sc_grammar.cpp — reader for grammar/sc-grammar.json.
 *
 * Two tables below are CODE, not data, and should not stay that way; each
 * entry carries the engine line it was read from, and each is overridable by
 * the contract file the moment the file grows the field (see the notes).
 * Recorded in util/scedit/tests/derivation-diff.md §6.
 */

#include "sc_grammar.hpp"

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace scedit {

namespace {

//! WHERE THE SUBFAMILY NAME SITS IN THE LINE.
//! The contract file says WHICH family a command draws from ("subfamily") but
//! not WHERE the name appears, and the position is not derivable from the
//! family. Until the file carries `subfamily_position`, this anchored table is
//! the authority; a command absent from it keeps SubfamilyPosition::Unknown and
//! its family check stays UNARMED (never guessed - C2).
struct BuiltinPlacement {
	const char *command;
	SubfamilyPosition pos;
	const char *key;
	const char *anchor;
};
const BuiltinPlacement kBuiltinPlacements[] = {
	// commandFlag: setFlag(args.begin()->first, args.begin()->second, val)
	{"flag", SubfamilyPosition::AppliedKey, "",
	 "app_command_interface.cpp:1183-1185"},
	// commandSet: for (const auto&i : args) evalCommandSet(i.first, i.second)
	{"set", SubfamilyPosition::EveryKey, "",
	 "app_command_interface.cpp:2119-2121"},
	// commandColor: m_color.find(args[W_PROPERTY]) -> "unknown property"
	{"color", SubfamilyPosition::ValueOfKey, "property",
	 "app_command_interface.cpp:1874-1885"},
	// `font`: the name IS args[W_TARGET] (:4152) but the rejection lives in
	// FontFactory::updateFont, unread - left UNARMED on purpose.
};

//! COMMANDS THAT APPLY ONLY args.begin() AND DROP THE REST SILENTLY.
//! Overridable per command by `"single_pair": true` in the contract file.
struct BuiltinSinglePair {
	const char *command;
	const char *anchor;
};
const BuiltinSinglePair kBuiltinSinglePair[] = {
	{"flag",     "app_command_interface.cpp:1183"},
	{"define",   "app_command_interface.cpp:4484"},
	{"add",      "app_command_interface.cpp:4498"},
	{"sub",      "app_command_interface.cpp:4511"},
	{"multiply", "app_command_interface.cpp:4525"},
	{"divide",   "app_command_interface.cpp:4538"},
	{"modulo",   "app_command_interface.cpp:4551"},
	{"tangent",  "app_command_interface.cpp:4564"},
	{"trunc",    "app_command_interface.cpp:4577"},
	{"sinus",    "app_command_interface.cpp:4590"},
};

//! Argument vocabularies may arrive from the extraction sweep in any of three
//! shapes; all three answer the same question, so all three are accepted.
void collectArgKeys(const json &args, std::set<std::string> &out)
{
	if (args.is_object()) {
		for (auto it = args.begin(); it != args.end(); ++it) {
			if (!it.key().empty() && it.key()[0] == '_')
				continue;   // annotation, not an argument
			out.insert(it.key());
		}
	} else if (args.is_array()) {
		for (const auto &a : args) {
			if (a.is_string())
				out.insert(a.get<std::string>());
			else if (a.is_object() && a.contains("name") && a.at("name").is_string())
				out.insert(a.at("name").get<std::string>());
		}
	}
}

//! A family's `names` entry: a plain string (v1) or an object with a `name`
//! field (D7 v2). Both shapes coexist on purpose — a family converts when its
//! doc pass fills content, one family at a time.
bool familyMemberName(const json &entry, std::string &out)
{
	if (entry.is_string()) {
		out = entry.get<std::string>();
		return true;
	}
	if (entry.is_object() && entry.contains("name") && entry.at("name").is_string()) {
		out = entry.at("name").get<std::string>();
		return true;
	}
	return false;
}

void readAnnotationMap(const json &fam, const char *key, std::map<std::string, std::string> &out)
{
	if (!fam.contains(key) || !fam.at(key).is_object())
		return;
	for (auto it = fam.at(key).begin(); it != fam.at(key).end(); ++it) {
		if (!it.key().empty() && it.key()[0] == '_')
			continue;
		out[it.key()] = it.value().is_string() ? it.value().get<std::string>() : std::string();
	}
}

} // namespace

bool Grammar::load(const std::string &path, std::string &err)
{
	std::ifstream in(path);
	if (!in) {
		err = "cannot open " + path;
		return false;
	}
	json g;
	try {
		in >> g;
	} catch (const std::exception &e) {
		err = path + ": " + e.what();
		return false;
	}

	try {
		const auto &fams = g.at("families");

		// --- families ------------------------------------------------------
		for (auto it = fams.begin(); it != fams.end(); ++it) {
			if (it.key() == "commands")
				continue;
			if (!it.key().empty() && it.key()[0] == '_')
				continue;   // annotation, not a family
			FamilyData fd;
			if (it.value().contains("names")) {
				for (const auto &n : it.value().at("names")) {
					std::string member;
					if (familyMemberName(n, member))
						fd.names.push_back(member);
				}
			}
			fd.name_set.insert(fd.names.begin(), fd.names.end());
			fd.sorted = fd.names;
			std::sort(fd.sorted.begin(), fd.sorted.end());
			readAnnotationMap(it.value(), "orphaned", fd.orphaned);
			readAnnotationMap(it.value(), "known_defective", fd.known_defective);
			families_[it.key()] = std::move(fd);
		}

		// --- commands ------------------------------------------------------
		const auto &cmds = fams.at("commands");
		for (auto it = cmds.begin(); it != cmds.end(); ++it) {
			if (!it.key().empty() && it.key()[0] == '_')
				continue;
			CommandData cd;
			cd.name = it.key();
			// `pretable` is EXPLICIT data. It used to be inferred from the
			// presence of a "registration" field, which stopped working the
			// moment that field became the registration SOURCE ANCHOR carried
			// by every command (args merge, 2026-08-04).
			if (it.value().contains("pretable") && it.value().at("pretable").is_boolean())
				cd.pretable = it.value().at("pretable").get<bool>();
			if (it.value().contains("subfamily"))
				cd.subfamily = it.value().at("subfamily").get<std::string>();
			if (it.value().contains("alias_of"))
				cd.alias_of = it.value().at("alias_of").get<std::string>();
			if (it.value().contains("args")) {
				collectArgKeys(it.value().at("args"), cd.arg_keys);
				cd.has_args = !cd.arg_keys.empty();
				cd.arg_keys_sorted.assign(cd.arg_keys.begin(), cd.arg_keys.end());
			}
			if (it.value().contains("args_complete") && it.value().at("args_complete").is_boolean())
				cd.args_complete = it.value().at("args_complete").get<bool>();
			if (it.value().contains("args_source") && it.value().at("args_source").is_string())
				cd.args_source = it.value().at("args_source").get<std::string>();
			cd.free_keys = !cd.has_args && it.value().contains("key_grammar");
			// data first, built-in table second
			if (it.value().contains("subfamily_position")) {
				const auto &sp = it.value().at("subfamily_position");
				if (sp.is_string()) {
					const std::string v = sp.get<std::string>();
					if (v == "every_key") cd.placement.pos = SubfamilyPosition::EveryKey;
					else if (v == "applied_key") cd.placement.pos = SubfamilyPosition::AppliedKey;
				} else if (sp.is_object() && sp.contains("value_of")) {
					cd.placement.pos = SubfamilyPosition::ValueOfKey;
					cd.placement.position_key = sp.at("value_of").get<std::string>();
				}
				cd.placement.anchor = "grammar/sc-grammar.json";
			}
			if (it.value().contains("single_pair") && it.value().at("single_pair").is_boolean()
			    && it.value().at("single_pair").get<bool>())
				single_pair_.insert(cd.name);

			if (!cd.pretable)
				command_lookup_.push_back(cd.name);
			commands_[cd.name] = std::move(cd);
		}
		// std::map order: the engine builds commandList by walking m_commands.
		std::sort(command_lookup_.begin(), command_lookup_.end());

		for (const auto &bp : kBuiltinPlacements) {
			auto it = commands_.find(bp.command);
			if (it == commands_.end() || it->second.subfamily.empty())
				continue;
			if (it->second.placement.pos != SubfamilyPosition::Unknown)
				continue;   // the contract file already answered
			it->second.placement.pos = bp.pos;
			it->second.placement.position_key = bp.key;
			it->second.placement.anchor = bp.anchor;
		}
		for (const auto &bs : kBuiltinSinglePair)
			if (commands_.count(bs.command))
				single_pair_.insert(bs.command);

		// --- aliases resolve ONCE, here ---------------------------------------
		// An alias entry (`alias_of`) carries no argument data of its own: the
		// engine registers a second name on the SAME enum, so the handler, the
		// keys and every claim about them are the canonical entry's, and the
		// file holds them once (I2). Every consumer reads CommandData, so this
		// is the one resolution point; `name` and `alias_of` stay the alias's
		// own. Runs after the built-in placement/single-pair passes so the
		// copy is of the FINISHED target. A dangling or chained alias is a
		// contract error here, not a nullptr somewhere later.
		for (auto &kv : commands_) {
			CommandData &cd = kv.second;
			if (cd.alias_of.empty())
				continue;
			auto t = commands_.find(cd.alias_of);
			if (t == commands_.end() || !t->second.alias_of.empty()) {
				err = path + ": command `" + cd.name + "` is an alias of `" + cd.alias_of + "`, which " +
				      (t == commands_.end() ? "does not exist" : "is itself an alias");
				return false;
			}
			const CommandData &c = t->second;
			cd.subfamily = c.subfamily;
			cd.placement = c.placement;
			cd.has_args = c.has_args;
			cd.arg_keys = c.arg_keys;
			cd.arg_keys_sorted = c.arg_keys_sorted;
			cd.args_complete = c.args_complete;
			cd.args_source = c.args_source;
			cd.free_keys = c.free_keys;
			if (single_pair_.count(c.name))
				single_pair_.insert(cd.name);
		}

		// --- obsolete tokens ------------------------------------------------
		auto obs = families_.find("obsolete_tokens");
		if (obs != families_.end())
			obsolete_ = obs->second.name_set;

		// --- lint seeds -----------------------------------------------------
		for (const auto &l : g.at("lint_seeds")) {
			LintSeed s;
			s.id = l.at("id").get<std::string>();
			s.severity = l.value("severity", std::string("warning"));
			s.rule = l.value("rule", std::string());
			s.source = l.value("source", std::string());
			seeds_[s.id] = s;
		}
	} catch (const std::exception &e) {
		err = path + ": unexpected contract shape: " + e.what();
		return false;
	}
	return true;
}

const CommandData *Grammar::command(const std::string &name) const
{
	auto it = commands_.find(name);
	return it == commands_.end() ? nullptr : &it->second;
}

const FamilyData *Grammar::family(const std::string &name) const
{
	auto it = families_.find(name);
	return it == families_.end() ? nullptr : &it->second;
}

const LintSeed *Grammar::seed(const std::string &id) const
{
	auto it = seeds_.find(id);
	return it == seeds_.end() ? nullptr : &it->second;
}

} // namespace scedit
