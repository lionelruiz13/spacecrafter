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
#include "appModule/save_screen.hpp"
#define TJE_IMPLEMENTATION
#include "tiny_jpeg.h"
#include "tools/log.hpp"


SaveScreen::SaveScreen(unsigned int _size)
{
	nb_threads = std::min(std::max(2, SDL_GetCPUCount()-4), 8);
	size_screen = _size;

	try {
		buffer.resize(nb_threads + 2);
		for (uint8_t i = 0; i < nb_threads + 2; ++i) {
			buffer[i].resize(3 * size_screen * size_screen);
			bufferReady.emplace(i);
		}
	} catch (...) {
		isAvariable = false;
		cLog::get()->write("SaveScreen : erreur création buffer individuel", LOG_TYPE::L_ERROR);
	}
}


SaveScreen::~SaveScreen()
{
	if (!isAvariable)
		return;

	stopStream();
}

void SaveScreen::saveScreenBuffer(const std::string &filename, int idx)
{
	if (!isAvariable)
		return;

	int bufferIdx = pBuffer[idx];
	pBuffer[idx] = -1;
	if (threads.empty()) {
		std::thread(&SaveScreen::saveScreenToFile, this, filename, bufferIdx).detach();
	} else {
		while (!requests.emplace({filename, bufferIdx})) {
			SDL_Delay(10);
		}
	}
}

int SaveScreen::getFreeIndex()
{
	subIdx %= 3;
	while (pBuffer[subIdx] >= 0) {
		SDL_Delay(10);
	}
	return subIdx++;
}

unsigned char *SaveScreen::getBuffer(int idx)
{
	int bufferIdx;
	while (!bufferReady.pop(bufferIdx)) {
		SDL_Delay(10);
	}
	pBuffer[idx] = bufferIdx;
	return buffer[bufferIdx].data();
}

void SaveScreen::saveScreenToFile(const std::string &fileName, int idx)
{
	//~ printf("dans SaveScreen::saveScreenToFile\n");
	tje_encode_to_file_at_quality(fileName.c_str(), 3,size_screen, size_screen, 3, buffer[idx].data());
	bufferReady.emplace(idx);
}

void SaveScreen::startStream()
{
	cLog::get()->write("Starting frame encoding stream", LOG_TYPE::L_INFO);
	requests.reopen();
	for (uint16_t i = 0; i < nb_threads; ++i) {
		threads.push_back(std::thread(&SaveScreen::threadLoop, this));
	}
}

void SaveScreen::stopStream()
{
	cLog::get()->write("Stopping frame encoding stream", LOG_TYPE::L_INFO);
	requests.close();
	for (auto &t : threads)
		t.join();
	threads.clear();
}

void SaveScreen::threadLoop()
{
	DispatchOutQueue<std::pair<std::string, int>, 8, 8> requestQuerry(requests);
	std::pair<std::string, int> req;
	requestQuerry.acquire();
	while (requestQuerry.pop(req)) {
		saveScreenToFile(req.first, req.second);
	}
	requestQuerry.release();
}
