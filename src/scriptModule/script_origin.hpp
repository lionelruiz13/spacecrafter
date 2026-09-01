/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2026 Association Sirius
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 */

#ifndef _SCRIPT_ORIGIN_HPP_
#define _SCRIPT_ORIGIN_HPP_

#include <string>

//! The channel a command line arrived by. The engine is required to carry
//! FILE and TCP [vixy 2026-08-31: "spacecrafter script engine must carry the
//! provenance (file/tcp + line)"]; every other producer is NONE and its
//! diagnostics go to the log with no origin, exactly as before.
enum class ScriptChannel {
	NONE = 0,  //!< no channel identified: the UI/TUI, a joypad binding, the
	           //!< mkfifo pipe, an HTTP `?command=` query, an engine-synthesised
	           //!< splice (`camera action lift_off`), or a command nested inside
	           //!< another one (`clear`, `media`). Mapped, deliberately not
	           //!< wired: INTENT 11.187.
	FILE,      //!< a line of a script file: `file` + `line` say which one
	TCP,       //!< a line read on the control socket: `connection` says which
	           //!< connection sent it
};

//! Where a command line came from: which channel, and everything that channel
//! knows about the line - for a file, the path and the physical line (1-based,
//! counted per `getline` over the whole file, comment and blank lines
//! included); for the control socket, the connection it was read on. The raw
//! text of the line comes along in both cases.
//!
//! Carried from `Script::load` (files) or `App::updateFromSharedData` (the TCP
//! drain) through the token queue, the loop replay and
//! `AppCommandInterface::executeCommand`, so a diagnostic can be reported at
//! the line that CAUSED it - which for an unclosed `struct if` is the opener,
//! not the end of the file where the damage surfaces - and written back into
//! the script as a `#!` annotation (ScriptAnnotator).
//!
//! WHAT EACH CONSUMER MAY ASSUME:
//!  - `ScriptAnnotator` (the `#!` writer) writes into a file, so it acts on
//!    `valid()` and on nothing else: `valid()` means FILE + a path + a line,
//!    and it means only that. A TCP origin is never `valid()`, and no other
//!    channel is either - the annotator has no file to write to for them.
//!  - a LOG line names the origin with `where()`, which is non-empty for FILE
//!    ("file:line") and for TCP ("tcp#<id>"), empty for NONE. A reader that
//!    wants "does this diagnostic name where it came from" asks `where()`;
//!    one that wants "may I write into that file" asks `valid()`. The two
//!    questions are different and this is where they part.
//!  - `connection` is the never-reused id `ServerSocket` hands to a slot
//!    (io.hpp `clientIdTab`), not the slot number: a slot is reused, so the id
//!    is what makes a later answer follow the connection that asked rather
//!    than whoever holds its slot now (I5, INTENT 5.47's own argument).
//!    0 = unknown.
//!
//! `channel` is the FIRST member on purpose: an aggregate initialisation
//! written for the three-field version of this struct
//! (`ScriptOrigin{file, line, text}`) no longer compiles, so a producer that
//! predates the channel is a build error rather than a line that silently
//! claims to come from nowhere. Build one with `fromFile`/`fromTcp`.
struct ScriptOrigin {
	ScriptChannel channel = ScriptChannel::NONE;
	std::string file;      //!< FILE only: full path of the script file, "" = no file
	unsigned line = 0;     //!< FILE only: 1-based physical line in `file`, 0 = none
	std::string text;      //!< the raw line (line ending stripped by getline, a CR of a CRLF file kept)
	unsigned connection = 0; //!< TCP only: the connection id the line was read on, 0 = unknown

	//! A line of a script file, at its physical line number.
	static ScriptOrigin fromFile(const std::string &file, unsigned line, const std::string &text) {
		ScriptOrigin o;
		o.channel = ScriptChannel::FILE;
		o.file = file;
		o.line = line;
		o.text = text;
		return o;
	}
	//! A line read on the control socket, from the connection `id` (io.hpp).
	static ScriptOrigin fromTcp(unsigned id, const std::string &text) {
		ScriptOrigin o;
		o.channel = ScriptChannel::TCP;
		o.text = text;
		o.connection = id;
		return o;
	}

	//! The `#!` writer's gate, and ONLY that: this origin names a line of a
	//! file that can be annotated. Its meaning has not moved since the
	//! annotator was written - a TCP origin does not widen it, it fails it.
	bool valid() const { return channel == ScriptChannel::FILE && !file.empty() && line != 0; }
	//! The raw line as TEXT: `text` with its line ending removed. ONE
	//! definition of "the line as the user has it" - a CRLF file's CR belongs
	//! to the file, never to a log line or to a quoted subject - asked by
	//! every reader that shows the line rather than executing it.
	std::string lineText() const {
		std::string s = text;
		while (!s.empty() && (s.back() == '\r' || s.back() == '\n'))
			s.pop_back();
		return s;
	}
	//! How a log line names this origin: "file:line", "tcp#<id>", or "" when
	//! there is nothing to name.
	std::string where() const {
		if (valid())
			return file + ":" + std::to_string(line);
		if (channel == ScriptChannel::TCP)
			return connection ? "tcp#" + std::to_string(connection) : std::string("tcp");
		return std::string();
	}
};

#endif // _SCRIPT_ORIGIN_HPP_
