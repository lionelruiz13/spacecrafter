/*
 * scedit -- sc_check.cpp
 *
 * Each rule below states the engine site it reports. Rules fire only on lines
 * the engine will actually execute: lines the script layer drops (comment,
 * blank), lines inside a `comment` block, and the comment tail after a '#'
 * (parse_model.comments.mid_line) are not analysed, because a finding on text
 * the engine never reads is a false positive by definition (C3).
 */

#include "sc_check.hpp"

#include <set>
#include "sc_tokenizer.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
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

//! DID-YOU-MEAN, DISPLAY CAP.
//! The engine's searchNeighbour has no distance threshold: it always logs a
//! nearest name, however far. `nearestNeighbour` in the tokenizer library
//! reproduces that exactly and stays that way -- it is the engine's behaviour and
//! C1 owns it. What is capped here is only whether scedit PRINTS the answer.
//!
//! The cap: TWO edits, or one edit per three characters of what the author
//! typed, whichever is more permissive. Two edits is the floor because the
//! ordinary typo shapes cost two whatever the word's length -- a transposition
//! ('zomo' for 'zoom') is two substitutions -- and a short name would otherwise
//! never get a suggestion. The proportional term is what keeps a long name
//! honest: past a third of its length the candidate no longer agrees with what
//! was typed, it is merely the closest entry in a list, and printing it spends
//! the reader's attention on a false lead. The corpus case that forced this:
//! `flag date_display_number` (19 characters) drew 'nebula_names' at distance 12
//! (doc/superscript.sts:303) while the useful 'datetime_display_number' sits at
//! distance 4 on the neighbouring `set` lines. The FINDING is unchanged in every
//! case; only the trailing hint disappears.
//! Recorded in util/scedit/tests/derivation-diff.md S5.7 and in the contract
//! file's `lint_seeds` entry for unknown-command.
std::size_t suggestionCap(const std::string &token)
{
	const std::size_t proportional = token.size() / 3;
	return proportional > 2 ? proportional : 2;
}

