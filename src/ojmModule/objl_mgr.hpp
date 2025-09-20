#ifndef _OBJL3D_MGR_HPP_
#define _OBJL3D_MGR_HPP_

#include "tools/vecmath.hpp"
#include <map>

class ObjL;


/**
 * \class ObjLMgr
 * \brief 3D objects manager in the software
 * \author Olivier NIVOIX
 * \date 21 juin 2018
 *
 * The purpose of this class is to group the management of all 3D objects in Body
 *
 * @section DESCRIPTION
 *
 * Any object displayed by Body is managed in this class.
 *
 * This separation allows to display the same 3D object for different planets.
 *
 * The layout is left to the ObjL files contained in the map.
 *
 * @section OPERATION
 *
 * A map is used as a container for the storage of ObjL.
 *
 * The Body classes come to get the object they are interested in.
 *
 */
class ObjLMgr {
public:
	ObjLMgr();

	//! set the relative path of the3D objects
	void setDirectoryPath(const std::string &directoryName) {
		defaultDirectory = directoryName;
	}

	~ObjLMgr();

	//! add an object3D in the map if it does not already exist
	//! @param name of the new object name
	bool insertObj(const std::string &_name){
		return this->insert(_name, false);
	}

	//! add the default 3D object in the map
	//! @param name of the new object name
	bool insertDefault(const std::string &_name) {
		return this->insert(_name, true);
	}

	//! select a 3D object in the map
	ObjL* select(const std::string &name);

	//! selects the default object
	inline ObjL *selectDefault() {
		return defaultObject;
	}

	bool checkDefaultObject() {
		return (defaultObject != nullptr);
	}

	// Ensure uniqueness and simplify reuse of ObjL elements
	static ObjLMgr *instance;
private:
	//! add a 3D object in the map if it doesn't already exist
	//! @param name of the new object name
	//! @param indicator of the default object
	bool insert(const std::string &name, bool _defaultObject= false);

	//! search in the list for a pointer to a model3D with the name of name
	ObjL* find(const std::string &name);
	//! map of 3D objects, identified by the name
	std::map <const std::string, ObjL*> objectMap;
	//! absolute path of the 3D objects
	std::string defaultDirectory;
	ObjL* defaultObject = nullptr;
};

#endif // SPHEROIDE_MGR_HPP
