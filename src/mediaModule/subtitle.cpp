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
	if (_vSub.empty()) {
		return;
	}

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
	// Clear previous data
	unloadFile();

	std::ifstream file( fileName.c_str() );
	if( !file.fail() ) {
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
	std::ifstream myStream(_FILE.c_str());

	if(myStream) { // if the file is open, we start the processing
		std::string line;
		int nbLine = 1;
		std::string str1;
		std::string str2;
		std::string str3;
		std::string str4;

		while(getline(myStream, line)) {
			switch(nbLine) {
				case 1: //line of the subtitle number, or the character
					str1 = line.c_str();
					nbLine++;
					break;
				case 2: //line of time-codes
					str2 = line.substr(0,12);
					str3 = line.substr(17,12);
					nbLine++;
					break;
				case 3: //message line
					str4 = line.c_str();
					addSub(TimeToMs(str2), TimeToMs(str3), str1, str4);
					nbLine++;
					break;
				default: //empty line, added in the vector
					nbLine = 1;
					break;
			}
		}

		// add empty subtitle between every subtitle to avoid issues during display
		std::vector<sub_Struct> vTemp;
		for(size_t i = 0; i < _vSub.size(); i++) {
			vTemp.push_back(_vSub[i]);
			if (i < _vSub.size() - 1 && _vSub[i].Tcode2 < _vSub[i+1].Tcode1) {
				vTemp.push_back({_vSub[i].Tcode2, _vSub[i+1].Tcode1, "", ""});
			}
		}
		if (vTemp.size() > 0 && vTemp[0].Tcode1 > 0) {
			vTemp.insert(vTemp.begin(), {0, vTemp[0].Tcode1, "", ""});
		}
		_vSub = vTemp;
	}
	else {
		std::cout << "ERROR: Unable to open the file for reading." << std::endl;
	}
}

void Subtitle::unloadFile()
{
	_FILE = "";
	_vSub.clear();
	_deltaTime = 0;
	_numSub = 0;
}

void Subtitle::writeToConsole(bool &toDisplay)
{
	if(toDisplay) {
		std::cout << "Msg : " << _deltaTime << " = " << _vSub[_numSub].msg << std::endl;
	}
}

std::string Subtitle::getSubtitleAt(int time)
{
	if (_vSub.empty()) {
		return "";
	}

	update(time);
	if((_numSub < (int)_vSub.size()) && (_numSub >= 0)) { // check bounds
		return _vSub[_numSub].msg;
	} else {
		return "";
	}
}

void Subtitle::addSub(int tc1, int tc2, std::string &c, std::string &msg)
{
	_vSub.push_back({tc1, tc2, c, msg});
}