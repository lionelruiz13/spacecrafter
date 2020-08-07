#ifndef _OBJ3D_HPP_
#define _OBJ3D_HPP_

#include "tools/vecmath.hpp"
#include <vector>
#include <list>
#include "renderGL/stateGL.hpp"
#include "ojmModule/ojml.hpp"

class Projector;

/**
 * \class ObjL
 * \brief Conteneur de fichiers OJML
 * \author Olivier NIVOIX
 * \date 21 juin 2018
 * 
 * Cette classe a pour but de regrouper trois objets OJM d'un même objet physique et d'en afficher un en fonction de sa distance supposée à l'écran
 * 
 * @section DESCRIPTION
 * 
 * Cette classe ne sert que de wrapper.
 * 
 * low représente l'objet vu de loin
 * medium le représente à une distance intermédiaire
 * high le représente à courte distance
 * 
 */

class ObjL {
public:
	ObjL();
	~ObjL();
	void draw(const float screenSize, GLenum mode= GL_TRIANGLES);
	bool init(const std::string &repertory, const std::string &name);

	bool isOk() {
		return isUsable;
	}

private:
	bool isUsable = false;

	OjmL *low = nullptr;
	OjmL *medium = nullptr;
	OjmL *high = nullptr;
};


#endif // MODEL_ASTEROID_HPP
