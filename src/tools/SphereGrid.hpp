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
#include "tools/Tree.hpp"

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

/*
 * Container separating elements into several zones.
 * Only elements in zones which are partially or fully visible were given by an iterator.
 */
template <typename T>
class SphereGrid {
public:
	typedef std::list<T> dataType_t;
	typedef std::pair<dataType_t, bool> elementType_t;
	typedef std::vector<elementType_t> dataCenterType_t;

	typedef struct {
		elementType_t *element;
		Vec3f corners[3];
		Vec3f center;
	} subGrid_t;

	class iterator;

	SphereGrid();
	~SphereGrid() {};
	//! define and build grid subdivisions
	void subdivise(int _nbSubdivision);
	//! insert un élément dans la grille
	void insert(T _element, Vec3f pos);
	//! supprime un élément de la grille
	void remove(const T &_element, const Vec3f &pos); // Optimized method
	void remove(const T &_element);
	template<typename F>
	void remove_if(F&& func);
	void erase(SphereGrid::iterator &it); // standard std::list::erase
	auto begin() {
		return SphereGrid::iterator(allDataCenter.begin(), allDataCenter.end());
	};
	void clear();
	auto end() {
		return SphereGrid::iterator(allDataCenter.end());
	};
	// eTest* next() const;
	// void setFov(Vec3f pos, float fov);
	//! Determine the fields of view which are visible
	void intersect(const Vec3f& _pos, float fieldAngle);
	// clear
private:
	//! Set visibility for all zones
	void setVisibility(Tree<subGrid_t> &data, int subdivisionLvl, bool isVisible);
	//! Build one subdivision and his content
	void buildSubdivision(Tree<subGrid_t> &data, int subdivisionLvl);
	//! intersect of one subdivision and his content
	void subIntersect(const Vec3f &pos, float fieldAngle, Tree<subGrid_t> &data, int subdivisionLvl);
	//! Renvoie un pointeur vers le centre le plus proche de -v
	auto *getNearest(const Vec3f& _v);
	//! Angle entre le centre d'un triange et l'un de ses sommets pour un niveau de division donné
	std::vector<float> angleLvl;
	//! Optimized container for access
	dataCenterType_t allDataCenter;
	//! Optimized container for search
	Tree<subGrid_t> dataCenter;
	//! Number of subdivisions
	int nbSubdivision = 0;
};

