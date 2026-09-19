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

enum class ScriptChannel {
	NONE = 0,  //!< no channel identified: the UI/TUI, a joypad binding, the
	FILE,      //!< a line of a script file: `file` + `line` say which one
	TCP,       //!< a line read on the control socket: `connection` says which
	           //!< connection sent it
};

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

	bool valid() const { return channel == ScriptChannel::FILE && !file.empty() && line != 0; }
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
