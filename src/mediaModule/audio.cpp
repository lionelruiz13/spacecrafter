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
 * there is two functions type
 * \li working with sounds
 * \li working with tracks
 * We can only play 1 track but multiple sounds
*/

#include <iostream>
#include <sstream>
#include "mediaModule/audio.hpp"
#include "tools/translator.hpp"
#include "tools/log.hpp"


Audio::Audio()
{
	track = nullptr;
	music_isPlaying = false;
	master_volume=SDL_MIX_MAXVOLUME/3*2;
	elapsed_seconds=0.0;
	cLog::get()->write("Audio initialization successful", LOG_TYPE::L_INFO);
	for (int i=0; i<MAX_CHANNELS; i++)
		sound[i]=nullptr;
}

Audio::~Audio()
{
	if(track) {
		music_isPlaying=false;
		Mix_HaltMusic(); // stop playing
		Mix_FreeMusic(track);  // free memory
	}
	for (int i=0; i<MAX_CHANNELS; i++) {
		if(sound[i] !=nullptr)
			Mix_FreeChunk(sound[i]);
	}
	Mix_CloseAudio();
	cLog::get()->write("Audio support end", LOG_TYPE::L_INFO);
}

//~ float Audio::music_get_length(std::string filename)
//~ {
//~ FILE *plop;
//~ string test="avconv -i "+ filename + " 2>&1 | grep 'Duration' | cut -d ' ' -f 4";
//~ plop=popen (test.c_str(), "r");
//~ float h, m, s;
//~ if ( fscanf(plop, "%f:%f:%f", &h,&m,&s) == 0) { // an
//~ pclose(plop);
//~ return 0.0;
//~ }
//~ pclose (plop);
//~ //printf("taille :%f h %f min %f s\n durée %f\n",h,m,s,(h*60+m)*60+s);
//~ return ((h*60+m)*60+s);
//~ }

void Audio::musicLoad(const std::string& filename)
{
	track = Mix_LoadMUS(filename.c_str());
	if (track == nullptr) {
		music_isPlaying = false;
		cLog::get()->write("Could not load audio file " +filename, LOG_TYPE::L_WARNING);
	} else music_isPlaying = 1;
	music_name = filename;
	elapsed_seconds=0.0;
}


void Audio::musicPlay(bool loop)
{
	if (track != nullptr) {
		music_isPlaying = true;
		elapsed_seconds = 0.0;
		if (loop) {
			if (Mix_PlayMusic(track, -1)< 0)
				cLog::get()->write("Error Mix_PlayMusic: "+ std::string(Mix_GetError()), LOG_TYPE::L_ERROR );
		} else {
			if (Mix_PlayMusic(track, 0) < 0)
				cLog::get()->write("Error Mix_PlayMusic: "+ std::string(Mix_GetError()), LOG_TYPE::L_ERROR );
		}
	}
}


// used solely to track elapsed seconds of play
void Audio::update(int delta_time)
{
	if (track ) elapsed_seconds += delta_time/1000.f;
}

// sychronize with elapsed time no longer starts playback if paused or disabled
void Audio::musicSync()
{
	if (track==nullptr) return;

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
	if (track==nullptr) return;

	if (music_isPlaying)
		Mix_SetMusicPosition(secondJump);
}


void Audio::musicRewind()
{
	Mix_RewindMusic();
}

void Audio::musicPause()
{
	if (track !=nullptr) {
		if (music_isPlaying==true) {
			Mix_PauseMusic();
			music_isPlaying=0;
		} else {
			Mix_ResumeMusic();
			music_isPlaying=1;
		}
	}
}

void Audio::musicResume()
{
	if (track !=nullptr) {
		Mix_ResumeMusic();
		music_isPlaying=1;
	}
}

void Audio::musicHalt()
{
	Mix_HaltMusic();
	music_isPlaying=0;
	elapsed_seconds=0.0;
}

void Audio::musicDrop()
{
	if (track !=nullptr) {
		Mix_HaltMusic();
		Mix_FreeMusic(track);
	}
	track=nullptr;
	music_isPlaying=0;
	elapsed_seconds=0.0;
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

void Audio::chunkLoad(const std::string& filename)
{
	for (int i=0; i<MAX_CHANNELS; i++) {
		if(sound[i] ==nullptr) {
			sound[i]=Mix_LoadWAV(filename.c_str());
			if (sound[i] == nullptr) {
				cLog::get()->write("Nothing to do because no existing sound with this name", LOG_TYPE::L_ERROR);
				return;
			}
			sound_name[i]=filename.c_str();
			return;
		}
	}
	cLog::get()->write("Nothing to do because no channel free", LOG_TYPE::L_WARNING);
}

void Audio::chunkPlay()
{
	for (int i=0; i<MAX_CHANNELS; i++) {
		if(sound[i] !=nullptr)
			Mix_PlayChannel(-1,sound[i],0);
	}
}

void Audio::chunkPause()
{
	Mix_Pause(-1);
}

void Audio::chunkDrop(const std::string& filename)
{
	for (int i=0; i<MAX_CHANNELS; i++) {
		if(sound_name[i] == filename.c_str()) {
			Mix_FreeChunk(sound[i]);
			sound[i]=nullptr;
			sound_name[i] =="";
			return;
		}
	}
	cLog::get()->write("Nothing to do because no existing sound with this name", LOG_TYPE::L_WARNING);
}

void Audio::chunkResume()
{
	Mix_Resume(-1);
}

void Audio::chunkHalt()
{
	Mix_HaltChannel(-1);
}
