#include "tools/call_system.hpp" //direxist
#include "tools/log.hpp"
#include <SDL2/SDL.h>

#include "ojmModule/objl.hpp"
#include <iostream>
#include <sstream>
#include <cmath>

// Modèles disponibles
//
// https://space.frieger.com/asteroids/asteroids/


ObjL::ObjL() 
{}

void ObjL::draw(const float screenSize, VkPrimitiveTopology mode)
{
	if (screenSize < 20) {
		this->low->draw(mode);
	} else if (screenSize >180) {
		this->high->draw(mode);
	} else {
		this->medium->draw(mode);
	}
}

ObjL::~ObjL() {
	if (low) delete low;
	if (medium) delete medium;
	if (high) delete high;
}

bool ObjL::init(const std::string &repertory, const std::string &_name)
{
	std::string nameL = repertory+"/"+ _name +"_1L.ojm";
	std::string nameM = repertory+"/"+ _name +"_2M.ojm";
	std::string nameH = repertory+"/"+ _name +"_3H.ojm";

	//~ cout << nameL << endl << nameM << endl << nameH << endl;

	if ( (CallSystem::fileExist(nameL)) && (CallSystem::fileExist(nameM)) && (CallSystem::fileExist(nameH)) ) {

		this->low = new OjmL(nameL);
		this->medium = new OjmL(nameM);
		this->high = new OjmL(nameH);

		if (this->low->getOk() && this->medium->getOk() && this->high->getOk())  {
			//~ printf("Les 3 ojm  %s sont ok\n", _name.c_str());
			cLog::get()->write("Loading object "+ _name);
			return true;
		} else {
			//~ printf("Erreur de chargement d'un ojm %s\n", _name.c_str());
			cLog::get()->write("Error loading object "+ _name, LOG_TYPE::L_ERROR);
			//on détruit l'objet puisqu'il n'est pas complet
			return false;
		}
	} else {
		cLog::get()->write("Error loading file object " + _name, LOG_TYPE::L_ERROR);
		return false;
	}
}
