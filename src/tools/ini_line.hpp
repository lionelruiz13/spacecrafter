#ifndef INI_LINE_HPP_
#define INI_LINE_HPP_

#include <string>

// Line grammar shared by every reader of the .ini body/system/anchor files
namespace IniLine {

enum class Kind {
    EMPTY,      // Blank or comment-only
    SECTION,    // '[header]', key holds the header text
    ENTRY,      // 'key = value', both trimmed
    MALFORMED,  // No '=', key holds the offending text
};

constexpr char COMMENT_CHAR = '#';

// [begin, end) byte offsets of the value inside the raw line
struct Span {
    std::size_t begin = 0, end = 0;
};

namespace impl {

inline bool isBlank(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

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

// Classify one raw line, valueSpan indexes line itself
inline Kind read(const std::string &line, std::string &key, std::string &value,
                 Span *valueSpan = nullptr)
{
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