template<typename T>
class SphereGrid<T>::iterator {
public:
	iterator(typename SphereGrid::dataCenterType_t::iterator _zoneBegin, const typename SphereGrid::dataCenterType_t::iterator &_zoneEnd) : iterLastZone(_zoneEnd) {
		// Move iterZone to the first non-empty container
		for (iterZone = _zoneBegin; iterZone->first.size() == 0 && iterZone != iterLastZone; iterZone++);
		iterElement = iterZone->first.begin();
		iterLastElement = iterZone->first.cend();
	}
	iterator(const typename SphereGrid::dataCenterType_t::iterator &_iterZone) : iterZone(_iterZone) {}
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
inline void SphereGrid<T>::buildSubdivision(Tree<subGrid_t> &data, int subdivisionLvl)
{
	subGrid_t tmp;
	Vec3f middleSegments[3];

	tmp.element = nullptr;
	tmp.center = data.value.center;
	for (u_char j = 0; j < 3; j++) {
		middleSegments[j] = data.value.corners[(j + 1) % 3] + data.value.corners[(j + 2) % 3];
		middleSegments[j].normalize();
		tmp.corners[j] = middleSegments[j];
	}
	data.push_back(tmp);

	for (u_char j = 0; j < 3; j++) {
		tmp.corners[0] = data.value.corners[j];
		tmp.corners[1] = middleSegments[(j + 1) % 3];
		tmp.corners[2] = middleSegments[(j + 2) % 3];
		tmp.center = tmp.corners[0] + tmp.corners[1] + tmp.corners[2];
		tmp.center.normalize();
		data.push_back(tmp);
	}

	if (subdivisionLvl < nbSubdivision) {
		for (u_char j = 0; j < 4; j++) {
			// build next subdivision
			buildSubdivision(data[j], subdivisionLvl + 1);
		}
	} else {
		for (u_char j = 0; j < 4; j++) {
			// create container
			allDataCenter.push_back(std::pair<dataType_t, bool>({}, false));
			// assign created container
			data[j].value.element = &(*allDataCenter.rbegin());
		}
	}
}

template<typename T>
void SphereGrid<T>::subdivise(int _nbSubdivision)
{
	// clear content to rebuild grid
	allDataCenter.clear();
	angleLvl.clear();
	allDataCenter.reserve(20 * pow(4, _nbSubdivision)); // This container mustn't need to reallocate memory
	void *ptr = allDataCenter.data(); // There must be no reallocation
	nbSubdivision = _nbSubdivision;
	for (auto &value: dataCenter) {
		value->clear();
		buildSubdivision(*value, 1);
	}

	// build angleLvl
	Tree<subGrid_t> *actual = &dataCenter;
	for (u_char i = 0; i <= nbSubdivision; i++) {
		angleLvl.push_back(actual->value.center.dot(actual->value.corners[0]));
		actual = &(*actual)[0];
	}
	assert(ptr == allDataCenter.data()); // There must be no reallocation
}

template<typename T>
SphereGrid<T>::SphereGrid()
{
	subGrid_t tmp;
	tmp.element = nullptr;

	for (u_char i = 0; i < 20; i++) {
		for (u_char j = 0; j < 3; j++) {
			tmp.corners[j] = icosahedron_corners[icosahedron_triangles[i].corners[j]];
		}
		tmp.center = tmp.corners[0] + tmp.corners[1] + tmp.corners[2];
		tmp.center.normalize();
		dataCenter.push_back(tmp);
	}
}

template<typename T>
auto *SphereGrid<T>::getNearest(const Vec3f& _v)
{
	Vec3f v=_v;
	float bestDot = -2.f;
	Tree<subGrid_t> *best = nullptr;
	v.normalize();

	Tree<subGrid_t> *actual = &dataCenter;
	for (u_char i = 0; i <= nbSubdivision; i++) {
		for (auto &data: *actual) {
			if (v.dot(data->value.center) >= bestDot) {
				bestDot = v.dot(data->value.center);
				best = data.get();
			}
		}
		actual = best;
	}
	return (actual->value.element);
}

template<typename T>
void SphereGrid<T>::insert(T _element, Vec3f pos)
{
	getNearest(pos)->first.push_back(std::move(_element));
}

template<typename T>
void SphereGrid<T>::remove(const T &_element, const Vec3f &v)
{
	getNearest(v)->first.remove(_element);
}

template<typename T>
void SphereGrid<T>::remove(const T &_element)
{
	for (auto &v: allDataCenter)
		v.first.remove(_element);
}

template<typename T>
template<typename F>
void SphereGrid<T>::remove_if(F&& func)
{
	for (auto &v: allDataCenter)
		v.first.remove_if(func);
}

template<typename T>
void SphereGrid<T>::erase(SphereGrid<T>::iterator &it)
{
	it.erase();
}

template<typename T>
void SphereGrid<T>::setVisibility(Tree<subGrid_t> &data, int subdivisionLvl, bool isVisible)
{
	for (auto &it: data) {
		if (subdivisionLvl < nbSubdivision) {
			setVisibility(*it, subdivisionLvl + 1, isVisible);
		} else {
			it->value.element->second = isVisible;
		}
	}
}

template<typename T>
void SphereGrid<T>::subIntersect(const Vec3f &pos, float fieldAngle, Tree<subGrid_t> &data, int subdivisionLvl)
{
	const float max = cosf(fieldAngle/2.f + angleLvl[subdivisionLvl]);

	if (subdivisionLvl == nbSubdivision) {
		// determine for all remaining zones if they were
		for (auto &it: data) {
			it->value.element->second = pos.dot(it->value.center) > max;
		}
		return;
	}

	const float min = cosf(fieldAngle/2.f - angleLvl[subdivisionLvl]);

	for (auto &it: data) {
		if (pos.dot(it->value.center) > max) {
			// all subZones were visible
			setVisibility(*it, subdivisionLvl + 1, true);
		} else if (pos.dot(it->value.center) > min) {
			// all subZones weren't visible
			setVisibility(*it, subdivisionLvl + 1, false);
		} else {
			// we must determine this for all subZones
			subIntersect(pos, fieldAngle, *it, subdivisionLvl + 1);
		}
	}
}

//! Return an array with the number of the zones in the field of view
template<typename T>
void SphereGrid<T>::intersect(const Vec3f& _pos, float fieldAngle)
{
	if (fieldAngle >= (3.1415926 - *angleLvl.rbegin()) * 2) { // Check if all zones are visible
		for (auto &element: allDataCenter)
			element.second = true;
		return;
	}

	Vec3f pos = _pos;
	pos.normalize();
	subIntersect(pos, fieldAngle, dataCenter, 0);
}

template<typename T>
void SphereGrid<T>::clear()
{
	for (auto &element: allDataCenter) {
		element.first.clear();
	}
}

#endif /* _SPHERE_GRID_H_ */
