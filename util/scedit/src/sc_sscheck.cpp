/*
 * scedit -- sc_sscheck.cpp -- the two-regime stellar-system-file checker.
 *
 * Every rule below names the engine site whose behaviour it reports; the ids
 * and severities live here rather than in the contract because the contract
 * describes the FORMAT and these describe this tool's reading of it. The
 * `--rules` surface prints the lot, so a rule cannot be silently absent.
 */

#include "sc_sscheck.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace scedit {

namespace {

// --- ids, in one place ------------------------------------------------------
const char *ID_UNKNOWN      = "ss-unknown-key";
const char *ID_DEAD         = "ss-dead-key";
const char *ID_EXPERIMENTAL = "ss-experimental-only-key";
const char *ID_LEGACY_ONLY  = "ss-legacy-only-key";
const char *ID_NEWFORMAT    = "ss-new-format-construct";
const char *ID_MIDCOMMENT   = "ss-mid-line-comment";
const char *ID_DOMAIN       = "ss-value-domain";
const char *ID_DUPNAME      = "ss-duplicate-name";
const char *ID_MALFORMED    = "ss-malformed-line";
const char *ID_SPACING      = "ss-legacy-spacing";
const char *ID_NONAME       = "ss-section-without-name";
const char *ID_MODFAMILY    = "ss-unknown-module-family";

// The three keys whose PRESENCE changes what a section is. These -- not "a key
// the old loader ignores" -- are what a legacy file may not carry.
bool isStructuralNewFormatKey(const std::string &k)
{
	return k == "relation" || k == "compose" || k == "body";
}

std::string trim(const std::string &s)
{
	std::size_t b = s.find_first_not_of(" \t\r\n");
	if (b == std::string::npos)
		return std::string();
	std::size_t e = s.find_last_not_of(" \t\r\n");
	return s.substr(b, e - b + 1);
}

std::string lower(std::string s)
{
	for (char &c : s)
		c = (char)std::tolower((unsigned char)c);
	return s;
}

//! The legacy reader's own key/value arithmetic, reproduced exactly so the
//! checker can compare it against what a careful reader would expect:
//! protosystem.cpp:132-137 -- `substr(0,pos-1)` and `substr(pos+2)`, which
//! assume `key<SP>=<SP>value` and quietly truncate anything else.
bool legacyKeyOf(const std::string &line, std::string &key)
{
	std::string l = line;
	if (!l.empty() && l.back() == '\r')
		l.pop_back();
	if (l.size() < 2 || l[0] == '#')
		return false;
	std::size_t pos = l.find('=');
	if (pos == std::string::npos || pos == 0)
		return false;
	key = l.substr(0, pos - 1);
	return true;
}

//! A '#' that is not in column 0. Returns its byte offset, or npos. The legacy
//! reader treats only a column-0 '#' as a comment (protosystem.cpp:134) while
//! tools/ini_line.hpp treats one anywhere (ini_line.hpp:16), so any other '#'
//! in a legacy file means two different things to the file's two readers.
std::size_t midLineHash(const std::string &line)
{
	std::size_t h = line.find('#');
	return (h == 0) ? std::string::npos : h;
}

Diagnostic mk(const std::string &path, std::size_t line, const char *sev,
              const std::string &msg, const char *id, std::size_t b, std::size_t e)
{
	Diagnostic d;
	d.file = path;
	d.line = line;
	d.severity = sev;
	d.message = msg;
	d.id = id;
	d.span.begin = b;
	d.span.end = e;
	return d;
}

} // namespace

