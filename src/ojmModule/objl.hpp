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
class LazyOjmL;

/**
 * \class ObjL
 * \brief OJML file container
 * \author Olivier NIVOIX
 * \date 21 juin 2018
 *
 * The purpose of this class is to group three OJM objects of the same physical object and to display one of them according to its supposed distance from the screen
 *
 * @section DESCRIPTION
 *
 * This class is only used as a wrapper.
 * Must be used with an indirectDrawIndexed.
 *
 * low represents the object seen from a distance
 * medium represents it at an intermediate distance
 * high represents it at short distance
 *
 */

class ObjL {
public:
	ObjL();
	virtual ~ObjL();
	void draw(VkCommandBuffer cmd, const float screenSize);
	bool init(const std::string &repertory, const std::string &name);
	void bind(VkCommandBuffer cmd);
	void bind(Pipeline &pipeline);
	inline void drawLow(VkCommandBuffer cmd) {
		low->draw(cmd);
	}

protected:
	std::unique_ptr<OjmL> low;
	std::unique_ptr<LazyOjmL> medium;
	std::unique_ptr<LazyOjmL> high;
};


#endif // MODEL_ASTEROID_HPP
