/*
 * scedit — spacecrafter script/system-file editor, headless core.
 *
 * Slice 1 (2026-08-03): grammar-contract loader + self-validation.
 * The grammar file (grammar/sc-grammar.json) is the single machine-readable
 * authority for the command surface consumed by this tool; until the engine
 * emits it, SOURCE WINS on divergence (see the file's _meta.authority_chain).
 *
 * This binary's validate mode is the SEED GATE: it re-derives every family
 * count from the data and compares against _meta.expected_counts (guards
 * accidental edits), and checks in-family uniqueness. Engine-vs-file
 * validation is a separate concern (future emitter / extraction passes).
 *
 * Slice 2 (2026-08-04): `--check FILE...` — static analysis of scripts, read
 * exactly the way the engine reads them (src/sc_tokenizer.hpp), reported
 * gcc-shaped per D6 (src/sc_check.hpp). Exit 0 clean / 1 findings / 2 usage or
 * I/O error.
 *
 * Slice 3 (2026-08-04): the editor — `scedit FILE` / `scedit --edit FILE`.
 * The interaction lives in src/sc_editcore.hpp (headless, tested without a
 * tty); src/sc_tui.hpp only draws it. `--ui-selftest` renders fixed frames
 * off-screen so a gate can assert what actually reaches the screen.
 */

#include <cstdio>
#include <fstream>
#include <string>
#include <set>
#include <vector>
#include <unistd.h>
#include <nlohmann/json.hpp>

#include "sc_check.hpp"
#include "sc_editcore.hpp"
#include "sc_grammar.hpp"
#include "sc_tui.hpp"

using json = nlohmann::json;

