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

// manage an audio track

#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>


// #define MAX_CHANNELS 8


class Audio {

public:
	Audio();
	~Audio();
	Audio(Audio const &) = delete;
	Audio& operator = (Audio const &) = delete;

	void setVolume(int _value);
	void incrementVolume(int value=5);
	void decrementVolume(int value=5);

	//music
	void musicPlay(const std::string& filename, bool loop);
	void musicPause();
	void musicResume();
	void musicRewind();
	//void musicHalt();
	void musicSync();
	void musicDrop();
	void musicJump(float secondJump);

	void update(int delta_time);

	// //sound
	// void chunkLoad(const std::string& filename);
	// void chunkPlay();
	// void chunkPause();
	// void chunkResume();
	// void chunkHalt();
	// void chunkDrop(const std::string& filename);

private:
	int master_volume;
	Mix_Music *track = nullptr;
	// Mix_Chunk *sound[MAX_CHANNELS];
	// std::string sound_name[MAX_CHANNELS];
	std::string music_name;
	double elapsed_seconds;  //! current offset into the track
	bool music_isPlaying;

};

#endif // _AUDIO_H