// ---------------------------------------------------------------------------
// regime, from the path and nothing else
// ---------------------------------------------------------------------------
SsRegime classifyPath(const std::string &path)
{
	// Normalise separators away and work on components, so a relative path, an
	// absolute one and a path with a trailing directory all answer the same.
	std::string p = path;
	std::replace(p.begin(), p.end(), '\\', '/');

	std::string base = p;
	const std::size_t slash = p.find_last_of('/');
	if (slash != std::string::npos)
		base = p.substr(slash + 1);
	const std::string dir = (slash == std::string::npos) ? std::string() : p.substr(0, slash);

	// COMPOSED: anything directly inside a `modularSystem` directory. The
	// machine-owned `.ini.disabled` twin is the same format and is checked as
	// such -- it is the file a user copies to adopt the format, so a finding in
	// it is a finding they are about to inherit.
	const std::size_t md = dir.rfind("modularSystem");
	if (md != std::string::npos && md + 13 == dir.size())
		return SsRegime::Composed;

	// LEGACY: `ssystem.ini` and the shipped `default_ssystem.ini`, plus the
	// `<name>_ssystem.ini` shape a second system uses.
	if (base == "ssystem.ini" || base == "default_ssystem.ini")
		return SsRegime::Legacy;
	if (base.size() > 12 && base.compare(base.size() - 12, 12, "_ssystem.ini") == 0)
		return SsRegime::Legacy;

	return SsRegime::NotStellarSystem;
}

const char *regimeName(SsRegime r)
{
	switch (r) {
	case SsRegime::Legacy:   return "legacy";
	case SsRegime::Composed: return "composed";
	default:                 return "not-a-stellar-system-file";
	}
}

// ---------------------------------------------------------------------------
// the contract file
// ---------------------------------------------------------------------------
bool SsGrammar::load(const std::string &path, std::string &err)
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
		// The discriminator, checked rather than assumed: being handed the
		// command grammar must SAY SO. Without this the tool would load it,
		// find no `keys`, and report every stellar-system key as unknown.
		const std::string contract = g.at("_meta").value("contract", std::string());
		if (contract != "stellar-system-file") {
			err = path + ": this is not the stellar-system contract (_meta.contract = '"
			      + (contract.empty() ? std::string("<absent>") : contract)
			      + "', expected 'stellar-system-file')";
			return false;
		}
		for (auto it = g.at("keys").begin(); it != g.at("keys").end(); ++it) {
			Key k;
			const auto &v = it.value();
			k.doc = v.value("doc", std::string());
			k.valueType = v.value("value_type", std::string());
			k.required = v.value("required", false);
			k.notes = v.value("notes", std::string());
			if (v.contains("default") && !v.at("default").is_null())
				k.defaultValue = v.at("default").get<std::string>();
			for (const auto &r : v.at("regimes")) {
				if (r == "legacy")   k.legacy = true;
				if (r == "composed") k.composed = true;
			}
			// A domain is an array for a plain enum; `coord_func` carries an
			// object of named groups, and every group's members are legal.
			if (v.contains("domain")) {
				const auto &d = v.at("domain");
				if (d.is_array())
					for (const auto &s : d)
						k.domain.push_back(s.get<std::string>());
				else if (d.is_object())
					for (auto g2 = d.begin(); g2 != d.end(); ++g2)
						for (const auto &s : g2.value())
							k.domain.push_back(s.get<std::string>());
			}
			keys_[it.key()] = k;
			keyNames_.push_back(it.key());
		}
		const auto &meta = g.at("_meta");
		if (meta.contains("dead_keys"))
			for (auto it = meta.at("dead_keys").begin(); it != meta.at("dead_keys").end(); ++it)
				dead_[it.key()] = it.value().value("why_no_reader", std::string());

		for (const auto &s : g.at("families").at("module_families").at("names"))
			families_.insert(s.get<std::string>());
		for (const auto &s : g.at("families").at("body_types").at("names"))
			bodyTypes_.insert(s.get<std::string>());
		const auto &cf = g.at("families").at("coord_func");
		for (const char *grp : {"intercepted_before_the_chain", "handled_by_the_creator_chain",
		                        "special_ephemerides", "composed_path_only"})
			if (cf.contains(grp))
				for (const auto &s : cf.at(grp))
					coordFuncs_.insert(s.get<std::string>());
	} catch (const std::exception &e) {
		err = path + ": malformed contract: " + e.what();
		return false;
	}
	loaded_ = true;
	return true;
}

