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

//! \brief How many BYTES the application's own log files may occupy in total.
//!
//! LOG_RETENTION_LAUNCHES bounds how many launches are kept; it bounds no
//! amount of writing.  ONE launch is unbounded under it, and that is not a
//! theoretical case: the owner ran the pre-Vulkan version non-stop for over a
//! year [vixy 2026-09-12], the script channel grows at 193 MB/h under the main
//! tester's own shows and 1.88 MB/h under the shipped playlist (Sec.11.218(m),
//! Sec.11.215(i)), so "the proxy the tester said (number of sessions) may fail
//! the same way".  The bound and its semantics are the owner's, verbatim:
//! "1 GiB TOTAL across channels; rotate within a session at the bound".
//!
//! TOTAL means the sum over the five channels of the current launch's file and
//! its numbered archives - 40 files at the steady state, all of them ours to
//! delete.  Deliberately NOT in that sum: the legacy dated script-YY.MM.DD.log
//! pile of the pre-Sec.11.230 layout (this build never writes, reads or deletes
//! it, so counting bytes that nothing here can free would make every line
//! rotate forever), EntityCore's own EntityCore-logs-*.txt pile (a different
//! writer, outside these five channels), and any other file in the directory.
//!
//! WHEN THE TOTAL CROSSES IT, THE CHANNEL HOLDING THE MOST BYTES ROTATES, as if
//! it were opening: its live file becomes <channel>.1.log, the oldest archive
//! is deleted, the window stays within LOG_RETENTION_LAUNCHES files, and a
//! fresh live file opens.  The largest channel rather than the channel being
//! written, because the budget is a TOTAL: after a session fills it on one
//! channel and goes quiet there, rotating whichever channel a line happens to
//! land on would rotate a 13-byte channel on every line, free nothing, and
//! append one report line each time - growth without bound, i.e. Sec.5.115
//! recreated by its own fix.  Rotating the largest sheds the bytes that are
//! actually over the budget, so the total falls within at most
//! LOG_RETENTION_LAUNCHES rotations, and it consumes a retention slot only on
//! the channel that grew.  Two consequences, accepted with the number and
//! recorded as such (Sec.11.237): an in-session rotation spends one of the
//! LOG_RETENTION_LAUNCHES slots, so the channel then reaches fewer launches
//! back; and a channel whose single live file approaches the whole budget keeps
//! its history only until the rotations walk it out of the window.
//!
//! Like LOG_RETENTION_LAUNCHES this is a COMPILED constant and not a config.ini
//! key, for the same reason (the channels are opened before the config is
//! parsed) plus one more: a key in checkConfigIni would rewrite the field's own
//! config.ini.  Changing the budget means changing this line and rebuilding -
//! which is what the D12 line written at each in-session rotation says.
//! Ledger: Sec.5.115 (the retention half: launches by Sec.11.230, bytes here),
//! Sec.11.233(b) (the owner's decision), Sec.11.237 (this change).
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

	//! One channel: its stream, the name its files are built from, and the two
	//! byte counts LOG_RETENTION_BYTES is checked against.  They live beside
	//! the stream because the channel owns its own file family (I4) and because
	//! write() then reaches the counter through the lookup it already does.
	struct Channel {
		std::ofstream file;
		//! <path>.log, <path>.1.log ... relative to logDirectory.
		std::string path;
		//! Bytes written into the live file since it was opened.  Kept by
		//! write() itself: it is the one place every byte of every channel
		//! passes, so the count is exact and costs one add (Sec.2.0 D11 - a
		//! file_size() per line would be a syscall per line).
		std::uintmax_t live = 0;
		//! Bytes held by <path>.1.log ... <path>.(LOG_RETENTION_LAUNCHES-1).log.
		//! Measured with file_size ONLY where it can change: at open and at
		//! each rotation.
		std::uintmax_t archived = 0;
	};

	//! What one rotation did, so the caller can put it where it belongs: the
	//! open-time report is buffered (no file exists yet to receive it) while an
	//! in-session rotation writes its line immediately.
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
	//! The sum of every open channel's live + archived bytes: the quantity
	//! LOG_RETENTION_BYTES bounds.  Maintained incrementally by write() and
	//! recomputed from the channels at every rotation.
	std::uintmax_t budgetUsed = 0;

	void writeConsole(const std::string&, const LOG_TYPE&);
	//! Rotate one channel's file family: delete what falls out of the window,
	//! shift the archives up, move the live file to <path>.1.log.  Says what it
	//! did rather than reporting it, because it serves both callers.
	RotationRecord rotate(const std::string& LogfilePath);
	//! The open-time D12 line, and the legacy-pile line when there is a pile.
	void reportOpen(const std::string& LogfilePath, const RotationRecord& rec);
	//! write() with writeMutex already held, and the place the byte counter is
	//! kept.  Returns nothing: the budget is checked by the caller, so a line
	//! written FROM a rotation cannot start another one.
	void writeLocked(const std::string& texte, const LOG_TYPE& type, const LOG_FILE& fichier);
	//! The bound acting: rotate the channel holding the most bytes, open a
	//! fresh live file for it, and write the one D12 line that says so.
	void rotateForBudget();
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
