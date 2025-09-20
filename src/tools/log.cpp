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
#include <time.h>
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

void cLog::openLog(const LOG_FILE& fichier, const std::string& LogfilePath, const bool keepHistory)
{
	std::ofstream file;

	if (keepHistory)
		file.open(logDirectory + LogfilePath + "-" + getDate() + LOG_EXTENSION, std::ofstream::out | std::ofstream::app);
	else
		file.open(logDirectory + LogfilePath + LOG_EXTENSION, std::ofstream::out | std::ofstream::trunc);

	if (!file.is_open()) {
		std::cerr << "(EE): Couldn't open file log!\n Please check file/directory permissions" << std::endl;
		throw;
	}
	logFile.insert(std::pair<const LOG_FILE, std::ofstream>(fichier, std::move(file)));
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

std::string cLog::getDate()
{
	time_t tTime = time(NULL);
	tm * tmTime = localtime (&tTime);
	char timestr[32];
	strftime(timestr, 21, "%y.%m.%d", tmTime);
	return std::string(timestr);
}
