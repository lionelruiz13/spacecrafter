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

namespace impl {

inline void trim(std::string &s)
{
    constexpr const char *BLANK = " \t\r\n";
    const auto b = s.find_first_not_of(BLANK);
    if (b == std::string::npos) {
        s.clear();
        return;
    }
    const auto e = s.find_last_not_of(BLANK);
    s = s.substr(b, e - b + 1);
}

} // namespace impl

// Classify one raw file line. `line` is taken by value: it is consumed here.
inline Kind read(std::string line, std::string &key, std::string &value)
{
    const auto comment = line.find('#');
    if (comment != std::string::npos)
        line.erase(comment);
    impl::trim(line);
    value.clear();
    if (line.empty()) {
        key.clear();
        return Kind::EMPTY;
    }
    if (line.front() == '[') {
        const auto close = line.find(']');
        key = (close == std::string::npos) ? line.substr(1) : line.substr(1, close - 1);
        return Kind::SECTION;
    }
    const auto eq = line.find('=');
    if (eq == std::string::npos) {
        key = std::move(line);
        return Kind::MALFORMED;
    }
    key = line.substr(0, eq);
    value = line.substr(eq + 1);
    impl::trim(key);
    impl::trim(value);
    if (key.empty()) {         // "= value": no key to bind to
        value.clear();
        key = std::move(line);
        return Kind::MALFORMED;
    }
    return Kind::ENTRY;
}

} // namespace IniLine

#endif /* end of include guard: INI_LINE_HPP_ */
