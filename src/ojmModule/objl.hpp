#ifndef _OBJ3D_HPP_
#define _OBJ3D_HPP_

#include "tools/vecmath.hpp"
#include <vector>
#include <list>
#include <memory>

#include "ojmModule/ojml.hpp"
#include <vulkan/vulkan.h>

class Projector;
class Pipeline;

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
 * Doit doit être utilisé avec un indirectDrawIndexed.
 *
 * low représente l'objet vu de loin
 * medium le représente à une distance intermédiaire
 * high le représente à courte distance
 *
 */

class ObjL {
public:
	ObjL();
	virtual ~ObjL();
	void draw(VkCommandBuffer &cmd, const float screenSize);
	bool init(const std::string &repertory, const std::string &name);
	void bind(VkCommandBuffer &cmd);
	void bind(Pipeline &pipeline);

	bool isOk() {
		return isUsable;
	}

protected:
	bool isUsable = false;

	std::unique_ptr<OjmL> low;
	std::unique_ptr<OjmL> medium;
	std::unique_ptr<OjmL> high;
};


#endif // MODEL_ASTEROID_HPP
