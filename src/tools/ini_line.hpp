#ifndef INI_LINE_HPP_
#define INI_LINE_HPP_

#include <string>

// THE line grammar of the spacecrafter `.ini` body/system/anchor family - ONE
// authority for every reader of it (INTENT I2; §5.38 / §5.39 / D29 §11.113(h)).
//
// WHAT-FOR: a caller hands one raw file line and gets back what it IS. It never
// needs to know where the '=' sits, how much space surrounds it, whether the
// line ends with a CR, or where a comment starts. Section SEMANTICS stay with
// the caller (each file family means something different by a section); only
// the line grammar lives here.
//
// THE GRAMMAR (readers only - see D13 below):
//   - a '#' starts a comment that runs to the end of the line, anywhere on it;
//     '#' is the family's comment character in every shipped corpus and in
//     every writer in this tree, and no other character is accepted (adding
//     ';' would invent grammar - veto point, INTENT §11.115);
//   - leading/trailing blanks (space, tab, CR, LF) are insignificant, around
//     the whole line and around both sides of the '=';
//   - '[header]' is a section; the text between the brackets is returned;
//   - 'key = value' is an entry, split at the FIRST '=';
//   - a non-empty line that is neither is MALFORMED and is reported as such,
//     so the caller can name it (§2(f)) instead of silently inventing a key.
//
// WHY IT EXISTS. Four readers each carried their own `line.substr(0, pos-1)` /
// `line.substr(pos+2)` arithmetic, which requires exactly one space on each
// side of the '='. On the shipped `galactic.ini` that arithmetic silently
// dropped five minus signs and two leading digits (§5.38: six of seventeen star
// systems at wrong coordinates), and on `ssystem.ini` it disagreed with the
// composed-format reader on seven keys (§5.39). One grammar, one place.
//
// D13 (downgrade must stay possible, §2.0): this is a READING relaxation only.
// Nothing in this tree may WRITE a comment, or any other construct an older
// build's parser cannot read, into a legacy file - what the writers emit is
// unchanged by this header's existence.
//
// ONE WRITER READS THROUGH IT TOO, and that is why `read` reports a Span: a
// rewrite that must preserve the author's line (INTENT §11.66(b)) has to know
// where the VALUE sits inside it, and the only thing that knows is the grammar
// (ModularSystemFormat::Section::set is the client). Asking the writer to
// re-find the '=' itself would be a second copy of this grammar - the exact
// defect this header was created to end (I2).
//
// NOT a consumer of this authority, by construction: the OLD path's own
// `ProtoSystem::load` (protosystem.cpp), which is the frozen comparison
// baseline (§11.52(b)) and must keep reading exactly what it always read.
namespace IniLine {

enum class Kind {
    EMPTY,      // blank or comment-only: nothing to do
    SECTION,    // '[header]' - `key` holds the header text, `value` is cleared
    ENTRY,      // 'key = value' - both trimmed, `key` never empty
    MALFORMED,  // non-empty, no '=' - `key` holds the offending text
};

// The family's comment character, named once so no reader and no writer has to
// spell it again (the veto point above: '#' and nothing else).
constexpr char COMMENT_CHAR = '#';

// Where a piece of text sits inside the RAW line: [begin, end), byte offsets
// into the string that was passed to `read`. Reported for ENTRY only, and only
// for the VALUE - it is what a line-preserving WRITER needs: replacing exactly
// that range leaves the key text, every blank around the '=' and any trailing
// comment byte-for-byte where the author put them (INTENT §11.66(b),
// b31-design §5.2). An empty value yields an empty span at its insertion point.
struct Span {
    std::size_t begin = 0, end = 0;
};

namespace impl {

inline bool isBlank(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

// Shrink [b, e) over `s` until neither end is a blank.
inline void trimRange(const std::string &s, std::size_t &b, std::size_t &e)
{
    while (b < e && isBlank(s[b]))
        ++b;
    while (e > b && isBlank(s[e - 1]))
        --e;
}

inline void trim(std::string &s)
{
    std::size_t b = 0, e = s.size();
    trimRange(s, b, e);
    s = s.substr(b, e - b);
}

} // namespace impl

// Classify one raw file line. Nothing is consumed: `line` is read in place, so
// the offsets `valueSpan` reports index the caller's own string.
inline Kind read(const std::string &line, std::string &key, std::string &value,
                 Span *valueSpan = nullptr)
{
    // The significant content of the line: everything before a comment, blanks
    // stripped off both ends. Every offset below is into `line` itself.
    std::size_t b = 0, e = line.find(COMMENT_CHAR);
    if (e == std::string::npos)
        e = line.size();
    impl::trimRange(line, b, e);
    value.clear();
    if (valueSpan)
        *valueSpan = {b, b};
    if (b == e) {
        key.clear();
        return Kind::EMPTY;
    }
    if (line[b] == '[') {
        const auto close = line.find(']', b);
        key = (close == std::string::npos || close >= e)
            ? line.substr(b + 1, e - b - 1)
            : line.substr(b + 1, close - b - 1);
        return Kind::SECTION;
    }
    const auto eq = line.find('=', b);
    if (eq == std::string::npos || eq >= e) {
        key = line.substr(b, e - b);
        return Kind::MALFORMED;
    }
    std::size_t kb = b, ke = eq;
    impl::trimRange(line, kb, ke);
    if (kb == ke) {            // "= value": no key to bind to
        key = line.substr(b, e - b);
        return Kind::MALFORMED;
    }
    std::size_t vb = eq + 1, ve = e;
    impl::trimRange(line, vb, ve);
    key = line.substr(kb, ke - kb);
    value = line.substr(vb, ve - vb);
    if (valueSpan)
        *valueSpan = {vb, ve};
    return Kind::ENTRY;
}

} // namespace IniLine

#endif /* end of include guard: INI_LINE_HPP_ */
