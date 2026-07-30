#include "ModularSystemFormat.hpp"
#include "tools/ini_line.hpp"
#include "tools/log.hpp"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <set>

namespace ModularSystemFormat {

namespace {

// What the grammar says this line is, in this layer's own vocabulary. The
// grammar stays in ini_line.hpp; the mapping lives here, once, so parse and
// every mutator classify identically (I2).
IniLine::Kind classify(Line &line)
{
    IniLine::Span span;
    const IniLine::Kind kind = IniLine::read(line.raw, line.key, line.value, &span);
    switch (kind) {
        case IniLine::Kind::ENTRY:
            line.kind = Line::Kind::KEY;
            line.valueSpan = span;
            break;
        case IniLine::Kind::EMPTY:
            line.kind = (line.raw.find(IniLine::COMMENT_CHAR) == std::string::npos)
                ? Line::Kind::BLANK : Line::Kind::COMMENT;
            break;
        case IniLine::Kind::SECTION:
            // A header is not a line OF a section, it opens one: the caller
            // that cares (parse) reads the header text out of `key`, and
            // Section::append clears it for anything that is stored as a line.
            line.kind = Line::Kind::RAW;
            break;
        case IniLine::Kind::MALFORMED:
            // Neither comment nor entry: carried verbatim, never interpreted.
            line.kind = Line::Kind::RAW;
            line.key.clear();
            line.value.clear();
            break;
    }
    return kind;
}

// Does `candidate` read back as exactly the entry it was built to be? This is
// the representability test, and it is a round-trip through the grammar rather
// than a list of forbidden characters: whatever the grammar would do to the
// text - swallow it into a comment, split it at the wrong '=', trim blanks off
// it - shows up here (I6: the class, not the instance).
// A line terminator is the one thing the round-trip cannot see, because it does
// not survive to be re-read: the grammar is given one line at a time, so text
// carrying a terminator reads back intact and then becomes two lines at emission
// - the second of them something nobody asked for. Only the text being INSERTED
// is tested for it; the rest of the line is the author's and is already one line
// (a CRLF file's '\r' must keep working).
bool readsBackAs(Line &candidate, const std::string &key, const std::string &value)
{
    if (key.find_first_of("\r\n") != std::string::npos
     || value.find_first_of("\r\n") != std::string::npos)
        return false;
    return classify(candidate) == IniLine::Kind::ENTRY
        && candidate.key == key && candidate.value == value;
}

bool isMachineAnnotation(const std::string &raw)
{
    const auto i = raw.find_first_not_of(" \t\r\n");
    return i != std::string::npos
        && raw.compare(i, std::strlen(ANNOTATION_MARKER), ANNOTATION_MARKER) == 0;
}

// Append one line of file text, putting back the newline a previous
// unterminated line still owes (a file that ends without one round-trips
// exactly, and stops doing so the moment something is appended after it).
void putLine(std::string &out, bool &pending, const std::string &text, bool eol = true)
{
    if (pending) {
        out += '\n';
        pending = false;
    }
    out += text;
    if (eol)
        out += '\n';
    else
        pending = true;
}

// A diagnosis is written as prose and prose has line breaks; the file has none
// that are not its own, so each becomes its own comment line.
std::vector<std::string> splitLines(const std::string &text)
{
    std::vector<std::string> out;
    std::size_t pos = 0;
    for (;;) {
        const auto nl = text.find('\n', pos);
        const std::size_t end = (nl == std::string::npos) ? text.size() : nl;
        out.push_back(text.substr(pos, end - pos));
        if (nl == std::string::npos)
            return out;
        pos = nl + 1;
    }
}

} // namespace

// ---------------------------------------------------------------- Section ---

Section Section::fromParams(const std::string &header, const stringHash_t &params)
{
    Section section;
    section.header = header;
    for (const auto &kv : params)
        section.appendEntry(kv.first, kv.second);
    return section;
}

stringHash_t Section::params() const
{
    stringHash_t out;
    for (const auto &kv : index)
        out.emplace(kv.first, lines[kv.second].value);
    return out;
}

const std::string *Section::find(const std::string &key) const
{
    const auto it = index.find(key);
    return (it == index.end()) ? nullptr : &lines[it->second].value;
}

void Section::reindex()
{
    index.clear();
    for (std::size_t i = 0; i < lines.size(); ++i) {
        if (lines[i].kind == Line::Kind::KEY)
            index[lines[i].key] = i; // a repeated key: the last one is effective
    }
}

void Section::append(Line line)
{
    if (line.kind == Line::Kind::KEY) {
        index[line.key] = lines.size();
    } else {
        line.key.clear(); // only an entry binds a name to a value
        line.value.clear();
    }
    lines.push_back(std::move(line));
}

void Section::appendRaw(const std::string &rawLine, bool endsWithNewline)
{
    Line line;
    line.raw = rawLine;
    line.eol = endsWithNewline;
    classify(line);
    append(std::move(line));
}

bool Section::appendEntry(const std::string &key, const std::string &value)
{
    Line candidate;
    candidate.raw = key + " = " + value;
    if (!readsBackAs(candidate, key, value)) {
        cLog::get()->write("ModularSystemFormat: section '[" + header + "]' cannot carry '"
            + key + "' = '" + value + "' - the line it would produce does not read back as "
            "itself. A key/value pair lives on ONE line, holds no '"
            + std::string(1, IniLine::COMMENT_CHAR) + "' (which starts a comment) and no "
            "leading or trailing blank. Nothing was written for this key. To fix: give it a "
            "single-line value without those.", LOG_TYPE::L_ERROR);
        return false;
    }
    // At the end of the section's DECLARATIONS, not after the blank line or the
    // trailing comment that separates it from the next section - a key placed
    // there reads as belonging to whatever follows.
    std::size_t at = lines.size();
    while (at > 0 && lines[at - 1].kind != Line::Kind::KEY)
        --at;
    lines.insert(lines.begin() + at, std::move(candidate));
    reindex();
    return true;
}

bool Section::set(const std::string &key, const std::string &value)
{
    const auto it = index.find(key);
    if (it == index.end())
        return appendEntry(key, value);
    Line &line = lines[it->second];
    if (line.value == value)
        return true; // it already says that: a rewrite of nothing is not a rewrite
    // Splice the new text into the value's own span. Everything else on the
    // line - the key as the author spelled it, the blanks around the '=', a
    // trailing comment - is untouched by construction.
    Line candidate = line;
    candidate.raw = line.raw.substr(0, line.valueSpan.begin);
    if (line.valueSpan.begin == line.valueSpan.end && !candidate.raw.empty()
            && candidate.raw.back() == '=')
        candidate.raw += ' '; // the value was empty: no spacing of its own to keep
    candidate.raw += value;
    candidate.raw.append(line.raw, line.valueSpan.end, std::string::npos);
    if (!readsBackAs(candidate, key, value)) {
        cLog::get()->write("ModularSystemFormat: section '[" + header + "]' cannot take '"
            + key + "' = '" + value + "' - the line it would produce does not read back as "
            "that value. A value holds no '" + std::string(1, IniLine::COMMENT_CHAR)
            + "' (which starts a comment), no newline and no leading or trailing blank. The "
            "line was left as it was ('" + line.raw + "'). To fix: give a value without those.",
            LOG_TYPE::L_ERROR);
        return false;
    }
    line = std::move(candidate);
    return true;
}

void Section::remove(const std::string &key, const std::string &reason)
{
    const auto it = index.find(key);
    if (it == index.end())
        return; // nothing to retire
    const std::size_t pos = it->second;
    // Comment the datum out where it stands: preserved, and inert.
    lines[pos].raw.insert(lines[pos].raw.begin(), IniLine::COMMENT_CHAR);
    classify(lines[pos]);
    std::vector<Line> why;
    for (const auto &text : splitLines(reason)) {
        Line line;
        line.raw = std::string(1, IniLine::COMMENT_CHAR) + " " + key
            + " was retired by spacecrafter: " + text;
        classify(line);
        why.push_back(std::move(line));
    }
    lines.insert(lines.begin() + pos, why.begin(), why.end());
    reindex();
}

void Section::annotate(const std::string &key, const std::string &reason, const std::string &text)
{
    for (auto &a : annotations) {
        if (a.key == key && a.reason == reason) {
            a.text = text; // the same diagnosis said twice is one annotation
            return;
        }
    }
    annotations.push_back({key, reason, text});
}

bool Section::hasAnnotation(const std::string &key, const std::string &reason) const
{
    for (const auto &a : annotations) {
        if (a.key == key && a.reason == reason)
            return true;
    }
    return false;
}

void Section::emitAnnotations(const std::string &key, std::string &out, bool &pending) const
{
    for (const auto &a : annotations) {
        if (a.key != key)
            continue;
        const std::string head = std::string(ANNOTATION_MARKER) + " "
            + (key.empty() ? std::string() : key + " ") + "[" + a.reason + "] ";
        for (const auto &text : splitLines(a.text))
            putLine(out, pending, head + text);
    }
}

bool Section::emit(std::string &out) const
{
    bool pending = false;
    if (!rawHeader.empty()) {
        putLine(out, pending, rawHeader);
    } else if (!header.empty()) {
        // A section this engine built: the shape every generated file has had,
        // one blank line ahead of each header.
        putLine(out, pending, "\n[" + header + "]");
    }
    // A diagnosis about a key this section does not carry has no datum to sit
    // above. It goes after the section's last declaration - not after the blank
    // line or the trailing comment that ends the section, where it would read as
    // belonging to whatever comes next (same rule as a new key).
    std::size_t lastKey = std::string::npos;
    for (std::size_t i = 0; i < lines.size(); ++i) {
        if (lines[i].kind == Line::Kind::KEY)
            lastKey = i;
    }
    std::set<std::string> annotated;
    const auto orphans = [&]() {
        for (const auto &a : annotations) {
            if (!index.count(a.key) && annotated.insert(a.key).second)
                emitAnnotations(a.key, out, pending);
        }
    };
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const Line &line = lines[i];
        if (isMachineAnnotation(line.raw))
            continue; // ours: re-emitted below from what the loader diagnosed
        if (line.kind == Line::Kind::KEY) {
            const auto it = index.find(line.key);
            if (it != index.end() && it->second == i && annotated.insert(line.key).second)
                emitAnnotations(line.key, out, pending);
        }
        putLine(out, pending, line.raw, line.eol);
        if (i == lastKey)
            orphans();
    }
    if (lastKey == std::string::npos)
        orphans(); // a section with no declaration at all
    return pending;
}

