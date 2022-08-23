/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2005 Robert Spearman
 * Copyright (C) 2009-2010 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
 * Copyright (C) 2014 of the LSS Team & Association Sirius
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

#include <iostream>
#include <string>
#include <dirent.h>
#include <cstdio>
#include <set>
#include "interfaceModule/app_command_interface.hpp"
#include "scriptModule/script_mgr.hpp"
#include "mediaModule/media.hpp"
#include "script.hpp"
#include "tools/utility.hpp"
#include "tools/log.hpp"
#include "tools/app_settings.hpp"
#include "tools/call_system.hpp"


ScriptMgr::ScriptMgr(std::shared_ptr<AppCommandInterface> command_interface,const std::string &_data_dir, std::shared_ptr<Media> _media )
{
	commander = command_interface.get();
	DataDir = _data_dir;
	scriptState = ScriptState::NONE;
	sR.recording = false;
	sR.record_elapsed_time = 0;
	multiplierRate=1;
	nbrLoop =0;
	isInLoop = false;
	repeatLoop = false;
	waitOnVideo = false;
	media = _media;
	script= new Script();
}

ScriptMgr::~ScriptMgr()
{
	delete script;
}

// path is used for loading script assets in one time
bool ScriptMgr::playScript(const std::string &fullFileName)
{
	cLog::get()->write("ScriptMgr: load "+ fullFileName, LOG_TYPE::L_INFO);
	cLog::get()->write("ScriptMgr: load "+ fullFileName, LOG_TYPE::L_INFO, LOG_FILE::SCRIPT);

	std::string script_file, script_path;
	CallSystem::splitFilename(fullFileName, script_path , script_file);
	//~ cout << "fullname: " << fullFileName << endl;
	//~ cout << "script_path: " << script_path << endl;
	//~ cout << "script_file: " << script_file << endl;

	if ( script->load(fullFileName, script_path) ) {
		multiplierRate=1;
		scriptState = ScriptState::PLAY;
		wait_time = 0;
		return 1;
	}
	return 0;
}

/*
 * adds the given script at the begining of the current script queue
 */
bool ScriptMgr::addScriptFirst(const std::string & script)
{
	std::vector <Token*> commands;
	Token *token=nullptr;
	std::istringstream iss(script);
	std::string line;

	//get the tokens into a vector
	while (getline(iss, line)){
		// transformation of the beginning of character strings by deleting spaces and tabs at the beginning of the string
		while (line[0]==' ' || line[0]=='\t') {
	        line.erase(0,1);
			//std::cout << str << std::endl;
		}
		// consideration of lines
		if ( line[0] != '#' && line[0] != 0 && line[0] != '\r' && line[0] != '\n') {
			token = new Token(line, getScriptPath());
			commands.push_back(token);
		}
	}
	//add the tokens to the queue in reverse order (since we add to the begining of the queue
	for (auto it = commands.rbegin(); it != commands.rend(); it++){
		this->script->addFirstInQueue(*it);
	}
	return true;
}

void ScriptMgr::cancelScript()
{
	cLog::get()->write("ScriptMgr: script end", LOG_TYPE::L_INFO, LOG_FILE::SCRIPT);
	// delete script object...
	script->clean();
	// images loaded are deleted from stel_command_interface directly
	scriptState = ScriptState::NONE;
	multiplierRate = 1;
	nbrLoop =0;
	indiceInLoop=0;
	DataDir ="";
	loopVector.clear();
	isInLoop = false;
	repeatLoop = false;
	waitOnVideo = false;
}

void ScriptMgr::pauseScript()
{
	if (scriptState==ScriptState::NONE)	return;

	scriptState=ScriptState::PAUSE;
	media->audioMusicPause();
	commander->executeCommand("timerate action pause");
	cLog::get()->write("ScriptMgr::script action pause", LOG_TYPE::L_INFO, LOG_FILE::SCRIPT);
}

void ScriptMgr::resumeScript()
{
	if (scriptState==ScriptState::NONE)	return;
	scriptState=ScriptState::PLAY;

	media->audioMusicResume();
	commander->executeCommand("timerate action resume");
	cLog::get()->write("ScriptMgr::script action resume", LOG_TYPE::L_INFO, LOG_FILE::SCRIPT);
}

bool ScriptMgr::isFaster() const
{
	return (multiplierRate!=1);
}

void ScriptMgr::fasterSpeed()
{
	if(scriptState != ScriptState::PLAY)
		return;

	if (multiplierRate==1)
		media->audioMusicMute();

	if (multiplierRate>4)
		return;

	multiplierRate *=2;
}

void ScriptMgr::slowerSpeed()
{
	if(scriptState != ScriptState::PLAY)
		return;

	if (multiplierRate>1)
		multiplierRate /=2;

	if (multiplierRate == 1) {
		media->audioMusicSync();
		media->audioMusicResume();
	}
}

void ScriptMgr::defaultSpeed()
{
	multiplierRate = 1;
	media->audioMusicSync();
	media->audioMusicResume();
}

std::string ScriptMgr::getRecordDate()
{
	time_t tTime = time(NULL);
	tm * tmTime = localtime (&tTime);
	char timestr[20];
	strftime(timestr, 20, "%y-%m-%d_%Hh%Mm%S", tmTime);
	return std::string(timestr);
}

