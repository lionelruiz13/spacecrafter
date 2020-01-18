/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2015 of the LSS Team & Association Sirius
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
#include <stdlib.h>
#include <stdlib.h>


class CallSystem
{
public:
	static void splitFilename (const std::string& str, std::string &path,std::string &fileName);
    static bool isReadable(const std::string &file);
    static bool fileExist ( const std::string& file);
    static bool dirExist ( const std::string& rep);

    static int fileCopy( const std::string &src, const std::string &dest) ;

    static void checkIniFiles(const std::string &CDIR, const std::string &DATA_ROOT);

    static void checkUserDirectory(const std::string &CDIR, std::string & dirResult);

    static void checkUserSubDirectory(const std::string &CDIR, std::string& dirResult);

    static bool useSystemCommand(const std::string & strCommand);

    static bool killAllPidFromVLC();

    static bool killAllPidFromMPlayer();

    static const std::string getRamInfo();
};

#endif
