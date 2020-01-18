#include "bodyModule/body_color.hpp"
#include "tools/utility.hpp"
#include "tools/log.hpp"

Vec3f BodyColor::defaultHalo=v3fNull;
Vec3f BodyColor::defaultLabel=v3fNull;
Vec3f BodyColor::defaultOrbit=v3fNull;
Vec3f BodyColor::defaultTrail=v3fNull;


BodyColor::BodyColor()
{}

BodyColor::BodyColor(const std::string &_halo, const std::string &_label, const std::string &_orbit, const std::string &_trail)
{
	if (_halo.empty())
		initialHalo = defaultHalo;
	else
		initialHalo  = Utility::strToVec3f(_halo);

	if (_label.empty())
		initialLabel = defaultLabel;
	else
		initialLabel = Utility::strToVec3f(_label);

	if (_orbit.empty())
		initialOrbit = defaultOrbit;
	else
		initialOrbit = Utility::strToVec3f(_orbit);

	if (_trail.empty())
		initialTrail = defaultTrail;
	else
		initialTrail = Utility::strToVec3f(_trail);

	reset();
}

BodyColor::~BodyColor()
{}

void BodyColor::reset()
{
	halo = initialHalo;
	label = initialLabel;
	orbit = initialOrbit;
	trail = initialTrail;
}

void BodyColor::set(const std::string &name, const Vec3f& value)
{
	switch (BodyColor::translate(name)) {
		case TYPE_COLOR::HALO :
			halo = value;
			break;

		case TYPE_COLOR::LABEL :
			label = value;
			break;

		case TYPE_COLOR::TRAIL :
			trail = value;
			break;

		case TYPE_COLOR::ORBIT :
			orbit = value;
			break;

		case TYPE_COLOR::ALL :
			halo = value;
			label = value;
			trail = value;
			orbit = value;
			break;

		default:
			break;
	}
}


void BodyColor::setDefault(const std::string &name, const Vec3f& value)
{
	switch (BodyColor::translate(name)) {
		case TYPE_COLOR::HALO :
			defaultHalo = value;
			break;

		case TYPE_COLOR::LABEL :
			defaultLabel = value;
			break;

		case TYPE_COLOR::TRAIL :
			defaultTrail = value;
			break;

		case TYPE_COLOR::ORBIT :
			defaultOrbit = value;
			break;

		case TYPE_COLOR::ALL :
			defaultHalo = value;
			defaultLabel = value;
			defaultTrail = value;
			defaultOrbit = value;
			break;

		default:
			break;
	}
}

void BodyColor::reset(const std::string &name)
{
	switch (BodyColor::translate(name)) {
		case TYPE_COLOR::HALO :
			halo = initialHalo;
			break;

		case TYPE_COLOR::LABEL :
			label = initialLabel;
			break;

		case TYPE_COLOR::TRAIL :
			trail = initialTrail;
			break;

		case TYPE_COLOR::ORBIT :
			orbit = initialOrbit;
			break;

		case TYPE_COLOR::ALL :
			halo = initialHalo;
			label = initialLabel;
			trail = initialTrail;
			orbit = initialOrbit;
			break;

		default:
			break;
	}
}

const Vec3f BodyColor::get(const std::string &_name) const
{
	switch (BodyColor::translate(_name)) {
		case TYPE_COLOR::HALO :
			return halo;
			break;

		case TYPE_COLOR::LABEL :
			return label;
			break;

		case TYPE_COLOR::TRAIL :
			return trail;
			break;

		case TYPE_COLOR::ORBIT :
			return orbit;
			break;

		default:
			return v3fNull;
			break;
	}
}

Vec3f BodyColor::getDefault(const std::string &_name)
{
	switch (BodyColor::translate(_name)) {
		case TYPE_COLOR::HALO :
			return defaultHalo;
			break;

		case TYPE_COLOR::LABEL :
			return defaultLabel;
			break;

		case TYPE_COLOR::TRAIL :
			return defaultTrail;
			break;

		case TYPE_COLOR::ORBIT :
			return defaultOrbit;
			break;

		default:
			return v3fNull;
			break;
	}
}

void BodyColor::setDefault(const std::string & _halo, const std::string & _label, const std::string & _orbit, const std::string & _trail)
{
	if (_halo.empty())
		defaultHalo = v3fNull;
	else
		defaultHalo  = Utility::strToVec3f(_halo);

	if (_label.empty())
		defaultLabel = v3fNull;
	else
		defaultLabel = Utility::strToVec3f(_label);

	if (_orbit.empty())
		defaultOrbit = v3fNull;
	else
		defaultOrbit = Utility::strToVec3f(_orbit);

	if (_trail.empty())
		defaultTrail = v3fNull;
	else
		defaultTrail = Utility::strToVec3f(_trail);
}

BodyColor::TYPE_COLOR BodyColor::translate(const std::string &value)
{
	if (value=="halo")
		return TYPE_COLOR::HALO;
	else if (value=="label")
		return TYPE_COLOR::LABEL;
	else if (value=="orbit")
		return TYPE_COLOR::ORBIT;
	else if (value=="trail")
		return TYPE_COLOR::TRAIL;
	else if (value=="all")
		return TYPE_COLOR::ALL;
	else {
		cLog::get()->write("Unknown value "+ value + " in bodyColor", LOG_TYPE::L_WARNING);
		return TYPE_COLOR::NONE;
	}
}