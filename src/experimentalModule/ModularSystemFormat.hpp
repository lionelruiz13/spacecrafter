#ifndef MODULAR_SYSTEM_FORMAT_HPP_
#define MODULAR_SYSTEM_FORMAT_HPP_

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
// 'name' key, legacy parity), 'key = value' lines. Differences from the
// legacy reader (ModularSystem::loadSystem, which stays UNTOUCHED): sections
// are returned in file order WITH their headers, whitespace around '=' is
// tolerated (the legacy reader requires exactly "key = value"), and nothing
// is interpreted.
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

struct Section {
    std::string header;  // text inside '[...]', decorative (readability only)
    stringHash_t params; // key/value pairs of the section, unordered (key
                         // order inside a section is not semantic)
};

// Parse a system file into ordered sections. Section ORDER is semantic and
// preserved (a parent must be declared before its children, a body before its
// modules - the findBody forward-reference rule). Returns false when the file
// cannot be opened; a parse never fails past that (unrecognized content is a
// capability-layer concern, not a format one).
bool parse(const std::string &path, std::vector<Section> &out);

// Serialize sections to `path` ATOMICALLY: write to a sibling temporary file
// in the SAME directory (rename atomicity holds same-filesystem only,
// INTENT §11.52(a)), then rename over the target. On any write failure the
// temporary is removed and the pre-existing target is left untouched (the
// disk-full case must discard the temp, never the original). `banner` lines
// are emitted first as '#' comments (pass what the file should tell its
// reader: ownership, regeneration policy, adoption workflow).
// This is the ONE serialization authority (INTENT §11.51(a): dual-use writer,
// generation at load AND future script-triggered save go through here).
bool write(const std::string &path, const std::vector<Section> &sections,
           const std::vector<std::string> &banner);

} // namespace ModularSystemFormat

#endif /* end of include guard: MODULAR_SYSTEM_FORMAT_HPP_ */
