/*
 * scedit — sc_check.cpp
 *
 * Each rule below states the engine site it reports. Rules fire only on lines
 * the engine will actually execute: lines the script layer drops (comment,
 * blank) and lines inside a `comment` block are not analysed, because a finding
 * on text the engine never runs is a false positive by definition (C3).
 */

#include "sc_check.hpp"
#include "sc_tokenizer.hpp"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <map>
#include <sstream>

namespace scedit {

std::string Diagnostic::format() const
{
	// D6: file:line: severity: message [-Wid]
	std::ostringstream o;
	o << file << ':' << line << ": " << severity << ": " << message << " [-W" << id << ']';
	return o.str();
}

namespace {

//! Human label for a family key: "set_names" -> "set name".
std::string familyLabel(const std::string &key)
{
	std::string out;
	std::string word;
	auto flush = [&]() {
		if (word.size() > 2 && word.back() == 's' && word[word.size() - 2] != 's')
			word.pop_back();
		if (!out.empty())
			out += ' ';
		out += word;
		word.clear();
	};
	for (char c : key) {
		if (c == '_') flush();
		else word.push_back(c);
	}
	flush();
	return out;
}

std::string quoteName(const std::string &s) { return "'" + s + "'"; }

class LineChecker {
public:
	LineChecker(const Grammar &g, const std::string &file, std::size_t lineno,
	            std::vector<Diagnostic> &out)
		: g_(g), file_(file), lineno_(lineno), out_(out) {}

	void emit(const char *id, const std::string &message)
	{
		Diagnostic d;
		d.file = file_;
		d.line = lineno_;
		d.id = id;
		const LintSeed *s = g_.seed(id);
		d.severity = s ? s->severity : std::string("warning");
		d.message = message;
		out_.push_back(std::move(d));
	}

	void run(const Line &L);

private:
	//! Returns true when `name` is one the engine will not accept.
	bool checkSubfamilyName(const Line &L, const CommandData &cd, const FamilyData &fam,
	                        const std::string &name, const std::string &consequence);

