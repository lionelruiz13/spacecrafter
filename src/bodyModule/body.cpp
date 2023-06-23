/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014 of the LSS Team & Association Sirius
 * Copyright (C) 2017 Immersive Adventure
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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
 *
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include <iostream>
#include <iomanip>

#include "bodyModule/body.hpp"
#include "navModule/observer.hpp"
#include "coreModule/projector.hpp"
#include "tools/s_font.hpp"
#include "../planetsephems/sideral_time.h"
#include "tools/log.hpp"
//#include "tools/fmath.hpp"
#include "tools/sc_const.hpp"
#include <chrono>
#include "bodyModule/ring.hpp"
#include "bodyModule/solarsystem_display.hpp"
#include "tools/file_path.hpp"
#include "bodyModule/trail.hpp"
#include "bodyModule/axis.hpp"
#include "bodyModule/halo.hpp"
#include "bodyModule/orbit_plot.hpp"
#include "bodyModule/atm_ext.hpp"
#include "tools/s_font.hpp"
#include "navModule/navigator.hpp"
#include "tools/translator.hpp"
#include "atmosphereModule/tone_reproductor.hpp"
#include "bodyModule/body_color.hpp"
#include "coreModule/time_mgr.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

// for compute tail shape
#define COMET_TAIL_SLICES 16u // segments around the perimeter
#define COMET_TAIL_STACKS 16u // cuts along the rotational axis


s_font* Body::planet_name_font = nullptr;
float Body::object_scale = 1.f;
float Body::object_size_limit = 9;
LinearFader Body::flagClouds;
std::shared_ptr<s_texture> Body::defaultTexMap = nullptr;
std::shared_ptr<s_texture> Body::tex_eclipse_map = nullptr;

AtmosphereParams *Body::defaultAtmosphereParams = nullptr;
std::shared_ptr<BodyTesselation> Body::bodyTesselation = nullptr;

