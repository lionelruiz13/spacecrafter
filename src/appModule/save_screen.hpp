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

 * @section EN BREF
 * Cette classe permet de sauvegarder simultanément plusieurs tampons
 * qui correspondent à des captures d'écran issus de app.hpp
 *
 *
 * @section DESCRIPTION
 * A l'initialisation de cette classe est construit n-1 buffers pour la
 * capture d'écran, ou n désigne le nombre de threads physiques disponibles
 * sur la machine.
 *
 * Protégé par un mutex, bool* tab indique quels buffers sont libres et
 * disponibles pour stocker le retour de glReadPixel en mémoire RAM
 *
 * Dès qu'un thread à terminé sa sauvegarde, il renseigne bool* tab qu'il libère
 * l'utilisation d'un buffer
 *
*/

class SaveScreen: public NoCopy  {
public:
	SaveScreen(unsigned int _size);
	~SaveScreen();

	//!fonction qui ordonne la sauvegarde d'un buffer connaissant son nom.
	void saveScreenBuffer(const std::string &fileName, int idx);

	//!réserve un indice libre de [0..nb_cores-1] boucle s'il le faut
	int getFreeIndex();

	//!renvoie un pointeur sur un espace mémoire pour la sauvegarde
	unsigned char* getBuffer(int idx);

	void startStream();
	void stopStream();
private:
	//!transforme un buffer en une image sur le disque
	void saveScreenToFile(const std::string &fileName, int idx);

	void threadLoop();

	unsigned int size_screen;	//!< taille carré de l'image à sauvegarder

	int subIdx = 0;
	int pBuffer[3] {-1, -1, -1};
	int nb_threads;
	std::vector<std::vector<unsigned char>> buffer;
	std::vector<std::thread> threads;
	bool isAvariable = true; //!< indique si le service de sauvegarde des images est opértationnel
	PushQueue<int, 15> bufferReady;
	DispatchInQueue<std::pair<std::string, int>, 7, 7> requests;
};

#endif //SAVE_SCREEN_HPP
