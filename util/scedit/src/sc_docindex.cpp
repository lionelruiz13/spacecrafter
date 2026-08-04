#include "sc_docindex.hpp"

#include <algorithm>
#include <fstream>
#include <set>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace scedit {

const char *const kNoDoc = "no documentation extracted — the contract file records none for this";

bool isCompletableLiteral(const std::string &s)
{
	if (s.empty())
		return false;
	for (unsigned char c : s) {
		const bool okc = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
		                 || (c >= '0' && c <= '9') || c == '_';
		if (!okc)
			return false;
	}
	return true;
}

bool Spec::valueDoc(const std::string &v, std::string &out) const
{
	auto it = value_docs.find(v);
	if (it == value_docs.end() || it->second.empty())
		return false;
	out = it->second;
	return true;
}

namespace {

std::string str(const json &j, const char *key)
{
	if (!j.is_object() || !j.contains(key))
		return std::string();
	const json &v = j.at(key);
	if (v.is_string())
		return v.get<std::string>();
	if (v.is_boolean())
		return v.get<bool>() ? "true" : "false";
	if (v.is_number())
		return v.dump();
	return std::string();   // null, object, array: not a sentence
}

//! Fill a Spec from an object carrying the six per-key facts (or a subset).
//! `doc` present-and-null is the honest gap, not a missing field: both land as
//! doc_known == false, and neither invents a sentence.
Spec specFrom(const json &o)
{
	Spec s;
	s.present = true;
	if (o.is_object() && o.contains("doc") && o.at("doc").is_string()) {
		s.doc = o.at("doc").get<std::string>();
		s.doc_known = !s.doc.empty();
	}
	s.value_domain = str(o, "value");
	s.default_prose = str(o, "default");
	s.required = str(o, "required");
	s.source = str(o, "source");
	s.notes = str(o, "notes");
	if (o.is_object() && o.contains("values") && o.at("values").is_array())
		for (const auto &v : o.at("values"))
			if (v.is_string())
				s.values.push_back(v.get<std::string>());
	if (o.is_object() && o.contains("value_docs") && o.at("value_docs").is_object())
		for (auto it = o.at("value_docs").begin(); it != o.at("value_docs").end(); ++it)
			if (it.value().is_string())
				s.value_docs[it.key()] = it.value().get<std::string>();
	// The explicit machine-readable default (D31's ghost-default). Absent from
	// every spec at HEAD; the code arms itself the day the doc pass writes it.
	if (o.is_object() && o.contains("default_value") && o.at("default_value").is_string()) {
		s.default_literal = o.at("default_value").get<std::string>();
		s.has_default_literal = !s.default_literal.empty();
	}

	std::set<std::string> cand;
	for (const auto &v : s.values)
		if (isCompletableLiteral(v))
			cand.insert(v);
	// A key of `value_docs` is a literal value by construction of that map, but
	// it still has to be typeable to be offered.
	for (const auto &kv : s.value_docs)
		if (isCompletableLiteral(kv.first))
			cand.insert(kv.first);
	s.completable.assign(cand.begin(), cand.end());
	return s;
}

} // namespace

bool DocIndex::load(const std::string &path, std::string &err)
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
	if (!g.is_object() || !g.contains("families")) {
		err = path + ": no `families` object";
		return false;
	}
	const json &fam = g.at("families");

	if (g.contains("parse_model") && g.at("parse_model").is_object()
	    && g.at("parse_model").contains("comments"))
		comment_line_doc_ = str(g.at("parse_model").at("comments"), "script_layer");

	if (fam.contains("commands")) {
		for (auto it = fam.at("commands").begin(); it != fam.at("commands").end(); ++it) {
			if (it.key().rfind("_", 0) == 0)
				continue;   // _source/_args_status annotations
			const json &e = it.value();
			if (!e.is_object())
				continue;
			CommandInfo ci;
			ci.name = it.key();
			if (e.contains("doc") && e.at("doc").is_string()) {
				ci.doc = e.at("doc").get<std::string>();
				ci.doc_known = !ci.doc.empty();
			}
			ci.registration = str(e, "registration");
			if (e.contains("args_complete") && e.at("args_complete").is_boolean())
				ci.args_complete = e.at("args_complete").get<bool>();
			ci.args_source = str(e, "args_source");
			if (e.contains("key_grammar") && e.at("key_grammar").is_object()) {
				ci.key_grammar = specFrom(e.at("key_grammar"));
				ci.has_key_grammar = true;
			}
			if (e.contains("args") && e.at("args").is_object()) {
				for (auto a = e.at("args").begin(); a != e.at("args").end(); ++a) {
					if (a.key().rfind("_", 0) == 0 || !a.value().is_object())
						continue;   // `_see` and friends are annotations, not keys
					Spec s = specFrom(a.value());
					++arg_specs_;
					if (s.has_default_literal)
						++default_literals_;
					ci.args[a.key()] = s;
				}
			}
			for (const auto &kv : ci.args)
				ci.keys.push_back(kv.first);   // std::map: byte-lexicographic
			commands_[ci.name] = ci;
			command_names_.push_back(ci.name);
		}
	}
	std::sort(command_names_.begin(), command_names_.end());

	for (auto f = fam.begin(); f != fam.end(); ++f) {
		if (f.key() == "commands" || !f.value().is_object() || !f.value().contains("names"))
			continue;
		const json &names = f.value().at("names");
		if (!names.is_array())
			continue;
		bool v2 = false;
		std::map<std::string, Spec> members;
		for (const auto &n : names) {
			if (n.is_string()) {
				Spec s;
				s.present = true;   // the name exists; the documentation does not
				members[n.get<std::string>()] = s;
				++family_names_;
				++family_names_v1_;
			} else if (n.is_object() && n.contains("name") && n.at("name").is_string()) {
				v2 = true;
				Spec s = specFrom(n);
				if (s.has_default_literal)
					++default_literals_;
				members[n.at("name").get<std::string>()] = s;
				++family_names_;
			}
		}
		family_is_v2_[f.key()] = v2;
		families_[f.key()] = members;
	}
	return true;
}

const CommandInfo *DocIndex::command(const std::string &name) const
{
	auto it = commands_.find(name);
	return it == commands_.end() ? nullptr : &it->second;
}

Spec DocIndex::familyMember(const std::string &family, const std::string &name) const
{
	auto f = families_.find(family);
	if (f == families_.end())
		return Spec();
	auto m = f->second.find(name);
	if (m == f->second.end())
		return Spec();
	return m->second;
}

std::vector<DocIndex::Dormant> DocIndex::dormantFeatures() const
{
	std::vector<Dormant> out;
	if (default_literals_ == 0)
		out.push_back({"ghost-text default on an empty value field (D31)",
		               "no arg spec carries a machine-readable `default_value`: all "
		               + std::to_string(arg_specs_) + " specs state the default as a "
		               "SENTENCE, which scedit will show but will not type for you"});
	if (family_names_v1_ != 0)
		out.push_back({"per-name documentation for " + std::to_string(family_names_v1_)
		               + " of the " + std::to_string(family_names_) + " family names",
		               "those families are still the v1 shape (a plain array of names); "
		               "the doc bar shows the name and says the doc is missing"});
	return out;
}

} // namespace scedit
