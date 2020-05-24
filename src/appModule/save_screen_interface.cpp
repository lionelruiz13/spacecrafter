#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <iomanip>
#include <fstream>
#include <iostream>

#include "appModule/save_screen.hpp"
// #define TJE_IMPLEMENTATION
// #include "tiny_jpeg.h"
#include "tools/log.hpp"

#include "appModule/save_screen_interface.hpp"
#include "eventModule/EventFps.hpp"
#include "eventModule/event_manager.hpp"

SaveScreenInterface::SaveScreenInterface(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
	minWH = std::min(width, height);
	saveScreen = new SaveScreen(minWH);
}


SaveScreenInterface::~SaveScreenInterface()
{
	delete saveScreen;
}

void SaveScreenInterface::startVideo()
{
	readScreen=ReadScreen::VIDEO;
	Event* event = new FpsEvent(FPS_ORDER::LOW_FPS);
	EventManager::getInstance()->queue(event);
}

void SaveScreenInterface::stopVideo()
{
	readScreen=ReadScreen::NONE;
	Event* event = new FpsEvent(FPS_ORDER::HIGH_FPS);
	EventManager::getInstance()->queue(event);
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
			writeScreenshot(fileNameScreenshot);
			fileNameScreenshot.clear();
			readScreen=ReadScreen::NONE;
			break;

		case ReadScreen::VIDEO : {
			std::ostringstream ss;
			ss << std::setw(6) << std::setfill('0') << fileNumber;
			writeScreenshot(videoBaseName + "-" + ss.str() + ".jpg");
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

void SaveScreenInterface::writeScreenshot(const std::string& filename)
{
	//cette fonction peut être bloquante
	saveScreen->getFreeIndex();
	glReadPixels( (width-minWH)/2, (height-minWH)/2, minWH, minWH, GL_RGB, GL_UNSIGNED_BYTE, saveScreen->getFreeBuffer());
	saveScreen->saveScreenBuffer(filename);
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