namespace {

int failures = 0;

void check(bool ok, const std::string &what) {
	if (ok) {
		std::printf("  ok      %s\n", what.c_str());
	} else {
		std::printf("  FAIL    %s\n", what.c_str());
		++failures;
	}
}

// A family member is either a plain string (v1 shape) or an object with a
// `name` field (D7 v2 shape). Both shapes coexist on purpose: a family converts
// when its doc pass fills content, one family at a time.
bool memberName(const json &entry, std::string &out) {
	if (entry.is_string()) { out = entry.get<std::string>(); return true; }
	if (entry.is_object() && entry.contains("name") && entry.at("name").is_string()) {
		out = entry.at("name").get<std::string>();
		return true;
	}
	return false;
}

// Count entries and enforce uniqueness for a names family, in either shape.
void checkNamesFamily(const json &family, const char *key, int expected) {
	const auto &names = family.at("names");
	std::set<std::string> uniq;
	int shaped = 0;
	for (const auto &n : names) {
		std::string name;
		if (memberName(n, name)) { uniq.insert(name); ++shaped; }
	}
	check((int)names.size() == expected,
	      std::string(key) + ": count " + std::to_string(names.size()) + " == expected " + std::to_string(expected));
	check(shaped == (int)names.size(),
	      std::string(key) + ": every entry is a name or carries one");
	check(uniq.size() == names.size(),
	      std::string(key) + ": all names unique");
}

// A v2 family entry that carries content must carry the SIX per-key facts the
// extraction was gated on (SWEEP_DISPATCH DoD: doc, value, default, required,
// source; `values` only when enumerated). A null doc is legal and means
// "flagged, not invented" (C2) — an ABSENT one is not.
void checkNamesFamilyV2Content(const json &family, const char *key) {
	int objects = 0, complete = 0;
	for (const auto &n : family.at("names")) {
		if (!n.is_object()) continue;
		++objects;
		if (n.contains("doc") && n.contains("value") && n.contains("default")
		    && n.contains("required") && n.contains("source"))
			++complete;
	}
	if (objects == 0) return;   // still v1 shape: nothing to check
	check(complete == objects,
	      std::string(key) + ": " + std::to_string(objects) +
	      " object entries, all carrying doc/value/default/required/source");
}

// THE MERGE'S OWN I2 EXPOSURE, CLOSED BY MEASUREMENT.
// The four extraction fragments (grammar/args/unit-N.json) stay in the tree as
// the granular source, and their content is also in the merged contract: two
// copies of the same facts, which is a pending silent desync unless something
// checks. This does. Every per-key and per-command fact in a fragment must be
// byte-identical in the merged file, `set` included (whose keys live in
// families.set_names in the D7 v2 shape, so they are compared there).
// Fragments absent (a consumer who took only the merged file) = skipped, said
// out loud, not silently passed.
void checkFragments(const json &g, const std::string &grammarPath) {
	const std::size_t slash = grammarPath.find_last_of('/');
	const std::string dir = slash == std::string::npos ? std::string(".") : grammarPath.substr(0, slash);
	static const char *kFields[] = {"handler", "lines", "aliases", "registration", "doc",
	                                "key_grammar", "branches", "exclusive_groups", "notes",
	                                "flagged", "signature", "handler_body_lines", "wait_parameter"};
	const auto &cmds = g.at("families").at("commands");

	// set's keys, rebuilt from the family, to compare against the fragment's
	json setArgs = json::object();
	for (const auto &n : g.at("families").at("set_names").at("names")) {
		if (!n.is_object() || !n.contains("name")) continue;
		json spec = n;
		spec.erase("name");
		setArgs[n.at("name").get<std::string>()] = spec;
	}

	int units = 0, entries = 0;
	std::vector<std::string> diffs;
	for (int u = 1; u <= 4; ++u) {
		const std::string p = dir + "/args/unit-" + std::to_string(u) + ".json";
		std::ifstream in(p);
		if (!in) continue;
		json f;
		try { in >> f; } catch (const std::exception &e) {
			diffs.push_back(std::string("unit-") + std::to_string(u) + ": " + e.what());
			continue;
		}
		++units;
		for (auto it = f.at("commands").begin(); it != f.at("commands").end(); ++it) {
			++entries;
			const std::string &name = it.key();
			if (!cmds.contains(name)) { diffs.push_back(name + ": absent from the merged file"); continue; }
			const auto &m = cmds.at(name);
			for (const char *k : kFields)
				if (it.value().contains(k) && (!m.contains(k) || m.at(k) != it.value().at(k)))
					diffs.push_back(name + "." + k);
			const json &fragArgs = it.value().at("args");
			const json &mergedArgs = name == "set" ? setArgs : m.at("args");
			if (mergedArgs != fragArgs)
				diffs.push_back(name + ".args");
		}
	}
	if (units == 0) {
		std::printf("  skip    fragments: grammar/args/unit-*.json not present next to the contract\n");
		return;
	}
	check(units == 4, "fragments: all 4 unit files present (found " + std::to_string(units) + ")");
	check(entries == 62, "fragments: 62 command entries (found " + std::to_string(entries) + ")");
	check(diffs.empty(), "fragments: every fact identical in the merged file" +
	      (diffs.empty() ? std::string() : " (differs: " + diffs.front() +
	       (diffs.size() > 1 ? " and " + std::to_string(diffs.size() - 1) + " more" : "") + ")"));
}

int validate(const json &g, const std::string &grammarPath) {
	const auto &meta = g.at("_meta");
	const auto &exp = meta.at("expected_counts");
	const auto &fam = g.at("families");

	// commands: object entries; those carrying `pretable: true` are the literals
	// compared BEFORE the m_commands lookup (comment/uncomment) and sit OUTSIDE
	// the m_commands count. (Until the args merge this was inferred from the
	// presence of a "registration" field; that field is now the registration
	// SOURCE ANCHOR carried by every command, so the marker had to become
	// explicit data rather than a shape accident.)
	int registered = 0, pretable = 0, withArgs = 0, argsIncomplete = 0, argsSourced = 0, aliases = 0;
	std::set<std::string> cmdNames;
	std::vector<std::string> missingArgsComplete, missingDoc, missingRegistration, badAliases;
	for (auto it = fam.at("commands").begin(); it != fam.at("commands").end(); ++it) {
		if (it.key().rfind("_", 0) == 0) continue; // _source/_args_status annotations
		cmdNames.insert(it.key());
		const auto &e = it.value();
		if (e.value("pretable", false)) ++pretable; else ++registered;
		// An alias names a canonical command and carries no argument data of
		// its own (resolved at load by both readers); if it states
		// args_complete at all, it must agree with its target.
		const bool isAlias = e.contains("alias_of");
		if (isAlias) {
			++aliases;
			const std::string t = e.at("alias_of").is_string() ? e.at("alias_of").get<std::string>() : std::string();
			const auto &all = fam.at("commands");
			if (t.empty() || !all.contains(t))
				badAliases.push_back(it.key() + " -> `" + t + "` (no such command)");
			else if (all.at(t).contains("alias_of"))
				badAliases.push_back(it.key() + " -> `" + t + "` (itself an alias)");
			else if (e.contains("args_complete") && all.at(t).value("args_complete", true) != e.at("args_complete"))
				badAliases.push_back(it.key() + ": states args_complete differently from `" + t + "`");
			if (e.contains("args") && e.at("args").is_object())
				for (auto a = e.at("args").begin(); a != e.at("args").end(); ++a)
					if (a.key().rfind("_", 0) != 0) { badAliases.push_back(it.key() + ": carries its own argument keys"); break; }
		}
		if (!e.contains("args_complete") || !e.at("args_complete").is_boolean()) {
			if (!isAlias) missingArgsComplete.push_back(it.key());
		} else if (!e.at("args_complete").get<bool>())
			++argsIncomplete;
		if (!e.contains("doc")) missingDoc.push_back(it.key());
		if (!e.contains("registration")) missingRegistration.push_back(it.key());
		if (e.contains("args") && e.at("args").is_object()) {
			int keys = 0;   // `_`-prefixed entries are annotations, not keys
			for (auto a = e.at("args").begin(); a != e.at("args").end(); ++a)
				if (a.key().rfind("_", 0) != 0) ++keys;
			if (keys) ++withArgs;
		}
		if (e.contains("args_source")) ++argsSourced;
	}
	check(registered == exp.at("commands").get<int>(),
	      "commands: registered " + std::to_string(registered) + " == expected " + std::to_string(exp.at("commands").get<int>()));
	check(pretable == 2, "commands: pre-table literals == 2 (comment, uncomment)");
	check(badAliases.empty(), "commands: " + std::to_string(aliases) + " alias entries, each naming a canonical "
	      "command and carrying no argument data of its own" +
	      (badAliases.empty() ? std::string() : " (" + badAliases.front() + ")"));
	check(missingRegistration.empty(),
	      "commands: every entry carries a `registration` source anchor" +
	      (missingRegistration.empty() ? std::string() : " (missing: " + missingRegistration.front() + ", ...)"));
	check(missingDoc.empty(),
	      "commands: every entry carries a `doc` slot (C6)" +
	      (missingDoc.empty() ? std::string() : " (missing: " + missingDoc.front() + ", ...)"));
	// `args_complete` is not optional data: "I listed some keys" and "I listed
	// all of them" are different claims, and a consumer that cannot tell which
	// one an entry makes has to choose between silence and false accusations.
	check(missingArgsComplete.empty(),
	      "commands: every entry answers `args_complete`" +
	      (missingArgsComplete.empty() ? std::string() : " (missing: " + missingArgsComplete.front() + ", ...)"));
	std::printf("  note    commands with extracted args: %d; args_complete:false: %d; keys sourced from a family: %d\n",
	            withArgs, argsIncomplete, argsSourced);

	checkNamesFamily(fam.at("flags"), "flags", exp.at("flags").get<int>());
	checkNamesFamily(fam.at("set_names"), "set_names", exp.at("set_names").get<int>());
	checkNamesFamilyV2Content(fam.at("set_names"), "set_names");
	checkNamesFamily(fam.at("color_names"), "color_names", exp.at("color_names").get<int>());
	checkNamesFamily(fam.at("obsolete_tokens"), "obsolete_tokens", exp.at("obsolete_tokens").get<int>());
	checkNamesFamily(fam.at("reserved_variables"), "reserved_variables", exp.at("reserved_variables").get<int>());
	checkNamesFamily(fam.at("font_targets"), "font_targets", exp.at("font_targets").get<int>());

	// every command naming a subfamily must name one that exists; and a command
	// sourcing its keys from a family must name one that exists too.
	for (auto it = fam.at("commands").begin(); it != fam.at("commands").end(); ++it) {
		if (it.key().rfind("_", 0) == 0) continue;
		if (it.value().contains("subfamily"))
			check(fam.contains(it.value().at("subfamily").get<std::string>()),
			      "subfamily of '" + it.key() + "' exists");
		if (it.value().contains("args_source")) {
			const std::string s = it.value().at("args_source").get<std::string>();
			const std::string want = "families.";
			bool named = false;
			if (s.rfind(want, 0) == 0) {
				const std::size_t e = s.find_first_not_of(
					"abcdefghijklmnopqrstuvwxyz_", want.size());
				named = fam.contains(s.substr(want.size(), e - want.size()));
			}
			check(named, "args_source of '" + it.key() + "' names an existing family");
		}
	}

	// argument token vocabulary: one entry per spelling, count re-derived
	{
		const auto &atv = g.at("argument_token_vocabulary");
		const auto &toks = atv.at("tokens");
		int roled = 0;
		std::set<std::string> roles;
		for (auto it = toks.begin(); it != toks.end(); ++it) {
			if (it.key().rfind("_", 0) == 0) continue;
			if (it.value().contains("role")) { ++roled; roles.insert(it.value().at("role").get<std::string>()); }
		}
		check((int)toks.size() == atv.at("_expected_count").get<int>(),
		      "argument tokens: count " + std::to_string(toks.size()) + " == expected " +
		      std::to_string(atv.at("_expected_count").get<int>()));
		check(roled == (int)toks.size(), "argument tokens: every spelling carries a role");
		std::set<std::string> legal = {"key", "value", "both", "engine_internal", "unreferenced"};
		bool ok = true;
		for (const auto &r : roles) if (!legal.count(r)) ok = false;
		check(ok, "argument tokens: every role is one of key|value|both|engine_internal|unreferenced");
	}

	// lint seeds: ids unique
	std::set<std::string> lintIds;
	for (const auto &l : g.at("lint_seeds")) lintIds.insert(l.at("id").get<std::string>());
	check(lintIds.size() == g.at("lint_seeds").size(), "lint_seeds: ids unique");

	checkFragments(g, grammarPath);

	std::printf("%s\n", failures ? "GRAMMAR INVALID" : "grammar self-consistent");
	return failures ? 1 : 0;
}

void listFamily(const json &g, const std::string &name) {
	const auto &fam = g.at("families");
	if (name == "commands") {
		for (auto it = fam.at("commands").begin(); it != fam.at("commands").end(); ++it)
			if (it.key().rfind("_", 0) != 0) std::printf("%s\n", it.key().c_str());
		return;
	}
	for (const auto &n : fam.at(name).at("names")) {
		std::string member;
		if (memberName(n, member)) std::printf("%s\n", member.c_str());
	}
}

void usage() {
	std::fprintf(stderr,
	             "usage: scedit [--grammar <file>] [--list commands|flags|set_names|color_names|obsolete_tokens|reserved_variables|font_targets]\n"
	             "       scedit [--grammar <file>] [--rules] --check FILE...\n"
	             "       scedit [--grammar <file>] --history FILE...\n"
	             "       scedit [--grammar <file>] [--edit] FILE\n"
	             "       scedit [--grammar <file>] --ui-selftest\n"
	             "default action: validate the grammar contract\n"
	             "exit: 0 clean, 1 findings, 2 usage or I/O error\n");
}

//! The contract file sits next to the tool in the source tree, and the editor
//! is launched from wherever the author's scripts are. So: if the DEFAULT path
//! does not resolve against the working directory, look beside the binary
//! before giving up. An explicit `--grammar` is never second-guessed — a path
//! the user named and that does not exist is an error, not a hint.
std::string resolveDefaultGrammar(const std::string &fallbackRelative) {
	std::ifstream in(fallbackRelative);
	if (in)
		return fallbackRelative;
	char buf[4096];
	const ssize_t n = ::readlink("/proc/self/exe", buf, sizeof(buf) - 1);
	if (n <= 0)
		return fallbackRelative;
	buf[n] = 0;
	std::string exe(buf);
	const std::size_t slash = exe.find_last_of('/');
	if (slash == std::string::npos)
		return fallbackRelative;
	const std::string dir = exe.substr(0, slash);
	for (const std::string &cand : {dir + "/" + fallbackRelative,
	                                dir + "/../" + fallbackRelative,
	                                dir + "/../../" + fallbackRelative}) {
		std::ifstream t(cand);
		if (t)
			return cand;
	}
	return fallbackRelative;
}

//! `--check`: report, for every line, where the engine's reading will differ
//! from what the author plainly meant. Diagnostics go to stdout (they are the
//! product); tool failures go to stderr.
int check(const std::string &grammarPath, const std::vector<std::string> &files, bool showRules) {
	scedit::Grammar g;
	std::string err;
	if (!g.load(grammarPath, err)) {
		std::fprintf(stderr, "scedit: %s\n", err.c_str());
		return 2;
	}
	if (showRules) {
		for (const auto &u : scedit::unarmedRules(g))
			std::printf("unarmed: %s: %s\n", u.id.c_str(), u.reason.c_str());
	}
	int findings = 0, ioErrors = 0;
	for (const auto &f : files) {
		std::string ioErr;
		auto diags = scedit::checkFile(g, f, ioErr);
		if (!ioErr.empty()) {
			std::fprintf(stderr, "scedit: %s\n", ioErr.c_str());
			++ioErrors;
			continue;
		}
		for (const auto &d : diags)
			std::printf("%s\n", d.format().c_str());
		findings += (int)diags.size();
	}
	if (ioErrors) return 2;
	return findings ? 1 : 0;
}

//! `--history`: the list the editor's error pane shows, for a caller with no
//! tty. Same reader as the pane (EditCore::errorHistory) — there is one
//! implementation of "where are the problems in this file", and this prints it.
//!
//! SHAPE, stable and stated in README § --history: one entry per line, seven
//! TAB-separated fields —
//!     file  line  source  id  severity  message  relation
//! `source` is `spacecrafter` or `scedit`; `id` is the lint id, or the literal
//! `#!` for an engine tail; `severity` is empty for an engine tail (the engine
//! states none); `relation` is empty for a scedit finding. A field never
//! contains a tab: the two prose fields are written with tabs turned into
//! spaces, so the shape cannot be broken by a message.
std::string oneLine(std::string s)
{
	for (char &c : s)
		if (c == '\t' || c == '\n' || c == '\r')
			c = ' ';
	return s;
}

int history(const std::string &grammarPath, const std::vector<std::string> &files)
{
	int entries = 0, ioErrors = 0;
	for (const auto &f : files) {
		scedit::EditCore core;
		std::string err;
		if (!core.open(grammarPath, f, err)) {
			std::fprintf(stderr, "scedit: %s\n", err.c_str());
			++ioErrors;
			continue;
		}
		for (const auto &e : core.errorHistory()) {
			std::printf("%s\t%zu\t%s\t%s\t%s\t%s\t%s\n",
			            f.c_str(), e.line,
			            e.source == scedit::EntrySource::Engine ? "spacecrafter" : "scedit",
			            e.id.c_str(), e.severity.c_str(),
			            oneLine(e.message).c_str(), oneLine(e.relation).c_str());
			++entries;
		}
	}
	if (ioErrors) return 2;
	return entries ? 1 : 0;
}

} // namespace

