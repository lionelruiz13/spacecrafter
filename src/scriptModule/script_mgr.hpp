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
#include <memory>
#include "tools/no_copy.hpp"

class AppCommandInterface;
class Media;
class Script;

class ScriptMgr: public NoCopy {

public:
	ScriptMgr(std::shared_ptr<AppCommandInterface> command_interface, const std::string &_data_dir, std::shared_ptr<Media> _media);
	~ScriptMgr();

	//! play a script
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
	void defaultSpeed();

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
		return (scriptState!=ScriptState::NONE);
	};

	//! is a script paused ?
	bool isPaused() {
		return (scriptState==ScriptState::PAUSE);
	};

	//! is a script being recorded?
	bool isRecording() const {
		return 	sR.recording;
	};

	bool isFaster() const;

	//! execute commands in running script
	void update(int delta_time);

	//! get list of scripts in a directory
	std::string getScriptList(const std::string &directory);

	//! indicates the absolute path from where the script was launched.
	std::string getScriptPath();

	//! @deprecated ?
	//! what is the use of this function if in load we already have the information ?
	void setPath(const std::string &path) {
		DataDir=path;
	}

	//! indicates if we are in a loop or not
	void setLoop(bool a) {
		isInLoop=a;
	}

	//! initialization of the loop system
	void initIterator() {
		indiceInLoop= 0;
		if (loopVector.size()) {
			repeatLoop = true;
			loopVector.pop_back(); //we remove the loop end structure too
		}
	}

	//! set the number of loop turns to do
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
	void setWaitOnVideoTermination(bool b) {
		waitOnVideo = b;
	}

	void setIsVideoPlayed(bool b) {
		isVideoPlayed = b;
	}

	void setFlagScriptPause(bool b) {
		flagScriptPause = b;
	}

	bool getFlagScriptPause(void) const {
		return flagScriptPause;
	}

private:
	// the states of the script engine with respect to the current scripts.
	enum class ScriptState : char {PLAY, PAUSE, NONE};
	ScriptState scriptState = ScriptState::NONE;
	// record management
	struct ScriptRecord {
		std::fstream rec_file;	//!< the pointer to the file
		bool recording;  		//!< is a script being recorded?
		uint64_t record_elapsed_time;  //!< ms since last command recorded
	};
	ScriptRecord sR;

	std::string getRecordDate();
	// external classes
	std::shared_ptr<Media> media;
	AppCommandInterface *commander;  //!< for executing script commands
	Script * script = nullptr; //!< currently loaded script
	int64_t wait_time=0;     //!< ms until next script command should be executed
	bool waitOnVideo=false; 			//!< if Video launch, say if script should wait on it.
	bool isVideoPlayed = false;		 	//!< say if a video is played
	std::string DataDir;
	int multiplierRate=1;
	bool isInLoop=false; 		//!< we are reading the instructions of a loop
	bool repeatLoop=false; 	//!< we are repeating a loop
	int nbrLoop=0;		//!< number of remaining loops
	std::vector<std::string> loopVector; //!< the vector that contains the loop instructions to be repeated
	unsigned int indiceInLoop=0; //!< indicates the place where we are in the loop
	bool flagScriptPause; //!< skip pause in script
};


#endif // _SCRIPT_MGR_H