std::string cappedSuggestion(const std::string &token, const std::vector<std::string> &candidates)
{
	const std::string near = nearestNeighbour(token, candidates);
	if (near.empty())
		return std::string();
	return levenshtein(token, near) <= suggestionCap(token) ? near : std::string();
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

//! BYTES THAT LOOK LIKE A SEPARATOR AND ARE NOT.
//! The engine splits on the C locale's whitespace only (SP TAB LF VT FF CR,
//! std::istringstream at parseCommand:141). Every other byte -- including every
//! space character Unicode has -- is an ordinary character that GLUES the words
//! around it into one token. In an editor they are all blank, so the author
//! cannot see the difference; that is what makes the class worth an id of its
//! own rather than only its consequences.
//! Longest sequences first, so the UTF-8 form is named before its trailing 0xA0.
struct InvisibleByte {
	const char *bytes;
	std::size_t len;
	const char *what;
};
const InvisibleByte kInvisible[] = {
	{"\xef\xbb\xbf", 3, "a byte-order mark (U+FEFF)"},
	{"\xe2\x80\xaf", 3, "a narrow no-break space (U+202F)"},
	{"\xe2\x80\xa8", 3, "a line separator (U+2028)"},
	{"\xe3\x80\x80", 3, "an ideographic space (U+3000)"},
	{"\xc2\xa0",     2, "a no-break space (U+00A0) written as UTF-8"},
	{"\xa0",         1, "a no-break space, the ISO-8859 spelling"},
};

//! U+2000..U+200B (en quad ... zero-width space) share the E2 80 8x prefix.
bool unicodeSpaceRun(const std::string &s, std::size_t i, std::size_t &len, std::string &what)
{
	if (i + 3 > s.size())
		return false;
	const unsigned char a = (unsigned char)s[i], b = (unsigned char)s[i + 1], c = (unsigned char)s[i + 2];
	if (a != 0xE2 || b != 0x80 || c < 0x80 || c > 0x8B)
		return false;
	len = 3;
	char buf[32];
	std::snprintf(buf, sizeof buf, "a space of the U+20%02X kind", 0x00 + (c - 0x80));
	what = buf;
	return true;
}

//! "0xA0" / "0xC2 0xA0"
std::string hexBytes(const std::string &s, std::size_t off, std::size_t len)
{
	std::string out;
	for (std::size_t i = 0; i < len; ++i) {
		char buf[8];
		std::snprintf(buf, sizeof buf, "0x%02X", (unsigned char)s[off + i]);
		if (!out.empty()) out += ' ';
		out += buf;
	}
	return out;
}

//! ONE place turns (id, message, span) into a Diagnostic: the severity comes
//! from the contract file's seed, with a fallback that keeps an id the file
//! does not know visible rather than dropped. Used by the per-line rules and
//! by the end-of-file rules alike (I2).
Diagnostic makeDiagnostic(const Grammar &g, const std::string &file, std::size_t line,
                          const char *id, const std::string &message, const Span &span)
{
	Diagnostic d;
	d.file = file;
	d.line = line;
	d.id = id;
	const LintSeed *s = g.seed(id);
	d.severity = s ? s->severity : std::string("warning");
	d.message = message;
	d.span = span;
	return d;
}

//! The KEY token of the first pair (line order) carrying `key`.
Span keySpanOf(const Line &L, const std::string &key)
{
	for (const auto &p : L.pairs)
		if (L.tokens[p.key].text == key)
			return L.tokens[p.key].span;
	return Span{};
}

//! The VALUE token of the last pair carrying `key` -- the value `args` keeps.
Span valueSpanOf(const Line &L, const std::string &key)
{
	Span s;
	for (const auto &p : L.pairs)
		if (L.tokens[p.key].text == key)
			s = L.tokens[p.value].span;
	return s;
}

//! A whole number written as such (strtol consuming every byte).
bool wholeNumber(const std::string &s, long &n)
{
	if (s.empty())
		return false;
	char *endp = nullptr;
	n = std::strtol(s.c_str(), &endp, 10);
	return endp && *endp == '\0';
}

class LineChecker {
public:
	LineChecker(const Grammar &g, const std::string &file, std::size_t lineno,
	            std::vector<Diagnostic> &out,
	            const std::set<std::string> *downstreamKeys = nullptr)
		: g_(g), file_(file), lineno_(lineno), out_(out), down_(downstreamKeys) {}

	void emit(const char *id, const std::string &message, const Span &span = Span{})
	{
		out_.push_back(makeDiagnostic(g_, file_, lineno_, id, message, span));
	}

	void run(const Line &L);

private:
	//! The rules that read the line's key/value shape.
	void rules(const Line &L, const CommandData &cd);
	//! Returns true when `name` is one the engine will not accept. `at` is
	//! where the name sits on the line.
	bool checkSubfamilyName(const Line &L, const CommandData &cd, const FamilyData &fam,
	                        const std::string &name, const Span &at,
	                        const std::string &consequence);
	//! Bytes the engine reads only: before the comment, when the line has one.
	void checkInvisibleSeparators(const Line &L);

	const Grammar &g_;
	std::string file_;
	std::size_t lineno_;
	std::vector<Diagnostic> &out_;
	//! The other contract's key vocabulary, lowercased; null when it was not
	//! supplied. See sc_check.hpp for why it is a set and why the case matters.
	const std::set<std::string> *down_ = nullptr;
};

bool LineChecker::checkSubfamilyName(const Line &L, const CommandData &cd,
                                     const FamilyData &fam, const std::string &name,
                                     const Span &at, const std::string &consequence)
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
			     "has no effect (the name exists in the engine source but no flag is bound to it)",
			     at);
			return true;
		}
	} else if (fam.name_set.count(name)) {
		auto def = fam.known_defective.find(name);
		if (def != fam.known_defective.end())
			emit("inert-command",
			     quoteName(cd.name + " " + name) + " is accepted and reported as successful but "
			     "does nothing: " + def->second, at);
		return false;   // known name
	}
	// AppCommandInit::searchNeighbour returns before suggesting when the name is
	// on the obsolete list, and the engine logs it as retired instead.
	if (g_.isObsolete(name)) {
		emit("deprecated",
		     quoteName(name) + " is no longer used in this software: the engine ignores it" +
		     consequence, at);
		return true;
	}
	std::string msg = quoteName(name) + " is not a known " + familyLabel(cd.subfamily) + consequence;
	std::string near = cappedSuggestion(name, fam.sorted);
	if (!near.empty())
		msg += "; did you mean " + quoteName(near) + "?";
	emit("unknown-parameter", msg, at);
	return true;
}

