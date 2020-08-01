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
#include <dirent.h>
#include <vector>
#include <map>
#include <sstream>

#ifdef LINUX
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#endif

#define BUFFER_SIZE 1024


#ifdef WIN32
#include <Windows.h>
#endif

bool CallSystem::isAbsolute(const std::string path)
{
#if WIN32
    return path[1] == ':';
#else
    return path[0] == '/';
#endif
}

void CallSystem::splitFilename (const std::string& str, std::string &pathFile,std::string &fileName)
{
  std::size_t found = str.find_last_of("/\\");
  pathFile = str.substr(0,found+1);
  fileName = str.substr(found+1);
}


bool CallSystem::isReadable(const std::string & fileName)
{
	std::ifstream fichier(fileName);
	return !fichier.fail();
}


bool CallSystem::fileExist(const std::string& fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

bool CallSystem::dirExist(const std::string& rep)
{
	DIR *dir = nullptr;
	if ( (dir = opendir(rep.c_str() )) != nullptr) {
		closedir(dir);
		return true;
	}
	else
		return false;
}


bool CallSystem::fileCopy( const std::string &src, const std::string &dest)
{
	std::ifstream source(src, std::ios::binary);
	if (!source.good())
		return false;
    std::ofstream destination(dest, std::ios::binary);
	if (!destination.good())
		return false;

    destination << source.rdbuf();

    source.close();
    destination.close();
	return true;
}


void CallSystem::checkIniFiles(const std::string &CDIR, const std::string &DATA_ROOT)
{
	#if LINUX
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
	#endif
}



void CallSystem::checkUserDirectory(const std::string &CDIR, std::string & dirResult)
{
	#if LINUX
	if (dirExist( CDIR ) ) {
		dirResult = "Check home directory ok\n";
	} else {

		if ( mkdir(CDIR.c_str(), S_IRWXU | S_IRWXG) == 0)  {
			dirResult = "Creating home directory succesfull\n";
		} else {
			fprintf(stderr, "Unable to create local home directory\nProgram stopped\n");
			exit(2);
		}
	}
	#endif
}



void CallSystem::checkUserSubDirectory(const std::string &CDIR, std::string& dirResult)
{
	#if LINUX
	// take name and true if Directory should be copied or not
	std::map<std::string,bool> listSubDirectory;

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
	listSubDirectory[REP_LANGUAGE]=false;

	std::string DATADIR=std::string(CONFIG_DATA_DIR)+"data/";

	std::string subDir;
	for (std::map<std::string,bool>::iterator it=listSubDirectory.begin(); it!=listSubDirectory.end(); ++it) {
		subDir= CDIR + it->first;
		if ( ! dirExist( subDir.c_str()) ) {
			if ( mkdir( subDir.c_str(), S_IRWXU | S_IRWXG) == 0)  {
				dirResult += "Creating home subdirectory "+it->first+" succesfull\n";

				//printf("cp  %s%s %s -r\n", DATADIR.c_str(), rep[i].c_str(), CDIR.c_str());
				if (it->second) {
					if (!useSystemCommand( std::string("cp ") + DATADIR + it->first+ " " + CDIR + " -r")) {
						fprintf(stderr, "I can't copy %s in %s directory\n",it->first.c_str() ,CDIR.c_str() );
						exit(-1);
					} else
						dirResult += "Copy " +it->first +" in "+ CDIR + " ok\n";
				}
			} else {
				std::string err = "Unable to create local home subdirectory "+ it->first +"\nProgram stopped\n";
				fprintf(stderr, "%s\n",err.c_str() );
				exit(3);
			}
		} else
			dirResult +="Check " + it->first + " subdirectory ok\n";
	}

	// cas des textures 
	subDir = CDIR +REP_TEXTURE;
	if (!dirExist(subDir)) {
	if ( mkdir( subDir.c_str(), S_IRWXU | S_IRWXG) == 0)  {
			dirResult += "Creating home subdirectory "+REP_TEXTURE+" succesfull\n";

			//printf("cp  %s%s %s -r\n", DATADIR.c_str(), rep[i].c_str(), CDIR.c_str());
			if (!useSystemCommand( std::string("cp ") + CONFIG_DATA_DIR + REP_TEXTURE + " " + CDIR + " -r" )) {
				fprintf(stderr, "I can't copy %s in %s directory\n",REP_TEXTURE.c_str() ,CDIR.c_str() );
				exit(-1);
			} else
				dirResult += "Copy " +REP_TEXTURE +" in "+ CDIR + " ok\n";
		}
		else {
			std::string err = "Unable to create local home subdirectory "+ REP_TEXTURE +"\nProgram stopped\n";
			fprintf(stderr, "%s\n",err.c_str() );
			exit(3);
		}
	}
	#endif
}

bool CallSystem::useSystemCommand(const std::string & strCommand)
{
	#if LINUX
	if (system(strCommand.c_str())==0)
		return true;
	else
		return false;
	#else
	return false;
	#endif	
}

bool CallSystem::killAllPidFromVLC()
{
	#if LINUX
		//recuperer le nombre de VLC lancés 
		const int LEN = 5;
		char line[LEN];
		FILE *cmd = popen("ps aux | grep vlc | wc -l", "r");
		fgets(line, LEN, cmd);
		pclose(cmd);
		if (std::stoi(line)>2)
			return useSystemCommand("killall vlc &");
		else
			return false;
	#else
		return false;
	#endif	
}


bool CallSystem::killAllPidFromMPlayer()
{
	#if LINUX
		//recuperer le nombre de mplayer lancés 
		const int LEN = 5;
		char line[LEN];
		FILE *cmd = popen("ps aux | grep mplayer | wc -l", "r");
		fgets(line, LEN, cmd);
		pclose(cmd);
		if (std::stoi(line)>2)
			return useSystemCommand("killall mplayer &");
		else
			return false;
	#else
		return false;
	#endif	
}


const std::string CallSystem::getRamInfo()
{
	#if LINUX
		struct sysinfo info;
		sysinfo( &info );
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
		return "";
	#endif
}