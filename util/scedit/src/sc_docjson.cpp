/*
 * scedit -- sc_docjson.cpp
 *
 * Serialisation only: every fact printed here was read by Grammar or DocIndex
 * from grammar/sc-grammar.json, and this file adds nothing to it except the
 * ranking arithmetic documented in the header.
 */

#include "sc_docjson.hpp"

#include <algorithm>
#include <cmath>
#include <set>

namespace scedit {

using nlohmann::json;

namespace {

//! `doc` and only `doc`: known -> the file's sentence, unknown -> JSON null.
json docOrNull(bool known, const std::string &doc)
{
	return known ? json(doc) : json(nullptr);
}

json emptyOrNull(const std::string &s)
{
	return s.empty() ? json(nullptr) : json(s);
}

//! Everything the file says about one named thing.
json specJson(const Spec &s)
{
	json j;
	j["present"] = s.present;
	j["doc"] = docOrNull(s.doc_known, s.doc);
	j["value_domain"] = s.value_domain;
	j["values"] = s.values;
	j["completable"] = s.completable;
	j["value_docs"] = s.value_docs;
	j["default"] = s.default_prose;
	j["default_literal"] = s.has_default_literal ? json(s.default_literal) : json(nullptr);
	j["required"] = s.required;
	j["source"] = s.source;
	j["notes"] = s.notes;
	return j;
}

//! The family a command draws its key vocabulary from, or "".
std::string familyOf(const Grammar &g, const std::string &command)
{
	const CommandData *cd = g.command(command);
	return cd ? cd->subfamily : std::string();
}

json memberList(const Grammar &g, const DocIndex &d, const std::string &family)
{
	json out = json::array();
	const FamilyData *fd = g.family(family);
	if (!fd)
		return out;
	for (const std::string &n : fd->names) {   // the file's order
		const Spec s = d.familyMember(family, n);
		json m;
		m["name"] = n;
		m["doc"] = docOrNull(s.doc_known, s.doc);
		out.push_back(m);
	}
	return out;
}

json commandJson(const Grammar &g, const DocIndex &d, const CommandInfo &ci)
{
	json j;
	j["kind"] = "command";
	j["name"] = ci.name;
	j["doc"] = docOrNull(ci.doc_known, ci.doc);
	j["registration"] = ci.registration;
	j["alias_of"] = emptyOrNull(ci.alias_of);
	j["args_complete"] = ci.args_complete;
	j["args_source"] = ci.args_source;
	j["keys"] = ci.keys;
	json args = json::object();
	for (const auto &kv : ci.args)
		args[kv.first] = specJson(kv.second);
	j["args"] = args;
	j["key_grammar"] = ci.has_key_grammar ? specJson(ci.key_grammar) : json(nullptr);
	const std::string fam = familyOf(g, ci.name);
	j["family"] = emptyOrNull(fam);
	if (!fam.empty())
		j["members"] = memberList(g, d, fam);
	return j;
}

//! An error a machine can act on: what was not found, in which vocabulary, and
//! the nearest name the CHECKER would have suggested for the same misspelling
//! (one did-you-mean policy for the whole tool -- sc_check.hpp).
json notFound(const std::string &kind, const std::string &message,
              const std::string &vocabulary, const std::string &token,
              const std::vector<std::string> &candidates)
{
	json j;
	j["error"] = kind;
	j["message"] = message;
	j["vocabulary"] = vocabulary;
	const std::string near = cappedSuggestion(token, candidates);
	j["did_you_mean"] = emptyOrNull(near);
	return j;
}

// ---------------------------------------------------------------- the ranking

//! words(): lowercase, cut on anything outside [a-z_], keep runs of 3+ that
//! START with a letter. This is a byte-for-byte port of the baseline's
//! `re.findall(r"[a-z][a-z_]+", s.lower())` filtered by `len(w) > 2`; bytes
//! outside ASCII cut a word exactly as they do there, because the pattern
//! cannot match them either.
std::set<std::string> wordsOf(const std::string &s)
{
	std::set<std::string> out;
	std::string cur;
	auto flush = [&]() {
		if (cur.size() > 2)
			out.insert(cur);
		cur.clear();
	};
	for (unsigned char raw : s) {
		const unsigned char c = (raw >= 'A' && raw <= 'Z') ? (unsigned char)(raw + 32) : raw;
		const bool letter = c >= 'a' && c <= 'z';
		if (cur.empty()) {
			if (letter)
				cur.push_back((char)c);
		} else if (letter || c == '_') {
			cur.push_back((char)c);
		} else {
			flush();
		}
	}
	flush();
	return out;
}

std::string spaced(const std::string &name)
{
	std::string out = name;
	for (char &c : out)
		if (c == '_')
			c = ' ';
	return out;
}

struct Page {
	std::string kind;      //!< command | key | member
	std::string command;
	std::string name;      //!< "" for a command page
	std::string family;    //!< member pages only
	bool doc_known = false;
	std::string doc;
	std::set<std::string> words;
	double score = 0.0;
};

Page makePage(const std::string &kind, const std::string &command, const std::string &name,
              const std::string &family, bool doc_known, const std::string &doc)
{
	Page p;
	p.kind = kind;
	p.command = command;
	p.name = name;
	p.family = family;
	p.doc_known = doc_known;
	p.doc = doc;
	p.words = wordsOf(spaced(name.empty() ? command : name) + " " + doc);
	return p;
}

//! Enumeration order IS the tie-break: the contract file's command order, and
//! within a command its keys then its family's names.
std::vector<Page> enumeratePages(const Grammar &g, const DocIndex &d, SearchScope scope)
{
	std::vector<Page> pages;
	for (const std::string &name : d.commandFileOrder()) {
		const CommandInfo *ci = d.command(name);
		if (!ci)
			continue;
		pages.push_back(makePage("command", name, "", "", ci->doc_known, ci->doc));
		if (scope != SearchScope::All)
			continue;
		for (const auto &kv : ci->args)
			pages.push_back(makePage("key", name, kv.first, "", kv.second.doc_known, kv.second.doc));
		const std::string fam = familyOf(g, name);
		const FamilyData *fd = fam.empty() ? nullptr : g.family(fam);
		if (!fd)
			continue;
		for (const std::string &m : fd->names) {
			const Spec s = d.familyMember(fam, m);
			pages.push_back(makePage("member", name, m, fam, s.doc_known, s.doc));
		}
	}
	return pages;
}

//! Six decimals, so a recorded gate compares a number and not a float's tail.
//! The RANKING uses the full-precision value; this rounds the printed one only.
double rounded(double v)
{
	return std::round(v * 1e6) / 1e6;
}

} // namespace

DocAnswer docLookup(const Grammar &g, const DocIndex &d,
                    const std::string &command, const std::string &name)
{
	DocAnswer a;
	const CommandInfo *ci = d.command(command);
	if (!ci) {
		a.value = notFound("unknown-command",
		                   "no command named '" + command + "' in the grammar contract",
		                   "families.commands (" + std::to_string(d.commandNames().size()) +
		                   " names the engine accepts)",
		                   command, g.commandLookupList());
		return a;
	}
	if (name.empty()) {
		a.found = true;
		a.value = commandJson(g, d, *ci);
		return a;
	}

	auto arg = ci->args.find(name);
	if (arg != ci->args.end()) {
		json j = specJson(arg->second);
		j["kind"] = "key";
		j["command"] = command;
		j["name"] = name;
		a.found = true;
		a.value = j;
		return a;
	}

	const std::string fam = familyOf(g, command);
	if (!fam.empty()) {
		const Spec s = d.familyMember(fam, name);
		if (s.present) {
			json j = specJson(s);
			j["kind"] = "member";
			j["command"] = command;
			j["family"] = fam;
			j["name"] = name;
			a.found = true;
			a.value = j;
			return a;
		}
	}

	// Not a key, not a family name: say which vocabulary was searched, and
	// whether that vocabulary is even claimed to be complete (args_complete).
	std::vector<std::string> candidates;
	std::string vocabulary;
	if (!fam.empty()) {
		const FamilyData *fd = g.family(fam);
		if (fd)
			candidates = fd->sorted;
		vocabulary = "families." + fam + " (" + std::to_string(candidates.size()) +
		             " names), the vocabulary '" + command + "' draws its keys from";
	} else {
		candidates = ci->keys;
		vocabulary = "the argument keys of '" + command + "' (" +
		             std::to_string(candidates.size()) + " extracted" +
		             (ci->args_complete ? ", the file claims the list is complete"
		                                : ", the file states the list is INCOMPLETE \xe2\x80\x94 an "
		                                  "unlisted key may still be legal") + ")";
	}
	a.value = notFound("unknown-name",
	                   "'" + command + "' has no key or family name '" + name + "'",
	                   vocabulary, name, candidates);
	a.value["command"] = command;
	return a;
}

nlohmann::json docCatalogue(const Grammar &g, const DocIndex &d)
{
	json out;
	out["kind"] = "catalogue";
	json cmds = json::array();
	for (const std::string &name : d.commandFileOrder()) {
		const CommandInfo *ci = d.command(name);
		if (!ci)
			continue;
		json c;
		c["name"] = name;
		c["doc"] = docOrNull(ci->doc_known, ci->doc);
		c["alias_of"] = emptyOrNull(ci->alias_of);
		const std::string fam = familyOf(g, name);
		c["family"] = emptyOrNull(fam);
		if (!fam.empty())
			c["members"] = memberList(g, d, fam);
		cmds.push_back(c);
	}
	out["commands"] = cmds;
	return out;
}

nlohmann::json docSearch(const Grammar &g, const DocIndex &d,
                         const std::string &query, SearchScope scope, std::size_t limit)
{
	std::vector<Page> pages = enumeratePages(g, d, scope);
	const std::set<std::string> qw = wordsOf(query);
	std::vector<const Page *> hits;
	for (Page &p : pages) {
		std::size_t common = 0;
		for (const std::string &w : qw)
			if (p.words.count(w))
				++common;
		if (common == 0)
			continue;   // no shared word is not a weak answer, it is no answer
		p.score = (double)common / (1.0 + std::sqrt((double)p.words.size()));
		hits.push_back(&p);
	}
	std::stable_sort(hits.begin(), hits.end(),
	                 [](const Page *a, const Page *b) { return a->score > b->score; });

	json out;
	out["kind"] = "search";
	out["query"] = query;
	out["scope"] = scope == SearchScope::All ? "all" : "commands";
	json results = json::array();
	for (const Page *p : hits) {
		if (limit && results.size() >= limit)
			break;
		json r;
		r["kind"] = p->kind;
		r["command"] = p->command;
		r["name"] = emptyOrNull(p->name);
		r["page"] = p->name.empty() ? p->command : p->command + " " + p->name;
		r["family"] = emptyOrNull(p->family);
		r["doc"] = docOrNull(p->doc_known, p->doc);
		r["score"] = rounded(p->score);
		results.push_back(r);
	}
	out["results"] = results;
	out["total_matches"] = hits.size();
	return out;
}

nlohmann::json diagnosticsJson(const std::vector<std::string> &files,
                               const std::vector<Diagnostic> &diags)
{
	json out;
	out["kind"] = "check";
	out["files"] = files;
	json list = json::array();
	std::size_t errors = 0, warnings = 0, infos = 0;
	for (const Diagnostic &dg : diags) {
		json j;
		j["file"] = dg.file;
		j["line"] = dg.line;
		j["severity"] = dg.severity;
		j["id"] = dg.id;
		j["message"] = dg.message;
		if (dg.span.empty())
			j["span"] = nullptr;   // the line as a whole
		else {
			json s;
			s["begin"] = dg.span.begin;
			s["end"] = dg.span.end;
			j["span"] = s;
		}
		list.push_back(j);
		if (dg.severity == "error")
			++errors;
		else if (dg.severity == "warning")
			++warnings;
		else
			++infos;
	}
	out["diagnostics"] = list;
	json counts;
	counts["error"] = errors;
	counts["warning"] = warnings;
	counts["info"] = infos;
	counts["total"] = diags.size();
	out["counts"] = counts;
	return out;
}

} // namespace scedit
