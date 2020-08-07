#ifndef _OBJL3D_MGR_HPP_
#define _OBJL3D_MGR_HPP_

#include "tools/vecmath.hpp"
#include <map>
#include "renderGL/shader.hpp"


class ObjL;


/**
 * \class ObjLMgr
 * \brief Gestionnaire d'objets 3D dans le logiciel 
 * \author Olivier NIVOIX
 * \date 21 juin 2018
 * 
 * Cette classe a pour but de regrouper la gestion de tous les objets 3D de Body
 * 
 * @section DESCRIPTION
 *
 * Tout objet affiché par Body est géré dans cette classe.
 *
 * Cette séparation permet de pouvoir afficher un même objet 3D pour différentes planètes.
 *
 * Le tracé est laissé à la charge des fichiers ObjL contenus dans la map.
 * 
 * @section FONCTIONNEMENT
 * 
 * Une map sert de conteneur pour le stockage des ObjL.
 * 
 * Les classes de Body viennent chercher l'objet qui les intéresse.
 * 
 */
class ObjLMgr {
public:
	ObjLMgr();

	//! fixer le chemin relatif des objets3D
	void setDirectoryPath(const std::string &directoryName) {
		defaultDirectory = directoryName;
	}

	~ObjLMgr();

	//! ajout d'un objet3D dans la map s'il n'existe pas déjà
	//! @param nom du nouvel objet name
	bool insertObj(const std::string &_name){
		return this->insert(_name, false);
	}

	//! ajout de l'objet3D par défaut dans la map
	//! @param nom du nouvel objet name
	bool insertDefault(const std::string &_name) {
		return this->insert(_name, true);
	}

	//! sélectionne un objet3D dans la map
	ObjL* select(const std::string &name);

	//! sélectionne l'objet par défaut
	ObjL* selectDefault() {
		return defaultObject;
	}

	bool checkDefaultObject() {
		return (defaultObject != nullptr);
	}

private:
	//! ajout d'un objet3D dans la map s'il n'existe pas déjà
	//! @param nom du nouvel objet name
	//! @param indicateur de l'objet par defaut
	bool insert(const std::string &name, bool _defaultObject= false);

	//! recherche dans la liste un pointeur sur un model3D au nom de name
	ObjL* find(const std::string &name);
	//! map des objet3D, identifié par le nom
	std::map <const std::string, ObjL*> objectMap;
	//! chemin absolu des objet3D
	std::string defaultDirectory;
	ObjL* defaultObject = nullptr;
};

#endif // SPHEROIDE_MGR_HPP
