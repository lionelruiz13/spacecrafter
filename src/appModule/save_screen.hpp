/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017-2020 of the LSS Team & Association Sirius
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

#ifndef SAVE_SCREEN_HPP
#define SAVE_SCREEN_HPP

#include <iostream>
#include <thread>
#include <vector>
#include "tools/no_copy.hpp"
#include "EntityCore/Tools/SafeQueue.hpp"
#include "EntityCore/SubBuffer.hpp"
/** @class SaveScreen

 * @section IN BRIEF
 * This class allows you to save several buffers simultaneously
 * that correspond to screenshots from app.hpp
 *
 *
 * @section DESCRIPTION
 * At the initialization of this class is built n-1 buffers for the
 * screenshot, where n denotes the number of physical threads available
 * on the machine.
 *
 * Protected by a mutex, bool* tab indicates which buffers are free and
 * available to store the return of glReadPixel in RAM
 *
 * As soon as a thread has finished saving, it informs bool* tab that it is free
 * the use of a buffer
 *
*/

class SaveScreen: public NoCopy  {
public:
	SaveScreen(unsigned int _size);
	~SaveScreen();

	//!function that orders the saving of a buffer knowing its name.
	void saveScreenBuffer(const std::string &fileName, int idx);

	//!reserves a free index of [0..nb_cores-1] loop if necessary
	int getFreeIndex();

	//!returns a pointer to a memory space for saving
	unsigned char* getBuffer(int idx);

	void startStream();
	void stopStream();
private:
	//!transforms a buffer into an image on the disk
	void saveScreenToFile(const std::string &fileName, int idx);

	void threadLoop();

	unsigned int size_screen;	//!< square size of the image to save

	int subIdx = 0;
	int pBuffer[3] {-1, -1, -1};
	int nb_threads;
	std::vector<std::vector<unsigned char>> buffer;
	std::vector<std::thread> threads;
	bool isAvariable = true; //!< indicates if the image backup service is operational
	PushQueue<int, 15> bufferReady;
	DispatchInQueue<std::pair<std::string, int>, 8, 8> requests;
};

#endif //SAVE_SCREEN_HPP