//! Scope: bytes OUTSIDE a quoted value, and before the comment when the line
//! has one. Inside a `"..."` run the engine already accepts spaces, so a
//! no-break space there is ordinary text the author meant -- reporting it would
//! be a C3 false positive; past the '#' the engine reads nothing at all.
//! Everywhere else the byte sits where a separator was meant, or turns a name
//! into a name nothing knows.
void LineChecker::checkInvisibleSeparators(const Line &L)
{
	const std::size_t limit = std::min(L.comment_begin, L.raw.size());
	auto insideQuotedValue = [&L](std::size_t off) {
		for (const auto &t : L.tokens)
			if (t.quoted && t.span.contains(off))
				return true;
		return false;
	};

	for (std::size_t i = 0; i < limit;) {
		std::size_t len = 0;
		std::string what;
		for (const auto &iv : kInvisible) {
			if (L.raw.compare(i, iv.len, iv.bytes, iv.len) == 0) {
				len = iv.len;
				what = iv.what;
				break;
			}
		}
		if (!len && !unicodeSpaceRun(L.raw, i, len, what)) {
			++i;
			continue;
		}
		if (!insideQuotedValue(i)) {
			emit("invisible-separator",
			     "column " + std::to_string(i + 1) + " holds " + what + " (byte " +
			     hexBytes(L.raw, i, len) + "), not a space: the engine separates words on space, "
			     "tab, CR, LF, VT and FF only, so what is written on either side of this byte is "
			     "read as ONE word", Span{i, i + len});
		}
		i += len;
	}
}

void LineChecker::run(const Line &L)
{
	// A line that is only blanks and/or a comment: the comment cut and the
	// leading-blank strip leave nothing, `commandstr >> command` fails, and
	// executeCommand:207 returns without acting. (Before the comment rule, an
	// INDENTED '#' executed as an unknown command -- the retired
	// `indented-comment` seed; parse_model.comments.script_layer.)
	if (!L.has_command)
		return;

	// --- the cause before its consequences ------------------------------------
	// A separator-lookalike byte changes what every later token IS, so it is
	// reported FIRST on the line. It does not suppress the rules that report
	// what the engine will then DO with the line: both statements are true and
	// separately actionable, and silencing a true finding because another rule
	// explains it would make the finding set depend on rule order.
	checkInvisibleSeparators(L);

	const CommandData *cd = g_.command(L.command);
	const Span cmdSpan = L.tokens.empty() ? Span{} : L.tokens.front().span;
	if (!cd) {
		if (g_.isObsolete(L.command)) {
			emit("deprecated",
			     "command " + quoteName(L.command) + " is no longer used in this software: "
			     "the line is ignored", cmdSpan);
			return;
		}
		std::string msg = "unknown command " + quoteName(L.command);
		std::string near = cappedSuggestion(L.command, g_.commandLookupList());
		if (!near.empty())
			msg += "; did you mean " + quoteName(near) + "?";
		emit("unknown-command", msg, cmdSpan);
		return;   // the key/value shape of a line the engine will not run is not news
	}

	rules(L, *cd);
}

