#ifndef _BODY_COMMON_HPP_
#define _BODY_COMMON_HPP_

#include <string>
#include "tools/scalable.hpp"

struct BodyTexture {
	std::string tex_map;
	std::string tex_map_alternative;
	std::string tex_norm;
  	std::string tex_night;
	std::string tex_specular;
	// std::string tex_cloud;
	// std::string tex_cloud_normal;
	std::string tex_heightmap;
	std::string tex_skin;
};



enum class BODY_FLAG : char {F_NONE, F_TRAIL, F_HINTS, F_AXIS, F_ORBIT, F_HALO, F_CLOUDS};
enum BODY_TYPE {SUN = 0, PLANET = 1, MOON = 2, DWARF = 3, ASTEROID = 4, KBO = 5,  COMET = 6, ARTIFICIAL = 7, OBSERVER = 8, CENTER = 9, UNKNOWN = 10, STAR = 11};

#endif // _BODY_COMMON_HPP_