Body::Body(std::shared_ptr<Body> parent,
           const std::string& englishName,
           BODY_TYPE _typePlanet,
           bool _flagHalo,
           double _radius,
           double oblateness,
           std::unique_ptr<BodyColor> _myColor,
           float _sol_local_day,
           float _albedo,
           std::unique_ptr<Orbit> _orbit,
           bool close_orbit,
           ObjL* _currentObj,
           double orbit_bounding_radius,
           const BodyTexture &_bodyTexture):
	englishName(englishName), initialRadius(_radius), one_minus_oblateness(1.0-oblateness),
	albedo(_albedo), axis_rotation(0.),
	tex_map(nullptr), tex_norm(nullptr), eye_sun(0.0f, 0.0f, 0.0f),
	lastJD(J2000), deltaJD(JD_SECOND/4), orbit(std::move(_orbit)), parent(parent), close_orbit(close_orbit),
    orbit_bounding_radius(orbit_bounding_radius), boundingRadius(-1), sun_half_angle(0.0)
	// tailFactors(-1., -1.), // mark "invalid"
	// tailActive(false),
	// tailBright(false),
	// deltaJDEtail(15.0*JD_MINUTE), // update tail geometry every 15 minutes only
	// lastJDEtail(0.0),
	// dustTailWidthFactor(1.5f),
	// dustTailLengthFactor(0.4f),
	// dustTailBrightnessFactor(1.5f),
{
	radius = _radius;
	myColor = std::move(_myColor);
	orbit_position = v3fNull;
	sol_local_day = _sol_local_day;
	typePlanet = _typePlanet;
	initialScale= 1.0;
	if (parent) {
        parent->satellites.push_back(this);
        switch (parent->getBodyType()) {
            case CENTER:
            case SUN:
            case STAR:
                is_satellite = false;
                break;
            default:
                is_satellite = true;
        }
		if (parent->getBodyType() == CENTER || (parent->getBodyType() == SUN && !parent->parent))
            tAround = tACenter;
		else
            tAround = tABody;
	} else {
		tAround = tANothing;
        is_satellite = false;
    }

	ecliptic_pos= v3dNull;
	rot_local_to_parent = Mat4d::identity();
	rot_local_to_parent_unprecessed = Mat4d::identity();
	mat_local_to_parent = Mat4d::identity();

	//fix graphical bug
	if (_radius==0.0)
		radius = 1/AU;

	if (_bodyTexture.tex_map.empty()) {
		tex_map = defaultTexMap;
	} else
		tex_map = std::make_shared<s_texture>(FilePath(_bodyTexture.tex_map,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID, true, true);

	if (!_bodyTexture.tex_skin.empty()) {
		tex_skin = std::make_shared<s_texture>(FilePath(_bodyTexture.tex_skin,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID, true, true);
	}

	if (!_bodyTexture.tex_norm.empty()) {  //preparation au bump shader
		tex_norm = std::make_shared<s_texture>(FilePath(_bodyTexture.tex_norm,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID, true, true);
	}

	if (!_bodyTexture.tex_heightmap.empty()) {  //preparation à la tesselation
		tex_heightmap = std::make_shared<s_texture>(FilePath(_bodyTexture.tex_heightmap,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID, true, true);
	}

	nameI18 = englishName;

	atmosphereParams = defaultAtmosphereParams;

	visibilityFader = true;

	currentObj = _currentObj;

	flags.flag_axis =false;
	flags.flag_trail =false;
	flags.flag_hints =false;
	flags.flag_orbit =false;
	flags.flag_halo =_flagHalo;

	hints = std::make_shared<Hints>(this);
	axis = std::make_shared<Axis>(this);
	halo = std::make_shared<Halo>(this);

	tex_current = tex_map;
}

Body::~Body()
{
    if (parent)
        parent->satellites.remove(this);
    if (isCenterOfInterest && SolarSystemDisplay::instance)
        SolarSystemDisplay::instance->invalidateCenterOfInterest();
}

void Body::switchMapSkin(bool a) {
	if ((a==true) && tex_skin!=nullptr) {
		tex_current = tex_skin;
        changed = true;
		return;
	}
	if (a==false) {
		tex_current = tex_map;
        changed = true;
		return;
	}
}

void Body::createTexSkin(const std::string &texName) {
	if (tex_skin != nullptr) {
		tex_current = tex_map;
	}
	tex_skin = std::make_shared<s_texture>(FilePath( texName,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID_REPEAT, true, true);
    changed = true;
}

void Body::setFlagHints(bool b)
{
	hints->setFlagHints(b);
	flags.flag_hints = b;
}

bool Body::getFlagHints(void) const
{
	return flags.flag_hints;
}

bool Body::getFlagAxis(void) const
{
	return flags.flag_axis;
}

void Body::setFlagAxis(bool b)
{
	axis->setFlagAxis(b);
	flags.flag_axis = b;
}

void Body::setFlagTrail(bool b)
{

	if(trail!= nullptr) {
		trail->setFlagTrail(b);
		flags.flag_trail = b;
	}
}

bool Body::getFlagTrail(void) const
{
	if(trail!=nullptr)
		return flags.flag_trail;
	return false;
}

void Body::setFlagHalo(bool b)
{
	flags.flag_halo = b;
}

bool Body::getFlagHalo(void) const
{
	return flags.flag_halo;
}


bool Body::setTexHaloMap(const std::string &texMap)
{
	return Halo::setTexHaloMap(texMap);
}


void Body::setFlagOrbit(bool b)
{
	if(orbitPlot != nullptr) {
		orbitPlot->setFlagOrbit(b);
		flags.flag_orbit = b;
	}
}

bool Body::getFlagOrbit(void)const
{
	return flags.flag_orbit;
}

// set a color
void Body::setColor(const std::string& colorName,  const Vec3f& oc)
{
	myColor->set(colorName, oc);
}

// retrieves a color from a parameter
const Vec3f Body::getColor(const std::string& colorName)
{
	return myColor->get(colorName);
}

void Body::deleteDefaultTexMap()
{
	defaultTexMap = nullptr;
	tex_eclipse_map = nullptr;
	Halo::deleteDefaultTexMap();
    Halo::destroySC_context();
}

void Body::createShader()
{
	OrbitPlot::createSC_context();
	Trail::createSC_context();
	Halo::createSC_context();
	Hints::createSC_context();
	Axis::createSC_context();
}

void Body::deleteShader()
{
    Axis::destroySC_context();
}

// Return the information std::string "ready to print" :)
std::string Body::getInfoString(const Navigator * nav) const
{
	double tempDE, tempRA;
	std::ostringstream oss;

	oss << _(englishName);  // UI translation can differ from sky translation
	oss.setf(std::ios::fixed);
	oss.precision(1);
	oss << std::endl;

	oss.precision(2);
	oss << _("Magnitude: ") << computeMagnitude(nav->getObserverHelioPos()) << std::endl;

	Vec3d equPos = getEarthEquPos(nav);
	Utility::rectToSphe(&tempRA,&tempDE,equPos);

	oss << _("RA/DE: ") << Utility::printAngleHMS(tempRA) << " / " << Utility::printAngleDMS(tempDE) << std::endl;

	// calculate alt az position
	Vec3d localPos = nav->earthEquToLocal(equPos);
	Utility::rectToSphe(&tempRA,&tempDE,localPos);
	tempRA = 3*M_PI - tempRA;  // N is zero, E is 90 degrees
	if (tempRA > M_PI*2) tempRA -= M_PI*2;

	oss << _("Alt/Az: ") << Utility::printAngleDMS(tempDE) << " / " << Utility::printAngleDMS(tempRA) << std::endl;

	oss.precision(8);
	oss << _("Distance: ") << equPos.length() << " " << _("AU");

	return oss.str();
}

//! Get sky label (sky translation)
std::string Body::getSkyLabel(const Navigator * nav) const
{
	return nameI18;
}


// Return the information std::string "ready to print" :)
std::string Body::getShortInfoString(const Navigator * nav) const
{
	std::ostringstream oss;
	oss << _(englishName);  // UI translation can differ from sky translation
	oss << " : " << _(getTypePlanet(typePlanet)) << " ";
	oss.setf(std::ios::fixed);
	oss.precision(2);
	oss << "  " << _("Magnitude: ") << computeMagnitude(nav->getObserverHelioPos());

	Vec3d equPos = getEarthEquPos(nav);
	oss.precision(4);
	oss << "  " <<  _("Distance: ") << equPos.length() << " " << _("AU");
	return oss.str();
}

void Body::getAltAz(const Navigator * nav, double *alt, double *az) const
{
	double _alt, _az;
	Vec3d equPos = getEarthEquPos(nav);
	Vec3d local_pos = nav->earthEquToLocal(equPos);
	Utility::rectToSphe(&_az,&_alt,local_pos);
	_az = 3*M_PI - _az;
	if (_az > M_PI*2) _az -= M_PI*2;
	*az=_az;
	*alt= _alt;
}

void Body::getRaDeValue(const Navigator *nav,double *ra, double *de) const
{
    Utility::rectToSphe(ra, de, getEarthEquPos(nav));
}

std::string Body::getShortInfoNavString(const Navigator * nav, const TimeMgr * timeMgr, const Observer* observatory) const
{
	std::ostringstream oss;
	double tempDE, tempRA;
	Vec3d equPos = getEarthEquPos(nav);
	Utility::rectToSphe(&tempRA,&tempDE,equPos);
	double latitude = observatory->getLatitude();
	double daytime = tan(tempDE)*tan(latitude*M_PI/180); // partial calculation to determinate if midnight sun or not
	oss << _("RA/DE: ") << Utility::printAngleHMS(tempRA) << "/" << Utility::printAngleDMS(tempDE);

	double jd=timeMgr->getJulian();
	double sidereal;
	double T;
	double Le;
	double HA;
	double GHA;
	double PA;

	T = (jd - 2451545.0) / 36525.0;
	Le = observatory->getLongitude();
	/* calc mean angle */
	sidereal = 280.46061837 + (360.98564736629 * (jd - 2451545.0)) + (0.000387933 * T * T) - (T * T * T / 38710000.0);
	HA=sidereal+Le-tempRA*180.0/M_PI;
	GHA=sidereal-tempRA*180.0/M_PI;
	while (HA>=360) HA-=360;
	while (HA<0)    HA+=360;
	if (HA<180) PA=HA;
	else PA=360-HA;
	while (GHA>=360) GHA-=360;
	while (GHA<0)    GHA+=360;
	while (tempRA>=2*M_PI) tempRA-=2*M_PI;
	while (tempRA<0)    tempRA+=2*M_PI;

	oss << _("SA ") << Utility::printAngleDMS(2*M_PI-tempRA)
	    << _(" GHA ") << Utility::printAngleDMS(GHA*M_PI/180.0)
	    << _(" LHA ") << Utility::printAngleDMS(HA*M_PI/180.0);
	// calculate alt az
	Vec3d local_pos = nav->earthEquToLocal(equPos);
	Utility::rectToSphe(&tempRA,&tempDE,local_pos);
	tempRA = 3*M_PI - tempRA;  // N is zero, E is 90 degrees
	if (tempRA > M_PI*2) tempRA -= M_PI*2;
	oss << "@" << _(" Az/Alt/coA: ") << Utility::printAngleDMS(tempRA) << "/" << Utility::printAngleDMS(tempDE) << "/" << Utility::printAngleDMS(((M_PI/180.f)*90.)-tempDE) << " LPA " << Utility::printAngleDMS(PA*M_PI/180.0);
	if (englishName == "Sun") {
		oss << _(" Day length: ") ;
		if (daytime<-1) oss << "00h00m00s";
		else if (daytime>1) oss << "24h00m00s";
		else {
			daytime=2*(M_PI-acos(daytime));
			oss << Utility::printAngleHMS(daytime);
		}
	}
	return oss.str();
}

double Body::getCloseFov(const Navigator* nav) const
{
	return atanf(radius*2.f/getEarthEquPos(nav).length())*180./M_PI * 4;
}

double Body::getSatellitesFov(const Navigator * nav) const
{

	if ( !satellites.empty() && englishName != "Sun") {
		double rad = getBoundingRadius();
		if ( rad > 0 ) return atanf(rad/getEarthEquPos(nav).length()) *180./M_PI * 4;
	}

	return -1.;

}

double Body::getParentSatellitesFov(const Navigator *nav) const
{
    return (is_satellite) ? parent->getSatellitesFov(nav) : -1.0;
}

// Set the orbital elements
void Body::set_rotation_elements(float _period, float _offset, double _epoch, float _obliquity,
                                 float _ascendingNode, float _precessionRate, double _sidereal_period, float _axial_tilt)
{
	re.period = _period;
	re.offset = _offset;
	re.epoch = _epoch;
	re.obliquity = _obliquity;
	re.ascendingNode = _ascendingNode;
	re.precessionRate = _precessionRate;
	re.sidereal_period = _sidereal_period;  // used for drawing orbit lines
	re.axialTilt = _axial_tilt; // Used for drawing tropic lines
}

void Body::setSphereScale(float s, bool initial_scale )
{
	radius = initialRadius * s;
	if (initial_scale)
		initialScale = s;
}

// Return the Body position in rectangular earth equatorial coordinate
Vec3d Body::getEarthEquPos(const Navigator * nav) const
{
	Vec3d v = get_heliocentric_ecliptic_pos();
	return nav->helioToEarthPosEqu(v);		// this is earth equatorial but centered
}

Vec3d Body::getObsJ2000Pos(const Navigator *nav) const
{
	return mat_vsop87_to_j2000.multiplyWithoutTranslation(
	           get_heliocentric_ecliptic_pos()
	           - nav->getObserverHelioPos());
}


// Compute the position in the parent Body coordinate system
// Actually call the provided function to compute the ecliptical position
// void Body::computePositionWithoutOrbits(const double date)
// {
// 	if (fabs(lastJD-date)>deltaJD) {
// 		if(orbit) orbit->positionAtTimevInVSOP87Coordinates(date, ecliptic_pos);
// 		lastJD = date;
// 	}
// }


void Body::compute_position(const double date)
{
	OsculatingFunctionType *oscFunc = orbit->getOsculatingFunction();

	if(orbitPlot != nullptr && orbitPlot->getOrbitFader().getInterstate()) {
		orbitPlot->computeOrbit(date);
	}

	double delta = date-lastJD;
	delta = fabs(delta);

	if(delta >= deltaJD ) {
		if(oscFunc)
			(*oscFunc)(date,date,ecliptic_pos);
		else
			orbit->positionAtTimevInVSOP87Coordinates(date,date,ecliptic_pos);
		lastJD = date;
	}
}

// Compute the transformation matrix from the local Body coordinate to the parent Body coordinate
void Body::compute_trans_matrix(double jd)
{
	axis_rotation = getSiderealTime(jd);

	// Special case - heliocentric coordinates are on ecliptic, not solar equator...
	if (parent) {
		rot_local_to_parent = Mat4d::zrotation(re.ascendingNode -re.precessionRate*(jd-re.epoch))
		                      * Mat4d::xrotation(re.obliquity);
		rot_local_to_parent_unprecessed = Mat4d::zrotation(re.ascendingNode)
		                                  * Mat4d::xrotation(re.obliquity);

	}

	mat_local_to_parent = Mat4d::translation(ecliptic_pos)
	                      * rot_local_to_parent;

}

Mat4d Body::getRotEquatorialToVsop87(void) const
{
	Mat4d rval = rot_local_to_parent;

    for (Body *p = parent.get(); p; p = p->parent.get()) {
        rval = p->rot_local_to_parent * rval;
    }
	return rval;
}

void Body::setRotEquatorialToVsop87(const Mat4d &m)
{
	Mat4d a = Mat4d::identity();

    for (Body *p = parent.get(); p; p = p->parent.get()) {
        a = p->rot_local_to_parent * a;
    }
	rot_local_to_parent = a.transpose() * m;
}

std::string Body::getTypePlanet(const BODY_TYPE str)  const
{
	switch (str) {
		case SUN:
			return "sun";
		case PLANET:
			return "planet";
		case MOON:
			return "moon";
		case DWARF:
			return "dwarf planet";
		case ASTEROID:
			return "asteroid";
		case KBO:
			return "kuiper belt object";
		case COMET:
			return "comet";
		case ARTIFICIAL:
			return "artificial";
		case OBSERVER:
			return "observer";
		case CENTER:
			return "center";
        case STAR:
            return "star";
		default :
			return "unknown";
	}
	return "";
}

// Compute the z rotation to use from equatorial to geographic coordinates
double Body::getSiderealTime(double jd) const
{
	if (englishName=="Earth")
        return get_apparent_sidereal_time(jd);

	double t = jd - re.epoch;
	double rotations = t / (double) re.period;
	double wholeRotations = floor(rotations);
	double remainder = rotations - wholeRotations;

	return remainder * 360. + re.offset;
}

// Get the Body position in the parent Body ecliptic coordinate
Vec3d Body::get_ecliptic_pos() const
{
	return ecliptic_pos;
}

// Return the heliocentric ecliptical position used only for earth shadow, lunar eclipse -- DIGITALIS: this statement is not true!
Vec3d Body::get_heliocentric_ecliptic_pos() const
{
	Vec3d pos = ecliptic_pos;

	for (Body *p = parent.get(); p; p = p->parent.get())
		pos += p->ecliptic_pos;
	return pos;
}


void Body::set_heliocentric_ecliptic_pos(const Vec3d &pos)
{
	ecliptic_pos = pos;
    for (Body *p = parent.get(); p; p = p->parent.get())
		ecliptic_pos -= p->ecliptic_pos;
}


// Compute the distance to the given position in heliocentric coordinate (in AU)
// double Body::compute_distance(const Vec3d& obs_helio_pos)
// {
// 	distance = (obs_helio_pos-get_heliocentric_ecliptic_pos()).length();
// 	return distance;
// }

// Get the phase angle for an observer at pos obs_pos in the heliocentric coordinate (dist in AU)
double Body::get_phase(Vec3d obs_pos) const
{
	const double sq = obs_pos.lengthSquared();
	const Vec3d heliopos = get_heliocentric_ecliptic_pos();
	const double Rq = heliopos.lengthSquared();
	const double pq = (obs_pos - heliopos).lengthSquared();
	const double cos_chi = (pq + Rq - sq)/(2.0*sqrt(pq*Rq));
	return (1.0 - acos(cos_chi)/M_PI) * cos_chi + sqrt(1.0 - cos_chi*cos_chi) / M_PI;
}

float Body::computeMagnitude(Vec3d obs_pos) const
{
	float rval = 0;
	const double sq = obs_pos.lengthSquared();
	const Vec3d heliopos = get_heliocentric_ecliptic_pos();
	const double Rq = heliopos.lengthSquared();
	const double pq = (obs_pos - heliopos).lengthSquared();
	const double cos_chi = (pq + Rq - sq)/(2.0*sqrt(pq*Rq));
	const double phase = (1.0 - acos(cos_chi)/M_PI) * cos_chi
	                     + sqrt(1.0 - cos_chi*cos_chi) / M_PI;
	const float F = 2.0 * albedo * radius * radius * phase / (3.0*pq*Rq);
	rval = -26.73f - 2.5f * log10f(F);
	return rval;
}

float Body::computeMagnitude(const Navigator * nav) const
{
	return computeMagnitude(nav->getObserverHelioPos());
}


// Return the radius of a circle containing the object on screen
float Body::getOnScreenSize(const Projector* prj, const Navigator * nav, bool orb_only)
{
	return angularSize * (180./M_PI) /prj->getFov()*prj->getViewportHeight();
}

// Return the angle (degrees) of the Body orb
float Body::get_angular_size(const Projector* prj, const Navigator * nav)
{
    return angularSize * (180 / M_PI);
}


// Return the radius of a circle containing the object and its satellites on screen
float Body::get_on_screen_bounding_size(const Projector* prj, const Navigator * nav)
{
	double rad = getBoundingRadius();
    double temp = getEarthEquPos(nav).lengthSquared()-rad*rad;
    if (temp < 0.) temp = 0.000001; // In case we're closer than the bounding radius
	return atanf(rad/sqrt(temp))*2.f*180./M_PI/prj->getFov()*prj->getViewportHeight();
}

// Start/stop accumulating new trail data (clear old data)
void Body::startTrail(bool b)
{
	if(trail != nullptr)
		trail->startTrail(b);
}

void Body::translateName(Translator& trans)
{
	nameI18 = trans.translateUTF8(englishName);
}

// adapt to vulkan ???
// void Body::computeComa(const float diameter)
// {
// 	StelPainter::computeFanDisk(0.5f*diameter, 3, 3, comaVertexArr, comaTexCoordArr);
// }

// adapt to vulkan ???
//! create parabola shell to represent a tail. Designed for slices=16, stacks=16, but should work with other sizes as well.
//! (Maybe slices must be an even number.)
// Parabola equation: z=x²/2p.
// xOffset for the dust tail, this may introduce a bend. Units are x per sqrt(z).
// void Body::computeParabola(const float parameter, const float radius, const float zshift,
// 						  QVector<Vec3d>& vertexArr, QVector<Vec2f>& texCoordArr,
// 						  QVector<unsigned short> &indices, const float xOffset)
// {
// 	// keep the array and replace contents. However, using replace() is only slightly faster.
// 	if (vertexArr.length() < static_cast<int>(((COMET_TAIL_SLICES*COMET_TAIL_STACKS+1))))
// 		vertexArr.resize((COMET_TAIL_SLICES*COMET_TAIL_STACKS+1));
// 	if (createTailIndices) indices.clear();
// 	if (createTailTextureCoords) texCoordArr.clear();
// 	// The parabola has triangular faces with vertices on two circles that are rotated against each other.
// 	float xa[2*COMET_TAIL_SLICES];
// 	float ya[2*COMET_TAIL_SLICES];
// 	float x, y, z;

// 	// fill xa, ya with sin/cosines. TBD: make more efficient with index mirroring etc.
// 	float da=M_PIf/COMET_TAIL_SLICES; // full circle/2slices
// 	for (unsigned short int i=0; i<2*COMET_TAIL_SLICES; ++i){
// 		xa[i]=-sin(i*da);
// 		ya[i]=cos(i*da);
// 	}

// 	vertexArr.replace(0, Vec3d(0.0, 0.0, static_cast<double>(zshift)));
// 	int vertexArrIndex=1;
// 	if (createTailTextureCoords) texCoordArr << Vec2f(0.5f, 0.5f);
// 	// define the indices lying on circles, starting at 1: odd rings have 1/slices+1/2slices, even-numbered rings straight 1/slices
// 	// inner ring#1
// 	for (unsigned short int ring=1; ring<=COMET_TAIL_STACKS; ++ring){
// 		z=ring*radius/COMET_TAIL_STACKS; z=z*z/(2*parameter) + zshift;
// 		const float xShift= xOffset*z*z;
// 		for (unsigned short int i=ring & 1; i<2*COMET_TAIL_SLICES; i+=2) { // i.e., ring1 has shifted vertices, ring2 has even ones.
// 			x=xa[i]*radius*ring/COMET_TAIL_STACKS;
// 			y=ya[i]*radius*ring/COMET_TAIL_STACKS;
// 			vertexArr.replace(vertexArrIndex++, Vec3d(static_cast<double>(x+xShift), static_cast<double>(y), static_cast<double>(z)));
// 			if (createTailTextureCoords) texCoordArr << Vec2f(0.5f+ 0.5f*x/radius, 0.5f+0.5f*y/radius);
// 		}
// 	}
// 	// now link the faces with indices.
// 	if (createTailIndices)
// 	{
// 		for (unsigned short i=1; i<COMET_TAIL_SLICES; ++i) indices << 0 << i << i+1;
// 		indices << 0 << COMET_TAIL_SLICES << 1; // close inner fan.
// 		// The other slices are a repeating pattern of 2 possibilities. Index @ring always is on the inner ring (slices-agon)
// 		for (unsigned short ring=1; ring<COMET_TAIL_STACKS; ring+=2) { // odd rings
// 			const unsigned short int first=(ring-1)*COMET_TAIL_SLICES+1;
// 			for (unsigned short int i=0; i<COMET_TAIL_SLICES-1; ++i){
// 				indices << first+i << first+COMET_TAIL_SLICES+i << first+COMET_TAIL_SLICES+1+i;
// 				indices << first+i << first+COMET_TAIL_SLICES+1+i << first+1+i;
// 			}
// 			// closing slice: mesh with other indices...
// 			indices << ring*COMET_TAIL_SLICES << (ring+1)*COMET_TAIL_SLICES << ring*COMET_TAIL_SLICES+1;
// 			indices << ring*COMET_TAIL_SLICES << ring*COMET_TAIL_SLICES+1 << first;
// 		}

// 		for (unsigned short int ring=2; ring<COMET_TAIL_STACKS; ring+=2) { // even rings: different sequence.
// 			const unsigned short int first=(ring-1)*COMET_TAIL_SLICES+1;
// 			for (unsigned short int i=0; i<COMET_TAIL_SLICES-1; ++i){
// 				indices << first+i << first+COMET_TAIL_SLICES+i << first+1+i;
// 				indices << first+1+i << first+COMET_TAIL_SLICES+i << first+COMET_TAIL_SLICES+1+i;
// 			}
// 			// closing slice: mesh with other indices...
// 			indices << ring*COMET_TAIL_SLICES << (ring+1)*COMET_TAIL_SLICES << first;
// 			indices << first << (ring+1)*COMET_TAIL_SLICES << ring*COMET_TAIL_SLICES+1;
// 		}
// 	}
// 	createTailIndices=false;
// 	createTailTextureCoords=false;
// }

void Body::update(int delta_time, const Navigator* nav, const TimeMgr* timeMgr)
{
	radius.update(delta_time);
    if (radius.isScaling())
        updateBoundingRadii();

    visibilityFader.update(delta_time);

	if(orbitPlot!= nullptr)
		orbitPlot->updateShader(delta_time);

	hints->updateShader(delta_time);

	if(trail!= nullptr) {
		trail->updateFader(delta_time);
		trail->updateTrail(nav, timeMgr);
	}

	// // The rest deals with updating tail geometries and brightness
	// double dateJDE = timeMgr->getJDay();

	// // adapt to spacecrafter
	// //if (!static_cast<KeplerOrbit*>(orbitPtr)->objectDateValid(dateJDE)) return; // don't do anything if out of useful date range. This allows having hundreds of comet elements.

	// if (fabs(lastJDEtail-dateJDE)>deltaJDEtail) {
	// 	lastJDEtail=dateJDE;

	// 	if (orbit->getUpdateTails()){
	// 		// Compute lengths and orientations from orbit object, but only if required.
	// 		tailFactors=getComaDiameterAndTailLengthAU();

	// 		// Note that we use a diameter larger than what the formula returns. A scale factor of 1.2 is ad-hoc/empirical (GZ), but may look better.
	// 		computeComa(1.0f*tailFactors[0]); // TBD: APPARENTLY NO SCALING? REMOVE 1.0 and note above.

	// 		tailActive = (tailFactors[1] > tailFactors[0]); // Inhibit tails drawing if too short. Would be nice to include geometric projection angle, but this is too costly.

	// 		if (tailActive)
	// 		{
	// 			float gasTailEndRadius=qMax(tailFactors[0], 0.025f*tailFactors[1]) ; // This avoids too slim gas tails for bright comets like Hale-Bopp.
	// 			float gasparameter=gasTailEndRadius*gasTailEndRadius/(2.0f*tailFactors[1]); // parabola formula: z=r²/2p, so p=r²/2z
	// 			// The dust tail is thicker and usually shorter. The factors can be configured in the elements.
	// 			float dustparameter=gasTailEndRadius*gasTailEndRadius*dustTailWidthFactor*dustTailWidthFactor/(2.0f*dustTailLengthFactor*tailFactors[1]);

	// 			// Find valid parameters to create paraboloid vertex arrays: dustTail, gasTail.
	// 			computeParabola(gasparameter, gasTailEndRadius, -0.5f*gasparameter, gastailVertexArr,  tailTexCoordArr, tailIndices);
	// 			// Now we make a skewed parabola. Skew factor (xOffset, last arg) is rather ad-hoc/empirical. TBD later: Find physically correct solution.
	// 			computeParabola(dustparameter, dustTailWidthFactor*gasTailEndRadius, -0.5f*dustparameter, dusttailVertexArr, tailTexCoordArr, tailIndices, 25.0f*static_cast<float>(static_cast<KeplerOrbit*>(orbitPtr)->getVelocity().norm()));
	// 			//dusttailColorArr.fill(Vec3f(0.3,0.3,0.3), dusttailVertexArr.length());


	// 			// 2014-08 for 0.13.1 Moved from drawTail() to save lots of computation per frame (There *are* folks downloading all 730 MPC current comet elements...)
	// 			// Find rotation matrix from 0/0/1 to eclipticPosition: crossproduct for axis (normal vector), dotproduct for angle.
	// 			Vec3d eclposNrm=eclipticPos+aberrationPush; eclposNrm.normalize();
	// 			gasTailRot=Mat4d::rotation(Vec3d(0.0, 0.0, 1.0)^(eclposNrm), std::acos(Vec3d(0.0, 0.0, 1.0).dot(eclposNrm)) );

	// 			// adapt to spacecrafter
	// 			Vec3d velocity=static_cast<KeplerOrbit*>(orbitPtr)->getVelocity(); // [AU/d]
	// 			// This was a try to rotate a straight parabola somewhat away from the antisolar direction.
	// 			//Mat4d dustTailRot=Mat4d::rotation(eclposNrm^(-velocity), 0.15f*std::acos(eclposNrm.dot(-velocity))); // GZ: This scale factor of 0.15 is empirical from photos of Halley and Hale-Bopp.
	// 			// The curved tail is curved towards positive X. We first rotate around the Z axis into a direction opposite of the motion vector, then again the antisolar rotation applies.
	// 			// In addition, we let the dust tail already start with a light tilt.
	// 			dustTailRot=gasTailRot * Mat4d::zrotation(atan2(velocity[1], velocity[0]) + M_PI) * Mat4d::yrotation(5.0*velocity.norm());

	// 			// adapt to vulkan ???
	// 			// Rotate vertex arrays:
	// 			Vec3d* gasVertices= static_cast<Vec3d*>(gastailVertexArr.data());
	// 			Vec3d* dustVertices=static_cast<Vec3d*>(dusttailVertexArr.data());
	// 			for (unsigned short int i=0; i<COMET_TAIL_SLICES*COMET_TAIL_STACKS+1; ++i)
	// 			{
	// 				gasVertices[i].transfo4d(gasTailRot);
	// 				dustVertices[i].transfo4d(dustTailRot);
	// 			}
	// 		}
	// 		orbit->setUpdateTails(false); // don't update until position has been recalculated elsewhere
	// 	}
	// }
	// gastailColorArr.fill(gasColor  *intensityFovScale, gastailVertexArr.length());
	// dusttailColorArr.fill(dustColor*intensityFovScale, dusttailVertexArr.length());
}

// Update bounding radii from child up to parent(s)
void Body::updateBoundingRadii()
{
	calculateBoundingRadius();
	for (Body *p = parent.get(); p; p = p->parent.get()) {
		p->calculateBoundingRadius();
	}
}

// Calculate a bounding radius in AU
// For a Body with satellites, this bounds the most distant satellite orbit
// for planets with no elliptical satellites, this is the ring (if one)
// or Body radius
// Caches result until next call, can retrieve with getBoundingRadius
double Body::calculateBoundingRadius()
{
	double d = radius;

    switch (myShader) {
        case SHADER_MOON_NORMAL_TES:
        case SHADER_NORMAL_TES:
        case SHADER_NIGHT_TES:
            d *= 1 + 0.01 * bodyTesselation->getPlanetAltimetryFactor();
            break;
        default:;
    }
    boundingRadius = d;

    // for (auto it : satellites) {
    //     if (it->orbit_bounding_radius > 0) {
    //         double tmp = it->boundingRadius + it->orbit_bounding_radius;
    //         if (d < tmp)
    //             d = tmp;
    //     }
    // }
    // boundingRadiusWithOrbit = d;
	return boundingRadius;
}

void Body::computeDraw(const Projector* prj, const Navigator* nav)
{
	eye_sun = nav->getHelioToEyeMat().getTranslation();

	mat = mat_local_to_parent;
	parent_mat = Mat4d::identity();

	// \todo account for moon orbit precession (independent of parent)
	// also does not allow for multiple levels of precession
	Body *p = parent.get();

	bool myParent = true;
	if (p) {   //this loop worked to enable moons when while but now with if ?

		// Some orbits are already precessed, namely elp82
		if(myParent && !useParentPrecession(lastJD)) {
			mat = Mat4d::translation(p->get_ecliptic_pos())
			      * mat
			      * p->get_rot_local_to_parent_unprecessed();
		}
		else {
			mat = Mat4d::translation(p->get_ecliptic_pos())
			      * mat
			      * p->get_rot_local_to_parent();
		}

		parent_mat = Mat4d::translation(p->get_ecliptic_pos())
		             * parent_mat;

		p = p->getParent();

		myParent = false;
	}

	model = mat;
	// view = nav->getHelioToEyeMat().convert();
	// vp = prj->getMatProjection().convert() * view;

	mat = nav->getHelioToEyeMat() * mat;

	// proj = prj->getMatProjection().convert();

	parent_mat = nav->getHelioToEyeMat() * parent_mat;

    eye_planet = mat.getTranslation();

	lightDirection = eye_sun - eye_planet;
    sun_half_angle = atan(696000.0/AU/lightDirection.length());  // hard coded Sun radius!
    for (p = parent.get(); p; p = p->getParent()) {
        if (p->getBodyType() == SUN) {
            eye_sun = (nav->getHelioToEyeMat() * p->mat_local_to_parent).getTranslation();
            sun_half_angle = atan(p->radius/AU/lightDirection.length());
            break;
        }
    }

	lightDirection.normalize();

	// Compute the 2D position and check if in the screen
	screen_sz = getOnScreenSize(prj, nav);

	// Do not draw anything else if was not visible
	// Draw the name, and the circle if it's not too close from the body it's turning around
	// this prevents name overlaping (ie for jupiter satellites)
	ang_dist = 300.f*atan(get_ecliptic_pos().length()/getEarthEquPos(nav).length())/prj->getFov();

	//Compute the angle of the axis
	axis->computeAxisAngle(prj, mat);

    // Compute the distance to the observer
    distance = eye_planet.length();

    const float halfFov = prj->getFov() * (M_PI / 360);
    if (distance > radius) {
        angularSize = atanf(radius / sqrt(distance*distance - radius*radius));
        const float tmp = halfFov + angularSize;
        angularSize *= 2;
        isVisible = (tmp > M_PI) ? true : (-eye_planet[2] >= cos(tmp) * distance);
    } else {
        angularSize = M_PI;
        isVisible = true;
    }
    isRelevant &= isVisible;

    const double rq = sqrt(eye_planet[0] * eye_planet[0] + eye_planet[1] * eye_planet[1]);
    double f;
        if (rq > distance * 1e-5) {
            f = asin(rq/distance);
            if (eye_planet[2] > 0)
                f = M_PI - f;
            f /= rq * halfFov;
        } else
            f = 1 / (distance * halfFov);
    screenPos = VulkanMgr::instance->rectToScreenf({eye_planet[0] * f, eye_planet[1] * f});
}

double Body::getAxisAngle() const {
	return axis->getAngle();
}

bool Body::drawGL(Projector* prj, const Navigator* nav, const Observer* observatory, const ToneReproductor* eye, bool depthTest, bool drawHomePlanet, bool needClearDepthBuffer)
{
	bool drawn = false;
    Context &context = *Context::instance;
    FrameMgr &frame = *context.frame[context.frameIdx];

    isRelevant = false;
	if(skipDrawingThisBody(observatory, drawHomePlanet)) {
		if(hasRings()) {
            if (cmds[context.frameIdx] == -1) {
                cmds[context.frameIdx] = frame.create(1);
                frame.setName(cmds[context.frameIdx], englishName + " " +  std::to_string(context.frameIdx));
            }
            VkCommandBuffer cmd = frame.begin(cmds[context.frameIdx], PASS_MULTISAMPLE_DEPTH);
			drawRings(cmd, prj,observatory,mat,1000.0,lightDirection,eye_planet,initialRadius);
            frame.compile();
            frame.toExecute(cmds[context.frameIdx], PASS_MULTISAMPLE_DEPTH);
		}
		return drawn;
	}

	handleVisibilityFader(observatory, prj, nav);

    drawHints(nav, prj);

    if (canSkip(nav, prj)) {
        drawHalo(nav, prj, eye);
        return drawn;
    }

    if (cmds[context.frameIdx] == -1) {
        cmds[context.frameIdx] = frame.create(1);
        frame.setName(cmds[context.frameIdx], englishName + " " +  std::to_string(context.frameIdx));
    }
    VkCommandBuffer cmd = frame.begin(cmds[context.frameIdx], PASS_MULTISAMPLE_DEPTH);

	if(isVisibleOnScreen()) {
        isRelevant = true; // It is the only place where we know this...
        if (needClearDepthBuffer || (!depthTest && hasRings())) {
            VkClearAttachment clearAttachment {VK_IMAGE_ASPECT_DEPTH_BIT, 0, {.depthStencil={1.f,0}}};
            VkClearRect clearRect {VulkanMgr::instance->getScreenRect(), 0, 1};
            vkCmdClearAttachments(cmd, 1, &clearAttachment, 1, &clearRect);
        }
        // drawOrbit(cmd, observatory,nav, prj);
    	drawTrail(cmd, nav, prj);
        if (screen_sz > 5) {
            context.helper->nextDraw(PASS_MULTISAMPLE_DEPTH);
            Halo::nextDraw(cmd);
        }
		if(hasRings()) {
            // Depth test is forced for ringed body
            drawAxis(cmd, prj,mat);
			drawBody(cmd, prj, nav, mat, screen_sz, true);
			drawRings(cmd, prj,observatory,mat,screen_sz,lightDirection,eye_planet,initialRadius);
		} else {
            // depth test if drawAxis (drawAxis if depthTest and Axis::actualdrawaxis)
            // if(!depthTest)
            //     cLog::get()->write("Failed to disable depth test", LOG_TYPE::L_WARNING);
            if (depthTest)
                drawAxis(cmd, prj,mat);
            drawBody(cmd, prj, nav, mat, screen_sz, depthTest);
		}
		drawn = true;
	} else {
        if (needClearDepthBuffer) {
            VkClearAttachment clearAttachment {VK_IMAGE_ASPECT_DEPTH_BIT, 0, {.depthStencil={1.f,0}}};
            VkClearRect clearRect {VulkanMgr::instance->getScreenRect(), 0, 1};
            vkCmdClearAttachments(cmd, 1, &clearAttachment, 1, &clearRect);
            drawn = true;
        }
        // drawOrbit(cmd, observatory,nav, prj);
        drawTrail(cmd, nav, prj);
    }
    frame.compile(cmds[context.frameIdx]);

	drawHalo(nav, prj, eye);

    frame.toExecute(cmds[context.frameIdx], PASS_MULTISAMPLE_DEPTH);

	// // to adapt...
	// // but tails should also be drawn if comet core is off-screen...
	// if (tailActive && tailBright)
	// {
	// 	drawTail(core,transfo,true);  // gas tail
	// 	drawTail(core,transfo,false); // dust tail
	// }

	return drawn;
}

// // adapt to vulkan ???
// void Body::drawTail(StelCore* core, StelProjector::ModelViewTranformP transfo, bool gas)
// {
// 	StelPainter sPainter(core->getProjection(transfo));
// 	sPainter.setBlending(true, GL_ONE, GL_ONE);

// 	tailTexture->bind();

// 	if (gas) {
// 		StelVertexArray vaGas(static_cast<const QVector<Vec3d> >(gastailVertexArr), StelVertexArray::Triangles,
// 				      static_cast<const QVector<Vec2f> >(tailTexCoordArr), tailIndices, static_cast<const QVector<Vec3f> >(gastailColorArr));
// 		sPainter.drawStelVertexArray(vaGas, true);

// 	} else {
// 		StelVertexArray vaDust(static_cast<const QVector<Vec3d> >(dusttailVertexArr), StelVertexArray::Triangles,
// 				      static_cast<const QVector<Vec2f> >(tailTexCoordArr), tailIndices, static_cast<const QVector<Vec3f> >(dusttailColorArr));
// 		sPainter.drawStelVertexArray(vaDust, true);
// 	}
// 	sPainter.setBlending(false);
// }

void Body::drawOrbit(VkCommandBuffer cmdBodyDepth, VkCommandBuffer cmdOrbit, const Observer* observatory, const Navigator* nav, const Projector* prj)
{
    if (isVisibleOnScreen()) {
        float _radius = radius;
        switch (myShader) {
            case SHADER_MOON_NORMAL_TES:
            case SHADER_NORMAL_TES:
            case SHADER_NIGHT_TES:
                _radius *= 1 + 0.01 * bodyTesselation->getPlanetAltimetryFactor();
                break;
            default:;
        }
        depthTraceInfo pdata {mat.convert(), prj->getClippingFov(), _radius, (float) one_minus_oblateness};
        BodyShader::getShaderDepthTrace()->layout->pushConstant(cmdBodyDepth, 0, &pdata);
        currentObj->draw(cmdBodyDepth, 1);
    }
    if (orbitPlot)
        orbitPlot->drawOrbit(cmdOrbit, nav, prj, parent_mat);
}

bool Body::skipDrawingThisBody(const Observer *observatory, bool drawHomePlanet)
{
	return !drawHomePlanet && observatory->isOnBody(this);
}

void Body::drawTrail(VkCommandBuffer cmd, const Navigator* nav, const Projector* prj)
{
	if(trail != nullptr)
		trail->drawTrail(cmd, nav, prj);
}

void Body::drawHints(const Navigator* nav, const Projector* prj)
{
	if (ang_dist>0.25)
		hints->drawHints(nav, prj);
}

void Body::drawAxis(VkCommandBuffer cmd, const Projector* prj, const Mat4d& mat)
{
	axis->drawAxis(cmd, prj, mat);
}

void Body::drawHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye)
{
	if (isVisible && flags.flag_halo && this->getOnScreenSize(prj, nav) < 10) {
		//~ cout << "drawing halo from Body " << this->englishName << " class size " << this->getOnScreenSize(prj, nav) << endl;
		// Workaround for depth buffer precision and near planets
		//StateGL::disable(GL_DEPTH_TEST);
		halo->drawHalo(nav, prj, eye);
	}
}

void Body::drawAtmExt(VkCommandBuffer cmd, const Projector *prj, const Navigator *nav, const Mat4f &mat, float screen_sz, bool depthTest)
{
    if (hasAtmosphere && screen_sz > 10 && get_angular_size(prj, nav) > 2 // Avoid graphical glitch with too small angular size
     && distance > radius * atmosphereParams->atmosphereRadiusFactor * 1.01) { // Avoid graphical glitch when too close of the atmosphere
        if (!atmExt) // AtmExt creation is fast AND never grouped
            atmExt = std::make_unique<AtmExt>(this, currentObj, atmosphereParams->tableAtmosphere);
        atmExt->draw(cmd, prj, nav, mat, eye_sun, eye_planet, one_minus_oblateness, Vec2i(bodyTesselation->getMinTesLevel(), bodyTesselation->getMaxTesLevel()), radius, radius * atmosphereParams->atmosphereRadiusFactor, screen_sz, depthTest);
    }
}

Vec3d Body::getPositionAtDate(double jDate) const
{
    Vec3d pos{};
    Vec3d tmp;

    for (const Body *b = this; b && b->getOrbit(); b = b->parent.get()) {
        b->getOrbit()->positionAtTimevInVSOP87Coordinates(jDate, tmp);
        pos += tmp;
    }
	return pos;
}

bool Body::canSkip(const Navigator* nav, const Projector* prj)
{
    // const bool useOrbit = orbitPlot->doDraw(nav, prj, parent_mat);
    const bool useTrail = (trail) && trail->doDraw(nav, prj);
    return (!useTrail && !isVisibleOnScreen());
}

void Body::preload(int keepFrames)
{
    int tmp = s_texture::setBigTextureLifetime(keepFrames);
    getSet(2048); // Assume the big texture is used for such screen_sz
    s_texture::setBigTextureLifetime(tmp);
}

bool Body::needBucket(const Observer *obs)
{
   return isVisibleOnScreen() || obs->isOnBody(this);
}

UShadowingBody Body::drawShadow(const ShadowParams &params)
{
    auto m = params.lookAt * model;
    Vec3f ret(m.r[12], m.r[13], boundingRadius + params.smoothRadius);
    auto scaling = radius/ret.v[2];
    auto idx = Context::instance->helper->drawShadower(this, params.smoothRadius/ret.v[2]);
    (m * Mat4d::scaling(Vec3d(scaling, scaling, scaling * one_minus_oblateness))).setMat3(Context::instance->shadowData[idx].shadowMat);
    return {ret, idx};
}

void Body::drawShadow(VkCommandBuffer drawCmd, int idx)
{
    BodyShader::getShaderShadowShape()->pipeline->bind(drawCmd);
    BodyShader::getShaderShadowShape()->layout->bindSet(drawCmd, *Context::instance->shadowData[idx].traceSet);
    currentObj->bind(drawCmd);
    currentObj->draw(drawCmd, Context::instance->shadowRes);
}