	const Grammar &g_;
	std::string file_;
	std::size_t lineno_;
	std::vector<Diagnostic> &out_;
};

bool LineChecker::checkSubfamilyName(const Line &L, const CommandData &cd,
                                     const FamilyData &fam, const std::string &name,
                                     const std::string &consequence)
{
	(void)L;
	// A name the engine header spells but never registers: same failure path as
	// a typo, but a different story, so a different id (seed `orphaned-flag`
	// is written for the flags family; other families fall back).
	auto orph = fam.orphaned.find(name);
	if (orph != fam.orphaned.end()) {
		if (cd.subfamily == "flags") {
			emit("orphaned-flag",
			     "flag " + quoteName(name) + " is never registered by the engine, so this line "
			     "has no effect (the name exists in the engine source but no flag is bound to it)");
			return true;
		}
	} else if (fam.name_set.count(name)) {
		auto def = fam.known_defective.find(name);
		if (def != fam.known_defective.end())
			emit("inert-command",
			     quoteName(cd.name + " " + name) + " is accepted and reported as successful but "
			     "does nothing: " + def->second);
		return false;   // known name
	}
	// AppCommandInit::searchNeighbour returns before suggesting when the name is
	// on the obsolete list, and the engine logs it as retired instead.
	if (g_.isObsolete(name)) {
		emit("deprecated",
		     quoteName(name) + " is no longer used in this software: the engine ignores it" +
		     consequence);
		return true;
	}
	std::string msg = quoteName(name) + " is not a known " + familyLabel(cd.subfamily) + consequence;
	std::string near = nearestNeighbour(name, fam.sorted);
	if (!near.empty())
		msg += "; did you mean " + quoteName(near) + "?";
	emit("unknown-parameter", msg);
	return true;
}

void LineChecker::run(const Line &L)
{
	// --- indented '#': not a comment at the script layer (script.cpp:114) ----
	if (!L.raw.empty() && (L.raw[0] == ' ' || L.raw[0] == '\t')) {
		std::size_t i = 0;
		while (i < L.raw.size() && (L.raw[i] == ' ' || L.raw[i] == '\t'))
			++i;
		if (i < L.raw.size() && L.raw[i] == '#') {
			emit("indented-comment",
			     "a '#' comment must start in column 1; indented, this line reaches the "
			     "parser and is executed as an unknown command");
			return;   // the rest of the line is comment prose, not a command
		}
	}

	if (!L.has_command)
		return;   // whitespace-only line: executeCommand:207 returns without acting

	const CommandData *cd = g_.command(L.command);
	if (!cd) {
		if (g_.isObsolete(L.command)) {
			emit("deprecated",
			     "command " + quoteName(L.command) + " is no longer used in this software: "
			     "the line is ignored");
			return;
		}
		std::string msg = "unknown command " + quoteName(L.command);
		std::string near = nearestNeighbour(L.command, g_.commandLookupList());
		if (!near.empty())
			msg += "; did you mean " + quoteName(near) + "?";
		emit("unknown-command", msg);
		return;   // the key/value shape of a line the engine will not run is not news
	}

	// --- parse-level facts ---------------------------------------------------
	if (L.has_dangling)
		emit("dangling-key",
		     "key " + quoteName(L.tokens[L.dangling_index].text) + " has no value: the engine "
		     "drops it silently and runs the rest of the line without it");

	{
		std::map<std::string, int> seen;
		for (const auto &p : L.pairs)
			++seen[L.tokens[p.key].text];
		for (const auto &p : L.pairs) {
			const std::string &k = L.tokens[p.key].text;
			auto it = seen.find(k);
			if (it == seen.end() || it->second < 2)
				continue;
			const int count = it->second;
			seen.erase(it);   // report each duplicated key once
			auto a = L.args.find(k);
			emit("duplicate-key",
			     "key " + quoteName(k) + " is given " + std::to_string(count) + " times on this "
			     "line: the engine keeps only the last value" +
			     (a != L.args.end() ? " (" + quoteName(a->second) + ")" : std::string()));
		}
	}

	// --- quoting the engine does not have ------------------------------------
	{
		// A single-quoted group: the engine has no such thing, so the value
		// stops at the first space and the rest becomes key/value pairs.
		for (std::size_t i = 0; i < L.tokens.size(); ++i) {
			const std::string t = L.rawText(L.tokens[i].span);
			const bool opens = !t.empty() && t[0] == '\'' && !(t.size() >= 2 && t.back() == '\'');
			if (!opens)
				continue;
			bool closed_later = false;
			for (std::size_t j = i + 1; j < L.tokens.size(); ++j) {
				const std::string u = L.rawText(L.tokens[j].span);
				if (!u.empty() && u.back() == '\'') { closed_later = true; break; }
			}
			if (closed_later) {
				emit("unsupported-quoting",
				     "single quotes do not group words here: only \" does, so this value ends "
				     "at the first space and the remaining words are read as key/value pairs");
				break;
			}
		}
		if (L.raw.find("\\\"") != std::string::npos)
			emit("unsupported-quoting",
			     "backslash is not an escape here: the engine has no escape sequence, so the "
			     "value ends at this \" and the backslash is kept as an ordinary character");
	}

	// --- one pair per line commands ------------------------------------------
	if (g_.isSinglePairCommand(cd->name) && L.args.size() > 1)
		emit("single-pair-only",
		     "command " + quoteName(cd->name) + " applies a single key/value pair per line: "
		     "only " + quoteName(L.args.begin()->first) + " (first in alphabetical order, not "
		     "line order) is applied; the other " + std::to_string(L.args.size() - 1) +
		     " pair(s) are dropped silently");

	// --- the family this command draws its name from --------------------------
	const FamilyData *fam = cd->subfamily.empty() ? nullptr : g_.family(cd->subfamily);
	if (fam) {
		switch (cd->placement.pos) {
			case SubfamilyPosition::AppliedKey:
				if (!L.args.empty())
					checkSubfamilyName(L, *cd, *fam, L.args.begin()->first, "");
				break;
			case SubfamilyPosition::EveryKey: {
				// commandSet walks `args` and folds with `&&`
				// (app_command_interface.cpp:2119-2121): the FIRST pair that
				// fails makes every later call short-circuit away, so the pairs
				// after it are never applied at all. Say so once, on the pair
				// that causes it — "there is a typo" and "this line does
				// nothing" are different messages to the author.
				bool aborted = false;
				for (auto it = L.args.begin(); it != L.args.end(); ++it) {
					std::string consequence;
					if (!aborted) {
						std::string rest;
						for (auto j = std::next(it); j != L.args.end(); ++j) {
							if (!rest.empty()) rest += ", ";
							rest += quoteName(j->first);
						}
						if (!rest.empty())
							consequence = "; the engine stops here, so " + rest +
							              " on this line is never applied";
					}
					if (checkSubfamilyName(L, *cd, *fam, it->first, consequence))
						aborted = true;
				}
				break;
			}
			case SubfamilyPosition::ValueOfKey: {
				auto it = L.args.find(cd->placement.position_key);
				if (it != L.args.end() && !it->second.empty())
					checkSubfamilyName(L, *cd, *fam, it->second, "");
				break;
			}
			case SubfamilyPosition::Unknown:
				break;
		}
	}

	// --- flag values: everything that is not on/toggle silently means off ------
	if (fam && cd->subfamily == "flags" && cd->placement.pos == SubfamilyPosition::AppliedKey
	    && !L.args.empty()) {
		const std::string &name = L.args.begin()->first;
		const std::string &value = L.args.begin()->second;
		// setFlag returns before converting the value when the name is unknown.
		if (fam->name_set.count(name) && value != "toggle"
		    && !isTrueValue(value) && !isFalseValue(value))
			emit("silent-off-value",
			     "flag value " + quoteName(value) + " is not an on-form ('on', 'true', '1'), an "
			     "off-form ('off', 'false', '0') or 'toggle': the engine reads everything else "
			     "as OFF, with no error");
	}

	// --- argument vocabulary, armed by data presence alone ---------------------
	if (cd->has_args) {
		const bool name_is_key = fam &&
			(cd->placement.pos == SubfamilyPosition::AppliedKey ||
			 cd->placement.pos == SubfamilyPosition::EveryKey);
		for (const auto &p : L.pairs) {
			const std::string &k = L.tokens[p.key].text;
			if (cd->arg_keys.count(k))
				continue;
			if (name_is_key && fam->name_set.count(k))
				continue;   // the key IS the family name, not an argument
			std::string msg = quoteName(k) + " is not an argument of command " + quoteName(cd->name);
			std::string near = nearestNeighbour(k, cd->arg_keys_sorted);
			if (!near.empty())
				msg += "; did you mean " + quoteName(near) + "?";
			emit("unknown-parameter", msg);
		}
	}

	// --- spelling that does not survive a round trip ---------------------------
	if (!cd->alias_of.empty())
		emit("alias-respelled",
		     "command " + quoteName(cd->name) + " is another spelling of " +
		     quoteName(cd->alias_of) + ": a recorded session writes it back as " +
		     quoteName(cd->alias_of));
}

} // namespace

std::vector<Diagnostic> checkBuffer(const Grammar &g, const std::string &path,
                                    const std::string &bytes)
{
	std::vector<Diagnostic> out;
	BlockSkipState skip;
	const auto lines = splitScriptLines(bytes);
	for (std::size_t i = 0; i < lines.size(); ++i) {
		Line L = tokenizeLine(lines[i]);
		if (L.kind != LineKind::Parsed)
			continue;
		if (skip.feed(L))
			continue;   // inside a `comment` block: the engine never runs this
		LineChecker(g, path, i + 1, out).run(L);
	}
	return out;
}

std::vector<Diagnostic> checkFile(const Grammar &g, const std::string &path,
                                  std::string &io_error)
{
	std::ifstream in(path, std::ios::binary);
	if (!in) {
		io_error = "cannot open " + path;
		return {};
	}
	std::ostringstream ss;
	ss << in.rdbuf();
	return checkBuffer(g, path, ss.str());
}

std::vector<UnarmedRule> unarmedRules(const Grammar &g)
{
	std::vector<UnarmedRule> out;
	// unknown-parameter has two halves; the family half is armed, the
	// argument-key half waits for the extraction sweep to land `args` data.
	int with_args = 0, subfam_unarmed = 0;
	std::vector<std::string> unarmed_names;
	for (const auto &name : g.commandLookupList()) {
		const CommandData *cd = g.command(name);
		if (!cd)
			continue;
		if (cd->has_args)
			++with_args;
		if (!cd->subfamily.empty() && cd->placement.pos == SubfamilyPosition::Unknown) {
			++subfam_unarmed;
			unarmed_names.push_back(name);
		}
	}
	if (with_args == 0)
		out.push_back({"unknown-parameter",
		               "argument-key half dormant: no command entry carries `args` data yet; "
		               "it arms per command by data presence, with no code change"});
	if (subfam_unarmed) {
		std::string names;
		for (const auto &n : unarmed_names) {
			if (!names.empty()) names += ", ";
			names += n;
		}
		out.push_back({"unknown-parameter",
		               "family-name half unarmed for: " + names +
		               " (the contract names the family but not where the name sits in the line, "
		               "and the engine's acceptance test for it was not read)"});
	}
	return out;
}

} // namespace scedit
