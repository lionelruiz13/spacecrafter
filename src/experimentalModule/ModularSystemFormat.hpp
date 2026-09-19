#ifndef MODULAR_SYSTEM_FORMAT_HPP_
#define MODULAR_SYSTEM_FORMAT_HPP_

#include "tools/ini_line.hpp"
#include "tools/utility.hpp"
#include <string>
#include <vector>

// Parse and write ssystem.ini-shaped files; a file read then written unchanged is byte-identical
// Callers must only write composed files and sessions, never a file an older build reads
namespace ModularSystemFormat {

// Comment lines starting with it are machine-owned, regenerated at each write
constexpr const char *ANNOTATION_MARKER = "#!sc:";

struct Line {
    enum class Kind : char {
        COMMENT,
        BLANK,
        KEY,      // key = value, possibly with a trailing comment
        RAW,      // Anything else, carried verbatim, never interpreted
    };
    Kind kind = Kind::RAW;
    std::string raw;         // As read, without its newline; written back as is
    std::string key, value;  // KEY only
    IniLine::Span valueSpan; // KEY only, where value sits inside raw
    bool eol = true;         // false only on an unterminated last line
};

class Section {
public:
    Section() = default;

    static Section fromParams(const std::string &header, const stringHash_t &params);

    const std::string &getHeader() const { return header; }
    // Lines before the first '[' of the file, a loader must skip it
    bool isPreamble() const { return header.empty() && rawHeader.empty(); }

    stringHash_t params() const;
    const std::string *find(const std::string &key) const;
    bool empty() const { return index.empty(); }

    // Rewrite in place or append; set and appendEntry refuse a value which can't survive the grammar
    bool set(const std::string &key, const std::string &value);
    // Comment the line out with reason above it, never delete it
    void remove(const std::string &key, const std::string &reason);

    // Put a machine-owned diagnosis above key; the same (key, reason) replaces
    void annotate(const std::string &key, const std::string &reason, const std::string &text);
    bool hasAnnotation(const std::string &key, const std::string &reason) const;

    void setHeader(const std::string &text) { header = text; }
    void appendRaw(const std::string &rawLine, bool endsWithNewline = true);
    void append(Line line);
    bool appendEntry(const std::string &key, const std::string &value);

    // Return true if the last line had no newline, to restore before appending more
    bool emit(std::string &out) const;

private:
    struct Annotation {
        std::string key, reason, text;
    };
    void reindex();
    void emitAnnotations(const std::string &key, std::string &out, bool &pending) const;

    std::string header;    // Text inside [...]
    std::string rawHeader; // Header line verbatim, empty if not from a file
    std::vector<Line> lines;                  // The authority
    std::map<std::string, std::size_t> index; // Derived: key -> its last line
    std::vector<Annotation> annotations;

    friend bool parse(const std::string &path, std::vector<Section> &out);
};

bool parse(const std::string &path, std::vector<Section> &out);

// Atomic; pass no banner for a parsed file, its preamble already holds one
bool write(const std::string &path, const std::vector<Section> &sections,
           const std::vector<std::string> &banner);

} // namespace ModularSystemFormat

#endif /* end of include guard: MODULAR_SYSTEM_FORMAT_HPP_ */
