/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2018-2021 Association Sirius
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


#ifndef SUBTITLE_HPP
#define SUBTITLE_HPP

#include <vector>
#include <iostream>
#include <fstream>
#include <string>

class Subtitle {
public:
	Subtitle();
	~Subtitle();

	/**
	* Ppdate allows, thanks to the time in parameter, to search directly the next message (if it exists)
	* and keeps it in memory until the display request
	*/
	void update(int time);

	//Primitives of file management
	/**
	*  Allows to load a file of type sub_title.srt
	*/
	void loadFile(const std::string& fileName);

	/**
	* This function will display, according to the parameters read in the file, the subtitle at the last requested time.
	* The subtitles are only displayed according to the user's request, with a boolean.
	*/
	void writeToConsole(bool &toDisplay);

private:
	/**
	 * Allows to convert, from the string format, the time in 00:00:00,000 format into an integer in milliseconds
	 */
	int TimeToMs(std::string& time);

	/**
	*   Function that reads and initializes the subtitle vector
	*/
	void readFile();

	/**
	 * Allows to add an element at the end of the vector
	 */
	void addSub(int tc1, int tc2, std::string &c, std::string &msg);

	int _deltaTime; // _deltaTime is used to keep in memory the last position requested by the user
	std::string _FILE; // allows to keep in memory the subtitle .srt file

	struct sub_Struct {
		int Tcode1;
		int Tcode2;
		std::string character;
		std::string msg;
	}; // Structure for processing subtitle data.

	std::vector<sub_Struct> _vSub;
	int _numSub; //keeps track of the location of the last displayed subtitle.
};

#endif //SOUS_TITRE_HPP