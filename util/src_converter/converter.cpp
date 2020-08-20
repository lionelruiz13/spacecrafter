/*
 * converter.cpp
 *
 * Copyright 2017 Immersive Adventure
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
 *
 */


#include <iostream>
#include <string>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "obj3D.hpp"
#include "obj_to_ojm.hpp"

static void display_copyright()
{	
	std::cout << std::endl;
	std::cout << "Converter Wavefront.obj to Spacecrafter.ojm" << std::endl;
	
	std::cout << "Property of Immersive Adventure"<< std::endl << "2018 - All rights reserved" << std::endl << std::endl;
	std::cout << "This program is distributed in the hope that it will be useful," << std::endl;
	std::cout << "but WITHOUT ANY WARRANTY; without even the implied warranty of" << std::endl;
	std::cout << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE" <<std::endl << std::endl;
}

static void display_help(char *argv)
{	
	std::cout << std::endl;
	std::cout << "Usage" << std::endl << std::endl;
	
	std::cout << argv << " [option] <file.obj>" << std::endl << std::endl;

	std::cout << "Convert obj file to ojm file" << std::endl << std::endl;

	std::cout << "Options:" << std::endl;
	std::cout << "   -c   display copyright" << std::endl;
	std::cout << "   -h   display this help" << std::endl;
	std::cout << "   -n   don't fusion the obj materials" << std::endl;
	std::cout << std::endl;
}

static std::string removeExtension(const std::string& filename)
{
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos)
		return filename;
    return filename.substr(0, lastdot); 
}


int main(int argc, char **argv)
{
	// variables globales
	bool toOptimize = true;
	bool displayCopyright = false;
	bool displayHelp = false;
	bool displayPrint = false;
	std::string name;

	// Traitement des données
	int c;
	while ((c = getopt (argc, argv, "hcnp")) != -1) {
		switch (c) {

			case 'c':
				displayCopyright = true;
				break;

			case 'h':
				displayHelp = true;
				break;

			case 'n':
				toOptimize = false;
				break;

			case 'p':
				displayPrint = true;
				break;

			default:
				break;
		}
	}
	//fin du traitement des données

	if (displayCopyright)
		display_copyright();

	if (displayHelp)
		display_help(argv[0]);

	if (name.empty() && optind < argc)
		name = removeExtension(argv[optind]);
	else {
		if (name.empty()) {
			std::cout << "Missing parameter! " << argv[0] << " <fileName.obj> or -h for help" << std::endl;
			return -1;
		}
	}

	//charge le fichier OBJ
	Obj3D obj3D(name +".obj");

	// affiche informations sur les datas du fichier OBJ
	if (displayPrint)
		obj3D.print();

	// création du convertisseur
	ObjToOjm converter;
	// import du fichier dans le convertisseur
	converter.importOBJ(&obj3D);
	if (displayPrint)	
		std::cout << argv[0] << ":  import converter without errors" << std::endl;

	//fusionne les matériaux identiques
	if (toOptimize)
		converter.fusionMaterials();

	//indexe des sommets afin de réduire le nombre de points à traiter
	converter.createUniqueIndexFromVertices();

	//exporte le résultat dans les différents formats
	converter.exportOJM(name + ".ojm");
	converter.exportV3D(name + ".v3d");

	if (displayPrint)
		std::cout << argv[0] << ":  export converter without errors" << std::endl;
	return 0;
}

