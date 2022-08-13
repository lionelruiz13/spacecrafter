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


#include <iostream>
#include <sstream>
#include "mediaModule/audio.hpp"
#include "tools/translator.hpp"
#include "tools/log.hpp"


#define FREQUENCY 	44100
#define CHUNKSIZE	2048
#define CHANNELS	2

Audio::Audio() : Audio(FREQUENCY, CHANNELS, CHUNKSIZE)
{}

Audio::Audio(int Frequency, int channel, int chunksize)
{
	if(Mix_OpenAudio(Frequency, MIX_DEFAULT_FORMAT, channel, chunksize ) < 0 ) {
		cLog::get()->write("Error Mix_OpenAudio: "+ std::string(Mix_GetError()), LOG_TYPE::L_DEBUG );
		cLog::get()->write("Error Mix_OpenAudio: Audio error, session with no sound", LOG_TYPE::L_WARNING );
		isDriverReady = false;
		return;
	}

	int flags=MIX_INIT_OGG;
	int initted=Mix_Init(MIX_INIT_OGG);
	if((initted&flags) != flags) {
		cLog::get()->write("Audio::audio Mix_Init: "+ std::string(Mix_GetError()), LOG_TYPE::L_DEBUG );
		cLog::get()->write("Error Mix_Init: OGG module error, session with no sound", LOG_TYPE::L_WARNING );
		isDriverReady = false;
		Mix_CloseAudio();
		return;
	}
	cLog::get()->write("Audio initialization successful", LOG_TYPE::L_INFO);
	isDriverReady = true;
	master_volume=SDL_MIX_MAXVOLUME/3*2;
}


Audio::~Audio()
{
	if (!isDriverReady) // nothing to close, nothing to do
		return;
	if(track!=nullptr) {
		musicDrop();
	}
	Mix_CloseAudio();
	Mix_Quit();
	cLog::get()->write("Audio support end", LOG_TYPE::L_INFO);
}


void Audio::musicLoad(const std::string& filename, bool _loop)
{
	if (!isDriverReady)	return;
	if (music_loaded) {
		cLog::get()->write("Another music was played ...  " +music_name, LOG_TYPE::L_DEBUG);
		this->musicDrop();
		state = A_STATE::V_NONE;
	}
	// réinitilisation de track pour garantir l'état futur de la classe
	track = nullptr;
	track = Mix_LoadMUS(filename.c_str());
	if (track == nullptr) {
		music_loaded = false;
		cLog::get()->write("Could not load audio file " +filename, LOG_TYPE::L_WARNING);
		state = A_STATE::V_NONE;
	}
	else  {
		music_loaded = true;
		music_isPlaying = false;
		music_name = filename;
		elapsed_seconds=0.0;
		loop = _loop;
		state = A_STATE::V_NONE;
	}
}


void Audio::musicPlay()
{
	if (!music_loaded)	return;
	cLog::get()->write("Audio::musicPlay play "+ music_name, LOG_TYPE::L_DEBUG );
	music_isPlaying = true;
	state = A_STATE::V_PLAY;
	if (loop) {
		if (Mix_PlayMusic(track, -1)< 0) {
			cLog::get()->write("Error Mix_PlayMusic: "+ std::string(Mix_GetError()), LOG_TYPE::L_ERROR );
			music_isPlaying = false;
			state = A_STATE::V_NONE;
		}
	}
	else {
		if (Mix_PlayMusic(track, 0) < 0) {
			cLog::get()->write("Error Mix_PlayMusic: "+ std::string(Mix_GetError()), LOG_TYPE::L_ERROR );
			music_isPlaying = false;
			state = A_STATE::V_NONE;
		}
	}
}


// used solely to track elapsed seconds of play
void Audio::update(int delta_time)
{
	if (!music_isPlaying) return;

	// test si le fichier est encore en lecture
	if (Mix_PlayingMusic()!=1 && Mix_PausedMusic()!=1) {
		this->musicHalt();
		return;
	}
	elapsed_seconds += delta_time/1000.f;
}

