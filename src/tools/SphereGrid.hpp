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
	const unsigned char corners[3];   // index der Ecken
};

#ifdef __GNUC__
constexpr float icosahedron_G = 0.5*(1.0+sqrt(5.0));
constexpr float icosahedron_b = 1.0/sqrt(1.0+icosahedron_G*icosahedron_G);
#else // constexpr sqrt is not supported for non-gcc compilers
constexpr float icosahedron_G = 1.6180339887498948482045868343656;
constexpr float icosahedron_b = 0.52573111211913360602566908484788;
#endif

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

/**
* \class SphereGrid
*
* \brief Container separating elements into several zones.
*
* @section REQUIREMENT
* Define subdivision before use it
*
* @section USAGE
* Call the "intersect" method to update zones visibility.
*
* Only elements in zones which are partially or fully visible were accessible by an iterator.
*
* \author Calvin Ruiz
*/
template <typename T>
class SphereGrid {
public:
	typedef std::list<T> dataType_t;
	typedef std::pair<dataType_t, bool> elementType_t;
	typedef std::vector<elementType_t> dataCenterType_t;

	struct subGrid_t {
		elementType_t *element;
		Vec3f corners[3];
		Vec3f center;
	};

	class iterator;
	class const_iterator;

	SphereGrid();
	~SphereGrid() {};
	//! @brief Set the number of grid subdivisions and builds them.
	//! A subdivision divides each zone into 4 sub-zones.
	void subdivise(int _nbSubdivision);
	//! Insert an element in this grid
	void insert(T _element, Vec3f pos, float objectRadius = 0);
	//! Remove the corresponding element from this grid (optimized version)
	void remove(const T &_element, const Vec3f &pos);
	//! Remove the corresponding element from this grid
	void remove(const T &_element);
	//! Remove the elements for which the function passed in parameter returns true
	template<typename F>
	void remove_if(F&& func);
	//! Standard std::list::erase
	void erase(SphereGrid::iterator &it);
	//! Returns an iterator on the first visible element that iterates through all visible elements
	SphereGrid::iterator begin() {
		return SphereGrid::iterator(allDataCenter.begin(), allDataCenter.end());
	};
	SphereGrid::const_iterator begin() const {
		return SphereGrid::const_iterator(allDataCenter.begin(), allDataCenter.end());
	};
	//! Returns an iterator on the first element that iterates through all the elements
	SphereGrid::iterator rawBegin() {
		return SphereGrid::iterator(allDataCenter.begin(), allDataCenter.end(), false);
	}
	SphereGrid::const_iterator rawBegin() const {
		return SphereGrid::const_iterator(allDataCenter.begin(), allDataCenter.end(), false);
	}
	//! Remove all elements from this grid
	void clear();
	SphereGrid::iterator end() {
		return SphereGrid::iterator(allDataCenter.end());
	};
	SphereGrid::const_iterator end() const {
		return SphereGrid::const_iterator(allDataCenter.end());
	};
	// void setFov(Vec3f pos, float fov);
	//! Determine which fields of view are visible
	void intersect(const Vec3f& _pos, float fieldAngle);
	//! Define maximal object radius in radian
	void setMaxObjectRadius(float radius) {maxObjectRadius = radius;}
private:
	//! Set visibility flag of all sub-zones
	void setVisibility(Tree<subGrid_t> &data, int subdivisionLvl, bool isVisible);
	//! Build one subdivision
	void buildSubdivision(Tree<subGrid_t> &data, int subdivisionLvl);
	//! Determine if one zone is visible
	void subIntersect(const Vec3f &pos, float fieldAngle, Tree<subGrid_t> &data, int subdivisionLvl);
	//! Return a pointer to the nearest zone of -v
	SphereGrid<T>::elementType_t *getNearest(const Vec3f& _v);
	//! Angle between the center of a triangular and one of its vertices for a given division level
	std::vector<float> angleLvl;
	//! Optimized container for access
	dataCenterType_t allDataCenter;
	//! Optimized container for search
	Tree<subGrid_t> dataCenter;
	//! Number of subdivisions
	int nbSubdivision = 0;
	//! Biggest object radius in radian
	float maxObjectRadius = 0;
};

