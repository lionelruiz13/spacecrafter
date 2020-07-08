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
	TCP
};

// tag à mettre en début de nom pour informer que l'on souhaite conserver l'historique
#define KEEP_LOG_HISTORY_TAG '$'
// ^
// |
// très bonne idée !!!

class cLog {
public:
	cLog(cLog const&) = delete;
	cLog& operator=(cLog const&) = delete;
	~cLog();

    //! pour obtenir le singleton
    static cLog *get()    {
		if (!singleton)
          singleton = new cLog();

       return singleton;
    }

	/*!
	*  \brief Ecrit une string dans un log
	*  \param type : enum du type (optionnel, INFO par defaut)
	*  \param fichier : enum du fichier (optionnel, INTERNAL par defaut)
	*/
	void write(const std::string& texte, const LOG_TYPE& type = LOG_TYPE::L_INFO, const LOG_FILE& fichier = LOG_FILE::INTERNAL);

	/*!
	*  \brief Ecrit un flux dans un log
	*  \param type : enum du type (optionnel, INFO par defaut)
	*  \param fichier : enum du fichier (optionnel, INTERNAL par defaut)
	*/
	inline void write(const std::ostringstream& texte, const LOG_TYPE& type = LOG_TYPE::L_INFO, const LOG_FILE& fichier = LOG_FILE::INTERNAL) {
		write(texte.str(), type, fichier);
	}

	/*!
	*  \brief Insère une marque dans le log
	*  \param fichier : enum du fichier (optionnel, INTERNAL par defaut)
	*/
	void mark(const LOG_FILE& fichier = LOG_FILE::INTERNAL);

	/*!
	*  \brief Configure l'état du Debug
	*  \param debugging : état du Debug souhaité (true ou false)
	*/
	void setDebug(bool debugging) {
		isDebug = debugging;
	}

	/*!
	*  \brief Retourn l'état du Debug
	*  \return true si le Debug est activé, false sinon
	*/
	bool getDebug() {
		return isDebug;
	}

	/*!
	*  \brief Ouvre les fichiers de log
	*  \param LogfilePath : chemin du fichier de log général
	*  \param scriptLogfilePath : chemin du fichier de log de script
	*  \param openglLogfilePath : chemin du fichier de log OpenGL
	*  \param tcpLogfilePath : chemin du fichier de log TCP
	*/

	void close();

	void openLog(const LOG_FILE& fichier, const std::string& LogfilePath);
private:
    static cLog *singleton;
	cLog();

	std::mutex writeMutex;
	std::map<const LOG_FILE, std::ofstream> logFile;

	void writeConsole(const std::string&, const LOG_TYPE&);
	std::string getDate();
	bool isDebug = false;
};

#endif
