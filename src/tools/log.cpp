/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017 Immersive Adventure
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include "tools/log.hpp"
#include <exception>
#include <string>
#include <cstdint>
#include <filesystem>
#include <SDL2/SDL.h>
#include "EntityCore/Core/VulkanMgr.hpp"

#define LOG_EE "(Error): "
#define LOG_WW "(Warn.): "
#define LOG_II "(Info ): "
#define LOG_DD "(Debug): "

// thanks to internet for color !!
// http://stackoverflow.com/questions/1961209/making-some-text-in-printf-appear-in-green-and-red
#ifdef WIN32
	#define LOG_RESET   ""
	#define LOG_BLACK   ""      /* Black */
	#define LOG_RED     ""      /* Red */
	#define LOG_GREEN   ""      /* Green */
	#define LOG_YELLOW  ""      /* Yellow */
	#define LOG_BLUE    ""      /* Blue */
	#define LOG_MAGENTA ""      /* Magenta */
	#define LOG_CYAN    ""      /* Cyan */
	#define LOG_WHITE   ""      /* White */
	#define LOG_BOLDBLACK   ""      /* Bold Black */
	#define LOG_BOLDRED     ""      /* Bold Red */
	#define LOG_BOLDGREEN   ""      /* Bold Green */
	#define LOG_BOLDYELLOW  ""      /* Bold Yellow */
	#define LOG_BOLDBLUE    ""      /* Bold Blue */
	#define LOG_BOLDMAGENTA ""      /* Bold Magenta */
	#define LOG_BOLDCYAN    ""      /* Bold Cyan */
	#define LOG_BOLDWHITE   ""      /* Bold White */
#else
	#define LOG_RESET   "\033[0m"
	#define LOG_BLACK   "\033[30m"      /* Black */
	#define LOG_RED     "\033[31m"      /* Red */
	#define LOG_GREEN   "\033[32m"      /* Green */
	#define LOG_YELLOW  "\033[33m"      /* Yellow */
	#define LOG_BLUE    "\033[34m"      /* Blue */
	#define LOG_MAGENTA "\033[35m"      /* Magenta */
	#define LOG_CYAN    "\033[36m"      /* Cyan */
	#define LOG_WHITE   "\033[37m"      /* White */
	#define LOG_BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
	#define LOG_BOLDRED     "\033[1m\033[31m"      /* Bold Red */
	#define LOG_BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
	#define LOG_BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
	#define LOG_BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
	#define LOG_BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
	#define LOG_BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
	#define LOG_BOLDWHITE   "\033[1m\033[37m"      /* Bold White */
#endif

const std::string LOG_EXTENSION=".log";

cLog* cLog::singleton = nullptr;

cLog::cLog()
{
}

//! The name of the file holding the launch k launches back, k in
//! 1 .. LOG_RETENTION_LAUNCHES-1.  The current launch is <base> + LOG_EXTENSION.
static std::string archiveName(const std::string& base, int k)
{
	return base + "." + std::to_string(k) + LOG_EXTENSION;
}

