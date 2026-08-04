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
 */

#include <cstdio>
#include <fstream>
#include <string>
#include <set>
#include <vector>
#include <nlohmann/json.hpp>

#include "sc_check.hpp"
#include "sc_grammar.hpp"

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

// Count entries and enforce uniqueness for a names-array family.
void checkNamesFamily(const json &family, const char *key, int expected) {
	const auto &names = family.at("names");
	std::set<std::string> uniq;
	for (const auto &n : names) uniq.insert(n.get<std::string>());
	check((int)names.size() == expected,
	      std::string(key) + ": count " + std::to_string(names.size()) + " == expected " + std::to_string(expected));
	check(uniq.size() == names.size(),
	      std::string(key) + ": all names unique");
}

int validate(const json &g) {
	const auto &meta = g.at("_meta");
	const auto &exp = meta.at("expected_counts");
	const auto &fam = g.at("families");

	// commands: object entries; those carrying "registration" are pre-table
	// literals (comment/uncomment) and sit OUTSIDE the m_commands count.
	int registered = 0, pretable = 0;
	std::set<std::string> cmdNames;
	for (auto it = fam.at("commands").begin(); it != fam.at("commands").end(); ++it) {
		if (it.key().rfind("_", 0) == 0) continue; // _source/_args_status annotations
		cmdNames.insert(it.key());
		if (it.value().contains("registration")) ++pretable; else ++registered;
	}
	check(registered == exp.at("commands").get<int>(),
	      "commands: registered " + std::to_string(registered) + " == expected " + std::to_string(exp.at("commands").get<int>()));
	check(pretable == 2, "commands: pre-table literals == 2 (comment, uncomment)");

	checkNamesFamily(fam.at("flags"), "flags", exp.at("flags").get<int>());
	checkNamesFamily(fam.at("set_names"), "set_names", exp.at("set_names").get<int>());
	checkNamesFamily(fam.at("color_names"), "color_names", exp.at("color_names").get<int>());
	checkNamesFamily(fam.at("obsolete_tokens"), "obsolete_tokens", exp.at("obsolete_tokens").get<int>());
	checkNamesFamily(fam.at("reserved_variables"), "reserved_variables", exp.at("reserved_variables").get<int>());
	checkNamesFamily(fam.at("font_targets"), "font_targets", exp.at("font_targets").get<int>());

	// every command naming a subfamily must name one that exists
	for (auto it = fam.at("commands").begin(); it != fam.at("commands").end(); ++it) {
		if (it.key().rfind("_", 0) == 0) continue;
		if (it.value().contains("subfamily"))
			check(fam.contains(it.value().at("subfamily").get<std::string>()),
			      "subfamily of '" + it.key() + "' exists");
	}

	// lint seeds: ids unique
	std::set<std::string> lintIds;
	for (const auto &l : g.at("lint_seeds")) lintIds.insert(l.at("id").get<std::string>());
	check(lintIds.size() == g.at("lint_seeds").size(), "lint_seeds: ids unique");

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
	for (const auto &n : fam.at(name).at("names"))
		std::printf("%s\n", n.get<std::string>().c_str());
}

void usage() {
	std::fprintf(stderr,
	             "usage: scedit [--grammar <file>] [--list commands|flags|set_names|color_names|obsolete_tokens|reserved_variables|font_targets]\n"
	             "       scedit [--grammar <file>] [--rules] --check FILE...\n"
	             "default action: validate the grammar contract\n"
	             "exit: 0 clean, 1 findings, 2 usage or I/O error\n");
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

} // namespace

int main(int argc, char **argv) {
	std::string grammarPath = "grammar/sc-grammar.json";
	std::string list;
	std::vector<std::string> checkFiles;
	bool checkMode = false, showRules = false;
	for (int i = 1; i < argc; ++i) {
		std::string a = argv[i];
		if (checkMode) { checkFiles.push_back(a); continue; }
		if (a == "--grammar" && i + 1 < argc) grammarPath = argv[++i];
		else if (a == "--list" && i + 1 < argc) list = argv[++i];
		else if (a == "--rules") showRules = true;
		else if (a == "--check") checkMode = true;
		else {
			usage();
			return 2;
		}
	}

	if (checkMode) {
		if (checkFiles.empty()) { usage(); return 2; }
		return check(grammarPath, checkFiles, showRules);
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
	return validate(g);
}