template<typename T>
class SphereGrid<T>::iterator {
public:
	iterator(typename SphereGrid<T>::dataCenterType_t::iterator _zoneBegin, const typename SphereGrid<T>::dataCenterType_t::iterator &_zoneEnd, bool _skipUnvisible = true) : iterLastZone(_zoneEnd), skipUnvisible(_skipUnvisible) {
		// Move iterZone to the first non-empty container
		for (iterZone = _zoneBegin; iterZone != iterLastZone && (iterZone->first.size() == 0 || (_skipUnvisible && !iterZone->second)); iterZone++);
		if (iterZone != iterLastZone) {
			iterElement = iterZone->first.begin();
			iterLastElement = iterZone->first.cend();
		}
	}
	iterator(const typename SphereGrid<T>::dataCenterType_t::iterator &_iterZone) : iterZone(_iterZone) {}
	bool operator!=(iterator compare) const {
		return iterZone != compare.iterZone;
	}
	bool operator!=(const_iterator compare) const {
		return iterZone != compare.iterZone;
	}
	void operator++() {
		if (++iterElement == iterLastElement) {
			if (skipUnvisible) {
				while ((!iterZone->second || iterElement == iterLastElement) && ++iterZone != iterLastZone) {
					iterElement = iterZone->first.begin();
					iterLastElement = iterZone->first.end();
				}
			} else {
				while (iterElement == iterLastElement && ++iterZone != iterLastZone) {
					iterElement = iterZone->first.begin();
					iterLastElement = iterZone->first.end();
				}
			}
		}
	}
	T &operator*() const {
		return *iterElement;
	}
	//! Destroy element pointed by this iterator
	//! Warning: This action invalidate this iterator.
	void erase() {
		if (this->iterZone != this->iterLastZone)
			this->iterZone->first.erase(this->iterElement);
	}
	typedef ptrdiff_t difference_type; //almost always ptrdiff_t
	typedef T value_type; //almost always T
	typedef T& reference; //almost always T& or const T&
	typedef T* pointer; //almost always T* or const T*
	typedef std::forward_iterator_tag iterator_category;  //usually std::forward_iterator_tag or similar
protected:
	iterator() {}
	typename SphereGrid<T>::dataCenterType_t::iterator iterZone;
	typename SphereGrid<T>::dataCenterType_t::const_iterator iterLastZone;
	typename SphereGrid<T>::dataType_t::iterator iterElement;
	typename SphereGrid<T>::dataType_t::const_iterator iterLastElement;
	bool skipUnvisible = true; // Can't be const for operator= (MSVC requirement)
};

template<typename T>
class SphereGrid<T>::const_iterator {
public:
	const_iterator(typename SphereGrid<T>::dataCenterType_t::const_iterator _zoneBegin, const typename SphereGrid<T>::dataCenterType_t::const_iterator &_zoneEnd, bool _skipUnvisible = true) : iterLastZone(_zoneEnd), skipUnvisible(_skipUnvisible) {
		// Move iterZone to the first non-empty container
		for (iterZone = _zoneBegin; iterZone != iterLastZone && (iterZone->first.size() == 0 || (_skipUnvisible && !iterZone->second)); iterZone++);
		if (iterZone != iterLastZone) {
			iterElement = iterZone->first.begin();
			iterLastElement = iterZone->first.cend();
		}
	}
	const_iterator(const typename SphereGrid<T>::dataCenterType_t::const_iterator &_iterZone) : iterZone(_iterZone) {}
	bool operator!=(iterator compare) const {
		return iterZone != compare.iterZone;
	}
	bool operator!=(const_iterator compare) const {
		return iterZone != compare.iterZone;
	}
	void operator++() {
		if (++iterElement == iterLastElement) {
			if (skipUnvisible) {
				while ((!iterZone->second || iterElement == iterLastElement) && ++iterZone != iterLastZone) {
					iterElement = iterZone->first.begin();
					iterLastElement = iterZone->first.end();
				}
			} else {
				while (iterElement == iterLastElement && ++iterZone != iterLastZone) {
					iterElement = iterZone->first.begin();
					iterLastElement = iterZone->first.end();
				}
			}
		}
	}
	const T &operator*() const {
		return *iterElement;
	}
	typedef ptrdiff_t difference_type; //almost always ptrdiff_t
	typedef T value_type; //almost always T
	typedef T& reference; //almost always T& or const T&
	typedef T* pointer; //almost always T* or const T*
	typedef std::forward_iterator_tag iterator_category;  //usually std::forward_iterator_tag or similar
private:
	const_iterator() {}
	typename SphereGrid<T>::dataCenterType_t::const_iterator iterZone;
	typename SphereGrid<T>::dataType_t::const_iterator iterElement;
	typename SphereGrid<T>::dataCenterType_t::const_iterator iterLastZone;
	typename SphereGrid<T>::dataType_t::const_iterator iterLastElement;
	bool skipUnvisible = true; // Can't be const for operator= (MSVC requirement)
};