void cLog::rotate(const std::string& LogfilePath)
{
	const std::string base = logDirectory + LogfilePath;
	const std::string live = base + LOG_EXTENSION;
	std::error_code ec;

	// What falls out of the window goes FIRST, so the shift below never has to
	// overwrite a file it was supposed to keep.  Its size is read while it
	// still exists: a D12 line has to say what was thrown away, not only that
	// something was (Sec.2.0 D12's CONTENT part).
	std::string deleted;
	std::uintmax_t deletedSize = 0;
	const std::string oldest = archiveName(base, LOG_RETENTION_LAUNCHES - 1);
	if (std::filesystem::exists(oldest, ec)) {
		deletedSize = std::filesystem::file_size(oldest, ec);
		if (ec)
			deletedSize = 0;
		if (std::filesystem::remove(oldest, ec))
			deleted = oldest;
	}

	// Descending, so each rename lands on a name that has just been vacated.
	for (int k = LOG_RETENTION_LAUNCHES - 2; k >= 1; --k)
		std::filesystem::rename(archiveName(base, k), archiveName(base, k + 1), ec);

	const bool rotated = std::filesystem::exists(live, ec);
	if (rotated)
		std::filesystem::rename(live, archiveName(base, 1), ec);

	// How much of the window is in use once the fresh file below is opened.
	int kept = 1;
	for (int k = 1; k < LOG_RETENTION_LAUNCHES; ++k) {
		if (std::filesystem::exists(archiveName(base, k), ec))
			++kept;
	}

	std::string line = "Log retention (" + LogfilePath + LOG_EXTENSION + "): ";
	if (rotated)
		line += "the previous launch is now " + LogfilePath + ".1" + LOG_EXTENSION;
	else
		line += "no earlier launch to rotate";
	line += ", now using " + std::to_string(kept) + " of the last "
	     +  std::to_string(LOG_RETENTION_LAUNCHES) + " launches, deleted ";
	if (deleted.empty())
		line += "nothing";
	else
		line += deleted + " (" + std::to_string(deletedSize)
		     +  " bytes, the launch that fell outside the window)";
	line += ". The window is the compiled constant LOG_RETENTION_LAUNCHES = "
	     +  std::to_string(LOG_RETENTION_LAUNCHES) + " in src/tools/log.hpp:"
	     +  " no config.ini key sets it, so keeping more or fewer launches means"
	     +  " changing that line and rebuilding.";
	openReport.push_back(line);

	// The layout before Sec.11.230 wrote one dated file per DAY on the script
	// channel (<path>-YY.MM.DD.log, appended across launches) and nothing ever
	// deleted one.  Such files are outside this window by construction: this
	// build neither writes nor removes them, so the only way they leave the
	// disk is by hand - which is exactly what the line must say.
	std::uintmax_t legacyBytes = 0;
	int legacyCount = 0;
	const std::string prefix = LogfilePath + "-";
	std::filesystem::directory_iterator it(
		logDirectory.empty() ? std::string(".") : logDirectory,
		std::filesystem::directory_options::skip_permission_denied, ec);
	const std::filesystem::directory_iterator last;
	for (; !ec && it != last; it.increment(ec)) {
		const std::string name = it->path().filename().string();
		if (name.size() <= prefix.size() + LOG_EXTENSION.size())
			continue;
		if (name.compare(0, prefix.size(), prefix) != 0)
			continue;
		if (name.compare(name.size() - LOG_EXTENSION.size(),
				LOG_EXTENSION.size(), LOG_EXTENSION) != 0)
			continue;
		++legacyCount;
		std::error_code sizeEc;
		const std::uintmax_t sz = std::filesystem::file_size(it->path(), sizeEc);
		if (!sizeEc)
			legacyBytes += sz;
	}
	if (legacyCount > 0)
		openReport.push_back("Log retention (" + LogfilePath + LOG_EXTENSION + "): "
			+ std::to_string(legacyCount)
			+ " legacy dated log file(s) of the pre-rotation layout are present ("
			+ prefix + "*" + LOG_EXTENSION + ", " + std::to_string(legacyBytes)
			+ " bytes in total). They are never written, read or deleted by this"
			  " build and the " + std::to_string(LOG_RETENTION_LAUNCHES)
			+ "-launch window does not cover them: delete them by hand if you"
			  " want the space back.");
}

void cLog::openLog(const LOG_FILE& fichier, const std::string& LogfilePath)
{
	std::ofstream file;

	// Every channel keeps the last LOG_RETENTION_LAUNCHES launches and the
	// CURRENT launch keeps the channel's own name, so this open is always
	// <LogfilePath>.log, truncated, with the previous launches numbered behind
	// it.  Until Sec.11.230 the script channel alone opened a per-day
	// <LogfilePath>-YY.MM.DD.log in APPEND mode and nothing ever capped it
	// (Sec.5.115: gigabyte-scale logs at clients).
	rotate(LogfilePath);
	file.open(logDirectory + LogfilePath + LOG_EXTENSION, std::ofstream::out | std::ofstream::trunc);

	if (!file.is_open()) {
		std::cerr << "(EE): Couldn't open file log!\n Please check file/directory permissions" << std::endl;
		throw;
	}
	logFile.insert(std::pair<const LOG_FILE, std::ofstream>(fichier, std::move(file)));
}