// sychronize with elapsed time no longer starts playback if paused or disabled
void Audio::musicSync()
{
	if (!music_loaded)	return;
	cLog::get()->write("Audio::musicSync "+ music_name, LOG_TYPE::L_DEBUG );
	if (music_isPlaying) {
		Mix_PauseMusic();
		state = A_STATE::V_PAUSE;
	}

	//special case from mp3 format music
	if (Mix_GetMusicType(nullptr) == MUS_MP3)
		Mix_RewindMusic();

	Mix_SetMusicPosition(elapsed_seconds);
	Mix_ResumeMusic();
	state = A_STATE::V_PLAY;
}

void Audio::musicJump(float secondJump)
{
	if (!music_loaded)	return;
	cLog::get()->write("Audio::musicJump "+ music_name, LOG_TYPE::L_DEBUG );
	Mix_SetMusicPosition(secondJump);
}


void Audio::musicRewind()
{
	if (!music_loaded)	return;
	Mix_RewindMusic();
	cLog::get()->write("Audio::musicRewind "+ music_name, LOG_TYPE::L_DEBUG );
}


void Audio::musicPause()
{
	if (!music_loaded)	return;

	if (music_isPlaying) {
		cLog::get()->write("Audio::musicPause get pause "+ music_name, LOG_TYPE::L_DEBUG );
		Mix_PauseMusic();
		music_isPlaying=false;
		state = A_STATE::V_PAUSE;
	}
	else
		this->musicResume();
}

void Audio::musicMute()
{
	if (!music_loaded)
		return;

	if (music_isPlaying) {
		cLog::get()->write("Audio::musicMusic get mute "+ music_name, LOG_TYPE::L_DEBUG );
		Mix_PauseMusic();
	}
}

void Audio::musicResume()
{
	if (!music_loaded)	return;
	cLog::get()->write("Audio::musicResume "+ music_name, LOG_TYPE::L_DEBUG );
	Mix_ResumeMusic();
	music_isPlaying=true;
	state = A_STATE::V_PLAY;
}

void Audio::musicHalt()
{
	if (!music_loaded)	return;
	cLog::get()->write("Audio::musicHalt "+ music_name, LOG_TYPE::L_DEBUG );
	Mix_HaltMusic();
	music_isPlaying=false;
	elapsed_seconds=0.0;
	state = A_STATE::V_STOP;
}

void Audio::musicDrop()
{
	if (!music_loaded)	return;
	cLog::get()->write("Audio::musicDrop "+ music_name, LOG_TYPE::L_DEBUG );
	Mix_HaltMusic();
	Mix_FreeMusic(track);

	track=nullptr;
	music_name.clear();
	music_isPlaying=false;
	music_loaded = false;
	elapsed_seconds=0.0;
	state = A_STATE::V_NONE;
}

void Audio::decrementVolume(int value)
{
	if (master_volume == 0)
		return;
	master_volume -= value;
	if (master_volume < 0) master_volume = 0;
	//~ cout << "audio : value = " << master_volume << endl;
	Mix_VolumeMusic(master_volume);
}

void Audio::incrementVolume(int value)
{
	master_volume += value;
	if (master_volume == SDL_MIX_MAXVOLUME)
		return;
	if (master_volume > SDL_MIX_MAXVOLUME) master_volume = SDL_MIX_MAXVOLUME;
	//~ cout << "audio : value = " << master_volume << endl;
	Mix_VolumeMusic(master_volume);
}

void Audio::setVolume(int _value)
{
	if (master_volume == _value)
		return;
	master_volume=_value;
	if (master_volume > SDL_MIX_MAXVOLUME) master_volume = SDL_MIX_MAXVOLUME;
	if (master_volume < 0) master_volume = 0;
	Mix_VolumeMusic(master_volume);
}
