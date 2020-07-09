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
#include <memory>


#define DEBUG 1


struct TopTriangle {
	const u_char corners[3];   // index der Ecken
};


constexpr float icosahedron_G = 0.5*(1.0+sqrt(5.0));
constexpr float icosahedron_b = 1.0/sqrt(1.0+icosahedron_G*icosahedron_G);
constexpr float icosahedron_a = icosahedron_b*icosahedron_G;

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

constexpr TopTriangle icosahedron_triangles[20]= {
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

template <typename T>
class Grid {
public:
    Grid();
    ~Grid() {};
	//! insert un élément dans la grille
    void insert(std::shared_ptr<T> _element, Vec3f pos);
	//! supprime un élément de la grille
    // void remove(eTest* _test);
    auto begin() {
		return Grid::iterator(allDataCenter.begin(), allDataCenter.end(), allDataCenter.begin()->first);
	};
    auto end() {
		return Grid::iterator(allDataCenter.end());
	};
    // eTest* next() const;
    // void setFov(Vec3f pos, float fov);
	//! Renvoie quel centre est le plus proche de -v
	int getNearest(const Vec3f& _v);
	//! Return an array with the number of the zones in the field of view
	void intersect(const Vec3f& _pos, float fieldAngle);
private:
	typedef std::list<std::shared_ptr<T>> dataType_t;
	typedef std::vector<std::pair<dataType_t, bool>> dataCenterType_t;

	class iterator;
	//! les centres de la grille
	std::vector<Vec3f> centers;
	//! en cache les centres à afficher de la grille
	//std::vector<int> centerToDisplay;
	//! nombre total de centres
	unsigned int nbCenters=0;
	//! angle que font 2 centres adjacents entre eux
	float angle;
	//! Optimized container for access
	dataCenterType_t allDataCenter;
	//! Optimized container for search
	std::vector<std::pair<dataType_t, bool>*> dataCenter;
};

template<typename T>
class Grid<T>::iterator {
public:
   iterator(auto _zoneBegin, auto _zoneEnd, auto &_element) :
	   iterZone(_zoneBegin), iterLastZone(_zoneEnd), iterElement(_element.begin()), iterLastElement(_element.end()) {}
   iterator(const auto &_iterZone) : iterZone(_iterZone) {}
   bool operator!=(iterator compare) const {
	   return iterZone != compare.iterZone;
   }
   void operator++() {
	   if (++iterElement == iterLastElement) {
		   while ((!iterZone->second || iterElement == iterLastElement) && ++iterZone != iterLastZone) {
			   iterElement = iterZone->first.begin();
			   iterLastElement = iterZone->first.end();
		   }
	   }
   }
   std::shared_ptr<T> &operator*() const {
	   return *iterElement;
   }
private:
   typename dataCenterType_t::iterator iterZone;
   typename dataCenterType_t::const_iterator iterLastZone;
   typename dataType_t::iterator iterElement;
   typename dataType_t::const_iterator iterLastElement;
};

template<typename T>
Grid<T>::Grid()
{
	// just take the icosahedron_corners
	for(int i=0; i<12; i++) {
		nbCenters++;
		centers.push_back(icosahedron_corners[i]);
		allDataCenter.push_back(std::pair<dataType_t, bool>{{}, true});
	}
};

template<typename T>
int Grid<T>::getNearest(const Vec3f& _v)
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

template<typename T>
void Grid<T>::insert(std::shared_ptr<T> _element, Vec3f pos)
{
	allDataCenter[getNearest(pos)].first.push_back(_element);
};

//! Return an array with the number of the zones in the field of view
template<typename T>
void Grid<T>::intersect(const Vec3f& _pos, float fieldAngle)
{
	Vec3f pos = _pos;
	pos.normalize();
	const float max = cosf(fieldAngle/2.f + angle);

	for (unsigned int i=0; i<nbCenters; i++) {
		allDataCenter[i].second = pos.dot(centers[i]) > max;
	}
	#ifdef DEBUG
		std::cout << "Grid::intersect display "<< std::endl;
		for(auto &i: allDataCenter) {
			std::cout << i.second << " ";
		}
		std::cout << std::endl;
	#endif
}


#include <iostream>

int main(int argc, char **argv)
{
	(void) argc;
	(void) argv;
	Grid<eTest> myGrid;
	for(auto i=0; i<12; i++ ) {
		Vec3f test0 = icosahedron_corners[icosahedron_triangles[i].corners[0]];
		Vec3f test1 = icosahedron_corners[icosahedron_triangles[i].corners[1]];
		Vec3f test2 = icosahedron_corners[icosahedron_triangles[i].corners[2]];

		myGrid.insert(std::make_shared<eTest>(test0), test0);
		myGrid.insert(std::make_shared<eTest>(test1), test1);
		myGrid.insert(std::make_shared<eTest>(test2), test2);
		std::cout << test0.dot(test0) << " " ;
		std::cout << test1.dot(test1) << " " ;
		std::cout << test2.dot(test2) << std::endl;
		std::cout << test0.dot(test1) << " " ;
		std::cout << test0.dot(test2) << " " ;
		std::cout << test1.dot(test2) << std::endl;
	}
	myGrid.intersect(Vec3f(1, 1, 1), 180);
	for (auto &val: myGrid) {
		std::cout << "Object <" << val.get() << "> at " << val->getPos() << " nearest to ";
		myGrid.getNearest(val->getPos());
	}
	return 0;
}
