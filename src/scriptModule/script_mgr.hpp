/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2005 Robert Spearman
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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

/* This class handles parsing of a simple command syntax for scripting,
   UI components, network commands, etc.
*/

#ifndef _SCRIPT_MGR_H_
#define _SCRIPT_MGR_H_

#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include "tools/no_copy.hpp"

class AppCommandInterface;
class Media;
class Script;

class ScriptMgr: public NoCopy {

public:
	ScriptMgr(AppCommandInterface * command_interface, const std::string &_data_dir, Media * _media);
	~ScriptMgr();

	//! lance un script 
	bool playScript(const std::string &fullFileName);

	//! place the given script at the begin of the command queue
	bool addScriptFirst(const std::string & script);

	//! play script called "startup.sts"
	bool playStartupScript();

	//! stop playing current script
	void cancelScript();

	//! pause current script
	void pauseScript();

	//! play faster current script
	void fasterSpeed();

	//! play slower current script
	void slowerSpeed();

	//! play script without acceleration
	void defaultSpeed(){
		multiplierRate = 1;
	}

	//! start playing paused script
	void resumeScript();

	//! begin recording user interactions
	void recordScript(const std::string &script_filename);

	//! record a command (if recording)
	void recordCommand(const std::string &commandline);

	//! stop recording user interactions
	void cancelRecordScript();

	//! is a script playing?
	bool isPlaying() const {
		return playing;
	};

	//! is a script paused ?
	bool isPaused() {
		return play_paused;
	};

	//! is a script being recorded?
	bool isRecording() const {
		return recording;
	};

	bool isFaster() const;

	//! execute commands in running script
	void update(int delta_time);

	//! get list of scripts in a directory
	std::string getScriptList(const std::string &directory);

	//! indique le chemin absolu d'où le script à été lancé.
	std::string getScriptPath();

	//! @deprecated ?
	//! a quoi sert cette fonction si dans load on a déjà l'information ?
	void setPath(const std::string &path) {
		DataDir=path;
	}

	//! indique si on est dans une boucle ou pas
	void setLoop(bool a) {
		isInLoop=a;
	}

	//! initialisation du systeme de boucle
	void initIterator() {
		indiceInLoop= 0;
		if (loopVector.size()) {
			repeatLoop = true;
			loopVector.pop_back(); //on enlève le struct loop end de trop
		}
	}

	//! fixe le nombre de tour de boucle à faire
	void setNbrLoop( int a) {
		nbrLoop=a;
	}

	void resetScriptLoop();

	int getMuliplierRate() const {
		return multiplierRate;
	}

	void waitOnVideoTermination() {
		waitOnVideo = !waitOnVideo;
	}

	void setIsVideoPlayed(bool b) {
		isVideoPlayed = b;
	} 

private:
	std::string getRecordDate();
	Media* media = nullptr;
	AppCommandInterface * commander = nullptr;  //!< for executing script commands
	Script * script = nullptr; //!< currently loaded script
	long int wait_time;     //!< ms until next script command should be executed
	unsigned long int record_elapsed_time;  //!< ms since last command recorded
	bool recording;  			//!< is a script being recorded?
	bool playing;    			//!< is a script playing?  (could be paused)
	bool play_paused;			//!< is script playback paused?
	bool waitOnVideo; 			//!< if Video launch, say if script should wait on it.
	bool isVideoPlayed;		 	//!< say if a video is played
	std::fstream rec_file;		//!< le pointeur sur le fichier
	std::string DataDir;

	int multiplierRate=1; 
	bool isInLoop; 		//!< on est entrain de lire les instructions d'une loop
	bool repeatLoop; 	//!< on est entrain de répéter une boucle
	int nbrLoop;		//!< nombre de tours de boucles restants
	std::vector<std::string> loopVector; //!< le vector qui contient les instructions de loop à répéter
	unsigned int indiceInLoop; //!< indique l'endroit ou l'on se trouve dans la loop
};


#endif // _SCRIPT_MGR_H
