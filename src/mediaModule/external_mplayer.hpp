/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2007 Digitalis Education Solutions, Inc.
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


#ifndef _EXTERNAL_MPLAYER_H_
#define _EXTERNAL_MPLAYER_H_

#include <string>


//! @class ExternalMplayer
//! 
//! @brief Permet l'utilisation du player externe nommé mplayer
//!
//! Mplayer est utilisé comme lecteur externe afin de lire des fichiers audio et vidéo.
//! On utilise mplayer en mode esclave, avec un mkfifo, en mode hide
//! cela signifie que mplayer tourne en fond de tache et qu'il écoute le mkfifo
//!
//! Si mplayer se ferme, alors la fonction update() qui se lance toutes les
//! secondes relance le serveur mplayer en cas de crash ou d'imprévu.
//! writeToMplayer est la fonction interne qui se charge d'envoyer le tampon d'écriture
//! au mkfifo.
//!


class ExternalMplayer {
public:
	ExternalMplayer(unsigned int _width, unsigned int _height);
	ExternalMplayer(ExternalMplayer const &) = delete;
	ExternalMplayer& operator=(ExternalMplayer const &) = delete;

	int play(const std::string &filename);
	void jumpAbsolute(int secondes);
	void jumpRelative(int secondes);
	void volume(int sound);
	void speed(int lvl);
	void execute(const std::string &msg);
	void pause();
	void stop();
	virtual ~ExternalMplayer();

	void update(int delta_time);
	void init(const std::string &mplayerFileName, const std::string &mplayerMkfifoName, bool enable_mplayer);
private:
	void initMkfifo(bool enable);
	void launchMplayer();
	std::string controlScript;
	bool isPlaying=false;
	bool serviceAvariable=false;
	std::string mplayerMkfifo;
	std::string mplayerName;
	int mkfifoFile;
	std::string order;
	bool writeToMplayer(const std::string &msg);
	pid_t mplayerPid;
	unsigned int width, height;
};

#endif // _EXTERNAL_MPLAYER_H_
