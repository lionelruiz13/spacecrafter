/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2018 of Association Sirius
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
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#ifndef _CPUINFO_H_
#define _CPUINFO_H_

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <iostream>

const int NUM_CPU_STATES = 10;
const int TAMPON_SIZE = 10;

/**
 * \class CPUInfo
 * \brief Création des Journaux d'utilisation des cores du CPU
 * \author Olivier NIVOIX
 * \date 16 juin 2018
 * 
 * Cette classe a pour but d'enregister régulièrement les activités des cores du processeur dans un fichier
 * 
 * @section DESCRIPTION
 * 
 * La classe utilise la lecture du fichier systeme /proc/stat qu'elle parse 
 * 
 * start() crée un thread qui enregistre périodiquement les informations de /proc/stat
 * 
 * stop() arrête le thread et ferme les enregistrements.
 * 
 * @section partie thread
 * 
 * Au lancement de start(), la fonction crée un thread qui boucle de cette façon:
 * 
 * std::this_thread::sleep_for(std::chrono::seconds(1));
 * this -> getCPUstate();
 * this -> archivingData();
 * 
 * archivingData() vide le tableau des informations grâce à la variable nbDiff qui varie de 0 à TAMPON_SIZE
 * 
 * @section FONCTIONNEMENT
 * 
 * La classe possède peu de méthodes.
 * 
 * CPUInfo cpuInfo;
 * cpuInfo.init("fichier_destination");
 * cpuInfo.start();
 * 
 * ... instructions diverses ...
 * 
 * cpuInfo.stop();
 * 
 */
 
class CPUInfo
{
public:
	CPUInfo();
	~CPUInfo();

	//! débute l'analyse des journaux du CPU
	void start();

	//! termine l'analyse des journaux du CPU
	void stop();

	//! récupère les paramètres necéssaires à la classe
	//! \param CPUlogFile : nom du fichier conservant les données du CPU
	//! \param GPUlogFile : nom du fichier conservant les données du GPU
	void init(const std::string &logCPU_file, const std::string &logGPU_file );
	//~ void display(std::vector<CoreData> entrie);

private:
	// description des entrées de /proc/stat
	enum CPUStates {
		S_USER = 0,
		S_NICE,
		S_SYSTEM,
		S_IDLE,
		S_IOWAIT,
		S_IRQ,
		S_SOFTIRQ,
		S_STEAL,
		S_GUEST,
		S_GUEST_NICE
	};

	//contient les info d'un coeur
	typedef struct CoreData {
		size_t times[NUM_CPU_STATES];
	} CoreData;

	// fonction principale pour le thread
	void mainFunc();
	// permet de réaliser une capture des données dans entrieA ou entrieB
	void getCPUstate();
	//fonction permetttant de lire une valeur du GPU
	void getGPUstate();
	// fonction de d'archivage des données dans un tableau
	void archivingData();
	// permet de réaliser une capture des données dans une entrie spécifiée
	void getCPUstate(std::vector<CoreData> &entrie);
	// réalise la soustraction entre deux entries
	void diffEntrie(const std::vector<CoreData> entrieA, const std::vector<CoreData> entrieB);
	// fonction qui permet de sauvegarder physiquement les infos sur le DD
	void saveToFile();
	// variables servant aux captures des données
	std::vector<CoreData> entrieA, entrieB, result;
	std::stringstream oss[TAMPON_SIZE];	// tampon des sorties pour CPUfileLog
	std::stringstream gpuOss;	 		// chaine de caractere des sorties du GPU
	unsigned int nbThread=0;	// Variable contenant le nombre de core disponibles sur la machine
	unsigned long int frame=0;	// Numéro de la frame analysée
	unsigned char diff = 0;		// compteur différences avant sauvegarde dans CPUfileLog
	bool isActived = true;		// Indicateur pour clore le thread
	std::thread t;				// thread
	std::ofstream CPUfileLog;	// fichier à destination des informations CPU
	std::ofstream GPUfileLog;	// fichier à destination des informations GPU
};


#endif // _CPUINFO_H_
