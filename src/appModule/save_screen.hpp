/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017 of the LSS Team & Association Sirius
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
#include <mutex>

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

class SaveScreen {
public:
	SaveScreen(unsigned int _size);
	~SaveScreen();
	SaveScreen(SaveScreen const &) = delete;
	SaveScreen& operator = (SaveScreen const &) = delete;

	//!fonction qui ordonne la sauvegarde d'un buffer connaissant son nom.
	void saveScreenBuffer(const std::string &fileName);

	//!réserve un indice libre de [0..nb_cores-1] boucle s'il le faut
	void getFreeIndex();

	//!renvoie un pointeur sur un espace mémoire pour la sauvegarde
	unsigned char* getFreeBuffer() {
		return buffer[freeSlot];
	}

private:
	//!thread servant à lancer la fonction de sauvegarde
	std::thread taskThread(const std::string &fileName, int bufferIndice) {
		return std::thread([=] { saveScreenToFile( fileName, bufferIndice); });
	}

	//!transforme un buffer en une image sur le disque
	void saveScreenToFile(const std::string &fileName, int bufferIndice);

	//! vérifie que tous les buffers ont été traités
	bool isAllFree();

	int freeSlot; //!< indique un indice libre de buffer

	std::thread * threadpool;	//!< contenant pour créer des threads à la volée.
	unsigned int size_screen;	//!< taille carré de l'image à sauvegarder
	unsigned int nb_cores;		//!< nombre de threads maxi

	std::mutex mtx;  //!< mutex sur tab
	unsigned char** buffer; //!< tableau de tableau de captures d'écran
	bool* tab; //!< tableau servant à protéger les buffers des IO threads/app
	bool isAvariable; //!< indique si le service de sauvegarde des images est opértationnel
};

#endif //SAVE_SCREEN_HPP
