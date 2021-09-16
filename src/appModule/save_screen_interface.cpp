/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2018-2020 of the LSS Team & Association Sirius
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include <SDL2/SDL.h>

#include <iomanip>
#include <fstream>
#include <iostream>

#include "appModule/save_screen.hpp"
#include "tools/log.hpp"

#include "appModule/save_screen_interface.hpp"
#include "eventModule/EventFps.hpp"
#include "eventModule/event_recorder.hpp"

#include "vulkanModule/Vulkan.hpp"

SaveScreenInterface::SaveScreenInterface(unsigned int _width, unsigned int _height, Vulkan *_master)
{
	width = _width;
	height = _height;
	master = _master;
	minWH = std::min(width, height);
	saveScreen = std::make_unique<SaveScreen>(minWH);
	master->setupInterceptor(this, writeScreenshot);
}


SaveScreenInterface::~SaveScreenInterface()
{
}

void SaveScreenInterface::startVideo()
{
	readScreen=ReadScreen::VIDEO;
	Event* event = new FpsEvent(FPS_ORDER::LOW_FPS);
	EventRecorder::getInstance()->queue(event);
}

void SaveScreenInterface::stopVideo()
{
	readScreen=ReadScreen::NONE;
	Event* event = new FpsEvent(FPS_ORDER::HIGH_FPS);
	EventRecorder::getInstance()->queue(event);
}

void SaveScreenInterface::takeVideoShot()
{
	if (readScreen==ReadScreen::VIDEO) {
		this->stopVideo();
	} else
		this->startVideo();
}

void SaveScreenInterface::readScreenShot()
{
	switch (readScreen) {
		case ReadScreen::NONE : break;

		case ReadScreen::SNAPSHOT :
			if (fileNameScreenshot.empty())
				fileNameScreenshot = getNextScreenshotFilename();
			fileNameNextScreenshot.swap(fileNameScreenshot);
			master->intercept();
			fileNameScreenshot.clear();
			readScreen=ReadScreen::NONE;
			break;

		case ReadScreen::VIDEO : {
			std::ostringstream ss;
			ss << std::setw(6) << std::setfill('0') << fileNumber;
			fileNameNextScreenshot = videoBaseName + "-" + ss.str() + ".jpg";
			master->intercept();
			fileNumber++;
			}
			break;

		default: break;
	}
}


void SaveScreenInterface::takeScreenShot(const std::string& _fileName)
{
	if (readScreen==ReadScreen::VIDEO)	//si déjà en capture vidéo, on ne fait pas un snap
		return;
	else
		readScreen= ReadScreen::SNAPSHOT;
	if (!_fileName.empty())
		fileNameScreenshot = _fileName;
}

void SaveScreenInterface::writeScreenshot(void *pSelf, void *pData, uint32_t width, uint32_t height)
{
	SaveScreenInterface *self = static_cast<SaveScreenInterface *>(pSelf);
	//cette fonction peut être bloquante
	self->saveScreen->getFreeIndex();
	unsigned char *data = static_cast<unsigned char *>(pData) + 4 * width * height; // Move at the end of the image
	unsigned char *buff = self->saveScreen->getFreeBuffer();
	for (uint32_t i = 0; i < height; ++i) { // Format is VK_FORMAT_B8G8R8A8_UNORM
		data -= 2 * 4 * width; // Flip Y axis
		for (uint32_t j = 0; j < width; ++j) {
			*(buff++) = data[2]; // R component
			*(buff++) = data[1]; // G component
			*(buff++) = data[0]; // B component
			data += 4;
		}
	}
	self->saveScreen->saveScreenBuffer(self->fileNameNextScreenshot);
}

//! Return the next sequential screenshot filename to use
std::string SaveScreenInterface::getNextScreenshotFilename()
{
	std::string tempName;

	time_t tTime = time ( NULL );
	tm * tmTime = localtime ( &tTime );
	char timestr[28];
	strftime( timestr, 24, "-%y.%m.%d-%H.%M.%S.jpg", tmTime );

	tempName = snapBaseName +timestr;
	return tempName;
}
