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

#ifndef SAVE_SCREEN_INTERFACE_HPP
#define SAVE_SCREEN_INTERFACE_HPP

#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <memory>
#include "tools/no_copy.hpp"
#include <vulkan/vulkan.h>
#include "EntityCore/Tools/SafeQueue.hpp"

/** @class SaveScreenInterface

 * @section EN BREF
 * Classe qui s'occupe des captures d'écran, dans le cas d'une simple capture d'écran ou
 * pour la réalisation d'une série vidéo.
 *
 * @section DESCRIPTION
 * La fonction phare est void readScreenShot();
 * Elle réalise une capture de l'écran en fonction des paramètes enregistrés dans la classe.
 *
*/

class SaveScreen;

class SaveScreenInterface : public NoCopy {
public:
	SaveScreenInterface(VkRect2D screenRect, VkImageLayout layout);
	~SaveScreenInterface();

    //! lit l'écran et le sauvegarde sur le disque dur
    void readScreenShot(VkCommandBuffer cmd, VkImage image);

    //! démarre la vidéo
    void startVideo();

    //! arrête la vidéo
    void stopVideo();

    void takeVideoShot();

    //! prend une capture d'écran
    //! de part sa nature, cette focntion peut être bloquante
    void takeScreenShot(const std::string& _fileName="");

    //! fixe le répertoire ou l'on doit stocker les fichiers vidéo
    void setVideoBaseName(const std::string& _value) {
        videoBaseName = _value;
    }
    //! fixe le répertoire ou l'on doit stocker les fichiers de capture d'écran
    void setSnapBaseName(const std::string& _value) {
        snapBaseName = _value;
    }

	void update();
private:
    void writeScreenshot(const std::string &filename, int idx);
    std::string getNextScreenshotFilename();
	void mainloop();

	std::unique_ptr<SaveScreen> saveScreen;
    enum class ReadScreen : char {NONE, SNAPSHOT, VIDEO};
    ReadScreen readScreen= ReadScreen::NONE;
    std::string fileNameScreenshot;
    std::string snapBaseName;
    std::string videoBaseName;
    unsigned int fileNumber = 0;
    unsigned int width;
    unsigned int height;
    unsigned int minWH;
	std::string fileNameNextScreenshot;
	bool shouldCapture = false;
	bool asyncEngine = false;
	SubBuffer buffers[3];
	void *pBuffers[3];
	int bufferIdx = 0;
	WorkQueue<std::pair<std::string, int>, 3> pendingIdx; // Pending buffer idx
	std::thread thread;
	VkImageMemoryBarrier preImageBarrier {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, VK_NULL_HANDLE, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
	VkImageMemoryBarrier postImageBarrier {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_READ_BIT, 0, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, VK_NULL_HANDLE, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
	VkBufferMemoryBarrier postBufferBarrier {VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_HOST_READ_BIT, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, VK_NULL_HANDLE, 0, 0};
	VkBufferImageCopy copyInfo {};
};

#endif //SAVE_SCREEN_INTERFACE_HPP
