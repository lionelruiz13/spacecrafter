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

#ifndef ncLog_H
#define ncLog_H

#include <ostream>
#include <iostream>
#include <fstream>
#include <mutex>
#include <sstream>


	/**
	 * \enum LOG_TYPE
	 * \brief Types de log
	 */
    enum class LOG_TYPE : char {
	    L_OTHER = 0,
        L_INFO,
	    L_DEBUG,
	    L_WARNING,
	    L_ERROR
    };


class ncLog {
public:
	ncLog(ncLog const&) = delete;
	ncLog& operator=(ncLog const&) = delete;
	~ncLog(){};

    //! pour obtenir le singleton
    static ncLog *get()    {
       
       if (!singleton)
          singleton = new ncLog();
       
       return singleton;
    }

	/*!
	*  \brief Ecrit une string dans un log
	*  \param type : enum du type (optionnel, INFO par defaut)
	*/
	void write(const std::string& texte, const LOG_TYPE& type = LOG_TYPE::L_INFO);

	/*!
	*  \brief Ecrit un flux dans un log
	*  \param type : enum du type (optionnel, INFO par defaut)
	*/
	inline void write(const std::ostringstream& texte, const LOG_TYPE& type = LOG_TYPE::L_INFO) {
		write(texte.str(), type);
	}

	/*!
	*  \brief Insère une marque dans le log
	*/
	void mark();

	/*!
	*  \brief Configure l'état du level à notifier
	*  \param debugging : valeur du level souhaité 
	*/
	void setLevel(LOG_TYPE _value) {
		level = _value;
	}

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
	*  \param LogfileName : nom complet du fichier de log 
	*/
	void open(const std::string& LogfileName);
    void close();

private:
    static ncLog *singleton;
    ncLog(){};
	std::mutex writeMutex;
	std::ofstream Logfile;

	void writeConsole(const std::string&, const LOG_TYPE&);
	std::string getDate();
    bool isDebug = false;
	LOG_TYPE level = LOG_TYPE::L_INFO;
};



#endif
