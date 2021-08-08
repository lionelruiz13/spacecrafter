/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2018-2021 Association Sirius
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */


#ifndef SUBTITLE_HPP
#define SUBTITLE_HPP

#include <vector>
#include <iostream>
#include <fstream>
#include <string>

class Subtitle {
public:
	Subtitle();
	~Subtitle();

	/**
	* Ppdate permet, grâce au temps en paramètre, de rechercher directement le prochain message (s'il existe)
	* et le garde en mémoire jusqu'à la demande d'affichage
	*/
	void update(int time);

	//Primitives de gestion du fichier
	/**
	*  Permet de charger un fichier de type sous_Titre.srt
	*/
	void loadFile(const std::string& fileName);

	/**
	* Cette fonction permetteras d'afficher, en fonction des paramètres lu dans le fichier, le sous-titre au dernier temps demandé.
	* Les sous_titre ne s'affichent que selon la demande de l'utilisateur, avec un boolean.
	*/
	void writeToConsole(bool &toDisplay);

private:
	/**
	 * Permet de convertir, à partir du format string, le temps au format 00:00:00,000 en un entier en millisecondes
	 */
	int TimeToMs(std::string& time);

	/**
	*   Fonction qui lit et initialise le vecteur de sous titre
	*/
	void readFile();

	/**
	 * Permet d'ajouter un élément à la fin du vecteur
	 */
	void addSub(int tc1, int tc2, std::string &c, std::string &msg);

	int _deltaTime; // _deltaTime sert à garder en mémoire la dernière position demandé par l'utilisateur
	std::string _FILE; // permet de garder en mémoire le fichier.srt de sous_titre

	struct sub_Struct {
		int Tcode1;
		int Tcode2;
		std::string character;
		std::string msg;
	}; // Structure de traitement des données de sous titre.

	std::vector<sub_Struct> _vSub;
	int _numSub; //garde en mémoire l'emplacement du dernier sous_titre affiché.
};

#endif //SOUS_TITRE_HPP