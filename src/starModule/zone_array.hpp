/*
 * The big star catalogue extension to Stellarium:
 * Author and Copyright: Johannes Gajdosik, 2006, 2007
 * The implementation of SpecialZoneArray<Star>::draw is based on
 * Stellarium, Copyright (C) 2002 Fabien Chereau,
 * and therefore has shared copyright.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#ifndef _ZONE_ARRAY_HPP_
#define _ZONE_ARRAY_HPP_

#include <SDL2/SDL_endian.h>


//#include "spacecrafter.hpp"
#include "starModule/zone_data.hpp"
#include "navModule/navigator.hpp"
#include "starModule/hip_star.hpp"
#include "starModule/hip_star_mgr.hpp"
//#include "tools/fmath.hpp"

#ifdef __linux__
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#endif /* LINUX */

class Projector;
class s_texture;

#ifdef WIN32
#include <io.h>
#define NOMINMAX
#include <windows.h>
#endif

// Patch by Rainer Canavan for compilation on irix with mipspro compiler part 1
#ifndef MAP_NORESERVE
#  ifdef MAP_AUTORESRV
#    if (defined(__sgi) && defined(_COMPILER_VERSION))
#      define MAP_NORESERVE MAP_AUTORESRV
#    endif
#  else
#    define MAP_NORESERVE 0
#  endif
#endif


namespace BigStarCatalog {

// A ZoneArray manages all ZoneData structures of a given GeodesicGrid level.

class ZoneArray {

public:
	static ZoneArray *create(const HipStarMgr &hip_star_mgr, const std::string &extended_file_name);
	virtual ~ZoneArray(void) {
		nr_of_zones = 0;
	}
	unsigned int getNrOfStars(void) const {
		return nr_of_stars;
	}
	virtual void updateHipIndex(HipIndexStruct hip_index[]) const {};
	virtual void searchAround(int index,const Vec3d &v,double cos_lim_fov, std::vector<ObjectBaseP > &result) = 0;

	virtual void draw(int index,bool is_inside, const float *rcmag_table, Projector *prj, Navigator *nav, int max_mag_star_name, float names_brightness, std::vector<starDBtoDraw> &starNameToDraw, std::map<std::string, bool> &selected_stars, bool atmosphere, bool isolateSelected) const = 0;

	bool isInitialized(void) const {
		return (nr_of_zones>0);
	}
	void initTriangle(int index, const Vec3d &c0, const Vec3d &c1, const Vec3d &c2);
	virtual void scaleAxis(void) = 0;
	const int level;
	const int mag_min;
	const int mag_range;
	const int mag_steps;
	double star_position_scale;
	const HipStarMgr &hip_star_mgr;

protected:
	static bool readStarFile(FILE *f, void *data,size_t size);
	ZoneArray(const HipStarMgr &hip_star_mgr,int level, int mag_min,int mag_range,int mag_steps);
	unsigned int nr_of_zones;
	unsigned int nr_of_stars;
	ZoneData *zones;
};


template<class Star> class SpecialZoneArray : public ZoneArray {
public:
	SpecialZoneArray(FILE *f,bool byte_swap,bool use_mmap, const HipStarMgr &hip_star_mgr,int level, int mag_min,int mag_range,int mag_steps);
	~SpecialZoneArray(void);

protected:
	SpecialZoneData<Star> *getZones(void) const {
		return static_cast<SpecialZoneData<Star>*>(zones);
	}
	Star *stars;

	Star *getStarPtr(int hip) {
		auto *itEnd = stars + nr_of_stars;
		for (auto *it = stars; it < itEnd; ++it) {
			if (it->getHip() == hip)
				return it;
		}
		return nullptr;
	}
private:
	void *mmap_start;
	#ifdef WIN32
	HANDLE mapping_handle;
	#endif
	void scaleAxis(void) override;
	void searchAround(int index,const Vec3d &v,double cos_lim_fov, std::vector<ObjectBaseP > &result) override;
	void draw(int index,bool is_inside, const float *rcmag_table, Projector *prj, Navigator *nav, int max_mag_star_name, float names_brightness, std::vector<starDBtoDraw> &starNameToDraw, std::map<std::string, bool> &selected_stars, bool atmosphere, bool isolateSelected) const override;
};

template<class Star> void SpecialZoneArray<Star>::scaleAxis(void)
{
	star_position_scale /= Star::max_pos_val;
	for (ZoneData *z=zones+(nr_of_zones-1); z>=zones; z--) {
		z->axis0 *= star_position_scale;
		z->axis1 *= star_position_scale;
	}
}

#define NR_OF_HIP 120416

struct HipIndexStruct {
	const SpecialZoneArray<Star1> *a;
	const SpecialZoneData<Star1> *z;
	Star1 *s;
};

class ZoneArray1 : public SpecialZoneArray<Star1> {
public:
	ZoneArray1(FILE *f,bool byte_swap,bool use_mmap, const HipStarMgr &hip_star_mgr, int level,int mag_min,int mag_range,int mag_steps)
		: SpecialZoneArray<Star1>(f,byte_swap,use_mmap,hip_star_mgr,level, mag_min,mag_range,mag_steps)
	{
	}

	/// @return Internal star, for overriding mag
	Star1 *star(int hip) {
		return getStarPtr(hip);
	}
	virtual void draw(int index,bool is_inside, const float *rcmag_table, Projector *prj, Navigator *nav, int max_mag_star_name, float names_brightness, std::vector<starDBtoDraw> &starNameToDraw, std::map<std::string, bool> &selected_stars, bool atmosphere, bool isolateSelected) const override;
private:
	void updateHipIndex(HipIndexStruct hip_index[]) const override;
};

} // namespace BigStarCatalog

#endif
