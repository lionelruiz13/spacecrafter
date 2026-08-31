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

//! Where a command line came from: the script file and the physical line
//! (1-based, counted per `getline` over the whole file, comment and blank
//! lines included) that produced it, plus the raw text of that line.
//!
//! Carried from `Script::load` through the token queue, the loop replay and
//! `AppCommandInterface::executeCommand`, so a diagnostic can be reported at
//! the line that CAUSED it - which for an unclosed `struct if` is the opener,
//! not the end of the file where the damage surfaces - and written back into
//! the script as a `#!` annotation (ScriptAnnotator). A line with no file
//! behind it (a TCP/HTTP/UI command, an engine-synthesised splice such as
//! `camera action lift_off`, a loop body replayed from memory before origins
//! were kept) carries an INVALID origin: its diagnostics go to the log only.
struct ScriptOrigin {
	std::string file;      //!< full path of the script file, "" = no file
	unsigned line = 0;     //!< 1-based physical line in `file`, 0 = none
	std::string text;      //!< the raw line as read (line ending stripped by getline, a CR of a CRLF file kept)

	bool valid() const { return !file.empty() && line != 0; }
	//! "file:line" for a log line, "" when invalid.
	std::string where() const { return valid() ? file + ":" + std::to_string(line) : std::string(); }
};

#endif // _SCRIPT_ORIGIN_HPP_
