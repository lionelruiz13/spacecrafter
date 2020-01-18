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

#include "log2.hpp"
#include <exception>
#include <string>
#include <time.h>

// thanks to internet for color !!
// http://stackoverflow.com/questions/1961209/making-some-text-in-printf-appear-in-green-and-red
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

#define LOG_EE "(EE): "
#define LOG_WW "(WW): "
#define LOG_II "(II): "
#define LOG_DD "(DD): "


ncLog* ncLog::singleton = nullptr;

void ncLog::open(const std::string& LogfilePath)
{
	Logfile.open(LogfilePath, std::ofstream::out | std::ofstream::trunc);
	if (!Logfile.is_open()) {
		std::cerr << "(EE): Couldn't open file log!\n Please check file/directory permissions" << std::endl;
		throw;
	}
}

void ncLog::close() {
    if (Logfile.is_open()) {
	    Logfile.close();
    }
	if (singleton != nullptr) {
		delete singleton;
	}
	singleton = nullptr;
}


void ncLog::write(const std::string& texte, const LOG_TYPE& type)
{
    if (level > type) {
        return;
    }
	writeMutex.lock();

	if (isDebug)
		writeConsole(texte, type);

	std::string ligne;

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
			break;
	}
	ligne.append(texte);
	ligne.append("\r\n");

	Logfile << ligne;
	Logfile.flush();

	writeMutex.unlock();
}

void ncLog::mark()
{
	write(std::string("================================================================="), LOG_TYPE::L_OTHER);
}

void ncLog::writeConsole(const std::string& texte, const LOG_TYPE& type)
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