void ScriptMgr::recordScript(const std::string &script_filename)
{
	if (sR.recording) {
		cLog::get()->write("ScriptMgr::Already recording script", LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
		sR.rec_file.close();
		sR.recording = false;
		cLog::get()->write("ScriptMgr::Script recording stopped.", LOG_TYPE::L_INFO, LOG_FILE::SCRIPT);
		return;
	}

	if (!script_filename.empty()) {
		sR.rec_file.open(script_filename.c_str(), std::fstream::out);
	} else {
		std::string sdir, other_script_filename;
		sdir = AppSettings::Instance()->getConfigDir();

		other_script_filename = sdir + "record_" + this->getRecordDate() + ".sts";
		sR.rec_file.open(other_script_filename.c_str(), std::fstream::out);
	}

	if (sR.rec_file.is_open()) {
		sR.recording = true;
		sR.record_elapsed_time = 0;
		sR.rec_file << "# Spacecrafter "<< AppSettings::Instance()->getVersion() << std::endl;
		sR.rec_file << "# Script recorded "<< this->getRecordDate() << std::endl << "#" << std::endl;
		cLog::get()->write("ScriptMgr::Now recording actions to file: " + script_filename, LOG_TYPE::L_INFO, LOG_FILE::SCRIPT);
	} else {
		cLog::get()->write("ScriptMgr::Error opening script file for writing: " + script_filename, LOG_TYPE::L_ERROR, LOG_FILE::SCRIPT);
	}
}

void ScriptMgr::recordCommand(const std::string &commandline)
{
	if (sR.recording) {
		// write to file...
		if (sR.record_elapsed_time) {
			//on s'occupe de toutes les attentes mais on ne garde qu'un chiffre après la virgule
			double timeToWait = (sR.record_elapsed_time/100)/10.f;
			if (timeToWait>0.4)
				sR.rec_file << "wait duration " << timeToWait << std::endl;
			else
				sR.rec_file << "wait duration 0.1"<< std::endl;
			sR.record_elapsed_time = 0;
		}
		sR.rec_file << commandline << std::endl;
		// For debugging
		cLog::get()->write("RECORD: " + commandline, LOG_TYPE::L_DEBUG, LOG_FILE::SCRIPT);
	}
}

void ScriptMgr::cancelRecordScript()
{
	// close file...
	sR.rec_file.close();
	sR.recording = false;
	cLog::get()->write("ScriptMgr::Script recording stopped.", LOG_TYPE::L_INFO, LOG_FILE::SCRIPT);
}

void ScriptMgr::resetScriptLoop()
{
	nbrLoop = 0;
	indiceInLoop = 0;
	loopVector.clear();
	isInLoop = false;
	repeatLoop = false;
}

// runs maximum of one command per update note that waits can drift by up to 1/fps seconds
void ScriptMgr::update(int delta_time)
{
	if (sR.recording) sR.record_elapsed_time += delta_time;

	/**	isVideoPlayed && waitOnVideo :
	 * case of video is playing and scriptMgr should wait on it :
	 * so next if must be false*/
	if (scriptState==ScriptState::PLAY && (isVideoPlayed && waitOnVideo ? false : true) ) {

    wait_time -= delta_time;
		if (wait_time<0)
			wait_time =0;

		while (wait_time==0) {
			std::string comd;

			unsigned long int wait=0;

			if (repeatLoop) {
				//~ printf("loop tour %i\n", nbrLoop);
				if (indiceInLoop < loopVector.size()) {
					commander->executeCommand(loopVector[indiceInLoop], wait);
					wait_time += wait;
					indiceInLoop++;
				} else { //at the end of the loop, we start again except if nbrLoop==0
					nbrLoop=nbrLoop-1;

					if (nbrLoop <= 0) {
						repeatLoop = false;
						indiceInLoop = 0;
						loopVector.clear();
					} else {
						indiceInLoop = 0;
					}
				}
			} else if ( (script->getFirst(comd,DataDir)) == 1 ) {

				if (isInLoop) {//on est dans une boucle et on doit copier la boucle dans une list.
					loopVector.push_back(comd);
				}
				commander->executeCommand(comd, wait);
				wait_time += wait;
			} else {
				// script done
				DataDir = "";
				commander->terminateScript();
				return;
			}
		}
	}
}

// get a list of script files from directory in alphabetical order
std::string ScriptMgr::getScriptList(const std::string &directory)
{
	std::multiset<std::string> items;
	std::multiset<std::string>::iterator iter;

	struct dirent *entryp;
	DIR *dp;
	std::string tmp;

	if ((dp = opendir(directory.c_str())) != NULL) {
		// TODO: sort the directory
		while ((entryp = readdir(dp)) != NULL) {
			tmp = entryp->d_name;

			if (tmp.length()>4 && tmp.find(".sts", tmp.length()-4)!=std::string::npos ) {
				items.insert(tmp + "\n");
				//cout << entryp->d_name << endl;
			}
		}
		closedir(dp);
	} else {
		cLog::get()->write("ScriptMgr::Unable to read script directory", LOG_TYPE::L_ERROR);
	}
	std::string result="";
	for (iter=items.begin(); iter!=items.end(); iter++ ) {
		result += (*iter);
	}

	return result;

}

std::string ScriptMgr::getScriptPath()
{
	if (DataDir=="") {
		//printf("getScriptPath1 return : %s\n", AppSettings::Instance()->getScriptDir().c_str());
		return AppSettings::Instance()->getScriptDir();
	} else {
		//printf("getScriptPath2 return : %s\n", DataDir.c_str());
		return DataDir;
	}
}

// look for a script called "startup.sts"
bool ScriptMgr::playStartupScript()
{
	std::string CDIR = AppSettings::Instance()->getScriptDir() + "fscripts/";
	return playScript(CDIR + "startup.sts");
}