template<typename T>
inline void SphereGrid<T>::buildSubdivision(Tree<subGrid_t> &data, int subdivisionLvl)
{
	subGrid_t tmp;
	Vec3f middleSegments[3];

	tmp.element = nullptr;
	tmp.center = data.value.center;
	for (unsigned char j = 0; j < 3; j++) {
		middleSegments[j] = data.value.corners[(j + 1) % 3] + data.value.corners[(j + 2) % 3];
		middleSegments[j].normalize();
		tmp.corners[j] = middleSegments[j];
	}
	data.push_back(tmp);

	for (unsigned char j = 0; j < 3; j++) {
		tmp.corners[0] = data.value.corners[j];
		tmp.corners[1] = middleSegments[(j + 1) % 3];
		tmp.corners[2] = middleSegments[(j + 2) % 3];
		tmp.center = tmp.corners[0] + tmp.corners[1] + tmp.corners[2];
		tmp.center.normalize();
		data.push_back(tmp);
	}

	if (subdivisionLvl < nbSubdivision) {
		for (unsigned char j = 0; j < 4; j++) {
			// build next subdivision
			buildSubdivision(data[j], subdivisionLvl + 1);
		}
	} else {
		for (unsigned char j = 0; j < 4; j++) {
			// create container
			allDataCenter.push_back(std::pair<dataType_t, bool>(dataType_t(), false));
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
		value.clear();
		buildSubdivision(value, 1);
	}

	// build angleLvl
	Tree<subGrid_t> *actual = &dataCenter;
	for (unsigned char i = 0; i <= nbSubdivision; i++) {
		angleLvl.push_back(acosf(actual->value.center.dot(actual->value.corners[0])));
		actual = &(*actual)[0];
	}
	assert(ptr == allDataCenter.data()); // There must be no reallocation
}

template<typename T>
SphereGrid<T>::SphereGrid()
{
	subGrid_t tmp;
	tmp.element = nullptr;

	for (unsigned char i = 0; i < 20; i++) {
		for (unsigned char j = 0; j < 3; j++) {
			tmp.corners[j] = icosahedron_corners[icosahedron_triangles[i].corners[j]];
		}
		tmp.center = tmp.corners[0] + tmp.corners[1] + tmp.corners[2];
		tmp.center.normalize();
		dataCenter.push_back(tmp);
	}
}

template<typename T>
SphereGrid<T>::elementType_t *SphereGrid<T>::getNearest(const Vec3f& _v)
{
	Vec3f v=_v;
	float bestDot = -2.f;
	Tree<subGrid_t> *best = nullptr;
	v.normalize();

	Tree<subGrid_t> *actual = &dataCenter;
	for (unsigned char i = 0; i <= nbSubdivision; i++) {
		for (auto &data: *actual) {
			if (v.dot(data.value.center) >= bestDot) {
				bestDot = v.dot(data.value.center);
				best = &data;
			}
		}
		actual = best;
	}
	return (actual->value.element);
}

template<typename T>
void SphereGrid<T>::insert(T _element, Vec3f pos, float objectRadius)
{
	if (objectRadius > maxObjectRadius)
		maxObjectRadius = objectRadius;
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
			setVisibility(it, subdivisionLvl + 1, isVisible);
		} else {
			it.value.element->second = isVisible;
		}
	}
}

template<typename T>
void SphereGrid<T>::subIntersect(const Vec3f &pos, float fieldAngle, Tree<subGrid_t> &data, int subdivisionLvl)
{
	const float max = cosf(std::min(fieldAngle/2.f + angleLvl[subdivisionLvl] + maxObjectRadius, (float) M_PI));

	if (subdivisionLvl == nbSubdivision) {
		// determine for all remaining zones if they were
		for (auto &it: data) {
			it.value.element->second = pos.dot(it.value.center) >= max;
		}
		return;
	}

	const float min = cosf(std::max(fieldAngle/2.f - angleLvl[subdivisionLvl] - maxObjectRadius, 0.f));
	for (auto &it: data) {
		float value = pos.dot(it.value.center);
		if (value < max) {
			// all subZones weren't visible
			setVisibility(it, subdivisionLvl + 1, false);
		} else if (value >= min) {
			// all subZones were visible
			setVisibility(it, subdivisionLvl + 1, true);
		} else {
			// we must determine this for all subZones
			subIntersect(pos, fieldAngle, it, subdivisionLvl + 1);
		}
	}
}

template<typename T>
void SphereGrid<T>::intersect(const Vec3f& _pos, float fieldAngle)
{
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
