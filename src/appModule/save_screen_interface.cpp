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
#include "tools/context.hpp"
#include "appModule/save_screen_interface.hpp"
#include "eventModule/EventFps.hpp"
#include "eventModule/event_recorder.hpp"
#include "EntityCore/EntityCore.hpp"

SaveScreenInterface::SaveScreenInterface(VkRect2D screenRect, VkImageLayout layout)
{
	width = screenRect.extent.width;
	height = screenRect.extent.height;
	minWH = std::min(width, height);
	saveScreen = std::make_unique<SaveScreen>(minWH);
	auto &context = *Context::instance;
	for (int i = 0; i < 3; ++i) {
		buffers[i] = context.readbackMgr->acquireBuffer(minWH * minWH * 4);
		pBuffers[i] = context.readbackMgr->getPtr(buffers[i]);
	}
	postBufferBarrier.buffer = buffers[0].buffer;
	postBufferBarrier.size = buffers[0].size;
	preImageBarrier.oldLayout = postImageBarrier.newLayout = layout;
	copyInfo.imageSubresource = VkImageSubresourceLayers{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
	copyInfo.imageOffset = VkOffset3D{screenRect.offset.x, screenRect.offset.y, 0};
	copyInfo.imageExtent = VkExtent3D{width, height, 1};
}


SaveScreenInterface::~SaveScreenInterface()
{
	if (asyncEngine) {
		pendingIdx.close();
		saveScreen->stopStream();
	}
	if (thread.joinable())
		thread.join();
}

void SaveScreenInterface::startVideo()
{
	if (asyncEngine)
		return;
	asyncEngine = true;
	saveScreen->startStream();
	if (thread.joinable())
		thread.join();
	thread = std::thread(&SaveScreenInterface::mainloop, this);
	readScreen=ReadScreen::VIDEO;
	Event* event = new FpsEvent(FPS_ORDER::LOW_FPS);
	EventRecorder::getInstance()->queue(event);
}

void SaveScreenInterface::stopVideo()
{
	if (!asyncEngine)
		return;
	asyncEngine = false;
	pendingIdx.close();
	saveScreen->stopStream();
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

void SaveScreenInterface::readScreenShot(VkCommandBuffer cmd, VkImage image)
{
	switch (readScreen) {
		case ReadScreen::NONE : break;

		case ReadScreen::SNAPSHOT :
			if (fileNameScreenshot.empty())
				fileNameScreenshot = getNextScreenshotFilename();
			fileNameNextScreenshot.swap(fileNameScreenshot);
			fileNameScreenshot.clear();
			readScreen=ReadScreen::NONE;
			shouldCapture = true;
			break;

		case ReadScreen::VIDEO : {
			std::ostringstream ss;
			ss << std::setw(6) << std::setfill('0') << fileNumber;
			fileNameNextScreenshot = videoBaseName + "-" + ss.str() + ".jpg";
			fileNumber++;
			shouldCapture = true;
			}
			break;

		default: break;
	}
	if (shouldCapture) {
		bufferIdx = saveScreen->getFreeIndex();
		preImageBarrier.image = image;
		postImageBarrier.image = image;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &preImageBarrier);
		copyInfo.bufferOffset = buffers[bufferIdx].offset;
		vkCmdCopyImageToBuffer(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, postBufferBarrier.buffer, 1, &copyInfo);
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &postImageBarrier);
		postBufferBarrier.offset = copyInfo.bufferOffset;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 1, &postBufferBarrier, 0, nullptr);
	}
}


void SaveScreenInterface::takeScreenShot(const std::string& _fileName)
{
	if (readScreen==ReadScreen::VIDEO)	//if already in video capture, we don't make a snap
		return;
	else
		readScreen= ReadScreen::SNAPSHOT;
	if (!_fileName.empty())
		fileNameScreenshot = _fileName;
}

void SaveScreenInterface::writeScreenshot(const std::string &fileName, int idx)
{
	Context::instance->readbackMgr->invalidate(buffers[idx]);
	unsigned char *src = static_cast<unsigned char *>(pBuffers[idx]) + 4 * minWH * (minWH - 1); // Move at the last line of the image
	unsigned char *dst = saveScreen->getBuffer(idx);
	// Flip Y axis
	for (uint32_t i = 0; i < minWH; ++i) { // Format is VK_FORMAT_B8G8R8A8_UNORM
		for (uint32_t j = 0; j < minWH; ++j) {
			*(dst++) = src[2]; // R component
			*(dst++) = src[1]; // G component
			*(dst++) = src[0]; // B component
			src += 4;
		}
		src -= 2 * 4 * minWH;
	}
	saveScreen->saveScreenBuffer(fileName, idx);
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

void SaveScreenInterface::update()
{
	if (!shouldCapture)
		return;
	if (asyncEngine) {
		while (!pendingIdx.emplace({fileNameNextScreenshot, bufferIdx}))
			SDL_Delay(10);
	} else
		writeScreenshot(fileNameNextScreenshot, bufferIdx);
	shouldCapture = false;
}

void SaveScreenInterface::mainloop()
{
	std::pair<std::string, int> args;
	pendingIdx.acquire();
	while (pendingIdx.pop(args)) {
		writeScreenshot(args.first, args.second);
	}
	pendingIdx.release();
}
