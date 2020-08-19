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
	std::string name;
	bool isRead = false;
	ObjToOjm converter;

	// Traitement des données
	int c;
	while ((c = getopt (argc, argv, "hcn")) != -1) {
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



	Obj3D obj3D(name +".obj");
	isRead = obj3D.init();

	if (isRead==true)
		std::cout << argv[0] << " : File " << name << " read without errors" << std::endl;
	else {
		std::cout << argv[0] << " : Errors detected while reading file "<< name << "  Aborting..." << std::endl;
		return -2;
	}

	// if (toVerbose)
	// 	obj3D.print();

	converter.importOBJ(&obj3D);
	std::cout << argv[0] << " :  import converter without errors" << std::endl;

	if (toOptimize)
		converter.fusionMaterials();

	converter.transform();

	converter.exportOJM(name + ".ojm");
	converter.exportV3D(name + ".v3d");
	std::cout << argv[0] << ":  export converter without errors" << std::endl;
	return 0;
}

