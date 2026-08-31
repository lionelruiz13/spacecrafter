#include "sc_editcore.hpp"

#include <algorithm>
#include <set>

namespace scedit {

std::string Completion::ghost() const
{
	if (!armed || candidates.empty() || selected >= candidates.size())
		return std::string();
	const std::string &c = candidates[selected];
	// The prefix is compared case-insensitively for commands and keys (the
	// engine lowercases both), so compare by LENGTH here: what is already typed
	// occupies the first prefix.size() bytes of the candidate whatever its case.
	if (c.size() <= prefix.size())
		return std::string();
	return c.substr(prefix.size());
}

namespace {

bool startsWith(const std::string &candidate, const std::string &prefix)
{
	return candidate.size() >= prefix.size()
	       && candidate.compare(0, prefix.size(), prefix) == 0;
}

int severityRank(const std::string &s)
{
	if (s == "error") return 3;
	if (s == "warning") return 2;
	if (s == "info") return 1;
	return 0;
}

//! Fill every empty field of `base` from `fallback`. Used to make one spec out
//! of "what the command says about this key" + "what the family says about this
//! name" + "what the command says about keys in general" — most specific first.
void fillFrom(Spec &base, const Spec &fallback)
{
	if (!fallback.present)
		return;
	base.present = true;
	if (!base.doc_known && fallback.doc_known) {
		base.doc = fallback.doc;
		base.doc_known = true;
	}
	if (base.value_domain.empty()) base.value_domain = fallback.value_domain;
	if (base.default_prose.empty()) base.default_prose = fallback.default_prose;
	if (base.required.empty()) base.required = fallback.required;
	if (base.source.empty()) base.source = fallback.source;
	if (base.notes.empty()) base.notes = fallback.notes;
	if (base.values.empty()) base.values = fallback.values;
	if (base.completable.empty()) base.completable = fallback.completable;
	if (base.value_docs.empty()) base.value_docs = fallback.value_docs;
	if (!base.has_default_literal && fallback.has_default_literal) {
		base.has_default_literal = true;
		base.default_literal = fallback.default_literal;
	}
}

} // namespace

bool EditCore::open(const std::string &grammarPath, const std::string &filePath, std::string &err)
{
	if (!grammar_.load(grammarPath, err))
		return false;
	if (!docs_.load(grammarPath, err))
		return false;
	path_ = filePath;
	if (!filePath.empty() && !doc_.loadFile(filePath, err))
		return false;
	cur_ = Cursor();
	afterEdit();
	return true;
}

bool EditCore::openBytes(const std::string &grammarPath, const std::string &label,
                         const std::string &bytes, std::string &err)
{
	if (!grammar_.load(grammarPath, err))
		return false;
	if (!docs_.load(grammarPath, err))
		return false;
	path_ = label;
	doc_ = Document::fromBytes(bytes);
	cur_ = Cursor();
	afterEdit();
	return true;
}

// --- cursor -----------------------------------------------------------------

void EditCore::moveTo(std::size_t line, std::size_t col)
{
	if (doc_.lineCount() == 0)
		return;
	cur_.line = std::min(line, doc_.lineCount() - 1);
	cur_.col = std::min(col, doc_.line(cur_.line).size());
	afterMove();
}

void EditCore::moveLeft()
{
	if (cur_.col > 0)
		moveTo(cur_.line, cur_.col - 1);
	else if (cur_.line > 0)
		moveTo(cur_.line - 1, doc_.line(cur_.line - 1).size());
}

void EditCore::moveRight()
{
	if (cur_.col < doc_.line(cur_.line).size())
		moveTo(cur_.line, cur_.col + 1);
	else if (cur_.line + 1 < doc_.lineCount())
		moveTo(cur_.line + 1, 0);
}

void EditCore::moveUp(std::size_t n)
{
	moveTo(cur_.line >= n ? cur_.line - n : 0, cur_.col);
}

void EditCore::moveDown(std::size_t n)
{
	moveTo(cur_.line + n, cur_.col);
}

void EditCore::moveHome() { moveTo(cur_.line, 0); }
void EditCore::moveEnd() { moveTo(cur_.line, doc_.line(cur_.line).size()); }

// --- editing ----------------------------------------------------------------

void EditCore::insertText(const std::string &bytes)
{
	if (bytes.empty())
		return;
	doc_.insert(cur_.line, cur_.col, bytes);
	cur_.col += bytes.size();
	afterEdit();
}

void EditCore::insertNewline()
{
	doc_.splitLine(cur_.line, cur_.col);
	cur_.line += 1;
	cur_.col = 0;
	afterEdit();
}

void EditCore::backspace()
{
	if (cur_.col > 0) {
		doc_.erase(cur_.line, cur_.col - 1, 1);
		cur_.col -= 1;
		afterEdit();
	} else if (cur_.line > 0) {
		const std::size_t join = doc_.line(cur_.line - 1).size();
		doc_.joinLine(cur_.line - 1);
		cur_.line -= 1;
		cur_.col = join;
		afterEdit();
	}
}

void EditCore::del()
{
	if (cur_.col < doc_.line(cur_.line).size()) {
		doc_.erase(cur_.line, cur_.col, 1);
		afterEdit();
	} else if (cur_.line + 1 < doc_.lineCount()) {
		doc_.joinLine(cur_.line);
		afterEdit();
	}
}

bool EditCore::acceptCompletion()
{
	const std::string g = completion_.ghost();
	if (!g.empty()) {
		insertText(g);
		return true;
	}
	if (completion_.armed && completion_.candidates.size() > 1) {
		cycleCompletion(+1);
		return false;
	}
	return false;
}

void EditCore::cycleCompletion(int delta)
{
	if (completion_.candidates.empty())
		return;
	const std::size_t n = completion_.candidates.size();
	const long s = (long)completion_.selected + delta;
	completion_.selected = (std::size_t)((s % (long)n + (long)n) % (long)n);
}

bool EditCore::save(std::string &err)
{
	if (path_.empty()) {
		err = "no file name: this buffer was not opened from a file";
		return false;
	}
	return doc_.saveFile(path_, err);
}

bool EditCore::saveAs(const std::string &path, std::string &err)
{
	if (!doc_.saveFile(path, err))
		return false;
	path_ = path;
	refreshDiagnostics();
	return true;
}

// --- state recomputation ----------------------------------------------------

void EditCore::afterMove()
{
	// The ENGINE's bytes, '\r' included: the tokenizer must read what
	// parseCommand would read, not what the editing model finds convenient
	// (constraint C1). The caret can never reach that byte — it is past the end
	// of the editable text — so every span below is still a text offset.
	line_ = tokenizeLine(doc_.engineLine(cur_.line));
	computeCompletion();
	computeDocBar();
}

void EditCore::afterEdit()
{
	refreshDiagnostics();
	afterMove();
}

void EditCore::refreshDiagnostics()
{
	diags_ = checkBuffer(grammar_, path_.empty() ? std::string("<buffer>") : path_, doc_.bytes());
}

std::size_t EditCore::commentBegin(std::size_t line) const
{
	return tokenizeLine(doc_.engineLine(line)).comment_begin;
}

MachineTail EditCore::machineTail(std::size_t line) const
{
	MachineTail mt;
	const Line L = tokenizeLine(doc_.engineLine(line));
	// A line the script layer drops whole is never dispatched, so the engine
	// never writes or reads a tail there; only a command line can carry one.
	if (!L.has_command || L.comment_begin == std::string::npos)
		return mt;
	const std::string &raw = doc_.line(line);
	const std::size_t at = raw.find("#!", L.comment_begin);
	if (at == std::string::npos)
		return mt;
	mt.begin = at;
	std::string text = raw.substr(at + 2);
	while (!text.empty() && (text.back() == '\r' || text.back() == ' ' || text.back() == '\t'))
		text.pop_back();
	std::size_t s = 0;
	while (s < text.size() && (text[s] == ' ' || text[s] == '\t'))
		++s;
	mt.text = text.substr(s);

	// The relation, one sentence per engine message (they join with "; ").
	const std::vector<const Diagnostic *> mine = diagnosticsForLine(line + 1);
	std::size_t from = 0;
	while (from <= mt.text.size()) {
		std::size_t sep = mt.text.find("; this ", from);
		std::string sentence = mt.text.substr(from, sep == std::string::npos ? std::string::npos : sep - from);
		if (!sentence.empty()) {
			const LintSeed *seed = grammar_.seedForEngineTail(sentence);
			std::string r;
			if (!seed) {
				r = "not a class scedit checks (the engine's generic channel, or a newer engine)";
			} else {
				bool agree = false;
				for (const auto *d : mine)
					if (d->id == seed->id)
						agree = true;
				r = agree ? "agrees with scedit's " + seed->id
				          : "scedit finds no " + seed->id + " here now: fixed since spacecrafter last ran this "
				            "script (it clears the tail on the next full run), or the two readings disagree - "
				            "worth reporting";
			}
			mt.relation += (mt.relation.empty() ? "" : " | ") + r;
		}
		if (sep == std::string::npos)
			break;
		from = sep + 2;
	}
	return mt;
}

std::vector<const Diagnostic *> EditCore::diagnosticsForLine(std::size_t oneBasedLine) const
{
	std::vector<const Diagnostic *> out;
	for (const auto &d : diags_)
		if (d.line == oneBasedLine)
			out.push_back(&d);
	return out;
}

std::string EditCore::severityForLine(std::size_t oneBasedLine) const
{
	std::string best;
	for (const auto &d : diags_)
		if (d.line == oneBasedLine && severityRank(d.severity) > severityRank(best))
			best = d.severity;
	return best;
}

// --- candidate sources ------------------------------------------------------

Spec EditCore::specForKey(const std::string &command, const std::string &key,
                          std::string *docScope) const
{
	Spec s;
	std::string scope;
	const CommandInfo *ci = docs_.command(command);
	if (ci) {
		auto it = ci->args.find(key);
		if (it != ci->args.end()) {
			s = it->second;
			if (s.doc_known)
				scope = "key";
		}
	}
	const CommandData *cd = grammar_.command(command);
	if (cd && !cd->subfamily.empty()
	    && (cd->placement.pos == SubfamilyPosition::EveryKey
	        || cd->placement.pos == SubfamilyPosition::AppliedKey)) {
		const bool had = s.doc_known;
		fillFrom(s, docs_.familyMember(cd->subfamily, key));
		if (!had && s.doc_known)
			scope = "key";
	}
	if (ci && ci->has_key_grammar) {
		const bool had = s.doc_known;
		fillFrom(s, ci->key_grammar);
		if (!had && s.doc_known)
			scope = "any key of `" + command + "`";
	}
	if (docScope)
		*docScope = scope;
	return s;
}

std::vector<std::string> EditCore::keyCandidates(const std::string &command,
                                                 const std::string &excluding, Source &src) const
{
	src = Source();
	src.what = "key";
	const CommandData *cd = grammar_.command(command);
	if (!cd)
		return {};   // unknown command: nothing is known, so nothing is offered

	std::vector<std::string> base;
	if (!cd->subfamily.empty()
	    && (cd->placement.pos == SubfamilyPosition::EveryKey
	        || cd->placement.pos == SubfamilyPosition::AppliedKey)) {
		const FamilyData *f = grammar_.family(cd->subfamily);
		if (f) {
			base = f->sorted;
			// The family IS the accepted vocabulary: an unregistered name is
			// refused by the engine, which is why `unknown-parameter` fires on it.
			src.openness = Openness::Exhaustive;
			src.family = cd->subfamily;
			src.what = "name from families." + cd->subfamily;
		}
	} else if (!cd->arg_keys_sorted.empty()) {
		base = cd->arg_keys_sorted;
		src.openness = grammar_.argKeysAreExhaustive(command) ? Openness::Exhaustive : Openness::Open;
		src.what = "key of `" + command + "`";
	}

	// A key already written on this line is not offered again: a second one is a
	// finding (`duplicate-key`), not a suggestion. The key being edited is of
	// course still offered.
	std::set<std::string> present;
	for (const auto &p : line_.pairs)
		present.insert(line_.tokens[p.key].text);
	if (line_.has_dangling)
		present.insert(line_.tokens[line_.dangling_index].text);
	present.erase(excluding);

	std::vector<std::string> out;
	for (const auto &n : base)
		if (!present.count(n))
			out.push_back(n);
	return out;
}

std::vector<std::string> EditCore::valueCandidates(const std::string &command,
                                                   const std::string &key, Source &src) const
{
	src = Source();
	src.what = key.empty() ? std::string("value") : "value of `" + key + "`";

	const CommandData *cd = grammar_.command(command);
	if (cd && cd->placement.pos == SubfamilyPosition::ValueOfKey
	    && !cd->placement.position_key.empty() && key == cd->placement.position_key) {
		const FamilyData *f = grammar_.family(cd->subfamily);
		if (f) {
			src.openness = Openness::Exhaustive;
			src.family = cd->subfamily;
			src.what = "name from families." + cd->subfamily;
			return f->sorted;
		}
	}

	const Spec s = specForKey(command, key);
	std::vector<std::string> out;
	// D31: the default comes first, so that the ghost on an empty field shows
	// the default. Dormant until a spec carries `default_value` (see
	// DocIndex::dormantFeatures) — the rest is byte-lexicographic.
	if (s.has_default_literal && isCompletableLiteral(s.default_literal))
		out.push_back(s.default_literal);
	for (const auto &c : s.completable)
		if (!(s.has_default_literal && c == s.default_literal))
			out.push_back(c);
	return out;
}

// --- the two things the renderer draws --------------------------------------

void EditCore::computeCompletion()
{
	completion_ = Completion();
	const std::string &raw = doc_.line(cur_.line);

	if (line_.kind == LineKind::Comment) {
		completion_.context = Context::CommentLine;
		return;
	}
	// In the comment after a '#': the engine reads none of it, so nothing
	// completes — a ghost there would be a promise about bytes with no meaning.
	if (cur_.col >= line_.comment_begin) {
		const MachineTail mt = machineTail(cur_.line);
		completion_.context = (mt.present() && cur_.col >= mt.begin) ? Context::MachineTail : Context::Comment;
		completion_.anchor = Span{cur_.col, cur_.col};
		return;
	}

	const Token *anchor = line_.tokenTouchingRawColumn(cur_.col);
	const bool atTokenEnd = anchor && cur_.col == anchor->span.end;
	// Everything after the last token is a place where appending is safe.
	const bool atLineTail = line_.tokens.empty()
	                        || cur_.col >= line_.tokens.back().span.end;

	std::string command = line_.command;
	std::string key;
	bool armable = false;

	if (anchor) {
		completion_.anchor = anchor->span;
		switch (anchor->role) {
		case TokenRole::Command:
			completion_.context = Context::CommandName;
			break;
		case TokenRole::Key:
		case TokenRole::DanglingKey:
			completion_.context = Context::ArgKey;
			break;
		case TokenRole::Value:
			completion_.context = Context::ArgValue;
			for (const auto &p : line_.pairs)
				if (&line_.tokens[p.value] == anchor)
					key = line_.tokens[p.key].text;
			break;
		}
		// Only at the end of the word: appending in the middle of `zo|om`
		// produces something the author did not ask for.
		armable = atTokenEnd && !(anchor->role == TokenRole::Value && anchor->quoted);
		if (armable)
			completion_.prefix = raw.substr(anchor->span.begin, cur_.col - anchor->span.begin);
	} else {
		// In whitespace. What comes next is decided by what came before.
		const Token *prev = nullptr;
		for (const auto &t : line_.tokens)
			if (t.span.end <= cur_.col)
				prev = &t;
		completion_.anchor = Span{cur_.col, cur_.col};
		if (!prev) {
			completion_.context = Context::CommandName;
		} else if (prev->role == TokenRole::Key || prev->role == TokenRole::DanglingKey) {
			completion_.context = Context::EmptyValue;
			key = prev->text;
		} else {
			completion_.context = line_.has_command ? Context::NewKey : Context::CommandName;
		}
		armable = atLineTail;
	}

	if (!armable)
		return;

	std::vector<std::string> base;
	Source src;
	switch (completion_.context) {
	case Context::CommandName:
		base = docs_.commandNames();
		src.openness = Openness::Exhaustive;
		src.what = "command";
		break;
	case Context::ArgKey:
	case Context::NewKey:
		// The key being edited is not "already present" for its own sake.
		base = keyCandidates(command,
		                     (completion_.context == Context::ArgKey && anchor)
		                         ? anchor->text : std::string(),
		                     src);
		break;
	case Context::ArgValue:
	case Context::EmptyValue:
		base = valueCandidates(command, key, src);
		break;
	default:
		return;
	}
	completion_.openness = src.openness;
	completion_.what = src.what;
	completion_.family = src.family;

	// Filter by what is typed. Commands and keys are lowercased by the engine,
	// so their matching is case-insensitive; a value keeps its case and is
	// matched byte for byte.
	const bool caseless = completion_.context != Context::ArgValue
	                      && completion_.context != Context::EmptyValue;
	const std::string want = caseless ? asciiLower(completion_.prefix) : completion_.prefix;
	for (const auto &c : base)
		if (startsWith(c, want))
			completion_.candidates.push_back(c);

	completion_.armed = !completion_.candidates.empty();
}

void EditCore::computeDocBar()
{
	docbar_ = DocBar();

	if (line_.kind == LineKind::Comment) {
		docbar_.path = "comment line";
		docbar_.doc = docs_.commentLineDoc();
		docbar_.documented = !docbar_.doc.empty();
		docbar_.doc_of = docbar_.documented ? "comment" : "";
		docbar_.source = "src/scriptModule/script.cpp:114";
		return;
	}
	// The `#!` tail, wherever the caret is on its line: the engine's sentence
	// and how it relates to what scedit finds here (the C1 signal).
	{
		const MachineTail mt = machineTail(cur_.line);
		if (mt.present())
			docbar_.annotation = mt.text + " -- " + mt.relation;
	}
	if (completion_.context == Context::MachineTail) {
		docbar_.path = "#! annotation, written by spacecrafter";
		docbar_.doc = docs_.machineTailDoc();
		docbar_.documented = !docbar_.doc.empty();
		docbar_.doc_of = docbar_.documented ? "comment" : "";
		docbar_.source = "src/scriptModule/script_annotator.hpp (parse_model.comments.machine_tail)";
		return;
	}
	if (completion_.context == Context::Comment) {
		docbar_.path = "comment";
		docbar_.doc = docs_.commentTailDoc();
		docbar_.documented = !docbar_.doc.empty();
		docbar_.doc_of = docbar_.documented ? "comment" : "";
		docbar_.source = "parseCommand (parse_model.comments.mid_line)";
		return;
	}

	const Token *anchor = line_.tokenTouchingRawColumn(cur_.col);
	const Context ctx = completion_.context;
	const std::string &command = line_.command;

	// The command word, wherever the caret is on the line, whenever it is not
	// itself the thing being documented.
	const CommandInfo *ci = docs_.command(command);
	const CommandData *cd = grammar_.command(command);

	auto describeKey = [&](const std::string &key) {
		std::string scope;
		const Spec s = specForKey(command, key, &scope);
		docbar_.path = "`" + command + "` " + (key.empty() ? "<key>" : "`" + key + "`");
		docbar_.doc = s.doc;
		docbar_.documented = s.doc_known;
		docbar_.doc_of = scope;
		docbar_.domain = s.value_domain;
		docbar_.values = s.values;
		docbar_.def = s.default_prose;
		docbar_.required = s.required;
		docbar_.source = s.source;
		if (!cd) {
			docbar_.note = command.empty()
			               ? "this line has no command, so nothing here has a meaning yet"
			               : "`" + command + "` is not a command the engine registers, so "
			                 "scedit knows nothing about its keys";
			return;
		}
		Source src;
		const std::vector<std::string> known = keyCandidates(command, key, src);
		if (src.openness == Openness::Open) {
			docbar_.note = "`" + command + "` accepts keys beyond the ones scedit knows "
			               "(args_complete: false), so an unlisted one is never called wrong";
		} else if (!key.empty() && src.openness == Openness::Exhaustive
		           && std::find(known.begin(), known.end(), key) == known.end()) {
			docbar_.note = src.family.empty()
			               ? "`" + key + "` is not an argument of `" + command + "`"
			               : "`" + key + "` is not a name in families." + src.family;
		}
	};

	switch (ctx) {
	case Context::CommandName: {
		const std::string name = anchor ? anchor->text : std::string();
		docbar_.path = name.empty() ? "command" : "command `" + name + "`";
		if (name.empty()) {
			docbar_.note = "the line has no command yet";
			return;
		}
		if (!grammar_.isCommand(name)) {
			docbar_.note = grammar_.isObsolete(name)
			               ? "the contract file lists `" + name + "` as an obsolete token"
			               : "`" + name + "` is not a command the engine registers";
			return;
		}
		const CommandInfo *n = docs_.command(name);
		if (n) {
			docbar_.doc = n->doc;
			docbar_.documented = n->doc_known;
			docbar_.doc_of = n->doc_known ? "command" : "";
			docbar_.source = n->registration;
			if (!n->args_source.empty())
				docbar_.note = n->args_source;
			else if (!n->args_complete)
				docbar_.note = "scedit knows only some of this command's keys "
				               "(args_complete: false), so it never calls one of them unknown";
		}
		break;
	}
	case Context::ArgKey:
		describeKey(anchor ? anchor->text : std::string());
		break;
	case Context::NewKey:
		describeKey(std::string());
		if (ci && ci->has_key_grammar && !ci->key_grammar.doc.empty()) {
			docbar_.doc = ci->key_grammar.doc;
			docbar_.documented = ci->key_grammar.doc_known;
			docbar_.doc_of = "any key of `" + command + "`";
		} else if (!docbar_.documented && !completion_.candidates.empty()) {
			// No sentence exists about this command's keys in general. Say what
			// IS known — the list — instead of a blank the author cannot act on.
			docbar_.note = "Tab offers the " + std::to_string(completion_.candidates.size())
			               + " keys scedit knows for `" + command + "`";
		}
		break;
	case Context::EmptyValue: {
		std::string key;
		for (const auto &t : line_.tokens)
			if (t.span.end <= cur_.col
			    && (t.role == TokenRole::Key || t.role == TokenRole::DanglingKey))
				key = t.text;
		describeKey(key);
		docbar_.path += " = <no value yet>";
		break;
	}
	case Context::ArgValue: {
		std::string key;
		for (const auto &p : line_.pairs)
			if (&line_.tokens[p.value] == anchor)
				key = line_.tokens[p.key].text;
		const std::string value = anchor ? anchor->text : std::string();
		describeKey(key);
		docbar_.path = "`" + command + "` `" + key + "` = `" + value + "`";
		// A family name as a value (color property X) documents itself from the
		// family, not from the key.
		if (cd && cd->placement.pos == SubfamilyPosition::ValueOfKey
		    && key == cd->placement.position_key) {
			const Spec fs = docs_.familyMember(cd->subfamily, value);
			if (fs.present) {
				docbar_.doc = fs.doc;
				docbar_.documented = fs.doc_known;
				docbar_.doc_of = fs.doc_known ? "value" : "";
				if (!fs.source.empty())
					docbar_.source = fs.source;
			} else {
				docbar_.note = "`" + value + "` is not a name of families." + cd->subfamily;
			}
			break;
		}
		std::string vd;
		const Spec s = specForKey(command, key);
		if (s.valueDoc(value, vd)) {
			docbar_.doc = vd;
			docbar_.documented = true;
			docbar_.doc_of = "value";
		}
		break;
	}
	default:
		break;
	}
}

} // namespace scedit
