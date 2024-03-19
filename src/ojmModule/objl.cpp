#include "tools/call_system.hpp" //direxist
#include "tools/log.hpp"
#include <SDL2/SDL.h>
#include "ojmModule/objl_mgr.hpp"
#include "ojmModule/objl.hpp"
#include "ojmModule/LazyOjmL.hpp"
#include <iostream>
#include <sstream>
#include <cmath>

// Available models
//
// https://space.frieger.com/asteroids/asteroids/


ObjL::ObjL()
{}

void ObjL::draw(VkCommandBuffer cmd, const float screenSize)
{
	if (screenSize < 20) {
		low->draw(cmd);
	} else if (high->priority != LoadPriority::DONE) {
		if (medium->priority != LoadPriority::DONE) {
			if (medium->priority >= LoadPriority::LAZY) {
				medium->priority = LoadPriority::NOW;
				high->priority = LoadPriority::PRELOAD;
			}
			low->draw(cmd);
		} else {
			if (high->priority == LoadPriority::LAZY)
				high->priority = LoadPriority::NOW;
			medium->draw(cmd);
		}
	} else if (screenSize > 180) {
		high->draw(cmd);
	} else {
		medium->draw(cmd);
	}
}

void ObjL::bind(VkCommandBuffer cmd)
{
	LazyOjmL::bind(cmd);
}

void ObjL::bind(Pipeline &pipeline)
{
	LazyOjmL::bind(pipeline);
}

ObjL::~ObjL()
{
	while (high->priority > LoadPriority::COMPLETED) {
		cLog::get()->write("Wait for currently loading OjmL...");
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
}

bool ObjL::init(const std::string &repertory, const std::string &_name)
{
	std::string nameL = repertory+"/"+ _name +"_1L.ojm";
	std::filesystem::path nameM = repertory+"/"+ _name +"_2M.ojm";
	std::filesystem::path nameH = repertory+"/"+ _name +"_3H.ojm";
	if (std::filesystem::exists(nameL + ".bin")) {
		nameL += ".bin";
		nameM.replace_extension(".ojm.bin");
		nameH.replace_extension(".ojm.bin");
		if (!std::filesystem::exists(nameM) || !std::filesystem::exists(nameH)) {
			cLog::get()->write("Could not find medium and/or high resoultion file object " + _name, LOG_TYPE::L_ERROR);
			return false;
		}
	} else if (!((std::filesystem::exists(nameL)) && (std::filesystem::exists(nameM)) && (std::filesystem::exists(nameH)))) {
		cLog::get()->write("Could not find file object " + _name, LOG_TYPE::L_ERROR);
		return false;
	}

	high = std::make_unique<LazyOjmL>(nameH, LoadPriority::LAZY);
	medium = std::make_unique<LazyOjmL>(nameM, LoadPriority::BACKGROUND);
	low = std::make_unique<OjmL>(nameL);

	if (low->getOk())  {
		//~ printf("The 3 ojm  %s are ok\n", _name.c_str());
		cLog::get()->write("Loading object "+ _name);
		// if (medium->getVertexCount() > 20000) {
		// 	cLog::get()->write("Performance Issue : Too many vertices for '" + nameM + "' (Keep Below 20 000)", LOG_TYPE::L_WARNING);
		// }
		if (low->getVertexCount() > 4000) {
			if (low->getVertexCount() > 40000) {
				cLog::get()->write("Major Performance Issue : Up to 10x over limit for '" + nameL + "'(Keep Below 4 000) - Fallback to EquiSphere", LOG_TYPE::L_ERROR);
				if (ObjLMgr::instance)
					low = ObjLMgr::instance->select("EquiSphere")->low->makeLink();
				else
					cLog::get()->write("FALLBACK FAILURE : Can't fallback to EquiSphere without ObjLMgr instance", LOG_TYPE::L_ERROR);
			} else
				cLog::get()->write("Performance Issue : Too many vertices for '" + nameL + "' (Keep Below 4 000)", LOG_TYPE::L_WARNING);
		}
		return true;
	} else {
		//~ printf("Error loading an ojm %s\n", _name.c_str());
		cLog::get()->write("Error loading object "+ _name, LOG_TYPE::L_ERROR);
		return false;
	}
}
