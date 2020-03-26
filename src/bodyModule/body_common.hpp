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
	std::string tex_cloud;
	std::string tex_cloud_normal;
	std::string tex_heightmap;
	std::string tex_skin;
};

struct BodyTesselation {
	Scalable<int> min_tes_level;
	Scalable<int> max_tes_level;
	Scalable<int> planet_altimetry_factor;
	Scalable<int> moon_altimetry_factor;
	Scalable<int> earth_altimetry_factor;
	int min_tes_level_ini;
	int max_tes_level_ini;
	int planet_altimetry_factor_ini;
	int moon_altimetry_factor_ini;
	int earth_altimetry_factor_ini;
};

enum class BODY_FLAG : char {F_NONE, F_TRAIL, F_HINTS, F_AXIS, F_ORBIT, F_HALO, F_CLOUDS};

#endif // _BODY_COMMON_HPP_