void cLog::reportOpenLog()
{
	if (openReportLogged)
		return;
	openReportLogged = true;
	// write() puts each line on the console itself when the console is already
	// on, so the console half is served either here or by setDebug, never
	// twice.  The report cannot be written from openLog: the first channel
	// rotates before any log file exists to receive it.
	if (isDebug)
		openReportOnConsole = true;
	for (const auto& line : openReport)
		write(line, LOG_TYPE::L_INFO, LOG_FILE::INTERNAL);
}

void cLog::reportOpenLogConsole()
{
	if (openReportOnConsole)
		return;
	openReportOnConsole = true;
	writeMutex.lock();
	for (const auto& line : openReport)
		writeConsole(line, LOG_TYPE::L_INFO);
	writeMutex.unlock();
}

void cLog::close() {
	if (singleton != nullptr) {
		for (auto &file: singleton->logFile) {
			file.second << LOG_II << "EOF" << std::endl;
			file.second.close();
		}
		delete singleton;
	}
	singleton = nullptr;
}

cLog::~cLog()
{
}


void cLog::write(const std::string& texte, const LOG_TYPE& type, const LOG_FILE& fichier)
{
	writeMutex.lock();
	std::string ligne;

	if (isDebug) {
		writeConsole(texte, type);
		char value[15];
		sprintf(value, "%012d: ", SDL_GetTicks());
		ligne.append(std::string(value));
	}
	if (!isWritingLog) {
		writeMutex.unlock();
		return;
	};

	switch(type) {
		case LOG_TYPE::L_WARNING :
			ligne.append(LOG_WW);
			break;
		case LOG_TYPE::L_ERROR :
			ligne.append(LOG_EE);
			break;
		case LOG_TYPE::L_DEBUG :
			ligne.append(LOG_DD);
			break;
		case LOG_TYPE::L_INFO :
			ligne.append(LOG_II);
			break;
		default :
			;
	}

	if (logFile.count(fichier)) {
		logFile.at(fichier) << ligne << texte << std::endl;
		logFile.at(fichier).flush();
	} else {
		logFile.at(LOG_FILE::INTERNAL) << ligne << texte << std::endl;
		logFile.at(LOG_FILE::INTERNAL).flush();
	}

	writeMutex.unlock();
}

void cLog::mark(const LOG_FILE& fichier)
{
	write("=================================================================", LOG_TYPE::L_OTHER, fichier);
}

void cLog::writeConsole(const std::string& texte, const LOG_TYPE& type)
{
	std::string ligne;
	switch(type) {
		case LOG_TYPE::L_WARNING :
			ligne.append(LOG_BOLDYELLOW);
			ligne.append(LOG_WW);
			ligne.append(LOG_RESET);
			ligne.append(LOG_BOLDWHITE);
			break;

		case LOG_TYPE::L_ERROR :
			ligne.append(LOG_BOLDRED);
			ligne.append(LOG_EE);
			ligne.append(LOG_RESET);
			ligne.append(LOG_BOLDWHITE);
			break;

		case LOG_TYPE::L_DEBUG :
			ligne.append(LOG_BOLDCYAN);
			ligne.append(LOG_DD);
			ligne.append(LOG_RESET);
			ligne.append(LOG_WHITE);
			break;

		case LOG_TYPE::L_INFO :
			ligne.append(LOG_BOLDGREEN);
			ligne.append(LOG_II);
			ligne.append(LOG_RESET);
			ligne.append(LOG_WHITE);
			break;

		default :
			ligne.append(LOG_WHITE);
	}

	ligne.append(texte);
	ligne.append(LOG_RESET);
	ligne.append("\r\n");

	if(type == LOG_TYPE::L_ERROR || type == LOG_TYPE::L_WARNING) {
		std::cerr << ligne;
	} else {
		std::cout << ligne;
	}

}

void cLog::writeECLog(const std::string &string, LogType type)
{
	// This class must exist while VulkanMgr exist.
	LOG_TYPE ltype;
	switch (type) {
		case LogType::ERROR:
			ltype = LOG_TYPE::L_ERROR;
			break;
		case LogType::WARNING:
			ltype = LOG_TYPE::L_WARNING;
			break;
		case LogType::DEBUG:
			ltype = LOG_TYPE::L_DEBUG;
			break;
		case LogType::INFO:
			ltype = LOG_TYPE::L_INFO;
			break;
		default:
			ltype = LOG_TYPE::L_OTHER;
	}
	singleton->write(string, ltype, LOG_FILE::VULKAN);
}
