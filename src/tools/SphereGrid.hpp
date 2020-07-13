/*
 * SphereGrid
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

#ifndef _SPHERE_GRID_H_
#define _SPHERE_GRID_H_

#include <vector>
#include <list>
#include <memory>

#include "tools/vecmath.hpp"

//#define DEBUG 1

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

constexpr char segments[30][2] = {
	{0, 1},
	{0, 6},
	{0, 7},
	{0, 9},
	{0, 10},
	{1, 4},
	{1, 5},
	{1, 9},
	{1, 10},
	{2, 3},
	{2, 4},
	{2, 5},
	{2, 8},
	{2, 11},
	{3, 6},
	{3, 7},
	{3, 8},
	{3, 11},
	{4, 5},
	{4, 10},
	{4, 11},
	{5, 8},
	{5, 9},
	{6, 7},
	{6, 8},
	{6, 9},
	{7, 10},
	{7, 11},
	{8, 9},
	{10, 11}
};

template <typename T>
class SphereGrid {
private:
	typedef std::list<T> dataType_t;
	typedef std::vector<std::pair<dataType_t, bool>> dataCenterType_t;

	class iterator;
public:
    SphereGrid();
    ~SphereGrid() {};
	//! insert un élément dans la grille
    void insert(T _element, Vec3f pos);
	//! supprime un élément de la grille
	void remove(T _element, const Vec3f &pos); // Optimized method
	void remove(T _element);
	void remove_if(const auto &func);
	void erase(SphereGrid::iterator &it); // standard std::list::erase
    auto begin() {
		return SphereGrid::iterator(allDataCenter.begin(), allDataCenter.end(), allDataCenter.begin()->first);
	};
	void clear();
    auto end() {
		return SphereGrid::iterator(allDataCenter.end());
	};
    // eTest* next() const;
    // void setFov(Vec3f pos, float fov);
	//! Renvoie quel centre est le plus proche de -v
	int getNearest(const Vec3f& _v);
	//! Return an array with the number of the zones in the field of view
	void intersect(const Vec3f& _pos, float fieldAngle);
	// clear
private:
	//! les centres de la grille
	std::vector<Vec3f> centers;
	//! en cache les centres à afficher de la grille
	//std::vector<int> centerToDisplay;
	//! nombre total de centres
	unsigned int nbCenters=0;
	//! angle que font 2 centres adjacents entre eux
	std::array<float, 4> angle;
	//! Optimized container for access
	dataCenterType_t allDataCenter;
	//! Optimized container for search
	//! Each level contain center of triangle
	std::array<std::pair<std::array<std::pair<std::array<std::pair<std::array<std::pair<std::pair<dataType_t, bool>*, Vec3f>, 6>, Vec3f>, 4>, Vec3f>, 4>, Vec3f>, 20> dataCenter;
};

template<typename T>
class SphereGrid<T>::iterator {
public:
	iterator(auto _zoneBegin, const auto &_zoneEnd, auto &_element) : iterLastZone(_zoneEnd) {
		// Move iterZone to the first non-empty container
		for (iterZone = _zoneBegin; iterZone->first.size() == 0 && iterZone != iterLastZone; iterZone++);
		iterElement = iterZone->first.begin();
		iterLastElement = iterZone->first.cend();
	}
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
	T &operator*() const {
	    return *iterElement;
	}
	void erase() {
		if (this->iterZone != this->iterLastZone)
			this->iterZone->first.erase(this->iterElement);
	}
	typedef ptrdiff_t difference_type; //almost always ptrdiff_t
    typedef T value_type; //almost always T
    typedef T& reference; //almost always T& or const T&
    typedef T* pointer; //almost always T* or const T*
    typedef std::forward_iterator_tag iterator_category;  //usually std::forward_iterator_tag or similar
private:
	typename dataCenterType_t::iterator iterZone;
	typename dataCenterType_t::const_iterator iterLastZone;
	typename dataType_t::iterator iterElement;
	typename dataType_t::const_iterator iterLastElement;
};

template<typename T>
SphereGrid<T>::SphereGrid()
{
	// just take the icosahedron_corners
	for(int i=0; i<12; i++) {
		nbCenters++;
		centers.push_back(icosahedron_corners[i]);
		allDataCenter.push_back(std::pair<dataType_t, bool>{{}, true});
	}
	// allDataCenter.reserve(12*4*4*4); // Because I know this space would be used
	// for (int i=0; i<20; i++) {
	//
	// }
	angle[0] = acos(icosahedron_corners[0].dot(icosahedron_corners[1]));
	#ifdef DEBUG
		std::cout << "Angle = " << angle[0] << std::endl;
	#endif
}

template<typename T>
int SphereGrid<T>::getNearest(const Vec3f& _v)
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
		std::cout << "SphereGrid::getNearest return " << bestI << std::endl;
	#endif
	return bestI;
}

template<typename T>
void SphereGrid<T>::insert(T _element, Vec3f pos)
{
	allDataCenter[getNearest(pos)].first.push_back(_element);
}

template<typename T>
void SphereGrid<T>::remove(T _element, const Vec3f &v)
{
	allDataCenter[getNearest(v)].first.remove(_element);
}

template<typename T>
void SphereGrid<T>::remove(T _element)
{
	for (auto &v: allDataCenter)
		v.first.remove(_element);
}

template<typename T>
void SphereGrid<T>::remove_if(const auto &func)
{
	for (auto &v: allDataCenter)
		v.first.remove_if(func);
}

template<typename T>
void SphereGrid<T>::erase(SphereGrid::iterator &it)
{
	it.erase();
}

//! Return an array with the number of the zones in the field of view
template<typename T>
void SphereGrid<T>::intersect(const Vec3f& _pos, float fieldAngle)
{
	if (fieldAngle >= (3.1415926 - angle[0]) * 2) {
		for (auto &element: allDataCenter)
			element.second = true;
		return;
	}

	Vec3f pos = _pos;
	pos.normalize();
	const float max = cosf(fieldAngle/2.f + angle[0]);

	for (unsigned int i=0; i<nbCenters; i++) {
		allDataCenter[i].second = pos.dot(centers[i]) > max;
	}
	#ifdef DEBUG
		std::cout << "SphereGrid::intersect display "<< std::endl;
		for(auto &i: allDataCenter) {
			std::cout << i.second << " ";
		}
		std::cout << std::endl;
	#endif
}

template<typename T>
void SphereGrid<T>::clear()
{
	for (auto &element: allDataCenter) {
		element.first.clear();
	}
}

// #include <iostream>
//
// int main(int argc, char **argv)
// {
// 	(void) argc;
// 	(void) argv;
// 	Grid<std::shared_ptr<eTest>> myGrid;
// 	for(auto i=0; i<12; i++ ) {
// 		Vec3f test0 = icosahedron_corners[icosahedron_triangles[i].corners[0]];
// 		Vec3f test1 = icosahedron_corners[icosahedron_triangles[i].corners[1]];
// 		Vec3f test2 = icosahedron_corners[icosahedron_triangles[i].corners[2]];
// 		std::shared_ptr<eTest> tmp = std::make_shared<eTest>(test1);
//
// 		myGrid.insert(std::make_shared<eTest>(test0), test0);
// 		myGrid.insert(tmp, test1);
// 		myGrid.insert(std::make_shared<eTest>(test2), test2);
// 		myGrid.remove(tmp);
// 		std::cout << test0.dot(test0) << " " ;
// 		std::cout << test1.dot(test1) << " " ;
// 		std::cout << test2.dot(test2) << std::endl;
// 		std::cout << test0.dot(test1) << " " ;
// 		std::cout << test0.dot(test2) << " " ;
// 		std::cout << test1.dot(test2) << std::endl;
// 	}
// 	myGrid.intersect(icosahedron_corners[0], 3.1415926);
// 	for (auto &val: myGrid) {
// 		std::cout << "Object <" << val.get() << "> at " << val->getPos() << " nearest to ";
// 		myGrid.getNearest(val->getPos());
// 	}
// 	return 0;
// }

#endif // SphereGrid