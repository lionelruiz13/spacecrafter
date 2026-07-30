#ifndef MODULAR_SYSTEM_FORMAT_HPP_
#define MODULAR_SYSTEM_FORMAT_HPP_

#include "tools/ini_line.hpp"
#include "tools/utility.hpp"
#include <string>
#include <vector>

// B24 composed-system file format - the PARSING layer only (INTENT §11.52(c):
// format is parsing-deep, never capability; this layer is a thin, swappable
// reader/writer over the capability model and must never grow engine
// knowledge - it does not know what a key MEANS, only where it lives).
//
// The file shape is the legacy ssystem.ini shape (deliberately - every legacy
// section is a valid ModularBody declaration): '#' comments, '[header]'
// section separators (header text is decorative - identity comes from the
// 'name' key, legacy parity), 'key = value' lines. The LINE grammar is not
// this file's to define: it is `tools/ini_line.hpp`, the one authority every
// reader of the family shares (INTENT §5.39/D29 - the legacy reader
// ModularSystem::loadSystem, the galactic reader and the anchor reader read
// through the same grammar, so a key can no longer mean two things). What is
// this layer's own: sections are returned in file order WITH their headers,
// and nothing is interpreted.
//
// A FILE THAT COMES BACK THROUGH THIS LAYER COMES BACK WHOLE (INTENT §11.66(b),
// b31-design §5.2). Read a file, write it again unchanged, and the result is
// byte-identical: comments, blank lines, spacing, key order, malformed lines,
// keys nothing in this engine understands, the absence of a final newline. The
// author's file is the author's, and an engine that rewrites it owes it back.
// That is why a Section is an ordered LINE LIST and not a map: a map cannot
// hold what it does not understand, and everything a map drops is somebody's
// work. Changing one value therefore rewrites one value, in place, and touches
// nothing else on the line; a key this engine retires is COMMENTED OUT with its
// reason, never deleted (deleting authored text is the D9 destruction class).
//
// WHICH FILES MAY BE WRITTEN AT ALL is not this layer's decision and this layer
// cannot enforce it: composed files and the session file ONLY. The legacy
// ssystem.ini / galactic.ini and anything else an older build's parser reads
// are READ-ONLY forever (D35 §11.113(n), and §2.0 D13: a downgrade must stay
// possible, so nothing - not even a comment - may be written into a file an
// older parser reads). The two contracts are distinct: §11.66(b) protects the
// AUTHOR's content, D13 protects the OLD PARSER's grammar. Callers own D13;
// this layer owns §11.66(b).
//
// What the sections MEAN (type=/body=/relation=/compose=...) is the
// capability layer's contract: ModularSystem::loadComposedSystem. In
// particular the ONE `type=` key selects the declaration kind AND, for a
// module, its family (D16 §11.79(j): type=<family> declares a BodyModule,
// anything else declares a ModularBody node).
// File placement, candidacy and the .ini/.ini.disabled ownership split are
// the SSystemFactory seam's contract (createModularSystem).
//
// Spelling status: the grammar keys are PRODUCT SURFACE (INTENT §2.0 D9).
// The `type=` respell (one key replacing declare=+module=) is Vixy-signed-off
// (D16, INTENT §11.79(j)); the remaining keys were ratified in the same batch.
namespace ModularSystemFormat {

// Comment lines that begin with this marker are MACHINE-OWNED: the writer emits
// them from the annotation set it was handed and drops any it finds before
// re-emitting, which is what makes a rewrite idempotent instead of an inflating
// pile of duplicated diagnostics (b31-design §5.3, check T9). A human comment is
// any other comment and is never touched. Nothing else in this tree may write a
// line starting with it.
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

// One '[header]' section and everything under it, in file order.
//
// THE LINE LIST IS THE AUTHORITY AND THE KEY INDEX IS DERIVED FROM IT (I2).
// Every mutator below goes through the lines; the index is rebuilt from them,
// never edited beside them, so the two cannot drift apart.
class Section {
public:
    Section() = default;

    // Build a section no file produced: one 'key = value' line per entry, in
    // the map's own order. For a generator that owns its output whole (the
    // composed twin) - a file being EDITED is parsed, never rebuilt this way,
    // or the edit would be a rewrite of everything.
    static Section fromParams(const std::string &header, const stringHash_t &params);

