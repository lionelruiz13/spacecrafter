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
#include <functional>
#include "mediaModule/audio.hpp"
#include "tools/translator.hpp"
#include "tools/log.hpp"

#define FREQUENCY 	44100
#define CHUNKSIZE	2048
#define CHANNELS	2

AudioMgr::AudioMgr()
{
	if(Mix_OpenAudio(FREQUENCY, MIX_DEFAULT_FORMAT, CHANNELS, CHUNKSIZE ) < 0 ) {
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

AudioMgr::~AudioMgr()
{
	if (!isDriverReady) // nothing to close, nothing to do
		return;
	audioMusic.reset(nullptr);

	Mix_CloseAudio();
	Mix_Quit();
	cLog::get()->write("Audio support end", LOG_TYPE::L_INFO);
}

void AudioMgr::musicLoad(const std::string& filename, bool loop)
{
	if (!isDriverReady)	return;
	audioMusic = std::make_unique<AudioMusic>(filename, loop);
	if (audioMusic->isLoaded()) {
		state = A_STATE::V_STOP;
	} else {
		audioMusic.reset(nullptr);
		state = A_STATE::V_NONE;
	}
}

void AudioMgr::musicPlay()
{
	if (!audioMusic)	return;
	if (state == A_STATE::V_STOP) {
		state = A_STATE::V_PLAY;
		audioMusic->play();
	}
}

// used solely to track elapsed seconds of play
void AudioMgr::update(int delta_time)
{
	if (audioMusic)
		if (!audioMusic->update(delta_time)) {
			state = A_STATE::V_STOP;
			audioMusic->halt();
		}
}

// sychronize with elapsed time no longer starts playback if paused or disabled
void AudioMgr::musicSync()
{
	if (audioMusic)
		audioMusic->sync();
}


void AudioMgr::musicJump(float secondJump)
{
	if (audioMusic)
		audioMusic->jump(secondJump);
}

void AudioMgr::musicRewind()
{
	if (audioMusic)
		audioMusic->rewind();
}

void AudioMgr::musicPause()
{
	if (!audioMusic) return;
	if (state == A_STATE::V_PLAY) {
		audioMusic->pause();
		state = A_STATE::V_PAUSE;
		return;
	}
	if (state == A_STATE::V_PAUSE) {
		audioMusic->resume();
		state = A_STATE::V_PLAY;
	}
}

void AudioMgr::musicResume()
{
	if (audioMusic && (state == A_STATE::V_PAUSE || state == A_STATE::V_STOP)) {
		audioMusic->resume();
		state = A_STATE::V_PLAY;
	}
}

void AudioMgr::musicHalt()
{
	if (audioMusic) {
		state = A_STATE::V_STOP;
		audioMusic->halt();
	}
}

void AudioMgr::musicDrop()
{
	if (audioMusic) {
		audioMusic.reset(nullptr);
		state = A_STATE::V_NONE;
	}
}

void AudioMgr::decrementVolume(int value)
{
	if (master_volume == 0)
		return;
	master_volume -= value;
	if (master_volume < 0) master_volume = 0;
	//~ cout << "audio : value = " << master_volume << endl;
	Mix_VolumeMusic(master_volume);
}

void AudioMgr::incrementVolume(int value)
{
	master_volume += value;
	if (master_volume == SDL_MIX_MAXVOLUME)
		return;
	if (master_volume > SDL_MIX_MAXVOLUME) master_volume = SDL_MIX_MAXVOLUME;
	//~ cout << "audio : value = " << master_volume << endl;
	Mix_VolumeMusic(master_volume);
}

void AudioMgr::setVolume(int _value)
{
	if (master_volume == _value)
		return;
	master_volume=_value;
	if (master_volume > SDL_MIX_MAXVOLUME) master_volume = SDL_MIX_MAXVOLUME;
	if (master_volume < 0) master_volume = 0;
	Mix_VolumeMusic(master_volume);
}


// class AudioMusic


AudioMusic::AudioMusic(const std::string& filename, bool _loop)
{
	std::cout << "creation " << filename << std::endl;
	track = nullptr;
	music_isPlaying = false;
	elapsed_seconds=0.0;

	track = Mix_LoadMUS(filename.c_str());
	if (track == nullptr) {
		music_loaded = false;
		cLog::get()->write("Could not load audio file " +filename, LOG_TYPE::L_WARNING);
	} else  {
		music_loaded = true;
		music_name = filename;
		loop = _loop;
	}
}

AudioMusic::~AudioMusic()
{
	std::cout << "destruction " << music_name << std::endl;
	if (music_loaded) {
		cLog::get()->write("Audio::musicDrop "+ music_name, LOG_TYPE::L_DEBUG );
		Mix_HaltMusic();
		Mix_FreeMusic(track);
	}
}

bool AudioMusic::update(int delta_time)
{
	// test si le fichier est encore en lecture
	if (Mix_PlayingMusic()!=1 && Mix_PausedMusic()!=1) {
		music_isPlaying =false;
		std::cout << "fin fichier " << music_name << std::endl;
		return false;
	}
	if (music_isPlaying) {
		elapsed_seconds += delta_time/1000.f;
	}
	//assume all good ...
	return true;
}

void AudioMusic::play()
{
	cLog::get()->write("Audio::musicPlay play "+ music_name, LOG_TYPE::L_DEBUG );
	music_isPlaying = true;
	std::cout << "play " << music_name << std::endl;
	if (loop) {
		if (Mix_PlayMusic(track, -1)< 0) {
			cLog::get()->write("Error Mix_PlayMusic: "+ std::string(Mix_GetError()), LOG_TYPE::L_ERROR );
			music_isPlaying = false;
		}
	} else {
		if (Mix_PlayMusic(track, 0) < 0) {
			cLog::get()->write("Error Mix_PlayMusic: "+ std::string(Mix_GetError()), LOG_TYPE::L_ERROR );
			music_isPlaying = false;
		}
	}
}

void AudioMusic::sync()
{
	std::cout << "sync " << music_name << std::endl;
	// if (!music_loaded) return;
	cLog::get()->write("Audio::musicSync "+ music_name, LOG_TYPE::L_DEBUG );
	if (music_isPlaying)
		Mix_PauseMusic();

	//special case from mp3 format music
	if (Mix_GetMusicType(nullptr) == MUS_MP3)
		Mix_RewindMusic();

	Mix_SetMusicPosition(elapsed_seconds);
	Mix_ResumeMusic();
}

void AudioMusic::jump(double secondJump)
{
	std::cout << "jump " << music_name << std::endl;
	// if (!music_loaded) return;
	cLog::get()->write("Audio::musicJump "+ music_name, LOG_TYPE::L_DEBUG );
	if (music_isPlaying)
		Mix_SetMusicPosition(secondJump);
}

void AudioMusic::rewind()
{
	std::cout << "rewind " << music_name << std::endl;
	Mix_RewindMusic();
	cLog::get()->write("Audio::musicRewind "+ music_name, LOG_TYPE::L_DEBUG );
}

void AudioMusic::pause()
{
	cLog::get()->write("Audio::musicPause with "+ music_name, LOG_TYPE::L_DEBUG );
	Mix_PauseMusic();
	//music_isPlaying = false;
	std::cout << "pause " << music_name << std::endl;
}

void AudioMusic::resume()
{
	cLog::get()->write("Audio::musicResume with "+ music_name, LOG_TYPE::L_DEBUG );
	Mix_ResumeMusic();
	music_isPlaying = true;
	std::cout << "resume " << music_name << std::endl;
}


void AudioMusic::halt()
{	
	std::cout << "halt " << music_name << std::endl;
	cLog::get()->write("Audio::musicHalt "+ music_name, LOG_TYPE::L_DEBUG );
	Mix_HaltMusic();
	music_isPlaying=false;
	elapsed_seconds=0.0;
}