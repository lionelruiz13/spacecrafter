#include "tools/call_system.hpp" //direxist
#include "tools/log.hpp"
#include <SDL2/SDL.h>

#include "ojmModule/objl.hpp"
#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/VertexArray.hpp"
#include <iostream>
#include <sstream>
#include <cmath>

// Modèles disponibles
//
// https://space.frieger.com/asteroids/asteroids/


ObjL::ObjL()
{}

void ObjL::draw(const float screenSize, void *pDrawData)
{
	if (screenSize < 20) {
		this->low->draw(pDrawData);
	} else if (screenSize >180) {
		this->high->draw(pDrawData);
	} else {
		this->medium->draw(pDrawData);
	}
}

void ObjL::bind(CommandMgr *cmdMgr)
{
	this->high->bind(cmdMgr);
}

void ObjL::bind(Pipeline *pipeline)
{
	this->high->bind(pipeline);
}

ObjL::~ObjL() {
	if (low) delete low;
	if (medium) delete medium;
	if (high) delete high;
}

bool ObjL::init(const std::string &repertory, const std::string &_name, ThreadContext *context)
{
	std::string nameL = repertory+"/"+ _name +"_1L.ojm";
	std::string nameM = repertory+"/"+ _name +"_2M.ojm";
	std::string nameH = repertory+"/"+ _name +"_3H.ojm";

	//~ cout << nameL << endl << nameM << endl << nameH << endl;

	if ( (CallSystem::fileExist(nameL)) && (CallSystem::fileExist(nameM)) && (CallSystem::fileExist(nameH)) ) {
		int vertexSize = 0;
		int indexSize = 0;
		this->high = new OjmL(nameH, context, true, &vertexSize, &indexSize);
		this->medium = new OjmL(nameM, context, true, &vertexSize, &indexSize);
		this->low = new OjmL(nameL, context, true, &vertexSize, &indexSize);

		if (this->low->getOk() && this->medium->getOk() && this->high->getOk())  {
			//~ printf("Les 3 ojm  %s sont ok\n", _name.c_str());
			cLog::get()->write("Loading object "+ _name);
			// Merge obj vertexBuffers
			VertexArray vertex(*this->high->getVertexArray());
			vertex.registerIndexBuffer(BufferAccess::STATIC, indexSize);
			vertex.build(vertexSize);
			this->high->initFrom(&vertex);
			this->medium->initFrom(&vertex);
			this->low->initFrom(&vertex);
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