    // --- identity -----------------------------------------------------
    const std::string &getHeader() const { return header; }
    // Content that belongs to no section: the lines before the file's first
    // '[' (a banner, a note). It declares nothing and a loader must skip it.
    bool isPreamble() const { return header.empty() && rawHeader.empty(); }

    // --- reading ------------------------------------------------------
    // The section's entries as one map, last occurrence winning (which is what
    // every loader in this tree already does with a repeated key). Derived from
    // the lines on each call: there is no second copy to go stale.
    stringHash_t params() const;
    // The effective value of `key`, or nullptr when the section has no such key.
    const std::string *find(const std::string &key) const;
    // Whether the section declares nothing (comments and blanks are not
    // declarations).
    bool empty() const { return index.empty(); }

    // --- writing ------------------------------------------------------
    // Give `key` the value `value`. A key the section already carries is
    // rewritten IN PLACE: its own key text, the blanks around its '=' and any
    // trailing comment stay exactly as the author wrote them. A key it does not
    // carry is appended at the end of the section.
    // Returns false, changing nothing and saying so in the log, when the value
    // cannot be represented in this format (it carries a comment character, a
    // newline, or blanks the grammar would trim back off): the test is a
    // round-trip through the grammar itself, so it holds for any value, not for
    // a list of characters somebody remembered.
    bool set(const std::string &key, const std::string &value);
    // Retire `key`: its line is commented out and `reason` is written above it.
    // Never deleted - a comment is both preserved and inert, and authored text
    // that an engine deletes is gone (D9). No-op when the key is not there.
    void remove(const std::string &key, const std::string &reason);

    // --- the loader's channel (b31-design §5.3) ------------------------
    // Only the loader knows a datum is wrong, which states are valid, what was
    // applied instead and what would fix it (§2(f)) - so the loader says it and
    // the writer merely places it, ABOVE the datum. Above, because the legacy
    // reader takes a value as the rest of its line: a trailing annotation would
    // be swallowed INTO the value, silently for a number and destructively for
    // a name or a texture path.
    // `reason` is the diagnosis' stable machine key: annotating the same
    // (key, reason) twice REPLACES, so a second load reaching the same verdict
    // produces the same file (T9). D12: an annotation is mandatory wherever a
    // default ACTED, and forbidden where it merely did nothing.
    // `text` may carry newlines; each line becomes its own marked comment line.
    // A `key` the section does not carry is annotated at the section's end -
    // there is no datum to sit above.
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

    // Append this section's bytes to `out`, annotations in place, machine-owned
    // comment lines regenerated. Returns true when the last line written was NOT
    // newline-terminated, so the caller can put the newline back before adding
    // anything after it (a file that ends without one round-trips exactly).
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

// Parse a system file into ordered sections, keeping EVERY line of it in order
// (see the whole-file contract above). Section ORDER is semantic and preserved
// (a parent must be declared before its children, a body before its modules -
// the findBody forward-reference rule); lines before the first '[' become the
// leading preamble section (isPreamble). Returns false when the file cannot be
// opened; a parse never fails past that (unrecognized content is a
// capability-layer concern, not a format one).
bool parse(const std::string &path, std::vector<Section> &out);

// Serialize sections to `path` ATOMICALLY: write to a sibling temporary file
// in the SAME directory (rename atomicity holds same-filesystem only,
// INTENT §11.52(a)), then rename over the target. On any write failure the
// temporary is removed and the pre-existing target is left untouched (the
// disk-full case must discard the temp, never the original). `banner` lines
// are emitted first as '#' comments (pass what the file should tell its
// reader: ownership, regeneration policy, adoption workflow) - a file that was
// PARSED carries its own banner in its preamble already, so a rewrite of one
// passes none.
// This is the ONE serialization authority (INTENT §11.51(a): dual-use writer,
// generation at load AND future script-triggered save go through here).
bool write(const std::string &path, const std::vector<Section> &sections,
           const std::vector<std::string> &banner);

} // namespace ModularSystemFormat

#endif /* end of include guard: MODULAR_SYSTEM_FORMAT_HPP_ */
