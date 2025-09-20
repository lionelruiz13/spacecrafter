/*
 * The big star catalogue extension to Stellarium:
 * Author and Copyright: Johannes Gajdosik, 2006, 2007
 *
 * Thanks go to Nigel Kerr for ideas and testing of BE/LE star repacking
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

#ifndef _STAR_HPP_
#define _STAR_HPP_

#include <string>
#include <stdint.h>
#include <SDL2/SDL_endian.h>

#include "starModule/zone_data.hpp"
#include "tools/object_type.hpp"

class ObjectBase;

namespace BigStarCatalog {

template <class Star> struct SpecialZoneArray;
template <class Star> struct SpecialZoneData;

/** structs for storing the stars in binary form. The idea is
* to store much data for bright stars (Star1), but only little or even
* very little data for faints stars (Star3). Using only 6 bytes for Star3
* makes it feasable to store hundreds of millions of them in main memory.
*/


static inline float IndexToBV(unsigned char b_v)
{
	return (float)b_v*(4.f/127.f)-0.5f;
}

struct Star1 { //! 28 bytes
	//~ int hip:17;                  //! 17 bits needed, 17 reserved
	//~ int dimm:7;                  //!  7 bits needed, 7 reserved
	//~ unsigned char component_ids; //!  5 bits needed, 7 reserved
	//~ Int32 x0;                    //! 32 bits needed, 32 reserved
	//~ Int32 x1;                    //! 32 bits needed, 32 reserved
	//~ unsigned char b_v;           //!  7 bits needed, 8 reserved
	//~ unsigned char mag;           //!  8 bits needed, 8 reserved
	//~ Uint16 sp_int;               //! 14 bits needed, 16 reserved
	//~ Int32 dx0,dx1,plx;           //! 96 bits needed, 96 reserved
private:
	union {
		uint8_t  uint8[28];
		uint16_t uint16[14];
		int32_t  uint32[7];
		int32_t  int32[7];
	} d;

public:

	//getter of index's color and fill hip_color
	inline int getBVIndex() const {
		return d.uint8[12];
	}

	//getter of Magnitude and fill hip_mag
	inline int getMag() const {
		return d.uint8[13];
	}

	//! @brief Override mag, for variable star
	inline void overrideMag(int mag) {
		d.uint8[13] = mag;
	}

	inline int getSpInt() const {
		return d.uint16[7];
	}
	inline int getX0() const {
		return SDL_SwapLE32(d.int32[1]);
	}
	inline int getX1() const {
		return SDL_SwapLE32(d.int32[2]);
	}
	inline int getDx0() const {
		return SDL_SwapLE32(d.int32[4]);
	}
	inline int getDx1() const {
		return SDL_SwapLE32(d.int32[5]);
	}
	inline int getPlx() const {
		return SDL_SwapLE32(d.int32[6]);
	}

	inline int getHip() const {
		return d.uint32[0] & 0x01ffff;
	}

	inline void setVariableStarIndex(int dimm) {
		d.uint8[2] = (d.uint8[2] & 1) | (dimm << 1);
	}

	inline int getVariableStarIndex() const {
		return d.uint8[2] >> 1;
	}

	inline int getComponentIds() const {
		return d.uint8[3] & 0x7f;
	}

	inline void setHidden() {
		d.uint8[3] |= 0x80;
	}

	inline void clearHidden() {
		d.uint8[3] &= 0x7f;
	}

	inline bool getHidden() const {
		return d.uint8[3] >> 7;
	}

	static constexpr double max_pos_val=0x7FFFFFFF;

	ObjectBaseP createStelObject(const SpecialZoneArray<Star1> *a, const SpecialZoneData<Star1> *z) const;

	Vec3d getJ2000Pos(const ZoneData *z,double movement_factor) const {
		Vec3d pos = z->center
		            + ((float)(getX0())+movement_factor*getDx0())*z->axis0
		            + ((float)(getX1())+movement_factor*getDx1())*z->axis1;
		pos.normalize();
		return pos;
	}
	float getBV(void) const {
		return IndexToBV(getBVIndex());
	}
	std::string getNameI18n(void) const;
	void print(void);
};


struct Star2 {  //! 10 bytes
	//~ int x0:20;
	//~ int x1:20;
	//~ int dx0:14;
	//~ int dx1:14;
	//~ unsigned int b_v:7;
	//~ unsigned int mag:5;
private :
	uint8_t d[10];

public :
	inline int getX0() const {
		uint32_t v = d[0] | d[1] << 8 | (d[2] & 0xF) << 16;
		return ((int32_t)v) << 12 >> 12;
	}

	inline int getX1() const {
		uint32_t v = d[2] >> 4 | d[3] << 4 | d[4] << 12;
		return ((int32_t)v) << 12 >> 12;
	}

	inline int getDx0() const {
		uint16_t v = d[5] | (d[6] & 0x3F) << 8;
		return ((int16_t)(v << 2)) >> 2;
	}

	inline int getDx1() const {
		uint16_t v = d[6] >> 6 | d[7] << 2 | (d[8] & 0xF) << 10;
		return ((int16_t)(v << 2)) >> 2;
	}

	inline int getBVIndex() const {
		return d[8] >> 4 | (d[9] & 0x7) << 4;
	}

	inline int getMag() const {
		return d[9] >> 3;
	}

	inline int getHip() const {
		return -1;
	}

	static constexpr double max_pos_val=((1<<19)-1);

	ObjectBaseP createStelObject(const SpecialZoneArray<Star2> *a, const SpecialZoneData<Star2> *z) const;

	Vec3d getJ2000Pos(const ZoneData *z,double movement_factor) const {
		Vec3d pos = z->center
		            + ((double)(getX0())+movement_factor*getDx0())*z->axis0
		            + ((double)(getX1())+movement_factor*getDx1())*z->axis1;
		pos.normalize();
		return pos;
	}
	float getBV(void) const {
		return IndexToBV(getBVIndex());
	}
	std::string getNameI18n(void) const {
		return "";
	}
	void print(void);
};



struct Star3 {  //! 6 bytes
	//  int x0:18;
	//  int x1:18;
	//  unsigned int b_v:7;
	//  unsigned int mag:5;
private:
	uint8_t d[6];
public:
	inline int getX0() const {
		uint32_t v = d[0] | d[1] << 8 | (d[2] & 0x3) << 16;
		return ((int32_t)v) << 14 >> 14;
	}

	inline int getX1() const {
		uint32_t v = d[2] >> 2 | d[3] << 6 | (d[4] & 0xF) << 14;
		return ((int32_t)v) << 14 >> 14;
	}

	inline int getBVIndex() const {
		return d[4] >> 4 | (d[5] & 0x7) << 4;
	}

	inline int getMag() const {
		return d[5] >> 3;
	}

	inline int getHip() const {
		return -1;
	}

	static constexpr double max_pos_val=((1<<17)-1)	;

	ObjectBaseP createStelObject(const SpecialZoneArray<Star3> *a, const SpecialZoneData<Star3> *z) const;

	Vec3d getJ2000Pos(const ZoneData *z,double) const {
		Vec3d pos = z->center + (double)(getX0())*z->axis0 + (double)(getX1())*z->axis1;
		pos.normalize();
		return pos;
	}
	float getBV(void) const {
		return IndexToBV(getBVIndex());
	}
	std::string getNameI18n(void) const {
		return "";
	}
	void print(void);
};


} // namespace BigStarCatalog

#endif
