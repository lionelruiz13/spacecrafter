/*
 * SphereObjL
 *
 * Copyright 2020 Association Sirius & Association Andromède
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#ifndef _SPHERE_OBJL_H_
#define _SPHERE_OBJL_H_

#include <vector>
#include <list>
#include <memory>
#include "ojmModule/objl.hpp"
#include "tools/vecmath.hpp"

// Set the subdivisions level for each resolution, a subdivision divide each triangle into 4 triangles
// Before any subdivision, there is 20 triangles
#define SUBDIVISE_HIGH_RES 7
#define SUBDIVISE_MEDIUM_RES 5
#define SUBDIVISE_LOW_RES 3

/**
* \class SphereObjL
*
* \brief Construct an optimal sphere ojm with a specified level of subdivision
*
* @section USAGE
* Use this
*
* \author Calvin Ruiz
*/
class SphereObjL : public ObjL {
public:
	struct OjmPoint {
		Vec3f pos;
		Vec2f tex;
		Vec3f normal;
	};
	struct Triangle {
		unsigned int p1;
		unsigned int p2;
		unsigned int p3;
	};

	SphereObjL();
	virtual ~SphereObjL();
private:

	inline unsigned int getIntersect(unsigned long p1, unsigned long p2) {
		if (p1 < p2) {
			auto &p = lines[p2 | (p1 << 32)];
			if (p)
			return p;
			p = points.size();
		} else {
			auto &p = lines[p1 | (p2 << 32)];
			if (p)
			return p;
			p = points.size();
		}
		auto &op1 = points[p1];
		auto &op2 = points[p2];
		OjmPoint point;
		point.pos = (op1.pos + op2.pos)/2;
		point.tex = (op1.tex + op2.tex)/2;
		if (fabs(op1.pos[2]) > 0.999999)
			point.tex[0] = op2.tex[0];
		if (fabs(op2.pos[2]) > 0.999999)
			point.tex[0] = op1.tex[0];
		point.pos.normalize();
		point.normal = point.pos;
		points.push_back(point);
		return points.size() - 1;
	}
	void construct(int nb_subdivision);
	void subdivise();
	std::vector<OjmPoint> points;
	std::thread asyncConstructor;
	std::map<unsigned long, unsigned int> lines;
	std::vector<Triangle> triangles;
	unsigned char subdiviseLevel = 0;
};

#endif /* _SPHERE_OBJL_H_ */