int main(int argc, char **argv) {
	std::string grammarPath = "grammar/sc-grammar.json";
	bool grammarGiven = false;
	std::string list;
	std::string editFile;
	std::vector<std::string> checkFiles;
	bool checkMode = false, showRules = false, editMode = false, uiSelfTest = false;
	bool historyMode = false;
	for (int i = 1; i < argc; ++i) {
		std::string a = argv[i];
		if (checkMode || historyMode) { checkFiles.push_back(a); continue; }
		if (a == "--grammar" && i + 1 < argc) { grammarPath = argv[++i]; grammarGiven = true; }
		else if (a == "--list" && i + 1 < argc) list = argv[++i];
		else if (a == "--rules") showRules = true;
		else if (a == "--history") historyMode = true;
		else if (a == "--check") checkMode = true;
		else if (a == "--ui-selftest") uiSelfTest = true;
		else if (a == "--edit" && i + 1 < argc) { editMode = true; editFile = argv[++i]; }
		else if (!a.empty() && a[0] != '-' && editFile.empty()) { editMode = true; editFile = a; }
		else {
			usage();
			return 2;
		}
	}
	if (!grammarGiven)
		grammarPath = resolveDefaultGrammar(grammarPath);

	if (uiSelfTest) return scedit::uiSelfTest(grammarPath);
	if (editMode) return scedit::runEditor(grammarPath, editFile);

	if (checkMode) {
		if (checkFiles.empty()) { usage(); return 2; }
		return check(grammarPath, checkFiles, showRules);
	}
	if (historyMode) {
		if (checkFiles.empty() || showRules) { usage(); return 2; }
		return history(grammarPath, checkFiles);
	}
	if (showRules) { usage(); return 2; }

	std::ifstream in(grammarPath);
	if (!in) {
		std::fprintf(stderr, "scedit: cannot open %s\n", grammarPath.c_str());
		return 2;
	}
	json g;
	try {
		in >> g;
	} catch (const std::exception &e) {
		std::fprintf(stderr, "scedit: %s: %s\n", grammarPath.c_str(), e.what());
		return 2;
	}

	if (!list.empty()) {
		listFamily(g, list);
		return 0;
	}
	std::printf("scedit grammar: %s\n", grammarPath.c_str());
	return validate(g, grammarPath);
}
