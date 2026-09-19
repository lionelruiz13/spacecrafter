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

#ifndef _SCRIPT_ANNOTATOR_HPP_
#define _SCRIPT_ANNOTATOR_HPP_

#include <map>
#include <string>
#include "scriptModule/script_origin.hpp"

//! The `#!` channel: error feedback written into the script file, at the end of the faulty line
//! Contract: the tail of a line from its first `#!` belongs to the engine, everything before it is never touched
//! Writes are batched per file at script end, bytes and line endings preserved; a failed write degrades to the log
class ScriptAnnotator {
public:
	//! Record a diagnostic for the line `at` names. Several diagnostics for
	//! one line are joined with "; ". Ignored when `at` is invalid (no file).
	void note(const ScriptOrigin &at, const std::string &message);
	//! A line that carried a `#!` tail when it was dispatched: candidate for
	//! clearing at a natural end if no diagnostic names it.
	void saw(const ScriptOrigin &at);
	//! Write every pending annotation to its file (see the contract above),
	//! clear stale tails when `naturalEnd`, then forget everything.
	void flush(bool naturalEnd);

	//! Byte offset of the first `#!` at or after the first `#` outside quotes; std::string::npos when none
	static std::size_t annotationBegin(const std::string &line);
	static bool hasAnnotation(const std::string &line) { return annotationBegin(line) != std::string::npos; }
	//! The line with its machine tail (and the blanks before it) removed.
	static std::string withoutAnnotation(const std::string &line);
	//! The line with `message` as its machine tail, replacing any present.
	static std::string withAnnotation(const std::string &line, const std::string &message);

private:
	struct LineNote {
		std::string text;      //!< the raw line as loaded, for the changed-since-load check
		std::string message;   //!< "" = seen with a tail, no diagnostic (clear candidate)
	};
	//! file -> line number -> note
	std::map<std::string, std::map<unsigned, LineNote>> files_;
};

#endif // _SCRIPT_ANNOTATOR_HPP_