const SsGrammar::Key *SsGrammar::find(const std::string &key) const
{
	auto it = keys_.find(key);
	return it == keys_.end() ? nullptr : &it->second;
}

const std::string *SsGrammar::deadKey(const std::string &key) const
{
	auto it = dead_.find(key);
	return it == dead_.end() ? nullptr : &it->second;
}

// ---------------------------------------------------------------------------
// the rules
// ---------------------------------------------------------------------------
std::vector<SsRule> ssRules()
{
	return {
		{ID_MALFORMED, "error",
		 "a non-empty line that is neither a section nor `key = value`; the legacy reader "
		 "turns it into a junk key rather than reporting it (protosystem.cpp:132-137)"},
		{ID_NEWFORMAT, "error",
		 "a construct in a LEGACY file that an older build's parser misreads: `relation=`, "
		 "`compose=`, `body=`, or a `[Node:FAMILY]` module header (INTENT S2.0 D13)"},
		{ID_MIDCOMMENT, "warning",
		 "a '#' outside column 0 in a legacy file: tools/ini_line.hpp reads it as a comment, "
		 "ProtoSystem::load reads it as data -- two readers of one file disagree"},
		{ID_DOMAIN, "warning",
		 "a value outside the domain the code enumerates for that key"},
		{ID_DUPNAME, "error",
		 "two sections declaring the same `name`; the second body is refused, because the "
		 "name IS the identity (D34, INTENT S11.109(c))"},
		{ID_UNKNOWN, "warning",
		 "a key no loader reads and that is not a known dead spelling -- usually a typo, and "
		 "the message names the nearest key that exists"},
		{ID_DEAD, "info",
		 "a key that IS in the shipped data and that no loader reads: it has no effect and "
		 "never had one in this build"},
		{ID_EXPERIMENTAL, "info",
		 "a key only the composed loader reads, written in a legacy file: harmless to the "
		 "old path, which ignores it, but it does nothing there"},
		{ID_LEGACY_ONLY, "info",
		 "a key only the legacy loader reads, written in a composed file"},
		{ID_SPACING, "warning",
		 "a `key=value` line whose key the legacy reader truncates: its arithmetic assumes "
		 "exactly one space each side of the '=' (protosystem.cpp:135)"},
		{ID_NONAME, "warning",
		 "a body section with no `name`: the section is skipped entirely "
		 "(protosystem.cpp:522-525)"},
		{ID_MODFAMILY, "error",
		 "a composed module section (one carrying `body =`) whose `type` is not a module "
		 "family (ModuleLoaderMgr.cpp:7-19)"},
	};
}

