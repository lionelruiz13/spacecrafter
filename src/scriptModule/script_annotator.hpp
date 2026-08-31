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

//! The `#!` channel: error feedback written INTO the script, at the end of
//! the faulty line, so an author reading the file in any text editor sees the
//! problem where it is [vixy 2026-08-30, FEATURE_REQUESTS "`#!` - the engine
//! annotates the faulty script line in place"].
//!
//! Contract:
//!  - `#!` is RESERVED machine syntax inside a comment: the tail of a line
//!    from its first `#!` (at or after the first `#` outside quotes) to the end
//!    of the line belongs to the engine. Everything before it - the command,
//!    an author's own `# comment` - is never touched.
//!  - A tail is written once and REPLACED when what the engine would emit
//!    differs; a file whose tails already say what this run found is not
//!    rewritten at all (byte comparison before any write).
//!  - A line that carried a tail when it was dispatched and got no diagnostic
//!    in a run that reached the natural end of the script has its tail
//!    REMOVED: a fixed fault does not keep a dead annotation. Only a natural
//!    end may clear, because structure faults (an opener never closed) are
//!    only known there.
//!  - Writes are batched per file at script end, through a sibling temp file
//!    and rename (same discipline as ModularSystemFormat). A line whose text
//!    no longer matches what was loaded (the file was edited while the script
//!    ran) is skipped with a warning; a file that cannot be written degrades
//!    to the log - the diagnostic is in the log in every case, the file is
//!    the better channel, never the only one.
//!  - Bytes are preserved: the file's own line endings (a CRLF file stays
//!    CRLF, the tail goes before the CR), ISO-8859 content untouched, and the
//!    tail itself is ASCII.
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

	//! Byte offset where the machine tail of a raw line starts: the first
	//! `#!` at or after the first `#` outside a "..." run (the same quote
	//! toggle as the parser's comment cut); std::string::npos when none.
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
