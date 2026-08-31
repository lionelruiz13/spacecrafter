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

#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>
#include "scriptModule/script_annotator.hpp"
#include "tools/log.hpp"

namespace {
const char MARK[] = "#!";

std::string rtrimBlanks(const std::string &s)
{
	std::size_t n = s.size();
	while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t'))
		--n;
	return s.substr(0, n);
}
} // namespace

std::size_t ScriptAnnotator::annotationBegin(const std::string &line)
{
	// The same toggle as parseCommand's comment cut: a '#' inside a "..." run
	// is text, so a value holding "#!" is never mistaken for a tail.
	bool inQuote = false;
	for (std::size_t i = 0; i < line.size(); ++i) {
		if (line[i] == '"')
			inQuote = !inQuote;
		else if (line[i] == '#' && !inQuote)
			return line.find(MARK, i);
	}
	return std::string::npos;
}

std::string ScriptAnnotator::withoutAnnotation(const std::string &line)
{
	const std::size_t at = annotationBegin(line);
	return at == std::string::npos ? line : rtrimBlanks(line.substr(0, at));
}

std::string ScriptAnnotator::withAnnotation(const std::string &line, const std::string &message)
{
	return withoutAnnotation(line) + " " + MARK + " " + message;
}

void ScriptAnnotator::note(const ScriptOrigin &at, const std::string &message)
{
	if (!at.valid())
		return;
	LineNote &n = files_[at.file][at.line];
	n.text = at.text;
	if (n.message.empty())
		n.message = message;
	else if (n.message.find(message) == std::string::npos)
		n.message += "; " + message;
}

void ScriptAnnotator::saw(const ScriptOrigin &at)
{
	if (!at.valid())
		return;
	LineNote &n = files_[at.file][at.line];
	if (n.text.empty())
		n.text = at.text;
}

void ScriptAnnotator::flush(bool naturalEnd)
{
	for (auto &fileEntry : files_) {
		const std::string &path = fileEntry.first;
		auto &notes = fileEntry.second;
		int pending = 0;
		for (const auto &ln : notes)
			if (!ln.second.message.empty())
				++pending;
		if (pending == 0 && !naturalEnd)
			continue;   // nothing to write, and a cancelled run may not clear

		std::string content;
		{
			std::ifstream in(path, std::ios::binary);
			if (!in) {
				cLog::get()->write("script annotation: cannot read '" + path + "' to place " + std::to_string(pending)
				    + " annotation(s); they are in this log only", LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
				continue;
			}
			std::ostringstream ss;
			ss << in.rdbuf();
			content = ss.str();
		}
		// Split on LF, keeping every piece; a CR of a CRLF ending stays on its
		// piece so the comparison with the loaded text is exact, and the tail
		// is inserted before it.
		std::vector<std::string> pieces;
		{
			std::size_t start = 0;
			while (true) {
				const std::size_t nl = content.find('\n', start);
				if (nl == std::string::npos) {
					pieces.push_back(content.substr(start));
					break;
				}
				pieces.push_back(content.substr(start, nl - start));
				start = nl + 1;
			}
		}
		int written = 0, cleared = 0, skipped = 0;
		for (const auto &ln : notes) {
			const unsigned lineNo = ln.first;
			const LineNote &n = ln.second;
			if (n.message.empty() && !naturalEnd)
				continue;
			if (lineNo == 0 || lineNo > pieces.size()) {
				cLog::get()->write("script annotation: '" + path + "' has no line " + std::to_string(lineNo)
				    + " any more (file shorter than when it was loaded); not written: " + n.message,
				    LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
				++skipped;
				continue;
			}
			std::string &piece = pieces[lineNo - 1];
			if (piece != n.text) {
				cLog::get()->write("script annotation: '" + path + "' line " + std::to_string(lineNo)
				    + " changed since the script was loaded; not written: " + n.message,
				    LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
				++skipped;
				continue;
			}
			const bool cr = !piece.empty() && piece.back() == '\r';
			const std::string body = cr ? piece.substr(0, piece.size() - 1) : piece;
			std::string next = n.message.empty() ? withoutAnnotation(body) : withAnnotation(body, n.message);
			if (cr)
				next += '\r';
			if (next == piece)
				continue;   // the tail already says this
			piece = next;
			if (n.message.empty()) ++cleared; else ++written;
		}
		if (written == 0 && cleared == 0)
			continue;
		std::string out;
		for (std::size_t i = 0; i < pieces.size(); ++i) {
			if (i) out += '\n';
			out += pieces[i];
		}
		// Sibling temp in the target's own directory: rename atomicity is
		// same-filesystem only (INTENT §11.52(a)).
		const std::string tmp = path + ".tmp";
		bool ok = false;
		{
			std::ofstream f(tmp, std::ios::trunc | std::ios::binary);
			if (f) {
				f.write(out.data(), out.size());
				f.flush();
				ok = static_cast<bool>(f);
			}
		}
		if (ok && std::rename(tmp.c_str(), path.c_str()) != 0)
			ok = false;
		if (!ok) {
			std::remove(tmp.c_str());
			cLog::get()->write("script annotation: cannot write '" + path + "' (" + std::to_string(written)
			    + " annotation(s) to place, " + std::to_string(cleared) + " to clear): the file or its directory is not writable "
			    "by this process, so the diagnostics stay in this log only; make the script writable to have them "
			    "placed on their lines", LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
			continue;
		}
		cLog::get()->write("script annotation: '" + path + "' rewritten - " + std::to_string(written)
		    + " '#!' line(s) written, " + std::to_string(cleared) + " cleared"
		    + (skipped ? ", " + std::to_string(skipped) + " skipped (see above)" : std::string()),
		    LOG_TYPE::L_INFO, LOG_FILE::SCRIPT);
	}
	files_.clear();
}