void LineChecker::rules(const Line &L, const CommandData &cdRef)
{
	const CommandData *cd = &cdRef;

	// --- parse-level facts ---------------------------------------------------
	if (L.has_dangling)
		emit("dangling-key",
		     "key " + quoteName(L.tokens[L.dangling_index].text) + " has no value: the engine "
		     "drops it silently and runs the rest of the line without it",
		     L.tokens[L.dangling_index].span);

	{
		std::map<std::string, int> seen;
		for (const auto &p : L.pairs)
			++seen[L.tokens[p.key].text];
		std::map<std::string, int> met;   // occurrences walked so far, to point at the repeat
		for (const auto &p : L.pairs) {
			const std::string &k = L.tokens[p.key].text;
			const int nth = ++met[k];
			auto it = seen.find(k);
			if (it == seen.end() || it->second < 2 || nth != 2)
				continue;   // report each duplicated key once, at its first repetition
			const int count = it->second;
			auto a = L.args.find(k);
			emit("duplicate-key",
			     "key " + quoteName(k) + " is given " + std::to_string(count) + " times on this "
			     "line: the engine keeps only the last value" +
			     (a != L.args.end() ? " (" + quoteName(a->second) + ")" : std::string()),
			     L.tokens[p.key].span);
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
			std::size_t closer = L.tokens.size();
			for (std::size_t j = i + 1; j < L.tokens.size(); ++j) {
				const std::string u = L.rawText(L.tokens[j].span);
				if (!u.empty() && u.back() == '\'') { closer = j; break; }
			}
			if (closer < L.tokens.size()) {
				emit("unsupported-quoting",
				     "single quotes do not group words here: only \" does, so this value ends "
				     "at the first space and the remaining words are read as key/value pairs",
				     Span{L.tokens[i].span.begin, L.tokens[closer].span.end});
				break;
			}
		}
		const std::size_t bs = L.raw.find("\\\"");
		if (bs != std::string::npos && bs < L.comment_begin)
			emit("unsupported-quoting",
			     "backslash is not an escape here: the engine has no escape sequence, so the "
			     "value ends at this \" and the backslash is kept as an ordinary character",
			     Span{bs, bs + 2});
	}

	// --- one pair per line commands ------------------------------------------
	if (g_.isSinglePairCommand(cd->name) && L.args.size() > 1) {
		Span dropped;   // the first pair, in line order, that is NOT the applied one
		for (const auto &p : L.pairs)
			if (L.tokens[p.key].text != L.args.begin()->first) {
				dropped = Span{L.tokens[p.key].span.begin, L.tokens[p.value].span.end};
				break;
			}
		emit("single-pair-only",
		     "command " + quoteName(cd->name) + " applies a single key/value pair per line: "
		     "only " + quoteName(L.args.begin()->first) + " (first in alphabetical order, not "
		     "line order) is applied; the other " + std::to_string(L.args.size() - 1) +
		     " pair(s) are dropped silently", dropped);
	}

	// --- the family this command draws its name from --------------------------
	const FamilyData *fam = cd->subfamily.empty() ? nullptr : g_.family(cd->subfamily);
	if (fam) {
		switch (cd->placement.pos) {
			case SubfamilyPosition::AppliedKey:
				if (!L.args.empty())
					checkSubfamilyName(L, *cd, *fam, L.args.begin()->first,
					                   keySpanOf(L, L.args.begin()->first), "");
				break;
			case SubfamilyPosition::EveryKey: {
				// commandSet walks `args` and folds with `&&`
				// (app_command_interface.cpp:2119-2121): the FIRST pair that
				// fails makes every later call short-circuit away, so the pairs
				// after it are never applied at all. Say so once, on the pair
				// that causes it -- "there is a typo" and "this line does
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
					if (checkSubfamilyName(L, *cd, *fam, it->first, keySpanOf(L, it->first),
					                       consequence))
						aborted = true;
				}
				break;
			}
			case SubfamilyPosition::ValueOfKey: {
				auto it = L.args.find(cd->placement.position_key);
				if (it != L.args.end() && !it->second.empty())
					checkSubfamilyName(L, *cd, *fam, it->second,
					                   valueSpanOf(L, cd->placement.position_key), "");
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
			     "as OFF, with no error", valueSpanOf(L, name));
	}

	// --- argument vocabulary, armed by data presence alone ---------------------
	// `has_args` says the entry lists keys; `args_complete` says the list is the
	// WHOLE accepted vocabulary. Only the second one licenses the word
	// "unknown": `body` and `camera` (and `flyto`, which IS camera) forward
	// their map to a grammar that is another contract file's deliverable, so
	// they stay silent on their keys by construction, not by omission.
	// A command whose vocabulary CONTINUES into another contract is complete
	// once that contract is in hand: `body` and `camera` forward their parsed
	// map to the stellar-system-file body grammar, and since F80 that grammar
	// exists (grammar/ss-grammar.json). The keys are read FROM it, never copied
	// into this file (I2) -- which is why the entry keeps naming the other
	// contract rather than growing a second list of its keys.
	// A command that NAMES a downstream contract is complete only when that
	// contract is actually in hand. Reading `args_complete` alone here would be
	// the worst of both: the entry says true (its vocabulary IS fully stated,
	// across two files), so a consumer holding only this file would call every
	// stellar-system key of a `body action load` line unknown -- thousands of
	// them. The entry's own text says a consumer without the second file must
	// treat the list as partial; this is that sentence, enforced.
	const bool hasDownstream = !cd->args_downstream_contract.empty();
	const bool downstreamAvailable = hasDownstream && down_;
	const bool vocabularyComplete = hasDownstream ? downstreamAvailable : cd->args_complete;
	if (cd->has_args && vocabularyComplete) {
		const bool name_is_key = fam &&
			(cd->placement.pos == SubfamilyPosition::AppliedKey ||
			 cd->placement.pos == SubfamilyPosition::EveryKey);
		for (const auto &p : L.pairs) {
			const std::string &k = L.tokens[p.key].text;
			if (cd->arg_keys.count(k))
				continue;
			if (downstreamAvailable && down_->count(k))
				continue;   // stated by the other contract, not missing here
			if (name_is_key && fam->name_set.count(k))
				continue;   // the key IS the family name, not an argument
			std::string msg = quoteName(k) + " is not an argument of command " + quoteName(cd->name);
			std::vector<std::string> cands = cd->arg_keys_sorted;
			if (downstreamAvailable)
				cands.insert(cands.end(), down_->begin(), down_->end());
			std::string near = cappedSuggestion(k, cands);
			if (!near.empty())
				msg += "; did you mean " + quoteName(near) + "?";
			emit("unknown-parameter", msg, L.tokens[p.key].span);
		}
	}

	// An alias (`flyto`, `div`, `mul`, `mod`) is not a finding: the engine runs
	// the same handler, and a recording keeps the spelling as typed
	// (parse_model.recording_alias_loss, corrected 2026-08-31). The seed
	// `alias-respelled` that claimed otherwise is retired.
}

} // namespace

