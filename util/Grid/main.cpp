/*
 * Grid
 * 
 * Copyright 2020 AssociationSirius
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

#include "../../src/tools/vecmath.hpp"
#include <vector>
#include <list>


#define DEBUG 1


struct TopTriangle {
	int corners[3];   // index der Ecken
};


const float icosahedron_G = 0.5*(1.0+sqrt(5.0));
const float icosahedron_b = 1.0/sqrt(1.0+icosahedron_G*icosahedron_G);
const float icosahedron_a = icosahedron_b*icosahedron_G;

const Vec3f icosahedron_corners[12] = {
	Vec3f( icosahedron_a, -icosahedron_b,            0.0),
	Vec3f( icosahedron_a,  icosahedron_b,            0.0),
	Vec3f(-icosahedron_a,  icosahedron_b,            0.0),
	Vec3f(-icosahedron_a, -icosahedron_b,            0.0),
	Vec3f(           0.0,  icosahedron_a, -icosahedron_b),
	Vec3f(           0.0,  icosahedron_a,  icosahedron_b),
	Vec3f(           0.0, -icosahedron_a,  icosahedron_b),
	Vec3f(           0.0, -icosahedron_a, -icosahedron_b),
	Vec3f(-icosahedron_b,            0.0,  icosahedron_a),
	Vec3f( icosahedron_b,            0.0,  icosahedron_a),
	Vec3f( icosahedron_b,            0.0, -icosahedron_a),
	Vec3f(-icosahedron_b,            0.0, -icosahedron_a)
};

const TopTriangle icosahedron_triangles[20]= {
    {{ 1, 0,10}}, //  1
    {{ 0, 1, 9}}, //  0
    {{ 0, 9, 6}}, // 12
    {{ 9, 8, 6}}, //  9
    {{ 0, 7,10}}, // 16
    {{ 6, 7, 0}}, //  6
    {{ 7, 6, 3}}, //  7
    {{ 6, 8, 3}}, // 14
    {{11,10, 7}}, // 11
    {{ 7, 3,11}}, // 18
    {{ 3, 2,11}}, //  3
    {{ 2, 3, 8}}, //  2
    {{10,11, 4}}, // 10
    {{ 2, 4,11}}, // 19
    {{ 5, 4, 2}}, //  5
    {{ 2, 8, 5}}, // 15
    {{ 4, 1,10}}, // 17
    {{ 4, 5, 1}}, //  4
    {{ 5, 9, 1}}, // 13
    {{ 8, 9, 5}}  //  8
};



class eTest {
public:
    eTest(Vec3f _pos) {
        pos = _pos;
    }
    ~eTest(){};
    Vec3f getPos() {
        return pos;
    }
private:
    Vec3f pos;
};



class Grid {
public:
    Grid();
    ~Grid();
	//! insert un élément dans la grille
    // void insert(eTest* _test, Vec3f pos);
	//! supprime un élément de la grille
    // void remove(eTest* _test);
    // eTest* begin() const;
    // eTest* end() const;
    // eTest* next() const;
    // void setFov(Vec3f pos, float fov);
private:
	//! Return an array with the number of the zones in the field of view
	void intersect(const Vec3f& _pos, float fieldAngle);
	//! Renvoie quel centre est le plus proche de -v
	int getNearest(const Vec3f& _v);
	//! les centres de la grille
	std::vector<Vec3f> centers;
	//! en cache les centres à afficher de la grille
	std::vector<int> centerToDisplay;
	//! nombre total de centres 
	unsigned int nbCenters;
	//! angle que font 2 centres adjacents entre eux
	float angle;
	std::vector<std::list<eTest*>> dataCenter;
};

Grid::Grid()
{
	// just take the icosahedron_corners
	for(int i=0; i<12; i++) {
		centers.push_back(icosahedron_corners[i]);
		dataCenter.push_back(std::list<eTest*>{});
	}

};

int Grid::getNearest(const Vec3f& _v)
{
	Vec3f v=_v;
	int bestI = -1;
	float bestDot = -2.f;

	v.normalize();
	for (unsigned int i=0; i<nbCenters; ++i) {
		if (v.dot(centers[i])>bestDot) {
			bestI = i;
			bestDot = v.dot(centers[i]);
		}
	}
	#ifdef DEBUG
		std::cout << "Grid::getNearest return " << bestI << std::endl;
	#endif
	return bestI;
}


//! Return an array with the number of the zones in the field of view
void Grid::intersect(const Vec3f& _pos, float fieldAngle)
{
	Vec3f pos = _pos;
	pos.normalize();
	float max = cosf(fieldAngle/2.f + angle);

	centerToDisplay.clear();
	for (unsigned int i=0; i<nbCenters; i++) {
		if (pos.dot(centers[i]) > max) {
			centerToDisplay.push_back(i);
		}
	}
	#ifdef DEBUG
		std::cout << "Grid::intersect display "<< std::endl;
		for(auto i : centerToDisplay)
			std::cout << i << " ";
		std::cout << std::endl;
	#endif
}


#include <iostream>

int main(int argc, char **argv)
{
	
	return 0;
}
