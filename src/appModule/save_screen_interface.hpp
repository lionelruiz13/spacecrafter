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

class Vulkan;

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
	SaveScreenInterface(unsigned int _width, unsigned int _height, Vulkan *_master);
	~SaveScreenInterface();

    //! lit l'écran et le sauvegarde sur le disque dur
    void readScreenShot();

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

private:
    static void writeScreenshot(void *pSelf, void *pData, uint32_t width, uint32_t height);
    std::string getNextScreenshotFilename();

	Vulkan *master;
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
};

#endif //SAVE_SCREEN_INTERFACE_HPP
