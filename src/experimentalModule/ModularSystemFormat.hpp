#ifndef MODULAR_SYSTEM_FORMAT_HPP_
#define MODULAR_SYSTEM_FORMAT_HPP_

#include "tools/ini_line.hpp"
#include "tools/utility.hpp"
#include <string>
#include <vector>

namespace ModularSystemFormat {

constexpr const char *ANNOTATION_MARKER = "#!sc:";

// One line of a file, verbatim. Kind is what the grammar said it was; `raw` is
// what the file said, and `raw` is what comes back out.
struct Line {
    enum class Kind : char {
        COMMENT,  // its content is entirely a comment
        BLANK,    // nothing but blanks
        KEY,      // 'key = value', possibly with a trailing comment
        RAW,      // anything else - carried verbatim, NEVER interpreted
    };
    Kind kind = Kind::RAW;
    std::string raw;         // the line as read, without its newline
    std::string key, value;  // KEY only, as the grammar returned them
    IniLine::Span valueSpan; // KEY only: where `value` sits inside `raw`
    bool eol = true;         // false only on a last line the file did not end
};

class Section {
public:
    Section() = default;

    static Section fromParams(const std::string &header, const stringHash_t &params);

    // --- identity -----------------------------------------------------
    const std::string &getHeader() const { return header; }
    // Content that belongs to no section: the lines before the file's first
    // '[' (a banner, a note). It declares nothing and a loader must skip it.
    bool isPreamble() const { return header.empty() && rawHeader.empty(); }

    stringHash_t params() const;
    // The effective value of `key`, or nullptr when the section has no such key.
    const std::string *find(const std::string &key) const;
    // Whether the section declares nothing (comments and blanks are not
    // declarations).
    bool empty() const { return index.empty(); }

    bool set(const std::string &key, const std::string &value);
    void remove(const std::string &key, const std::string &reason);

    void annotate(const std::string &key, const std::string &reason, const std::string &text);
    // Whether this section already carries a diagnosis for (key, reason).
    bool hasAnnotation(const std::string &key, const std::string &reason) const;

    // --- construction, for parse and for callers that build a file -----
    void setHeader(const std::string &text) { header = text; }
    // Take a raw line as-is: the grammar classifies it, the index follows.
    void appendRaw(const std::string &rawLine, bool endsWithNewline = true);
    // Same, for a line the grammar has already classified (what `parse` has).
    void append(Line line);
    // Append 'key = value' as a new line. Same representability contract as set.
    bool appendEntry(const std::string &key, const std::string &value);

    bool emit(std::string &out) const;

private:
    struct Annotation {
        std::string key, reason, text;
    };
    void reindex();
    void emitAnnotations(const std::string &key, std::string &out, bool &pending) const;

    std::string header;    // text inside '[...]', decorative (readability only)
    std::string rawHeader; // the header LINE verbatim; empty when this section
                           // did not come from a file (emit then formats one)
    std::vector<Line> lines;                  // THE authority
    std::map<std::string, std::size_t> index; // key -> its effective line: DERIVED
    std::vector<Annotation> annotations;      // what the loader diagnosed here

    friend bool parse(const std::string &path, std::vector<Section> &out);
};

bool parse(const std::string &path, std::vector<Section> &out);

bool write(const std::string &path, const std::vector<Section> &sections,
           const std::vector<std::string> &banner);

} // namespace ModularSystemFormat

#endif /* end of include guard: MODULAR_SYSTEM_FORMAT_HPP_ */
