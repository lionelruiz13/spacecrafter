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


#include <regex>
#include "tools/log.hpp"
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
		cLog::get()->write("Subtitle file "+ fileName + " loaded.", LOG_TYPE::L_INFO);
	}
	else {
		cLog::get()->write("Subtitle file "+ fileName + " does not exist or is not readable.", LOG_TYPE::L_ERROR);
	}
}

bool Subtitle::parseTimeRange(const std::string& line, int& startMs, int& endMs) {
	// Expected format: "00:00:20,000 --> 00:00:24,400" (, or . as millisecond separator) (there may be more spaces around -->)
	static const std::regex timeRangeRegex(R"((\d{2}:[0-5]\d:[0-5]\d[,\.]\d{3})\s*-->\s*(\d{2}:[0-5]\d:[0-5]\d[,\.]\d{3}))");
	std::smatch match;
	if (!std::regex_match(line, match, timeRangeRegex) || match.size() != 3) {
		return false;
	}
	std::string timecodePartStart = match[1]; // 2nd capture group
	std::string timecodePartEnd = match[2]; // 3rd capture group
	startMs = TimeToMs(timecodePartStart);
	endMs = TimeToMs(timecodePartEnd);
	if (endMs < startMs) {
		cLog::get()->write("Subtitle parsing warning: End timecode is before start timecode. Skipping to next subtitle.", LOG_TYPE::L_WARNING);
		return false;
	}
	return true;
}

static inline std::string trim(const std::string& str){
	size_t firstNonSpace = str.find_first_not_of(" \t");
	if (firstNonSpace == std::string::npos) {
		return "";
	}
	size_t lastNonSpace = str.find_last_not_of(" \t");
	return str.substr(firstNonSpace, lastNonSpace - firstNonSpace + 1);
}

void Subtitle::readFile()
{
	std::ifstream myStream(_FILE.c_str());

	if(myStream) { // if the file is open, we start the processing
		std::string line = "";
		std::string trimmedLine = "";
		std::string subtitleNumber = "";
		int timeStart = -1;
		int timeEnd = -1;
		std::string subtitleContent = "";
		int lineNumber = 0;

		enum State {
			EXPECT_INDEX,
			EXPECT_TIMECODE,
			EXPECT_CONTENT
		} state = EXPECT_INDEX;

		auto finalizeSubtitleBlock = [&](){
			if (timeStart != -1 && timeEnd != -1 && !subtitleContent.empty()) {
				// We have finished reading a subtitle block
				addSub(timeStart, timeEnd, subtitleNumber, subtitleContent);
			} else {
				cLog::get()->write("Subtitle: Incomplete subtitle block before line " + std::to_string(lineNumber) + ". Skipping.", LOG_TYPE::L_WARNING);
			}
			subtitleNumber.clear(); subtitleContent.clear();
			timeStart = timeEnd = -1;
			state = EXPECT_INDEX;
		};


		while(getline(myStream, line)) {
			lineNumber++;
			// If we are on the first line, check for BOM (Byte Order Mark) and remove it
			if (lineNumber == 1 && line.size() >= 3 &&
				static_cast<unsigned char>(line[0]) == 0xEF &&
				static_cast<unsigned char>(line[1]) == 0xBB &&
				static_cast<unsigned char>(line[2]) == 0xBF) {
				line = line.substr(3);
				cLog::get()->write("Subtitle: BOM found and removed.", LOG_TYPE::L_INFO);
			}
			// When we get a line remove any trailing \r characters
			while (!line.empty() && line.back() == '\r') {
				line.pop_back();
			}

			// Trim leading and trailing whitespace
			trimmedLine = trim(line);

			if (trimmedLine.empty()) {
				// Between two subtitles, we have an empty line
				if (state == EXPECT_CONTENT) {
					finalizeSubtitleBlock();
				} else {
					cLog::get()->write("Subtitle: Unexpected empty line at line " + std::to_string(lineNumber) + ". Skipping current subtitle block.", LOG_TYPE::L_WARNING);
					// Reset for next subtitle
					subtitleNumber = subtitleContent = "";
					timeStart = timeEnd = -1;
					state = EXPECT_INDEX;
				}
				continue; // skip empty lines
			}

			switch (state) {
				case EXPECT_INDEX:
					// This line contains subtitle number
					if (trimmedLine.find_first_not_of("0123456789") == std::string::npos) {
						subtitleNumber = trimmedLine;
						state = EXPECT_TIMECODE;
					} else if (parseTimeRange(trimmedLine, timeStart, timeEnd)) {
						// some files may omit the index so we try to parse timecode directly
						// Handle case where index is missing and we directly get timecode
						subtitleNumber = "";
						state = EXPECT_CONTENT;
					} else {
						cLog::get()->write("Subtitle: Expected subtitle index/timecode at line " + std::to_string(lineNumber) + ". Skipping line.", LOG_TYPE::L_WARNING);
					}
					break;
				case EXPECT_TIMECODE:
					if (parseTimeRange(trimmedLine, timeStart, timeEnd)) {
						state = EXPECT_CONTENT;
					} else {
						cLog::get()->write("Subtitle: Invalid subtitle timecode at line " + std::to_string(lineNumber) + ". Skipping to next subtitle.", LOG_TYPE::L_WARNING);
						timeStart = timeEnd = -1;
						state = EXPECT_INDEX;
					}
					break;
				case EXPECT_CONTENT:
					// This line contains subtitle content
					if (!subtitleContent.empty()) subtitleContent += "\n";
					subtitleContent += line;
					break;
				default:
					break;
			}
		}
		// Handle last subtitle
		if (state == EXPECT_CONTENT) {
			finalizeSubtitleBlock();
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
		if (vTemp.size() > 0) {
			vTemp.push_back({vTemp.back().Tcode2, INT32_MAX, "", ""});
		}
		_vSub = vTemp;
	}
	else {
		cLog::get()->write("Subtitle: Unable to open the file for reading.", LOG_TYPE::L_ERROR);
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