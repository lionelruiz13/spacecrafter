#ifndef INI_LINE_HPP_
#define INI_LINE_HPP_

#include <string>

// Line grammar of the `.ini` body/system/anchor family, for every reader of it; section semantics stay with the caller
// '#' comments to the end of the line, blanks are insignificant, an entry splits at the first '='
// Reading only: nothing may write into a legacy file what an older parser cannot read
namespace IniLine {

enum class Kind {
    EMPTY,      // blank or comment-only: nothing to do
    SECTION,    // '[header]' - `key` holds the header text, `value` is cleared
    ENTRY,      // 'key = value' - both trimmed, `key` never empty
    MALFORMED,  // non-empty, no '=' - `key` holds the offending text
};

// The family's comment character, named once for every reader and writer; no other character is accepted
constexpr char COMMENT_CHAR = '#';

// [begin, end) byte offsets of the value inside the raw line, for a writer which keeps the rest of the line as it is
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
