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
	NONE = 0,  //!< UI, joypad, pipe, HTTP query, nested or engine-made command
	FILE,     //!< A line of a script file
	TCP,       //!< A line read on the control socket
};

//! Where a command line came from; build one with fromFile / fromTcp
struct ScriptOrigin {
	ScriptChannel channel = ScriptChannel::NONE;
	std::string file;      //!< FILE only, full path
	unsigned line = 0;     //!< FILE only, 1-based, 0 = none
	std::string text;      //!< Raw line, the CR of a CRLF file is kept
	unsigned connection = 0; //!< TCP only, 0 = unknown

	static ScriptOrigin fromFile(const std::string &file, unsigned line, const std::string &text) {
		ScriptOrigin o;
		o.channel = ScriptChannel::FILE;
		o.file = file;
		o.line = line;
		o.text = text;
		return o;
	}
	static ScriptOrigin fromTcp(unsigned id, const std::string &text) {
		ScriptOrigin o;
		o.channel = ScriptChannel::TCP;
		o.text = text;
		o.connection = id;
		return o;
	}

	//! Return true only for a line of a file, never for TCP
	bool valid() const { return channel == ScriptChannel::FILE && !file.empty() && line != 0; }
	//! Return text without its line ending
	std::string lineText() const {
		std::string s = text;
		while (!s.empty() && (s.back() == '\r' || s.back() == '\n'))
			s.pop_back();
		return s;
	}
	//! Return "file:line", "tcp#<id>" or ""
	std::string where() const {
		if (valid())
			return file + ":" + std::to_string(line);
		if (channel == ScriptChannel::TCP)
			return connection ? "tcp#" + std::to_string(connection) : std::string("tcp");
		return std::string();
	}
};

#endif // _SCRIPT_ORIGIN_HPP_
