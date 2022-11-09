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


#include "mediaModule/subtitle.hpp"

Subtitle::Subtitle()
{
	_deltaTime = 0;
	_numSub = 0;
}

Subtitle::~Subtitle()
{}

void Subtitle::update(int time)
{
	int i = _numSub;
	bool find = false;

	if(time > _deltaTime) { // we look if the video goes in the direction of reading, or that the cursor is placed after the last found message
		while(!find && (i < (int)_vSub.size())) { //we look for the new message to display from the last found message.
			if( (_vSub[i].Tcode1 < time) && (time < _vSub[i].Tcode2)) {
				_numSub = i; // save the new position
				find = true; // we say that we have found
			}
			i++;
		}
	}
	else {   // we look if the video goes in the opposite direction of reading, or that the cursor is placed before the last found message.
		while(!find && (i >= 0)) { //look for the new message to be displayed starting from the last message found.
			if( (_vSub[i].Tcode1 < time) && (time < _vSub[i].Tcode2)) {
				_numSub = i; // we save the new position
				find = true; // we say that we have found
			}
			i--;
		}
	}
	_deltaTime = time; // we keep in memory the time that has been requested to reuse it later.
}

int Subtitle::TimeToMs(std::string& time)
{
	int H = std::stoi(time.substr(0,2)) * 1000 * 3600;
	int M = std::stoi(time.substr(3,2)) * 1000 * 60;
	int S = std::stoi(time.substr(6,2)) * 1000;
	int MS = std::stoi(time.substr(9,3));
	int result = H + M + S + MS;
	return result;
}

//File management primitives
void Subtitle::loadFile(const std::string& fileName)
{
	std::ifstream fichier( fileName.c_str() );
	if( !fichier.fail() ) {
		_FILE = fileName.c_str();
		readFile();
		std::cout << "Existing file and load.\n";
	}
	else {
		std::cout << "File does not exist or is not readable.\n";
	}
}

void Subtitle::readFile()
{
	std::ifstream monFlux(_FILE.c_str());

	if(monFlux) { // if the file is open, we start the processing
		std::string ligne;
		int nbLigne = 1;
		std::string str1;
		std::string str2;
		std::string str3;
		std::string str4;

		while(getline(monFlux, ligne)) {
			switch(nbLigne) {
				case 1: //line of the subtitle number, or the character
					str1 = ligne.c_str();
					nbLigne++;
					break;
				case 2: //line of time-codes
					str2 = ligne.substr(0,12);
					str3 = ligne.substr(17,12);
					nbLigne++;
					break;
				case 3: //message line
					str4 = ligne.c_str();
					addSub(TimeToMs(str2), TimeToMs(str3), str1, str4);
					nbLigne++;
					break;
				default: //empty line, added in the vector
					nbLigne = 1;
					break;
			}
		}
	}
	else {
		std::cout << "ERROR: Unable to open the file for reading." << std::endl;
	}
}

void Subtitle::writeToConsole(bool &toDisplay)
{
	if(toDisplay) {
		std::cout << "Msg : " << _deltaTime << " = " << _vSub[_numSub].msg << std::endl;
	}
}

void Subtitle::addSub(int tc1, int tc2, std::string &c, std::string &msg)
{
	_vSub.push_back({tc1, tc2, c, msg});
}