/*
 * scedit — sc_tokenizer.cpp
 *
 * Derivation of AppCommandInterface::parseCommand (app_command_interface.cpp
 * :124-176) and of the script-layer comment rule (script.cpp:114).
 * Every clause is mapped in util/scedit/tests/derivation-diff.md; the comments
 * below carry the engine line numbers so the mapping is checkable in place.
 *
 * Method note (why this is written as a simulation and not as arithmetic):
 * the engine mutates the line before tokenizing it, so raw byte offsets shift.
 * Rather than reason about the shift, the normalisation is replayed on a
 * (buffer, offset-vector) pair — every surviving byte carries its own raw
 * offset, so the raw span of a token is read off, never computed. The rule that
 * produced the shift can change without the mapping needing a second proof.
 */

#include "sc_tokenizer.hpp"

#include <algorithm>
#include <numeric>

namespace scedit {

namespace {

//! std::istringstream's whitespace, C locale (std::locale::global() is never
//! touched: src/main.cpp:242 sets LC_TIME only).
inline bool isStreamSpace(unsigned char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

/*!
 * The std::istringstream the engine parses with, reduced to the two operations
 * parseCommand uses: `>> token` (:145, :148) and `.get()` (:159).
 * `good_` models the stream's good() bit as far as the quote loop reads it.
 */
class StreamSim {
public:
	StreamSim(const std::string &s) : s_(s) {}

	//! operator>>(std::string&). Returns false on failure (nothing extracted);
	//! on failure the engine's target string keeps its previous value, which is
	//! why a dangling key never reaches `arguments`.
	bool extract(std::string &out, std::size_t &begin, std::size_t &end)
	{
		while (p_ < s_.size() && isStreamSpace(static_cast<unsigned char>(s_[p_])))
			++p_;
		if (p_ >= s_.size()) {
			good_ = false;         // eofbit|failbit
			return false;
		}
		begin = p_;
		while (p_ < s_.size() && !isStreamSpace(static_cast<unsigned char>(s_[p_])))
			++p_;
		end = p_;
		out.assign(s_, begin, end - begin);
		// operator>> sets eofbit when the token ran to the end of the stream.
		// The quote loop tests good(), so model it.
		if (p_ >= s_.size())
			good_ = false;
		return true;
	}

	//! istream::get(). Returns -1 (EOF) and drops good() when there is nothing
	//! left — exactly the condition the engine's quote loop breaks on.
	int get()
	{
		if (p_ < s_.size())
			return static_cast<unsigned char>(s_[p_++]);
		good_ = false;
		return -1;
	}

	bool good() const { return good_; }
	std::size_t pos() const { return p_; }

private:
	const std::string &s_;
	std::size_t p_ = 0;
	bool good_ = true;
};

} // namespace

// ---------------------------------------------------------------------------
// small engine predicates
// ---------------------------------------------------------------------------

std::string asciiLower(const std::string &s)
{
	std::string r = s;
	for (char &c : r)
		if (c >= 'A' && c <= 'Z')
			c = static_cast<char>(c - 'A' + 'a');
	return r;
}

bool isTrueValue(const std::string &v)
{
	// Utility::isTrue, utility.hpp:160 — masking each byte with 0x5f and
	// comparing to "TRUE"/"ON" is exactly a bit-5-insensitive byte compare.
	switch (v.size()) {
		case 4: {
			static const char t[4] = {'T', 'R', 'U', 'E'};
			for (int i = 0; i < 4; ++i)
				if ((static_cast<unsigned char>(v[i]) & 0x5f) != static_cast<unsigned char>(t[i]))
					return false;
			return true;
		}
		case 2: {
			static const char t[2] = {'O', 'N'};
			for (int i = 0; i < 2; ++i)
				if ((static_cast<unsigned char>(v[i]) & 0x5f) != static_cast<unsigned char>(t[i]))
					return false;
			return true;
		}
		case 1:
			return v[0] == '1';
		default:
			return false;
	}
}

bool isFalseValue(const std::string &v)
{
	// Utility::isFalse, utility.hpp:172. Note the engine masks 3 bytes with
	// 0x5f5f5f for the size-3 case ("OFF"); byte-wise is the same test.
	switch (v.size()) {
		case 5: {
			static const char t[5] = {'F', 'A', 'L', 'S', 'E'};
			for (int i = 0; i < 5; ++i)
				if ((static_cast<unsigned char>(v[i]) & 0x5f) != static_cast<unsigned char>(t[i]))
					return false;
			return true;
		}
		case 3: {
			static const char t[3] = {'O', 'F', 'F'};
			for (int i = 0; i < 3; ++i)
				if ((static_cast<unsigned char>(v[i]) & 0x5f) != static_cast<unsigned char>(t[i]))
					return false;
			return true;
		}
		case 1:
			return v[0] == '0';
		default:
			return false;
	}
}

std::size_t levenshtein(const std::string &a, const std::string &b)
{
	// AppCommandInit::LevensteinDistance, app_command_init.cpp:337-365.
	const std::string &source = (a.size() > b.size()) ? b : a;
	const std::string &target = (a.size() > b.size()) ? a : b;
	const std::size_t min_size = source.size(), max_size = target.size();
	std::vector<std::size_t> lev(min_size + 1);
	for (std::size_t i = 0; i <= min_size; ++i)
		lev[i] = i;
	for (std::size_t j = 1; j <= max_size; ++j) {
		std::size_t previous_diagonal = lev[0], previous_diagonal_save;
		++lev[0];
		for (std::size_t i = 1; i <= min_size; ++i) {
			previous_diagonal_save = lev[i];
			if (source[i - 1] == target[j - 1])
				lev[i] = previous_diagonal;
			else
				lev[i] = std::min(std::min(lev[i - 1], lev[i]), previous_diagonal) + 1;
			previous_diagonal = previous_diagonal_save;
		}
	}
	return lev[min_size];
}

std::string nearestNeighbour(const std::string &source, const std::vector<std::string> &candidates)
{
	// AppCommandInit::searchNeighbour, app_command_init.cpp:368-387.
	// `minDistance = 99999`, strict `<`: the FIRST candidate at the minimum
	// wins, so the caller's order is part of the answer (the engine feeds
	// std::map key order = alphabetical).
	std::size_t minDistance = 99999;
	std::string solution;
	for (const auto &c : candidates) {
		std::size_t d = levenshtein(source, c);
		if (d < minDistance) {
			minDistance = d;
			solution = c;
		}
	}
	return solution;
}

// ---------------------------------------------------------------------------
// line classification and splitting
// ---------------------------------------------------------------------------

LineKind classifyLine(const std::string &raw)
{
	// script.cpp:114 — `line[0]` on an empty std::string is the NUL terminator,
	// which is exactly the `line[0] != 0` arm of the test.
	const char c = raw.empty() ? '\0' : raw[0];
	if (c == '#')
		return LineKind::Comment;
	if (c == '\0' || c == '\r' || c == '\n')
		return LineKind::Blank;
	return LineKind::Parsed;
}

std::vector<std::string> splitScriptLines(const std::string &file_bytes)
{
	// Script::loadInternal (script.cpp:110-120) reads with std::getline on a
	// text-mode ifstream: '\n' is the delimiter and is consumed, '\r' is not.
	std::vector<std::string> out;
	std::size_t start = 0;
	while (start <= file_bytes.size()) {
		std::size_t nl = file_bytes.find('\n', start);
		if (nl == std::string::npos) {
			// Trailing bytes with no newline: getline returns them, then eof
			// ends the loop. Nothing at all: the extra getline of the engine's
			// `while(!eof())` clears `line` and the script layer drops it.
			if (start < file_bytes.size())
				out.push_back(file_bytes.substr(start));
			break;
		}
		out.push_back(file_bytes.substr(start, nl - start));
		start = nl + 1;
	}
	return out;
}

// ---------------------------------------------------------------------------
// Line: raw <-> parsed mapping
// ---------------------------------------------------------------------------

const Token *Line::tokenAtRawColumn(std::size_t raw_off) const
{
	for (const auto &t : tokens)
		if (t.span.contains(raw_off))
			return &t;
	return nullptr;
}

const Token *Line::tokenTouchingRawColumn(std::size_t raw_off) const
{
	for (const auto &t : tokens)
		if (t.span.touches(raw_off))
			return &t;
	return nullptr;
}

bool Line::rawToNormalized(std::size_t raw_off, std::size_t &norm_off) const
{
	// rawOfNorm is strictly increasing, so a binary search answers both
	// "which normalized byte is this" and "was it erased".
	auto it = std::lower_bound(rawOfNorm.begin(), rawOfNorm.end(), raw_off);
	if (it == rawOfNorm.end() || *it != raw_off)
		return false;
	norm_off = static_cast<std::size_t>(it - rawOfNorm.begin());
	return true;
}

std::size_t Line::normalizedToRaw(std::size_t norm_off) const
{
	if (norm_off < rawOfNorm.size())
		return rawOfNorm[norm_off];
	// one-past-the-end maps one past the last surviving raw byte
	return rawOfNorm.empty() ? raw.size() : rawOfNorm.back() + 1;
}

std::string Line::rawText(const Span &s) const
{
	if (s.begin >= raw.size())
		return std::string();
	return raw.substr(s.begin, std::min(s.end, raw.size()) - s.begin);
}

// ---------------------------------------------------------------------------
// tokenizeLine — the derivation of parseCommand
// ---------------------------------------------------------------------------

Line tokenizeLine(const std::string &raw)
{
	Line line;
	line.raw = raw;
	line.kind = classifyLine(raw);
	if (line.kind != LineKind::Parsed)
		return line;   // the script layer never hands this to parseCommand

	// --- normalisation, replayed on (buffer, raw-offset) pairs --------------
	std::string s = raw;
	std::vector<std::size_t> off(s.size());
	std::iota(off.begin(), off.end(), std::size_t(0));

	// :129-132 — strip leading spaces and tabs (and ONLY those: a leading '\r'
	// survives, which is why the script layer filters it first).
	{
		std::size_t k = 0;
		while (k < s.size() && (s[k] == ' ' || s[k] == '\t'))
			++k;
		if (k) {
			s.erase(0, k);
			off.erase(off.begin(), off.begin() + k);
		}
	}

	// :135-139 — every occurrence of ' " ' loses the byte AFTER the quote.
	// The engine re-searches from the start after each erase; replayed as-is,
	// because a cascade (one erase creating a new match) is engine behaviour.
	{
		std::size_t found = s.find(" \" ");
		while (found != std::string::npos) {
			s.erase(found + 2, 1);
			off.erase(off.begin() + found + 2);
			found = s.find(" \" ");
		}
	}
	// `erased` is read back off the surviving-offset vector, so it covers BOTH
	// normalisation steps and cannot drift from the map they produced.
	{
		std::vector<std::size_t> all;
		std::size_t next_kept = 0;
		for (std::size_t i = 0; i < raw.size(); ++i) {
			if (next_kept < off.size() && off[next_kept] == i)
				++next_kept;
			else
				all.push_back(i);
		}
		line.erased = std::move(all);
	}
	line.normalized = s;
	line.rawOfNorm = off;

	auto rawSpan = [&](std::size_t nb, std::size_t ne) -> Span {
		Span sp;
		sp.begin = (nb < off.size()) ? off[nb] : raw.size();
		sp.end = (ne == 0) ? sp.begin
		                   : ((ne - 1 < off.size()) ? off[ne - 1] + 1 : raw.size());
		return sp;
	};

	// --- tokenisation -------------------------------------------------------
	StreamSim stream(s);

	// :145-146 — the command token, lowercased. On failure (whitespace-only
	// line) the engine's `command` keeps the value executeCommand cleared it
	// to: empty, and executeCommand:207 returns 0 without doing anything.
	{
		std::string tok;
		std::size_t b = 0, e = 0;
		if (stream.extract(tok, b, e)) {
			Token t;
			t.role = TokenRole::Command;
			t.text = asciiLower(tok);
			t.span = rawSpan(b, e);
			line.tokens.push_back(t);
			line.command = t.text;
			line.has_command = true;
		}
	}

	// :148-167 — `while (commandstr >> key >> value)`.
	int pair_index = 0;
	while (true) {
		std::string key, value;
		std::size_t kb = 0, ke = 0, vb = 0, ve = 0;

		if (!stream.extract(key, kb, ke))
			break;                       // no key left: loop ends, nothing dropped

		if (!stream.extract(value, vb, ve)) {
			// :148 — the value extraction failed mid-pair, so the loop body
			// never runs and this key is SILENTLY DROPPED by the engine.
			Token t;
			t.role = TokenRole::DanglingKey;
			t.text = asciiLower(key);
			t.span = rawSpan(kb, ke);
			t.pair = pair_index;
			line.dangling_index = line.tokens.size();
			line.has_dangling = true;
			line.tokens.push_back(t);
			break;
		}

		Token vt;
		vt.role = TokenRole::Value;
		vt.pair = pair_index;
		std::size_t v_end_norm = ve;

		if (value[0] == '"') {                                    // :149
			vt.quoted = true;
			if (value[value.length() - 1] == '"') {               // :151
				// one word in quotes. NOTE the degenerate case the engine
				// hits for a lone `"`: length 1, so this branch is taken and
				// substr(1, (size_t)-1) yields "" — a lone quote is an EMPTY
				// value, not an opening quote.
				value = value.substr(1, value.length() - 2);      // :153
				vt.quote_closed = true;
			} else {
				value = value.substr(1, value.length() - 1);      // :156
				while (true) {                                    // :158-162
					int nextc = stream.get();
					if (nextc == '"') {
						vt.quote_closed = true;
						break;
					}
					if (!stream.good())
						break;
					value.push_back(static_cast<char>(nextc));
				}
				v_end_norm = stream.pos();
				if (!vt.quote_closed)
					line.has_unclosed_quote = true;
			}
		}

		Token kt;
		kt.role = TokenRole::Key;
		kt.text = asciiLower(key);                                // :165
		kt.span = rawSpan(kb, ke);
		kt.pair = pair_index;

		vt.text = value;
		vt.span = rawSpan(vb, v_end_norm);

		Line::Pair p;
		p.key = line.tokens.size();
		line.tokens.push_back(kt);
		p.value = line.tokens.size();
		line.tokens.push_back(vt);
		line.pairs.push_back(p);

		line.args[kt.text] = vt.text;                             // :166 (map: last wins)
		++pair_index;
	}

	return line;
}

// ---------------------------------------------------------------------------
// BlockSkipState
// ---------------------------------------------------------------------------

bool BlockSkipState::feed(const Line &line)
{
	if (line.kind != LineKind::Parsed || !line.has_command || line.command.empty())
		return false;                       // executeCommand:207 returns 0

	// executeCommand:215-222 — three literal comparisons BEFORE the skip test,
	// so these three always run, skipping or not.
	if (line.command == "comment") {        // :215-216 -> commandComment (:3160)
		skipping_ = true;
		return false;
	}
	if (line.command == "uncomment") {      // :218-219 -> commandUncomment (:3167)
		skipping_ = false;
		return false;
	}
	if (line.command == "struct") {         // :221-222 -> commandStruct (:4600)
		auto itIf = line.args.find("if");
		const bool if_case = (itIf != line.args.end() && !itIf->second.empty() && !skipping_);
		if (!if_case) {                     // :4664-4672, the comment case
			auto itC = line.args.find("comment");
			if (itC != line.args.end() && !itC->second.empty())
				skipping_ = isTrueValue(itC->second);
		}
		// `struct loop` also moves the same flag (:4691-4693) but only for
		// runtime-evaluated counts — see the header note; not modelled.
		return false;
	}

	return skipping_;                       // :225-228
}

} // namespace scedit
