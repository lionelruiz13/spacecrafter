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

/**
 * @file audio.hpp
 * @brief manage audio track
 *
 * @author Olivier NIVOIX
 * @version 2
 *
 */

#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "tools/no_copy.hpp"


//! enumération des états de lecture d'un fichier audio
enum class A_STATE: char { V_NONE, // fichier non chargé
                           V_PAUSE,// lecture du fichier en pause
                           V_PLAY, // lecture du fichier en cours
                           V_STOP  // lecture du fichier stoppée
                         };



/**
 * @class Audio
 * @brief Initialise la lib SDL2_mixer et effectue les opérations sur un fichier musique AudioMusic
 *
 * La classe sert d'intermédiaire entre Média et l'audio.
 * Elle effectue tous les contrôles pour pouvoir lire un fichier musique.
 *
 * Par défaut, elle initialise le driver audio pour lire des musiques en 44100 Hz, stéréo, en unsigned 16 bits
 *
 */


class Audio: public NoCopy {

public:
	Audio();
	Audio(int Frequency, int channel, int chunksize);
	~Audio();
	//! indique combien de temps s'est écoulé depuis le dernier tour de boucle
	void update(int delta_time);
	//! fixe le volume audio
	void setVolume(int _value);
	//! diminue le volume audio
	void incrementVolume(int value=5);
	//! augmente le volume audio
	void decrementVolume(int value=5);

	/* ordres possibles pour la lecture d'un fichier audio*/
	//! charge un fichier dans le gestionnaire
	void musicLoad(const std::string& filename, bool _loop);
	//! entame la lecture d'un fichier audio chargé précédemment
	void musicPlay();
	//! met la lecture du fichier en pause. Si réapliqué, recommence la lecture du fichier
	void musicPause();
	//! reprend la lecture du fichier en pause.
	void musicResume();
	//! recommence la lecture du fichier au départ
	void musicRewind();
	//! stoppe la lecture du fichier
	void musicHalt();
	//! synchronise le fichier son avec le temps écoulé dans la théorie
	void musicSync();
	//! décharge le fichier du gestionnaire
	void musicDrop();
	//! réalise un saut à la position voulue, exprimée en seconde.
	void musicJump(float secondJump);

private:
	int master_volume;				//!< volume de sortie du gestionnaire de son
	Mix_Music *track = nullptr;		//!< pointeur interne vers le flux audio
	std::string music_name;			//!< nom du fichier du flux audio
	double elapsed_seconds;  		//!< current offset into the track
	bool music_isPlaying = false;		//!< indique qu'une musique est entrain d'être jouée.
	bool music_loaded = false;			//!< donne l'état du flux
	bool isDriverReady;					//!< indique si le soft peut utiliser le driver audio
	A_STATE state = A_STATE::V_NONE;	//!< Etat du fichier musique
	bool loop;							//!< Le flux tourne t'il en boucle ?

};

#endif // _AUDIO_H
