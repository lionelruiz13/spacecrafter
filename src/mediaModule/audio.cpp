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
 * @class Audio
 * @brief manage audio tracks and sounds with SDL mixer
*/

#include <iostream>
#include <sstream>
#include "mediaModule/audio.hpp"
#include "tools/translator.hpp"
#include "tools/log.hpp"

#define FREQUENCY 	44100
#define CHUNKSIZE	2048
#define CHANNELS	2

Audio::Audio()
{
	if(Mix_OpenAudio(FREQUENCY, MIX_DEFAULT_FORMAT, CHANNELS, CHUNKSIZE ) < 0 ) {
		cLog::get()->write("Error Mix_OpenAudio: "+ std::string(Mix_GetError()), LOG_TYPE::L_ERROR );
		exit(-1);
	} else
		cLog::get()->write("SDL Sound loaded", LOG_TYPE::L_INFO);

	int flags=MIX_INIT_OGG;
	int initted=Mix_Init(MIX_INIT_OGG);
	if((initted&flags) != flags) {
		cLog::get()->write("Audio::audio Mix_Init: "+ std::string(Mix_GetError()), LOG_TYPE::L_ERROR );
	} else
	cLog::get()->write("Audio initialization successful", LOG_TYPE::L_INFO);

	track = nullptr;
	music_loaded = false;
	music_isPlaying = false;
	elapsed_seconds=0.0;
	master_volume=SDL_MIX_MAXVOLUME/3*2;
}

Audio::~Audio()
{
	if (music_loaded)
		musicDrop();

	Mix_CloseAudio();
	cLog::get()->write("Audio support end", LOG_TYPE::L_INFO);
	Mix_Quit();
}


void Audio::musicLoad(const std::string& filename)
{
	if (music_loaded) {
		cLog::get()->write("Another music was already loaded. I will stop it : " +music_name, LOG_TYPE::L_DEBUG);
		this->musicDrop();
	}

	track = Mix_LoadMUS(filename.c_str());
	if (track == nullptr) {
		music_loaded = false;
		cLog::get()->write("Could not load audio file " +filename, LOG_TYPE::L_WARNING);
	} else  {
		cLog::get()->write("Audio::musicLoad load "+ filename, LOG_TYPE::L_DEBUG );
		// test OGG or WAV
		music_loaded = true;
		music_name = filename;
		Mix_HookMusicFinished(this->musicEnd);
	}
}

void Audio::musicEnd()
{
	std::cout << "music get end"<< std::endl;
}

void Audio::musicPlay(bool loop)
{
	if (music_loaded) {
		cLog::get()->write("Audio::musicPlay play "+ music_name, LOG_TYPE::L_DEBUG );
		music_isPlaying = true;
		elapsed_seconds = 0.0;
		if (loop) {
			if (Mix_PlayMusic(track, -1)< 0)
				cLog::get()->write("Error Mix_PlayMusic: "+ std::string(Mix_GetError()), LOG_TYPE::L_ERROR );
		} else {
			if (Mix_PlayMusic(track, 0) < 0)
				cLog::get()->write("Error Mix_PlayMusic: "+ std::string(Mix_GetError()), LOG_TYPE::L_ERROR );
		}
	} else {
		cLog::get()->write("Audio::musicPlay want to play audio but no track enable.", LOG_TYPE::L_DEBUG);
	}
}


// used solely to track elapsed seconds of play
void Audio::update(int delta_time)
{
	if (music_loaded) {
		if (Mix_PlayingMusic()!=1) {
			if (Mix_PausedMusic()!=1) {
				cLog::get()->write("Audio::update seen track ended...", LOG_TYPE::L_DEBUG);
				this->musicDrop();
			}
		}
		elapsed_seconds += delta_time/1000.f;
	}
}

// sychronize with elapsed time no longer starts playback if paused or disabled
void Audio::musicSync()
{
	if (!music_loaded) return;
	cLog::get()->write("Audio::musicSync "+ music_name, LOG_TYPE::L_DEBUG );
	if (music_isPlaying)
		Mix_PauseMusic();

	//special case from mp3 format music
	if (Mix_GetMusicType(nullptr) == MUS_MP3)
		Mix_RewindMusic();

	Mix_SetMusicPosition(elapsed_seconds);
	Mix_ResumeMusic();
}

void Audio::musicJump(float secondJump)
{
	if (!music_loaded) return;
	cLog::get()->write("Audio::musicJump "+ music_name, LOG_TYPE::L_DEBUG );
	if (music_isPlaying)
		Mix_SetMusicPosition(secondJump);
}


void Audio::musicRewind()
{
	Mix_RewindMusic();
	cLog::get()->write("Audio::musicRewind "+ music_name, LOG_TYPE::L_DEBUG );
}

void Audio::musicPause()
{
	if (music_loaded) {
		if (music_isPlaying==true) {
			cLog::get()->write("Audio::musicPause with "+ music_name, LOG_TYPE::L_DEBUG );
			Mix_PauseMusic();
			music_isPlaying = false;
		} else
			musicResume();
	}
}

void Audio::musicResume()
{
	if (music_loaded) {
		cLog::get()->write("Audio::musicResume with "+ music_name, LOG_TYPE::L_DEBUG );
		Mix_ResumeMusic();
		music_isPlaying = true;
	}
}

void Audio::musicHalt()
{
	cLog::get()->write("Audio::musicHalt "+ music_name, LOG_TYPE::L_DEBUG );
	Mix_HaltMusic();
	music_isPlaying=false;
	elapsed_seconds=0.0;
}

void Audio::musicDrop()
{
	if (music_loaded) {
		cLog::get()->write("Audio::musicDrop "+ music_name, LOG_TYPE::L_DEBUG );
		Mix_HaltMusic();
		Mix_FreeMusic(track);
	}
	music_name.clear();
	track = nullptr;
	music_loaded = false;
	music_isPlaying = false;
	elapsed_seconds = 0.0;
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