std::vector<Diagnostic> checkBuffer(const Grammar &g, const std::string &path,
                                    const std::string &bytes,
                                    const std::set<std::string> *downstreamKeys)
{
	std::vector<Diagnostic> out;
	BlockSkipState skip;
	const auto lines = splitScriptLines(bytes);
	for (std::size_t i = 0; i < lines.size(); ++i) {
		Line L = tokenizeLine(lines[i]);
		if (L.kind != LineKind::Parsed)
			continue;
		const std::size_t seen = skip.unmatched().size();
		const bool skipped = skip.feed(L, i + 1);
		// A closer that closes nothing. All three forms are REPORTED by the
		// engine and ignored -- `reportScriptError` at commandStruct :4773
		// (else), :4778 (end) and :4846 (loop end), which logs at L_ERROR and
		// writes a `#!` tail onto the offending line (app_command_interface.cpp
		// :215-239, code e3afca8f).
		//
		// These three messages used to QUOTE the engine's log text ("end
		// without if", "else without if") and one described the loop form as
		// logging nothing. Both were true of the engine that if_swap.cpp:45,:76
		// used to be, and code 2b8ec034 (2026-08-31) moved those writes to the
		// caller and gave them new words -- so the quotes named strings the
		// engine had stopped writing, and nothing checked them (F76, measured
		// live: the log line, the `#!`, and this file's own reading of it all
		// agree, and none of them says "end without if"). The engine's exact
		// sentence has ONE home that IS checked, the grammar's `engine_tail`
		// data, which f63_scedit_agree.py maps against the running engine;
		// prose here says what HAPPENS and quotes nothing (I2).
		for (std::size_t u = seen; u < skip.unmatched().size(); ++u) {
			const auto &m = skip.unmatched()[u];
			if (m.what == "end")
				out.push_back(makeDiagnostic(g, path, i + 1, "end-without-if",
					"this 'struct if end' closes nothing: no 'struct if' block is open here, so the "
					"engine reports an error on this line and ignores it \xe2\x80\x94 either this 'end' is one "
					"too many, or the block it was meant to close was never opened", m.span));
			else if (m.what == "else")
				out.push_back(makeDiagnostic(g, path, i + 1, "else-without-if",
					"this 'struct if else' flips nothing: no 'struct if' block is open here, so the "
					"engine reports an error on this line and ignores it", m.span));
			else
				out.push_back(makeDiagnostic(g, path, i + 1, "loop-end-without-loop",
					"this 'struct loop end' closes nothing: no 'struct loop' is open here, so the "
					"engine reports an error on this line, resets an empty loop and nothing repeats "
					"\xe2\x80\x94 either this 'end' is one too many, or the 'struct loop <n>' it was meant to "
					"close is missing", m.span));
		}
		if (skipped)
			continue;   // inside a `comment` block: the engine never runs this
		LineChecker(g, path, i + 1, out, downstreamKeys).run(L);
	}

	// --- still open at the end of the file --------------------------------------
	// Reported at the OPENER: that is the root, EOF is only where the damage
	// surfaces. ifSwap stays pushed until `script action end` (:2810), which the
	// end of the script runs -- so the loss is bounded to this file's own tail.
	for (const auto &b : skip.openIfs())
		out.push_back(makeDiagnostic(g, path, b.line, "unclosed-struct",
			quoteName(b.text) + " is never closed: no 'struct if end' follows before the end of "
			"the file, so whenever this test fails the engine skips every line from here to the "
			"end of the file, silently; put 'struct if end' where the block should stop", b.span));
	for (const auto &b : skip.openLoops()) {
		long n = 0;
		std::string consequence;
		if (!wholeNumber(b.count, n))
			consequence = "depending on the value of " + quoteName(b.count) + " the lines after "
			              "it either run once and are never repeated, or are skipped to the end of "
			              "the file";
		else if (n > 1)
			consequence = "the lines after it run ONCE and are never repeated \xe2\x80\x94 the repetition "
			              "only starts at 'struct loop end'";
		else if (n < 1)
			consequence = "every line after it is skipped to the end of the file";
		else
			consequence = "a count of 1 repeats nothing, so the lines after it run once \xe2\x80\x94 "
			              "harmless today, but the block has no end";
		out.push_back(makeDiagnostic(g, path, b.line, "unclosed-struct",
			quoteName(b.text) + " is never closed: no 'struct loop end' follows before the end of "
			"the file, so " + consequence + "; put 'struct loop end' where the block should stop",
			b.span));
	}

	std::stable_sort(out.begin(), out.end(),
	                 [](const Diagnostic &a, const Diagnostic &b) { return a.line < b.line; });
	return out;
}

