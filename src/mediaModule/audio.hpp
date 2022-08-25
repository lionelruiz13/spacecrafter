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


//! enumeration of playback states of an audio file
enum class A_STATE: char { V_NONE, // file not loaded
                           V_PAUSE,// file is paused
                           V_PLAY, // file is playing
                           V_STOP  // file playback stopped
                         };



/**
 * @class Audio
 * @brief Initialize the lib SDL2_mixer and perform operations on a music file AudioMusic
 *
 * The class is used as an intermediary between Media and Audio.
 * It performs all the controls to play a music file.
 *
 * By default, it initializes the audio driver to read music in 44100 Hz, stereo, in unsigned 16 bits
 *
 */


class Audio: public NoCopy {

public:
	Audio();
	Audio(int Frequency, int channel, int chunksize);
	~Audio();
	//! indicates how much time has elapsed since the last loop
	void update(int delta_time);
	//! fixes the audio volume
	void setVolume(int _value);
	//! decreases the audio volume
	void incrementVolume(int value=5);
	//! increases the audio volume
	void decrementVolume(int value=5);

	/* possible commands for playing an audio file*/
	//! loads a file into the manager
	void musicLoad(const std::string& filename, bool _loop);
	//! starts playing a previously loaded audio file
	void musicPlay();
	//! pauses the playback of the file. If reapplied, starts playing the file again
	void musicPause();
    //! Pause the audio playing without pausing the read position. Require sync on resume.
	void musicMute();
	//! resumes the playback of the paused file.
	void musicResume();
	//! restarts the playback of the file from the beginning
	void musicRewind();
	//! stops the playback of the file
	void musicHalt();
	//! synchronizes the sound file with the time elapsed in the theory
	void musicSync();
	//! unloads the file from the manager
	void musicDrop();
	//! makes a jump to the desired position, expressed in seconds.
	void musicJump(float secondJump);

private:
	int master_volume;				//!< sound manager output volume
	Mix_Music *track = nullptr;		//!< internal pointer to the audio stream
	std::string music_name;			//!< file name of the audio stream
	double elapsed_seconds;  		//!< current offset into the track
	bool music_isPlaying = false;		//!< indicates that music is being played.
	bool music_loaded = false;			//!< gives the state of the stream
	bool isDriverReady;					//!< indicates if the software can use the audio driver
	A_STATE state = A_STATE::V_NONE;	//!< Status of the music file
	bool loop;							//!< Is the stream running in a loop ?

};

#endif // _AUDIO_H
