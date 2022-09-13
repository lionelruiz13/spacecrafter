/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2015-2020 of the LSS Team & Association Sirius
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
 *
 */

#ifndef _CALLSYSTEM_HPP_
#define _CALLSYSTEM_HPP_

#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <stdlib.h>


/**
 * @file call_system.hpp
 * @brief Functions linked to system calls
 *
 * @class CallSystem
 *
 * @brief Static class that provides functions linked to system calls.
 *
 */
class CallSystem
{
public:
    // test if the file exists and is accessible
    static bool isReadable(const std::string& fileName);
    // test if the file exists
    static bool fileExist(const std::string& fileName);
    // test if the directory exists
    static bool dirExist(const std::string& rep);
	//! Recursively create missing directory to ensure the path exist
	static void ensurePathExist(const std::string &path);
    //! returns true if the given path is absolute
    static bool isAbsolute(const std::string path);
    //! copies the src file to the destination dest
    static bool fileCopy(const std::string &src, const std::string &dest) ;
    //! Checks if the user ini files are present and rebuilds them if needed
    static void checkIniFiles(const std::string &CDIR, const std::string &DATA_ROOT);
    //! Checks if userDir exists and creates it. returns the result in logResult
    static void checkUserDirectory(const std::string &userDir, std::string & logResult);
    //! Checks if user subdirectories are present and rebuilds them if needed
    static void checkUserSubDirectory(const std::string &CDIR, std::string& dirResult);
	//! Check if the given user subdirectory exists, rebuild it if it don't
	static void checkUserSubDirectory(const std::string &CDIR, const std::string &subDirectory, std::ostringstream &out);
    //! runs a system command
    static bool useSystemCommand(const std::string & strCommand);
    //! deletes the prgm program via its pid
    static bool killAllPidFrom(const std::string& prgm);
    //! Gives information about the amount of RAM available on the machine
    static const std::string getRamInfo();
};

#endif