std::vector<Diagnostic> checkFile(const Grammar &g, const std::string &path,
                                  std::string &io_error,
                                  const std::set<std::string> *downstreamKeys)
{
	std::ifstream in(path, std::ios::binary);
	if (!in) {
		io_error = "cannot open " + path;
		return {};
	}
	std::ostringstream ss;
	ss << in.rdbuf();
	return checkBuffer(g, path, ss.str(), downstreamKeys);
}

std::vector<UnarmedRule> unarmedRules(const Grammar &g)
{
	std::vector<UnarmedRule> out;
	// unknown-parameter has two halves; the family half arms per command by
	// `subfamily` + a known placement, the argument-key half by `args` data +
	// `args_complete`. Both report what is NOT covered, so the gap is visible.
	int with_args = 0, subfam_unarmed = 0;
	std::vector<std::string> unarmed_names, incomplete_names, sourced_names, free_names;
	for (const auto &name : g.commandLookupList()) {
		const CommandData *cd = g.command(name);
		if (!cd)
			continue;
		if (cd->has_args && cd->args_complete)
			++with_args;
		if (!cd->args_complete)
			incomplete_names.push_back(name);
		if (!cd->args_source.empty())
			sourced_names.push_back(name);
		if (cd->free_keys && cd->args_source.empty())
			free_names.push_back(name);   // one bucket per command
		if (!cd->subfamily.empty() && cd->placement.pos == SubfamilyPosition::Unknown) {
			++subfam_unarmed;
			unarmed_names.push_back(name);
		}
	}
	auto join = [](const std::vector<std::string> &v) {
		std::string s;
		for (const auto &n : v) {
			if (!s.empty()) s += ", ";
			s += n;
		}
		return s;
	};
	if (with_args == 0)
		out.push_back({"unknown-parameter",
		               "argument-key half dormant: no command entry carries `args` data yet; "
		               "it arms per command by data presence, with no code change"});
	else
		out.push_back({"unknown-parameter",
		               "argument-key half ARMED for " + std::to_string(with_args) +
		               " commands; the lines below account for every registered command that is "
		               "not one of them"});
	if (!incomplete_names.empty())
		out.push_back({"unknown-parameter",
		               "argument-key half DELIBERATELY dormant for: " + join(incomplete_names) +
		               " (the entry says args_complete:false \xe2\x80\x94 the rest of the key vocabulary is "
		               "another contract's deliverable, so an unlisted key is not known to be wrong)"});
	if (!sourced_names.empty())
		out.push_back({"unknown-parameter",
		               "argument-key half not needed for: " + join(sourced_names) +
		               " (the keys are a family, checked by the family-name half; running both "
		               "would report the same key twice)"});
	if (!free_names.empty())
		out.push_back({"unknown-parameter",
		               "argument-key half not applicable to: " + join(free_names) +
		               " (these commands have no fixed key list \xe2\x80\x94 the key is a flag name, a "
		               "variable name or free text; the entry's `key_grammar` says which). Nothing "
		               "is missing here: every other registered command IS checked"});
	if (subfam_unarmed) {
		out.push_back({"unknown-parameter",
		               "family-name half unarmed for: " + join(unarmed_names) +
		               " (the contract names the family but not where the name sits in the line, "
		               "and the engine's acceptance test for it was not read)"});
	}
	return out;
}

} // namespace scedit
