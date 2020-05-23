/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017  Association Sirius
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

#ifndef CLOCK_HPP
#define CLOCK_HPP

#include <cstdint>
#include <iostream>
#include <SDL2/SDL.h>

#include "tools/no_copy.hpp"

/*! @class Fps
* @brief classe s'occupant du framerate et des FPS
*
* @description
* La classe Fps gère le framerate du logiciel. Elle utilise pour cela deux conditions
* - la fonction wait :  elle s'occupe de la durée d'une frame par rapport à une autre. (vue locale, soumise à l'imperfection des arrondis d'entiers de ms)
* - la fonction afterOneSecond : elle s'occupe de de la durée des frames sur une période d'une seconde afin de déterminer le FPS
* - afterOneSecond est lancée via un trigger SDL dans App.hpp toutes les 1000 ms.
*/
class Fps  : public NoCopy {
public:
	Fps(){};
	~Fps(){};

	//! Initialise les paramètres de l'horloge
	void init() {
		initCount= SDL_GetTicks();
		lastCount= SDL_GetTicks();
	};

	//! renvoie le nombre de frames affichées depuis le lancement du logiciel
	unsigned long int getElapsedFrame() const {
		return numberFrames;
	}

	//! ajoute la durée théorique d'une frame écoulée
	void addCalculatedTime(int delta_time) {
		calculatedTime += delta_time;
	}

	//! ajoute une frame
	void addFrame();

	//! renvoie la durée d'un tour de boucle 
	unsigned int getDeltaTime() const {
		if (recVideoMode)
			return frameVideoDuration;
		else
			return tickCount - lastCount;
	}

	//! renvoie la durée théorique d'un tour de boucle lors d'une vidéo
	unsigned int getVideoDeltaTime() {
		return frameVideoDuration;
	}

	//! indique à quel FPS le logiciel doit tourner en mode capture vidéo
	void setVideoFps(float fps) {
		videoFPS = fps;
	}

	//! indique à quel FPS le logiciel doit tourner en mode normal
	void setMaxFps(float fps) {
		maxFPS = fps;
	}

	//! bascule en mode enregistrement de vidéo
	void selectVideoFps() {
		recVideoMode = true;
		frameVideoDuration= (unsigned int) (SECONDEDURATION/videoFPS);
	}

	//! bascule en mode normal 
	void selectMaxFps() {
		recVideoMode = false;
		frameDuration= (unsigned int) (SECONDEDURATION/maxFPS);
	}

	//! Prend une mesure de temps
	void setTickCount() {
		tickCount = SDL_GetTicks();
	}

	//! Modifie le temps de référence de l'horloge
	void setLastCount() {
		lastCount = tickCount;
	}

	//! Indique la durée d'une frame en int
	// unsigned int getFrameDuration() const {
	// 	return frameDuration;
	// }

	//! indique le FPS actuel
	int getFps() const {
		return fps;
	}

	// Détermine la durée d'attente entre deux frames pour obtenir le FPS théorique
	void wait();

	//! Calcule le FPS par seconde et corrige les différences
	void afterOneSecond();

	//! fonction callback lancée par SDL2
	static Uint32 callbackfunc(Uint32 interval, void *param);
private:
	uint64_t numberFrames=0;
	int frame = 0;
	int fps = 0;
	uint64_t calculatedTime = 0;
	float videoFPS=0.f;
	float maxFPS=0.f;
	uint64_t lastCount = 0;
	uint64_t initCount = 0;
	uint64_t tickCount = 0;
	uint16_t frameDuration=0;
	uint16_t frameVideoDuration=0;
	bool recVideoMode = false;

	const float SECONDEDURATION=1000.0;
};

#endif
