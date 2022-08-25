#include "tools/call_system.hpp" //direxist
#include <SDL2/SDL.h>

#include "ojmModule/objl_mgr.hpp"
#include "ojmModule/objl.hpp"
#include "ojmModule/SphereObjL.hpp"
#include <iostream>
#include <sstream>
#include <cmath>
#include "tools/log.hpp"

// Models ?
//
// https://space.frieger.com/asteroids/asteroids/



ObjLMgr::ObjLMgr()
{
	defaultObject = new SphereObjL();
	objectMap["EquiSphere"] = defaultObject;
}


ObjLMgr::~ObjLMgr()
{
	defaultObject = nullptr;
	std::map<std::string, ObjL *>::iterator it;
	for (it=objectMap.begin(); it!=objectMap.end(); ++it) {
		delete (it->second);
	}
	objectMap.clear();
}


ObjL* ObjLMgr::select(const std::string &name)
{
	if (name =="")
		return defaultObject;

	ObjL* tmp=nullptr;
	tmp = find(name);

	if (tmp == nullptr) {
		//~ printf("Selected object %s non-existent!\n", name.c_str());
		cLog::get()->write("object "+ name + " selected non-existent", LOG_TYPE::L_ERROR);
		return defaultObject;
	}
	return tmp;
}


ObjL* ObjLMgr::find(const std::string &_name)
{
	auto it = objectMap.find(_name);
	if (it != objectMap.end())
		return it->second;
	else
		return nullptr;
}
	//~ std::map<std::string, ObjL* >::iterator it;
	//~ for (it=objectMap.begin(); it!=objectMap.end(); ++it) {
		//~ if (it->first == _name) {
			//~ //printf("Return from %s\n", _name.c_str());
			//~ return it->second;
		//~ }
	//~ }
	//~ //printf("Object %s non-existent!\n", _name.c_str());
	//~ //cLog::get()->write("Object "+ _name + " non-existent", LOG_TYPE::L_ERROR);
	//~ return nullptr;
//~ }


bool ObjLMgr::insert(const std::string &name, bool _defaultObject)
{
	ObjL* tmp=nullptr;
	// check that the object is not already integrated
	if (!_defaultObject) {
		tmp = find(name);
		if (tmp !=nullptr)
			return true; //already loaded
	}

	std::string fullDirectory=defaultDirectory+name;
	if ( CallSystem::dirExist(fullDirectory) ) {
		tmp = new ObjL();
		if (tmp->init(fullDirectory, name))  {
			objectMap.insert(std::pair<std::string,ObjL*>(name, tmp));
			//~ printf("ObjL insert %s\n", name.c_str());
			cLog::get()->write("Succesfull loading model3D "+ name, LOG_TYPE::L_INFO);
			if (_defaultObject)
				defaultObject = tmp;
			return true;
		} else {
			if (_defaultObject)
				cLog::get()->write("Error loading the default model3D "+ name, LOG_TYPE::L_ERROR);
			else
				cLog::get()->write("Error loading model3D "+ name, LOG_TYPE::L_ERROR);
			//~ printf("Error loading model3D %s\n", name.c_str());
			delete tmp;
			return false;
		}
	} else {
		cLog::get()->write("Directory error "+ defaultDirectory, LOG_TYPE::L_ERROR);
		//~ printf("error about the directory %s\n", repertory.c_str());
		return false;
	}
}
