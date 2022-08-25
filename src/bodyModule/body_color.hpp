#ifndef _BODY_COLOR_HPP_
#define _BODY_COLOR_HPP_

#include "tools/vecmath.hpp"
#include <string>

/*
*	color management of a Body
*
* This class is a container of information grouping all the colors of a body
* halo, label, orbit and trail
* It contains the initial colors of the body and the default colors (static variables)
*
*/

//TODO : const std::string &name should be an ENUM

class BodyColor {

public:
	//! Constructor
	BodyColor();

	//! Constructor
	//! @param halo the color of the halo of the body
	//! @param label the color of the body's label
	//! @param orbit the color of the body's orbit
	//! @param trail the color of the body's trail
	BodyColor(const std::string &_halo, const std::string &_label, const std::string &_orbit, const std::string &_trail);
	~BodyColor();

	const Vec3f getHalo() const {
		return halo;
	};

	const Vec3f getLabel() const {
		return label;
	};

	const Vec3f getOrbit() const {
		return orbit;
	};

	const Vec3f getTrail() const {
		return trail;
	};

	void set(const std::string &name, const Vec3f& value);
	const Vec3f get(const std::string &_name) const;

	void reset();
	void reset(const std::string &name);

	static Vec3f getDefault(const std::string &_name);
	static void setDefault(const std::string &name, const Vec3f& value);
	static void setDefault(const std::string &_halo, const std::string &_label, const std::string &_orbit, const std::string &_trail);

private:
	enum class TYPE_COLOR : char { LABEL, HALO, ORBIT, TRAIL, ALL, NONE};

	static TYPE_COLOR translate (const std::string &value);

	Vec3f halo;			// halo color
	Vec3f label;		// label color
	Vec3f orbit;		// orbit color
	Vec3f trail;		// trail color

	Vec3f initialHalo;		// halo color at loading
	Vec3f initialLabel;		// label color at loading
	Vec3f initialOrbit;		// orbit color at loading
	Vec3f initialTrail;		// trail color at loading

	static Vec3f defaultHalo;
	static Vec3f defaultLabel;
	static Vec3f defaultOrbit;
	static Vec3f defaultTrail;
};

#endif  //_BODY_COLOR_HPP_
