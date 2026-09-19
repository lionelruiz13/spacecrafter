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
#include <cstdint>


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

constexpr int LOG_RETENTION_LAUNCHES = 8;

constexpr std::uintmax_t LOG_RETENTION_BYTES = 1024u * 1024u * 1024u;

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
	//! report is pushed here - once, whoever turns it on and whenever.  This is
	//! also where the console's fate stops being unknown, which is what lets
	//! rotateForBudget stop buffering for it.
	void setDebug(bool debugging) {
		debugDecided = true;
		isDebug = debugging;
		if (debugging)
			reportOpenLogConsole();
		else
			std::vector<std::string>().swap(budgetReport);
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

	void openLog(const LOG_FILE& fichier, const std::string& LogfilePath);

	void reportOpenLog();

	void setDirectory(const std::string &directory) {
		logDirectory = directory;
	}

	static void writeECLog(const std::string &string, LogType type);
private:
    static cLog *singleton;
	cLog();

	struct Channel {
		std::ofstream file;
		//! <path>.log, <path>.1.log ... relative to logDirectory.
		std::string path;
		std::uintmax_t live = 0;
		std::uintmax_t archived = 0;
	};

	struct RotationRecord {
		bool rotated = false;
		std::string deleted;
		std::uintmax_t deletedSize = 0;
		int kept = 0;
		std::uintmax_t archived = 0;
	};

	std::mutex writeMutex;
	std::map<const LOG_FILE, Channel> logFile;
	std::string logDirectory = "";
	std::uintmax_t budgetUsed = 0;

	void writeConsole(const std::string&, const LOG_TYPE&);
	RotationRecord rotate(const std::string& LogfilePath);
	//! The open-time D12 line, and the legacy-pile line when there is a pile.
	void reportOpen(const std::string& LogfilePath, const RotationRecord& rec);
	void writeLocked(const std::string& texte, const LOG_TYPE& type, const LOG_FILE& fichier);
	//! The bound acting: rotate the channel holding the most bytes, open a
	//! fresh live file for it, and write the one D12 line that says so.
	void rotateForBudget();
	void reportOpenLogConsole();
	//! One line per channel opened (plus one per legacy pile found), kept for
	//! the lifetime of the process: both sinks are served from this one text.
	std::vector<std::string> openReport;
	std::vector<std::string> budgetReport;
	bool openReportLogged = false;
	bool openReportOnConsole = false;
	bool debugDecided = false;
	bool isDebug = false;
	bool isWritingLog = true;
};

#endif
