/*
 * scedit -- sc_tui.hpp
 *
 * WHAT THIS IS FOR
 * ================
 * The terminal. Nothing else: this layer draws `scedit::EditCore`'s state and
 * hands it key and mouse events. Every decision -- what completes, what the
 * documentation bar says, where a finding is -- was already taken in the
 * headless core, which is why the editor can be tested without a tty.
 *
 * WHY FTXUI (decision D1, scedit/INTENT.md S3): C++, vendorable, mouse and
 * layout built in. The vendoring record is util/scedit/README.md S Vendoring.
 *
 * BYTES IN, GLYPHS OUT -- AND ONLY OUT
 * ===================================
 * The buffer is ISO-8859 bytes and stays that way. A terminal wants UTF-8, so
 * this layer maps each byte to one display cell for DRAWING ONLY, and the map
 * is one-to-one so that a column of the screen is a byte of the file:
 *   0x20-0x7E  the character itself
 *   0xA0       a visible marker -- the no-break space is the byte
 *              `invisible-separator` exists for, and an editor that draws it as
 *              a space hides the very defect the tool was written to show
 *   \t \r 0x00-0x1F 0x7F   a dim marker, one cell, so nothing is invisible
 *   0x80-0xFF  decoded as ISO-8859-1 for the screen
 * Typing a character the file cannot hold (a code point above U+00FF from a
 * UTF-8 terminal) is REFUSED with a message, never transcoded in.
 *
 * LIVE MODE, AND THE TWO THINGS IT IS NOT
 * =======================================
 * With `--tcp`, the editor holds a `TcpClient` (sc_tcpclient.hpp) beside the
 * core: the caret's line can be sent as a command, the open file can be played,
 * and what the engine says arrives in a pane. Without `--tcp` there is no
 * socket, no key for one, and not a word about one on the screen.
 *
 * (1) It is not a driver. Nothing leaves this process without a keystroke: no
 *     auto-connect, no reconnect after a drop, no replay, no keep-alive.
 * (2) It is not a poller. The engine gives no end-of-script event on any
 *     channel a client can see, and it rewrites the played FILE when a run
 *     ends -- so after a play, and only then, the editor reads that file at most
 *     once a second, for at most five minutes or until it changes, whichever
 *     comes first. That is the one thing here that happens on a clock, it is
 *     I3's admitted case (an external writer with no notification), and the
 *     safety net is elsewhere: `EditCore::save` compares the file on EVERY
 *     save, forever, whether a play happened or not. The socket is drained four
 *     times a second while connected, which is not the same act -- draining is
 *     reading what the peer has already sent.
 */

#ifndef SCEDIT_SC_TUI_HPP
#define SCEDIT_SC_TUI_HPP

#include <string>

#include "sc_tcpclient.hpp"

namespace scedit {

//! Live mode's two settings, as data: whether `--tcp` was given at all, and
//! where it points. Passing `enabled = false` is what makes an editor with no
//! network in it -- the socket is never created and the keys are never bound.
struct LiveOptions {
	bool enabled = false;
	Endpoint endpoint;   //!< defaults to the shipped 127.0.0.1:7805
};

//! Run the interactive editor on `file` (which may be empty for a new buffer).
//! Returns a process exit code: 0 normal, 2 the contract or the file could not
//! be read.
int runEditor(const std::string &grammarPath, const std::string &file,
              const LiveOptions &live = LiveOptions());

//! Render a fixed set of (buffer, cursor) cases into an off-screen terminal and
//! print each frame, plus the dim mask of the cursor's row. No tty involved:
//! this is what lets a ctest assert that the doc bar and the GREY ghost text
//! actually reach the screen, rather than merely exist in the core.
int uiSelfTest(const std::string &grammarPath);

} // namespace scedit

#endif // SCEDIT_SC_TUI_HPP