std::vector<Diagnostic> ssCheckBuffer(const SsGrammar &g, const std::string &path,
                                      const std::string &bytes, SsRegime regime)
{
	std::vector<Diagnostic> out;
	if (regime == SsRegime::NotStellarSystem || !g.loaded())
		return out;

	struct SectionInfo { std::size_t line; std::string header; bool isModule = false; };
	SectionInfo cur;
	bool inSection = false;
	std::map<std::string, std::size_t> nameFirstSeen;   // name value -> line
	std::vector<std::pair<std::size_t, std::string>> pendingSectionNames;

	std::size_t lineNo = 0, pos = 0;
	while (pos <= bytes.size()) {
		std::size_t nl = bytes.find('\n', pos);
		if (nl == std::string::npos)
			nl = bytes.size();
		const std::string line = bytes.substr(pos, nl - pos);
		++lineNo;
		const std::string t = trim(line);

		if (t.empty() || t[0] == '#') {
			pos = nl + 1;
			if (nl == bytes.size()) break;
			continue;
		}

		// ---- section header --------------------------------------------
		if (t[0] == '[') {
			const std::size_t close = t.find(']');
			const std::string header = (close == std::string::npos)
			                           ? t.substr(1) : t.substr(1, close - 1);
			// `[end]` is the legacy sentinel and is load-bearing: the legacy
			// reader flushes a body when it sees the NEXT header, so the file
			// needs a final one. It is not a body and must never be reported.
			if (lower(header) == "end") {
				pos = nl + 1;
				if (nl == bytes.size()) break;
				continue;
			}
			inSection = true;
			cur = SectionInfo{lineNo, header, header.find(':') != std::string::npos};
			if (cur.isModule && regime == SsRegime::Legacy) {
				const std::size_t colon = header.find(':');
				out.push_back(mk(path, lineNo, "error",
					"Module-section header '[" + header + "]' in a legacy file. The legacy "
					"reader has no module sections: it reads this as a body literally named '"
					+ header + "'. Module declarations belong in a composed file "
					"(~/.spacecrafter/modularSystem/<Name>.ini). To fix: move the section "
					"there, or drop the ':" + header.substr(colon + 1) + "' suffix if a body "
					"was meant.", ID_NEWFORMAT,
					line.find('['), line.find('[') + t.size()));
			}
			pos = nl + 1;
			if (nl == bytes.size()) break;
			continue;
		}

		// ---- mid-line '#', legacy only ----------------------------------
		if (regime == SsRegime::Legacy) {
			const std::size_t h = midLineHash(line);
			if (h != std::string::npos)
				out.push_back(mk(path, lineNo, "warning",
					"A '#' here is read two different ways by the two readers of this file: "
					"the shared line grammar (tools/ini_line.hpp) cuts a comment from it, "
					"while the legacy body reader only treats a '#' in COLUMN 0 as a comment "
					"(protosystem.cpp:134) and keeps these bytes as data. To fix: put the "
					"comment on its own line starting at column 0, or remove it.",
					ID_MIDCOMMENT, h, line.size()));
		}

		// ---- entry -------------------------------------------------------
		const std::size_t eq = line.find('=');
		if (eq == std::string::npos) {
			out.push_back(mk(path, lineNo, "error",
				"Line is neither a section header nor `key = value`. The legacy reader does "
				"not report this: it stores a junk key built from the whole line and carries "
				"on, so the value here is simply lost. To fix: add the '=', or comment the "
				"line out with a '#' in column 0.",
				ID_MALFORMED, 0, line.size()));
			pos = nl + 1;
			if (nl == bytes.size()) break;
			continue;
		}

		const std::string key = trim(line.substr(0, eq));
		const std::string value = trim(line.substr(eq + 1));
		const std::size_t keyBegin = line.find_first_not_of(" \t");
		const std::size_t keyEnd = keyBegin + key.size();

		if (key.empty()) {
			out.push_back(mk(path, lineNo, "error",
				"Entry with an empty key. To fix: write `key = value`, or comment the line "
				"out with a '#' in column 0.", ID_MALFORMED, 0, line.size()));
			pos = nl + 1;
			if (nl == bytes.size()) break;
			continue;
		}

		// The legacy substr arithmetic, compared against the key a reader sees.
		if (regime == SsRegime::Legacy) {
			std::string lk;
			if (legacyKeyOf(line, lk) && lk != key)
				out.push_back(mk(path, lineNo, "warning",
					"The legacy reader will read this key as '" + lk + "', not '" + key
					+ "': its arithmetic takes everything before the '=' minus one character "
					"and the value from two characters after it (protosystem.cpp:135-136), "
					"so the line must be spelled exactly `key = value` with one space on "
					"each side. To fix: put exactly one space on each side of the '='.",
					ID_SPACING, keyBegin, keyEnd));
		}

		if (key == "name" && inSection) {
			auto it = nameFirstSeen.find(value);
			if (it != nameFirstSeen.end())
				out.push_back(mk(path, lineNo, "error",
					"A body named '" + value + "' is already declared at line "
					+ std::to_string(it->second) + ". The name IS the body's identity and it "
					"must be unique across every loaded system: the loader refuses the second "
					"one and keeps the first (protosystem.cpp:546-549, "
					"ModularSystem.cpp:1057-1060). To fix: rename this body.",
					ID_DUPNAME, line.find(value), line.find(value) + value.size()));
			else
				nameFirstSeen[value] = lineNo;
		}

		// ---- structural new-format keys in a legacy file -----------------
		if (regime == SsRegime::Legacy && isStructuralNewFormatKey(key)) {
			out.push_back(mk(path, lineNo, "error",
				"'" + key + "' is a composed-format key and this is a legacy file. It "
				"changes what a section IS, so a build older than the composed format reads "
				"this file differently -- and a legacy file has to stay readable by one "
				"(INTENT S2.0 D13, downgrade must stay possible). To fix: make this change "
				"in the composed file (~/.spacecrafter/modularSystem/<Name>.ini) instead."
				+ std::string(key == "relation"
				  ? " In a legacy file the same state comes from `coord_func = "
				    "location_orbit`." : ""),
				ID_NEWFORMAT, keyBegin, keyEnd));
		}

		// ---- vocabulary ---------------------------------------------------
		const SsGrammar::Key *k = g.find(key);
		if (!k) {
			if (const std::string *why = g.deadKey(key)) {
				out.push_back(mk(path, lineNo, "info",
					"'" + key + "' is read by no loader in this build, so this line has no "
					"effect. " + *why + " To fix: remove the line, or leave it -- it is "
					"inert either way.", ID_DEAD, keyBegin, keyEnd));
			} else {
				const std::string sug = cappedSuggestion(key, g.allKeyNames());
				out.push_back(mk(path, lineNo, "warning",
					"'" + key + "' is not a key any loader reads."
					+ (sug.empty() ? std::string(" To fix: remove the line, or check the key "
					                             "name against the contract.")
					               : " Did you mean '" + sug + "'? Note that keys are "
					                 "case-sensitive: no reader lowercases them."),
					ID_UNKNOWN, keyBegin, keyEnd));
			}
		} else {
			// regime reach
			if (regime == SsRegime::Legacy && !k->legacy && k->composed
			    && !isStructuralNewFormatKey(key))
				out.push_back(mk(path, lineNo, "info",
					"'" + key + "' is read only by the composed loader. In this legacy file "
					"it is read when the experimental path loads it and ignored by the old "
					"render path, which never looks at this key. That is harmless -- an "
					"unknown key costs nothing there -- but it does nothing for the old path.",
					ID_EXPERIMENTAL, keyBegin, keyEnd));
			if (regime == SsRegime::Composed && k->legacy && !k->composed)
				out.push_back(mk(path, lineNo, "info",
					"'" + key + "' is read only by the legacy loader, and this is a composed "
					"file, which the legacy loader never reads. The key has no effect here.",
					ID_LEGACY_ONLY, keyBegin, keyEnd));

			// value domain
			const std::size_t vBegin = value.empty() ? eq + 1 : line.find(value, eq + 1);
			const std::size_t vEnd = vBegin + value.size();
			if (!k->domain.empty() && !value.empty()) {
				const bool ok = std::find(k->domain.begin(), k->domain.end(), value)
				                != k->domain.end();
				// A composed NODE may carry `type = BODY`, the canonical marker
				// for a body-type-less node (INTENT S11.89(c)); a legacy file
				// may not -- there `BODY` is simply not a body type.
				const bool bodyMarker = (key == "type" && value == "BODY"
				                         && regime == SsRegime::Composed && !cur.isModule);
				// In a composed MODULE section `type` names a family, not a
				// body type: a different domain entirely.
				if (key == "type" && cur.isModule && regime == SsRegime::Composed) {
					if (!g.moduleFamilies().count(value)) {
						std::string names;
						for (const auto &f : g.moduleFamilies())
							names += (names.empty() ? "" : ", ") + f;
						out.push_back(mk(path, lineNo, "error",
							"'" + value + "' is not a module family. This section binds a "
							"body with `body =`, so its `type` names the module to build. "
							"Valid families are: " + names + ". To fix: set `type` to one of "
							"them, or remove `body =` to declare a body instead.",
							ID_MODFAMILY, vBegin, vEnd));
					}
				} else if (!ok && !bodyMarker) {
					std::string names;
					std::size_t shown = 0;
					for (const auto &d : k->domain) {
						if (shown++ == 12) { names += ", ..."; break; }
						names += (names.empty() ? "" : ", ") + d;
					}
					const std::string sug = cappedSuggestion(value, k->domain);
					out.push_back(mk(path, lineNo, "warning",
						"'" + value + "' is not a value '" + key + "' accepts."
						+ (sug.empty() ? std::string() : " Did you mean '" + sug + "'?")
						+ " Accepted: " + names + ".",
						ID_DOMAIN, vBegin, vEnd));
				}
			}
			// booleans: the two loaders do not agree on what true is, so the
			// message has to say WHICH reader will disagree with the author.
			if (!value.empty() && k->valueType == "bool_suppress") {
				// The opposite polarity: the module is ON unless the value is
				// one the negative predicate accepts. `no` is the trap here --
				// it reads as "not false", so it leaves the module ON.
				const std::string lv = lower(value);
				const bool off = (lv == "false" || lv == "off" || lv == "0");
				const bool on = (lv == "true" || lv == "on" || lv == "1");
				if (!off && !on)
					out.push_back(mk(path, lineNo, "warning",
						"'" + value + "' does NOT switch '" + key + "' off. This key is read "
						"the opposite way round from the other booleans: it turns its module "
						"off only for 'false', 'off' or '0' (any case), and anything else -- "
						"including 'no' -- leaves it on (Utility::isFalse, "
						"tools/utility.hpp:172). To fix: write 'false'.",
						ID_DOMAIN, vBegin, vEnd));
			} else if (!value.empty() && k->valueType.compare(0, 5, "bool_") == 0) {
				const std::string lv = lower(value);
				const bool legacyTrue = (lv == "true" || lv == "1");
				const bool composedTrue = (lv == "true" || lv == "on" || lv == "1");
				const bool anyFalse = (lv == "false" || lv == "0" || lv == "off"
				                       || lv == "no");
				if (!legacyTrue && !composedTrue && !anyFalse) {
					out.push_back(mk(path, lineNo, "warning",
						"'" + value + "' reads as FALSE. A boolean here is true only for "
						"'true' or '1' (legacy reader) or 'true', 'on' or '1' (composed "
						"reader); everything else is false and nothing is reported at run "
						"time, so a misspelling looks exactly like a deliberate 'off'. "
						"To fix: write 'true' or 'false'.",
						ID_DOMAIN, vBegin, vEnd));
				} else if (composedTrue && !legacyTrue) {
					out.push_back(mk(path, lineNo, "warning",
						"'" + value + "' means TRUE to the composed loader and FALSE to the "
						"legacy one (Utility::isTrue accepts 'on', Utility::strToBool does "
						"not) -- so this one line makes the two render paths disagree about "
						"'" + key + "'. To fix: write 'true'.",
						ID_DOMAIN, vBegin, vEnd));
				}
			}
		}

		pos = nl + 1;
		if (nl == bytes.size()) break;
	}

	std::stable_sort(out.begin(), out.end(),
	                 [](const Diagnostic &a, const Diagnostic &b) { return a.line < b.line; });
	return out;
}

std::vector<Diagnostic> ssCheckFile(const SsGrammar &g, const std::string &path,
                                    SsRegime regime, std::string &io_error)
{
	std::ifstream in(path, std::ios::binary);
	if (!in) {
		io_error = "cannot open " + path;
		return {};
	}
	std::ostringstream ss;
	ss << in.rdbuf();
	return ssCheckBuffer(g, path, ss.str(), regime);
}

} // namespace scedit
