/*
 * scedit — sc_tui.hpp
 *
 * WHAT THIS IS FOR
 * ================
 * The terminal. Nothing else: this layer draws `scedit::EditCore`'s state and
 * hands it key and mouse events. Every decision — what completes, what the
 * documentation bar says, where a finding is — was already taken in the
 * headless core, which is why the editor can be tested without a tty.
 *
 * WHY FTXUI (decision D1, scedit/INTENT.md §3): C++, vendorable, mouse and
 * layout built in. The vendoring record is util/scedit/README.md § Vendoring.
 *
 * BYTES IN, GLYPHS OUT — AND ONLY OUT
 * ===================================
 * The buffer is ISO-8859 bytes and stays that way. A terminal wants UTF-8, so
 * this layer maps each byte to one display cell for DRAWING ONLY, and the map
 * is one-to-one so that a column of the screen is a byte of the file:
 *   0x20-0x7E  the character itself
 *   0xA0       a visible marker — the no-break space is the byte
 *              `invisible-separator` exists for, and an editor that draws it as
 *              a space hides the very defect the tool was written to show
 *   \t \r 0x00-0x1F 0x7F   a dim marker, one cell, so nothing is invisible
 *   0x80-0xFF  decoded as ISO-8859-1 for the screen
 * Typing a character the file cannot hold (a code point above U+00FF from a
 * UTF-8 terminal) is REFUSED with a message, never transcoded in.
 */

#ifndef SCEDIT_SC_TUI_HPP
#define SCEDIT_SC_TUI_HPP

#include <string>

namespace scedit {

//! Run the interactive editor on `file` (which may be empty for a new buffer).
//! Returns a process exit code: 0 normal, 2 the contract or the file could not
//! be read.
int runEditor(const std::string &grammarPath, const std::string &file);

//! Render a fixed set of (buffer, cursor) cases into an off-screen terminal and
//! print each frame, plus the dim mask of the cursor's row. No tty involved:
//! this is what lets a ctest assert that the doc bar and the GREY ghost text
//! actually reach the screen, rather than merely exist in the core.
int uiSelfTest(const std::string &grammarPath);

} // namespace scedit

#endif // SCEDIT_SC_TUI_HPP