// ------------------------------------------------------------ parse/write ---

bool parse(const std::string &path, std::vector<Section> &out)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        return false;
    const std::string content((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
    // The implicit first section (content before any '[') carries the file's
    // banner and its file-level keys; dropped when the file has neither.
    out.clear();
    out.emplace_back();
    std::size_t pos = 0;
    while (pos < content.size()) {
        const auto nl = content.find('\n', pos);
        const bool terminated = (nl != std::string::npos);
        const std::size_t end = terminated ? nl : content.size();
        Line line;
        line.raw = content.substr(pos, end - pos);
        line.eol = terminated;
        pos = terminated ? nl + 1 : content.size();
        if (classify(line) == IniLine::Kind::SECTION) {
            out.emplace_back();
            out.back().header = std::move(line.key);
            out.back().rawHeader = std::move(line.raw);
        } else {
            // EVERY other line, whatever it is: comment, blank, entry, or text
            // this layer cannot read. Dropping one loses somebody's work.
            out.back().append(std::move(line));
        }
    }
    if (out.front().isPreamble() && out.front().lines.empty())
        out.erase(out.begin());
    return true;
}

bool write(const std::string &path, const std::vector<Section> &sections,
           const std::vector<std::string> &banner)
{
    std::string content;
    bool pending = false;
    for (const auto &line : banner)
        putLine(content, pending, std::string(1, IniLine::COMMENT_CHAR) + " " + line);
    for (const auto &section : sections) {
        if (pending) {
            content += '\n';
            pending = false;
        }
        pending = section.emit(content);
    }
    // Sibling temp in the target's own directory (INTENT §11.52(a): rename
    // atomicity is same-filesystem only - never /tmp).
    const std::string tmp = path + ".tmp";
    {
        std::ofstream file(tmp, std::ios::trunc | std::ios::binary);
        if (!file) {
            cLog::get()->write("ModularSystemFormat: can't create temporary file '" + tmp
                + "' - target '" + path + "' left untouched. Check that the directory exists and is writable.",
                LOG_TYPE::L_ERROR);
            return false;
        }
        file.write(content.data(), content.size());
        file.flush();
        if (!file) {
            file.close();
            std::remove(tmp.c_str()); // discard the temp, NEVER the original
            cLog::get()->write("ModularSystemFormat: write to '" + tmp
                + "' failed (disk full?) - temporary discarded, target '" + path + "' left untouched.",
                LOG_TYPE::L_ERROR);
            return false;
        }
    }
    if (std::rename(tmp.c_str(), path.c_str()) != 0) {
        std::remove(tmp.c_str());
        cLog::get()->write("ModularSystemFormat: can't rename '" + tmp + "' over '" + path
            + "' - temporary discarded, target left untouched.", LOG_TYPE::L_ERROR);
        return false;
    }
    return true;
}

} // namespace ModularSystemFormat
