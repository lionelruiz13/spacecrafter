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

#ifndef cLog_H
#define cLog_H

#include <ostream>
#include <iostream>
#include <fstream>
#include <mutex>
#include <sstream>
#include <map>
#include <string>
#include <vector>


/**
 * \enum LOG_TYPE
 * \brief Types de log
 */
enum class LOG_TYPE : char {
	L_WARNING,
	L_ERROR,
	L_DEBUG,
	L_INFO,
	L_OTHER
};

/**
 * \enum LOG_FILE
 * \brief Fichiers de log
 */
enum class LOG_FILE : char {
	INTERNAL,
	SCRIPT,
	SHADER,
	TCP,
	VULKAN
};

// From VulkanMgr
enum class LogType : unsigned char;

//! \brief How many launches of the application keep a log, on EVERY channel.
//!
//! The current launch's file plus LOG_RETENTION_LAUNCHES-1 numbered archives:
//! spacecrafter.log, spacecrafter.1.log ... spacecrafter.7.log, and the same
//! for script, tcp, shader and vulkan.  openLog rotates at open and deletes
//! whatever falls outside the window, so no channel can grow without bound.
//!
//! The NUMBER is the main tester's, and the unit is his choice too: asked on
//! 2026-09-05 whether the window should be "the last N days" or "N launches",
//! he answered [stated: tester (Lionel RUIZ), via owner commit 6ffb017]:
//! "8 launches".  The launch unit is why the per-day script-YY.MM.DD.log
//! layout had to go: a window expressed in launches needs the launch boundary,
//! which a dated file does not carry (eight sessions in one day were one file).
//! Ledger: Sec.5.115 (the uncapped script log - gigabyte-scale at clients, and
//! measured here at 1.88 MB/h on a shipped playlist and 193 MB/h on the
//! tester's own corpus), Sec.11.173(b) (why the cap is UNIFORM across every
//! channel rather than script-only), Sec.11.230 (this change).
//!
//! It is a COMPILED constant and deliberately NOT a config.ini key: the log
//! files are opened at main.cpp:215-219, before checkConfigIni and before the
//! config is parsed (main.cpp:258, :264-266), so a rotation has no key to read
//! at the moment it must act.  Changing the window means changing this line
//! and rebuilding - which is what the D12 line written at every open says.
constexpr int LOG_RETENTION_LAUNCHES = 8;

class cLog {
public:
	cLog(cLog const&) = delete;
	cLog& operator=(cLog const&) = delete;
	~cLog();

    //! to get the singleton
    static cLog *get() {
		if (!singleton)
          singleton = new cLog();

       return singleton;
    }

	/*!
	*  \brief Write a string to a log
	*  \param type : type enum (optional, INFO by default)
	*  \param fichier : file enum (optional, INTERNAL by default)
	*/
	void write(const std::string& texte, const LOG_TYPE& type = LOG_TYPE::L_INFO, const LOG_FILE& fichier = LOG_FILE::INTERNAL);

	/*!
	*  \brief Writes a stream to a log
	*  \param type : enum of type (optional, INFO by default)
	*  \param fichier : file enum (optional, INTERNAL by default)
	*/
	inline void write(const std::ostringstream& texte, const LOG_TYPE& type = LOG_TYPE::L_INFO, const LOG_FILE& fichier = LOG_FILE::INTERNAL) {
		write(texte.str(), type, fichier);
	}

	/*!
	*  \brief Inserts a mark in the log
	*  \param fichier : file enum (optional, INTERNAL by default)
	*/
	void mark(const LOG_FILE& fichier = LOG_FILE::INTERNAL);

	/*!
	*  \brief Set the Debug state
	*  \param debugging : desired Debug state (true or false)
	*/
	//! Turning the console on is the moment the console becomes a reachable
	//! sink for the open-time rotation report, which was written to the log
	//! file long before this value was known (main.cpp:220 vs :268), so the
	//! report is pushed here - once, whoever turns it on and whenever.
	void setDebug(bool debugging) {
		isDebug = debugging;
		if (debugging)
			reportOpenLogConsole();
	}

	void setWriteLog(bool writelog) {
		isWritingLog = writelog;
	}

	/*!
	*  \brief Returns the Debug state
	*  \return true if the Debug is activated, false otherwise
	*/
	bool getDebug() {
		return isDebug;
	}

	void close();

	//! Open one channel's log file, rotating the previous launches first
	//! (see LOG_RETENTION_LAUNCHES).  The file is always <LogfilePath>.log:
	//! the CURRENT launch keeps the channel's own name on every channel.
	void openLog(const LOG_FILE& fichier, const std::string& LogfilePath);

	//! Write the rotation report of every channel opened so far to the
	//! INTERNAL channel.  Called once, from main.cpp, as soon as all the
	//! channels are open: openLog itself cannot write it, because the first
	//! channel rotates before any file exists to write into.
	void reportOpenLog();

	void setDirectory(const std::string &directory) {
		logDirectory = directory;
	}

	static void writeECLog(const std::string &string, LogType type);
private:
    static cLog *singleton;
	cLog();

	std::mutex writeMutex;
	std::map<const LOG_FILE, std::ofstream> logFile;
	std::string logDirectory = "";

	void writeConsole(const std::string&, const LOG_TYPE&);
	//! Rotate one channel's file family and record what that did in openReport.
	void rotate(const std::string& LogfilePath);
	void reportOpenLogConsole();
	//! One line per channel opened (plus one per legacy pile found), kept for
	//! the lifetime of the process: both sinks are served from this one text.
	std::vector<std::string> openReport;
	bool openReportLogged = false;
	bool openReportOnConsole = false;
	bool isDebug = false;
	bool isWritingLog = true;
};

#endif
