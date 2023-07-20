/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2018 of Association Sirius
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

#ifndef SCRIPT_INTERFACE_HPP
#define SCRIPT_INTERFACE_HPP

#include <string>
#include <memory>

class ScriptMgr;

class ScriptInterface {
public:
    ScriptInterface(std::shared_ptr<ScriptMgr> _scriptMgr);
    ~ScriptInterface();


	std::string getSelectedScript() const {
		return SelectedScript;
	}

	std::string getSelectedScriptDirectory() const {
		return SelectedScriptDirectory;
	}

	std::string getScriptPath() const;

	void slowerSpeed();
	void fasterSpeed();
	void defaultSpeed();
	void resumeScript();
	void cancelScript();
	void pauseScript();
	// void resetScriptTimer();

	void cancelRecordScript();
	void initScriptIterator();

	void setScriptNbrLoop(int a);

	void setScriptLoop(bool _value);
	void resetScriptLoop();

	void setScriptPath(const std::string& _path);

	bool addScriptFirst(const std::string& string);

	bool playScript(const std::string& _script);

	void recordScript(const std::string &script_filename);
	void recordCommand(const std::string &commandline);

	void setSelectedScript(std::string filename) {
		SelectedScript = filename + ".sts";
	}

	void setSelectedScriptDirectory(std::string path) {
		SelectedScriptDirectory = path;
	}

	std::string getScriptList(const std::string &directory) const;

	bool isScriptPlaying() const;
	bool isScriptRecording() const;
	bool isScriptPaused() const;
    bool isScriptPauseDisabled() const;
    void setScriptPauseDisabled(bool b);

	void waitOnVideoTermination() const;
	void setWaitOnVideoTermination(bool b) const;

	void setIsVideoPlayed(bool b) const;

private:
    std::shared_ptr<ScriptMgr> scriptMgr;

    // Script related
	std::string SelectedScript;  //! script filename (without directory) selected in a UI to run when exit UI
	std::string SelectedScriptDirectory;  //! script directory for same
};



#endif //SCRIPT_INTERFACE_HPP
