/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014 Association Sirius
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
#include "tools/call_system.hpp"
#include "spacecrafter.hpp"
#include "tools/init_parser.hpp"
#include "tools/log.hpp"
#include <vector>
#include <map>
#include <sstream>
#include <filesystem>

#define BUFFER_SIZE 1024

bool CallSystem::isAbsolute(const std::string path)
{
    return std::filesystem::path(path).is_absolute();
}

bool CallSystem::isReadable(const std::string & fileName)
{
	std::ifstream fichier(fileName);
	return !fichier.fail();
}


bool CallSystem::fileExist(const std::string& fileName)
{
    std::error_code ec; // To avoid throw
    return std::filesystem::exists(fileName, ec);
}

bool CallSystem::dirExist(const std::string& rep)
{
    std::error_code ec;
    return std::filesystem::is_directory(rep, ec) && !ec;
}

void CallSystem::ensurePathExist(const std::string &path)
{
    if (!std::filesystem::exists(path))
        std::filesystem::create_directories(path);
}

bool CallSystem::fileCopy( const std::string &src, const std::string &dst)
{
    std::error_code ec;
    std::filesystem::copy_file(src, dst, ec);
    return !ec;
}

void CallSystem::checkIniFiles(const std::string &CDIR, const std::string &DATA_ROOT)
{
	std::string DATADIR=std::string(CONFIG_DATA_DIR)+"data/";

	std::vector<std::string> listIniFile;
	listIniFile.push_back("config.ini");
	listIniFile.push_back("anchor.ini");
	listIniFile.push_back("landscapes.ini");
	listIniFile.push_back("ssystem.ini");
	listIniFile.push_back("stars.ini");
	listIniFile.push_back("joypad.ini");
	listIniFile.push_back("deepsky_objects.fab");

	for (std::vector<std::string>::iterator it = listIniFile.begin() ; it != listIniFile.end(); ++it) {
		if ( ! fileExist(CDIR+(*it))) {
			cLog::get()->write("No "+ (*it) +" file, i will try to copy default_" + (*it), LOG_TYPE::L_INFO);
			fileCopy(DATADIR +"default_"+(*it), CDIR + (*it));
		}
	}
}

void CallSystem::checkUserDirectory(const std::string &userDir, std::string & logResult)
{
    if (std::filesystem::exists(userDir)) {
        logResult = "Check home directory ok\n";
    } else {
        logResult = "Home directory successfully created\n";
        std::filesystem::create_directories(userDir); // This throw error on failure
    }
}

void CallSystem::checkUserSubDirectory(const std::string &CDIR, std::string& dirResult)
{
	// take name and true if Directory should be copied or not
	std::map<std::string,bool> listSubDirectory;
    std::ostringstream out;

	listSubDirectory[REP_AUDIO]=true;
	listSubDirectory[REP_FONT]=true;
	listSubDirectory[REP_FTP]=true;
	listSubDirectory[REP_LANDSCAPE]=true;
	listSubDirectory[REP_LOG]=false;
	listSubDirectory[REP_SCREENSHOT]=false;
	listSubDirectory[REP_SCRIPT]=true;
	listSubDirectory[REP_PICTURE]=false;
	//~ listSubDirectory[REP_TEXTURE]=false;
	listSubDirectory[REP_VFRAME]=false;
	listSubDirectory[REP_VIDEO]=true;
	listSubDirectory[REP_MEDIA]=false;
	listSubDirectory[REP_VR360]=false;
	listSubDirectory[REP_WEB]=false;
	listSubDirectory[REP_SKY_CULTURE]=true;
	listSubDirectory[REP_MODEL3D]=true;
	listSubDirectory[REP_LANGUAGE]=true;

    std::filesystem::path subDir;
	for (auto &entry : listSubDirectory) {
        subDir = CDIR + entry.first;
        if (!std::filesystem::exists(subDir)) {
            if (std::filesystem::create_directories(subDir)) {
                out << "Successfully created home subdirectory " << entry.first << '\n';
                if (entry.second) {
                    std::error_code ec;
                    std::filesystem::copy(std::string(CONFIG_DATA_DIR)+"data/"+entry.first, subDir, std::filesystem::copy_options::recursive, ec);
                    if (ec) {
                        out << "Completed copy of " << entry.first << " in " << CDIR << '\n';
                    } else {
                        std::cerr << "Failed to copy " << entry.first << " in " << CDIR << "\nAbort !\n";
                        exit(-1);
                    }
                }
            } else {
                std::cerr << "Failed to create local home subdirectory " << entry.first << "\nAbort !\n";
                exit(3);
            }
		} else
			out << "Check " << entry.first << " subdirectory ok\n";
	}

    checkUserSubDirectory(CDIR, REP_TEXTURE, out);
    checkUserSubDirectory(CDIR, REP_LANGUAGE, out);
    dirResult += out.str();
}

void CallSystem::checkUserSubDirectory(const std::string &CDIR, const std::string &subDirectory, std::ostringstream &out)
{
    std::filesystem::path subDir = CDIR + subDirectory;
    if (!std::filesystem::exists(subDir)) {
        std::error_code ec;
        std::filesystem::copy(std::string(CONFIG_DATA_DIR)+"data/"+subDirectory, subDir, std::filesystem::copy_options::recursive, ec);
        if (ec) {
            out << "Completed copy of " << subDirectory << " in " << CDIR << '\n';
        } else {
            std::cerr << "Failed to copy " << subDirectory << " in " << CDIR << "\nAbort !\n";
            exit(-1);
        }
    }
}

bool CallSystem::useSystemCommand(const std::string & strCommand)
{
	// #if LINUX // Redundant : This condition is true for linux and windows
	if (system(strCommand.c_str())==0)
		return true;
	else
		return false;
	// #else
	// 	return false;
	// #endif
}

bool CallSystem::killAllPidFrom(const std::string& prgm)
{
	#ifdef __linux__
		std::string command = "ps aux | grep " + prgm + " | wc -l";
		//recuperer le nombre de prgm lancés
		const int LEN = 5;
		char line[LEN];
		FILE *cmd = popen(command.c_str(), "r");
		fgets(line, LEN, cmd);
		pclose(cmd);
		if (std::stoi(line)>2) {
			std::string order = "killall "+prgm+" &";
			return useSystemCommand(order);
		} else
			return false;
	#else
		return false;
	#endif
}

const std::string CallSystem::getRamInfo()
{
	#ifdef __linux__
		struct sysinfo info;
		sysinfo(&info);
		std::ostringstream strInfo;
		strInfo << "Memory information" << std::endl;
		strInfo << "Total ram " << (size_t)info.totalram * (size_t)info.mem_unit << std::endl;
		strInfo << "Free  ram " << (size_t)info.freeram * (size_t)info.mem_unit << std::endl;
		strInfo << "Shared ram " << (size_t)info.sharedram * (size_t)info.mem_unit << std::endl;
		strInfo << "Buffer ram " << (size_t)info.bufferram * (size_t)info.mem_unit << std::endl;
		strInfo << "Total swap " << (size_t)info.totalswap * (size_t)info.mem_unit << std::endl;
		strInfo << "Free  swap " << (size_t)info.freeswap * (size_t)info.mem_unit; //<< std::endl;
		// std::cout << "procs " << (size_t)info.procs << std::endl;
		return strInfo.str();
	#else
		return {};
	#endif
}
