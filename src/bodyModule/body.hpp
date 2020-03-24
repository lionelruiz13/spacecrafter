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

#ifndef _BODY_HPP_
#define _BODY_HPP_

#pragma once

#include "tools/object_base.hpp"
#include "tools/utility.hpp"
#include "tools/vecmath.hpp"
#include "coreModule/callbacks.hpp"
#include "tools/fader.hpp"
#include "bodyModule/orbit.hpp"
#include <list>
#include <string>
#include "tools/shader.hpp"
#include "tools/stateGL.hpp"
#include "ojmModule/objl.hpp"
#include "bodyModule/orbit_plot.hpp"
//#include "bodyModule/atmosphere_ext.hpp"
#include "tools/scalable.hpp"
#include "bodyModule/bodyShader.hpp"
#include "rotation_elements.hpp"
#include "tools/scalable.hpp"



class Trail;
class Hints;
class Axis;
class Orbit2D;
class Orbit3D;
class Halo;
class s_font;
class Translator;
class ToneReproductor;
class Navigator;
class Projector;
class Observatory;
class Observer;

class BodyColor;

struct BodyTexture {
	std::string tex_map;
	std::string tex_map_alternative;
	std::string tex_norm;
  	std::string tex_night;
	std::string tex_specular;
	std::string tex_cloud;
	std::string tex_cloud_normal;
	std::string tex_heightmap;
	std::string tex_skin;
};

struct BodyTesselation {
	Scalable<int> min_tes_level;
	Scalable<int> max_tes_level;
	Scalable<int> planet_altimetrie_factor;
	Scalable<int> moon_altimetrie_factor;
	Scalable<int> earth_altimetrie_factor;
	int min_tes_level_ini;
	int max_tes_level_ini;
	int planet_altimetrie_factor_ini;
	int moon_altimetrie_factor_ini;
	int earth_altimetrie_factor_ini;
};


typedef struct body_flags {
	bool flag_trail, flag_hints, flag_axis, flag_orbit, flag_halo;
} body_flags;

enum BODY_TYPE {SUN = 0, PLANET = 1, MOON = 2, DWARF = 3, ASTEROID = 4, KBO = 5,  COMET = 6, ARTIFICIAL = 7, OBSERVER = 8, UNKNOWN = 10};

typedef struct AtmosphereParams {
	bool hasAtmosphere = false;
	float limInf = 0.f;
	float limSup = 0.f;
	float limLandscape = 0.f;
} AtmosphereParams;


class Body : public ObjectBase {

	friend class Trail;
	friend class Hints;
	friend class Axis;
	friend class Orbit2D;
	friend class Orbit3D;
	friend class OrbitPlot;
	friend class Halo;
	friend class BodyShader;

public:
	Body(Body *parent,
	     const std::string& englishName,
	     BODY_TYPE _typePlanet,
	     bool _flagHalo,
	     double radius,
	     double oblateness,
	     BodyColor* _myColor,
	     float _sol_local_day,
	     float _albedo,
	     Orbit *orbit,
	     bool close_orbit,
	     ObjL* _currentObj,
	     double orbit_bounding_radius,
	     const BodyTexture* _bodyTexture);
	virtual ~Body();

	double getRadius(void) const {
		return radius;
	}

	std::string getSkyLabel(const Navigator * nav) const;
	std::string getInfoString(const Navigator * nav) const override;
	std::string getTypePlanet(const BODY_TYPE str) const;

	//is still use ?
	void getRaDeValue(const Navigator *nav,double *ra, double *de) const override {
	}
	void getAltAz(const Navigator * nav, double *alt, double *az) const;

	std::string getShortInfoString(const Navigator * nav) const;
	std::string getShortInfoNavString(const Navigator * nav, const TimeMgr * timeMgr, const Observer* observatory) const;
	double getCloseFov(const Navigator * nav) const;
	double getSatellitesFov(const Navigator * nav) const;
	double getParentSatellitesFov(const Navigator * nav) const;
	float getMag(const Navigator * nav) const override {
		return computeMagnitude(nav);
	}

	Orbit *getOrbit() {
		return orbit;
	}

	const Orbit * getOrbit()const {
		return orbit;
	}

	/** Translate Body name using the passed translator */
	void translateName(Translator& trans);

	// Compute the z rotation to use from equatorial to geographic coordinates
	double getSiderealTime(double jd) const;
	Mat4d getRotEquatorialToVsop87(void) const;
	void setRotEquatorialToVsop87(const Mat4d &m);

	const RotationElements &getRotationElements(void) const {
		return re;
	}

	// Compute the position in the parent Body coordinate system
	void computePositionWithoutOrbits(double date);
	void compute_position(double date);

	// Compute the transformation matrix from the local Body coordinate to the parent Body coordinate
	void compute_trans_matrix(double date);

	// Get the phase angle for an observer at pos obs_pos in the heliocentric coordinate (in AU)
	double get_phase(Vec3d obs_pos) const;

	// Get the magnitude for an observer at pos obs_pos in the heliocentric coordinate (in AU)
	virtual float computeMagnitude(const Vec3d obs_pos) const;
	float computeMagnitude(const Navigator * nav) const;

	// calcule tous les éléments nécessaires pour préparer le draw
	virtual void computeDraw(const Projector* prj, const Navigator* nav);

	// Draw the Planet, if hint_ON is != 0 draw a circle and the name as well
	// Return the squared distance in pixels between the current and the  previous position this Body was drawn at.
	virtual bool drawGL(Projector* prj, const Navigator* nav, const Observer* observatory, const ToneReproductor* eye, bool depthTest, bool drawHomePlanet, bool selected);

	// Set the orbital elements
	void set_rotation_elements(float _period, float _offset, double _epoch, float _obliquity, float _ascendingNode, float _precessionRate, double _sidereal_period, float _axial_tilt);

	double getRotAscendingnode(void) const {
		return re.ascendingNode;
	}
	double getRotObliquity(void) const {
		return re.obliquity;
	}
	double getAxialTilt(void) const {
		return re.axialTilt;
	}

	double getSiderealDay(void) const {
		return re.period;
	}

	// Get the Body position in the parent Body ecliptic coordinate
	Vec3d get_ecliptic_pos() const;

	// Return the heliocentric ecliptical position
	Vec3d get_heliocentric_ecliptic_pos() const;
	void set_heliocentric_ecliptic_pos(const Vec3d &pos);

	// Compute the distance to the given position in heliocentric coordinate (in AU)
	double compute_distance(const Vec3d& obs_helio_pos);
	double getDistance(void) const {
		return distance;
	}

	OBJECT_TYPE getType(void) const override {
		return OBJECT_BODY;
	}

	// Return the Body position in rectangular earth equatorial coordinate
	Vec3d getEarthEquPos(const Navigator *nav) const;
	// observer centered J2000 coordinates
	Vec3d getObsJ2000Pos(const Navigator *nav) const;

	std::string getEnglishName(void) const override {
		return englishName;
	}
	std::string getNameI18n(void) const override {
		return nameI18;
	}

	const Body *get_parent(void) const {
		return parent;
	}

	// modifiable
	Body *getParent(void) {
		return parent;
	}

	static void setFont(s_font* f) {
		planet_name_font = f;
	}

	//cela parle de magnitude avec les étoiles
	static void setScale(float s) {
		object_scale = s;
	}
	//cela parle de magnitude avec les étoiles
	static float getScale(void) {
		return object_scale;
	}
	//fonction qui modifie multiplicativement par s le rayon du corps celeste
	// afin de savoir si on doit garder en mémoire cette taille d'affichage, on utilise le bool
	virtual void setSphereScale(float s, bool initial_scale =  false);

	// renvoie le ratio rayon_actuel/rayon_initial
	float getSphereScale(void) {
		return (radius/initialRadius);
	}

	static void setSizeLimit(float s) {
		object_size_limit = s;
	}
	static float getSizeLimit(void) {
		return object_size_limit;
	}

	static void setMinTes(int v, bool ini = false) {
		if (ini) {
			bodyTesselation->min_tes_level.set(v);
			bodyTesselation->min_tes_level_ini = v;
		} else
			bodyTesselation->min_tes_level = v;
	}

	static void setMaxTes(int v, bool ini = false) {
		if (ini) {
			bodyTesselation->max_tes_level_ini = v;
			bodyTesselation->max_tes_level.set(v);
		} else
		bodyTesselation->max_tes_level = v;
	}

	static void setPlanetTes(int v, bool ini = false) {
		if (ini) {
			bodyTesselation->planet_altimetrie_factor_ini = v;
			bodyTesselation->planet_altimetrie_factor.set(v);
		} else
		bodyTesselation->planet_altimetrie_factor = v;
	}

	static void setMoonTes(int v, bool ini = false) {
		if (ini) {
			bodyTesselation->moon_altimetrie_factor_ini = v;
			bodyTesselation->moon_altimetrie_factor.set(v);
		} else
			bodyTesselation->moon_altimetrie_factor = v;
	}

	static void setEarthTes(int v, bool ini = false) {
		if (ini) {
			bodyTesselation->earth_altimetrie_factor_ini = v;
			bodyTesselation->earth_altimetrie_factor.set(v);
		} else
			bodyTesselation->earth_altimetrie_factor = v;
	}

	// fixe une couleur
	void setColor(const std::string& colorName,  const Vec3f& oc);

	// récupère une couleur d'un paramètre
	const Vec3f getColor(const std::string& colorName);

	//! Start/stop accumulating new trail data (clear old data)
	void startTrail(bool b);

	virtual void update(int delta_time,const Navigator* nav, const TimeMgr* timeMgr);

	void setFlagHints(bool b);

	bool getFlagHints(void) const;

	bool getFlagAxis(void) const;

	void setFlagAxis(bool b);

	void setFlagOrbit(bool b);

	bool getFlagOrbit(void)const;

	void setFlagTrail(bool b);

	bool getFlagTrail(void) const;

	void setFlagHalo(bool b);

	bool getFlagHalo(void)const;

	static void setFlagClouds(bool b) {
		Body::flagClouds = b;
	}

	static bool getFlagClouds(void) {
		return Body::flagClouds;
	}

	bool isSatellite() const {
		return is_satellite;
	}

	// remove from parent satellite list
	virtual void removeSatellite(Body *planet);

	// for depth buffering of orbits
	void updateBoundingRadii();
	virtual double calculateBoundingRadius();

	double getBoundingRadius() const {
		return boundingRadius;
	}

	// Return the radius of a circle containing the object on screen
	virtual float getOnScreenSize(const Projector* prj, const Navigator * nav, bool orb_only = false) override;

	// Return the radius of a circle containing the object and satellites on screen
	float get_on_screen_bounding_size(const Projector* prj, const Navigator * nav);


	// Return the angle of the Body orb
	float get_angular_size(const Projector* prj, const Navigator * nav);

	// See if simple Body not needing depth buffer
	bool hasSatellite() {
		return !satellites.empty();
	}

	//return the duration of a day in the Body
	float getSolLocalDay() const {
		return sol_local_day;
	}

	// TODO selectionne le shader approprié au Body
	virtual void selectShader ()= 0;

	void reinitParam() {
		radius= initialRadius * initialScale;
	}

	static void createShader();
	static void deleteShader();

	const Mat4d get_rot_local_to_parent_unprecessed() const {
		return rot_local_to_parent_unprecessed;
	}

	const Mat4d get_rot_local_to_parent() const {
		return rot_local_to_parent;
	}

	static bool setTexEclipseMap(const std::string &texMap) {
		tex_eclipse_map = new s_texture(texMap, TEX_LOAD_TYPE_PNG_SOLID);
		if (tex_eclipse_map != nullptr)
			return true;
		else
			return false;
	}

	static bool setTexDefaultMap(const std::string &texMap) {
		defaultTexMap = new s_texture(texMap, TEX_LOAD_TYPE_PNG_SOLID_REPEAT,1);
		if (defaultTexMap != nullptr)
			return true;
		else
			return false;
	}

	static void createDefaultAtmosphereParams() {
		defaultAtmosphereParams =  new(AtmosphereParams);
		assert(defaultAtmosphereParams != nullptr);
		defaultAtmosphereParams->hasAtmosphere = false;
		defaultAtmosphereParams->limInf = 0.f;
		defaultAtmosphereParams->limSup = 0.f;
		defaultAtmosphereParams->limLandscape = 10000.f;
	}

	static void deleteDefaultatmosphereParams() {
		if (defaultAtmosphereParams)
			delete defaultAtmosphereParams;
		defaultAtmosphereParams = nullptr;
	}

	static void createTesselationParams() {
		bodyTesselation =  new(BodyTesselation);
		assert(bodyTesselation != nullptr);
		bodyTesselation->min_tes_level.set(1);
		bodyTesselation->max_tes_level.set(1);
		bodyTesselation->planet_altimetrie_factor.set(1);
		bodyTesselation->moon_altimetrie_factor.set(1);
		bodyTesselation->earth_altimetrie_factor.set(1);
		bodyTesselation->min_tes_level_ini=1;
		bodyTesselation->max_tes_level_ini=1;
		bodyTesselation->planet_altimetrie_factor_ini=1;
		bodyTesselation->moon_altimetrie_factor_ini=1;
		bodyTesselation->earth_altimetrie_factor_ini=1;
	}

	static void deleteTesselationParams() {
		if (bodyTesselation)
			delete bodyTesselation;
		bodyTesselation = nullptr;
	}

	static void resetTesselationParams() {
		bodyTesselation->min_tes_level = 1;
		bodyTesselation->max_tes_level = 1;
		bodyTesselation->planet_altimetrie_factor = 1;
		bodyTesselation->moon_altimetrie_factor = 1;
		bodyTesselation->earth_altimetrie_factor = 1;		
	}

	static void updateTesselation (int delta_time) {
		bodyTesselation->min_tes_level.update(delta_time);
		bodyTesselation->max_tes_level.update(delta_time);
		bodyTesselation->planet_altimetrie_factor.update(delta_time);
		bodyTesselation->moon_altimetrie_factor.update(delta_time);
		bodyTesselation->earth_altimetrie_factor.update(delta_time);
	}

	void setAtmExt(double radiusFactor, const std::string &gradient);

	static bool setTexHaloMap(const std::string &texMap);

	static void deleteDefaultTexMap();

	std::list<Body *> getSatellites() {
		return satellites;
	}

	Vec3d getPositionAtDate(double jDate) const;

	const Body * getParent()const {
		return parent;
	}

	const AtmosphereParams* getAtmosphereParams() const {
		return this->atmosphereParams;
	}

	void setAtmosphereParams(AtmosphereParams* tmp ) {
		atmosphereParams = tmp;
	}

	void switchMapSkin(bool a);

	bool getSwitchMapSkin() {
		return tex_current!=tex_map;
	}

	void createTexSkin(const std::string &texName);

protected:

	bool useParentPrecession(double jd) {
		return getOrbit()->useParentPrecession(jd);
	}

	virtual bool skipDrawingThisBody(const Observer* observatory, bool drawHomePlanet);

	virtual bool hasRings() {
		return false;
	}

	virtual void drawRings(const Projector* prj,const Mat4d& mat,double screen_sz, Vec3f& _lightDirection, Vec3f& _planetPosition, float planetRadius) {
		return;
	}

	virtual void drawOrbit(const Observer* observatory, const Navigator* nav, const Projector* prj);

	virtual void drawTrail(const Navigator* nav, const Projector* prj);

	virtual void drawHints(const Navigator* nav, const Projector* prj);

	virtual void handleVisibilityFader(const Observer* observatory, const Projector* prj, const Navigator * nav) {
		return;
	}

	virtual void drawAxis(const Projector* prj, const Mat4d& mat);

	void drawAtmExt(const Projector* prj, const Navigator* nav, const Observer* observatory);

	virtual bool isVisibleOnScreen() {
		return screen_sz > 1 && isVisible;
	}

	// Draw the 3D body: pshere or model3d
	virtual void drawBody(const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz);

	virtual void drawHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye);

	std::string englishName; 			// english Body name
	std::string nameI18;					// International translated name
	RotationElements re;			// Rotation param
	Scalable<double> radius;					// actual Body radius in UA
	double initialRadius;			// real Body radius in UA
	double initialScale; 			// return the initial scale between actual radius et real radius
	double one_minus_oblateness;    // (polar radius)/(equatorial radius)
	Vec3d ecliptic_pos; 			// Position in UA in the rectangular ecliptic coordinate system centered on the parent Planet
	Vec3d screenPos;			// Used to store temporarily the 2D position on screen
	BODY_TYPE typePlanet;			//get the type of Body in univers: real planet, moon, dwarf ...

	BodyColor* myColor=nullptr;
	AtmosphereParams* atmosphereParams=nullptr;

	static AtmosphereParams *defaultAtmosphereParams;
	static BodyTesselation *bodyTesselation; 	// all global parameters with shader tesselaiton 
	float sol_local_day;			//time of a sideral day in this planet
	float albedo;					// Body albedo
	Mat4d rot_local_to_parent;
	Mat4d rot_local_to_parent_unprecessed;  // currently used for moons (Moon elliptical orbit required this)
	Mat4d mat_local_to_parent;		// Transfo matrix from local ecliptique to parent ecliptic
	float axis_rotation;			// Rotation angle of the Body on it's axis
	s_texture * tex_map=nullptr;			// Body map texture
	s_texture * tex_skin=nullptr;			// Body skin texture 
	s_texture * tex_norm=nullptr;			// Body normal map
	s_texture * tex_heightmap=nullptr;		// Body height map for Tessellation
	s_texture * tex_current=nullptr;		// current body texture to display

	Vec3f eye_sun;
	Vec3f eye_planet;
	SHADER_USE myShader;  			// the name of the shader used for his display
	shaderProgram *myShaderProg;	// Shader moderne

	// static shaderProgram *shaderBump;
	// static shaderProgram *shaderNight;
	// static shaderProgram *shaderRinged;
	// static shaderProgram *shaderNormal;
	// static shaderProgram *shaderMoonNormal;
	// static shaderProgram *shaderMoonBump;
	// static shaderProgram *shaderArtificial;

	ObjL *currentObj = nullptr;

	double distance;				// Temporary variable used to store the distance to a given point it is used for sorting while drawing

	double lastJD;
	double deltaJD;

	Orbit *orbit=nullptr;            // orbit object for this body
	Vec3f orbit_position;    // position de la planete

	Body *parent;				// Body parent i.e. sun for earth

	std::list<Body *> satellites;		// satellites of the Planet

	static s_font* planet_name_font;// Font for names
	static float object_scale;
	static float object_size_limit;  // in pixels


	static LinearFader flagClouds;

	//LinearFader trail_fader;
	LinearFader visibilityFader;  // allows related lines and labels to fade in/out

	bool close_orbit; // whether to close orbit loop
	bool is_satellite;  // whether has a Body as a parent

	double orbit_bounding_radius; // AU calculated at load time for elliptical orbits at least DIGITALIS
	double boundingRadius;  // Cached AU value for use with depth buffer

	static s_texture *defaultTexMap;  		// Default texture map for bodies if none supplied
	static s_texture *tex_eclipse_map;  	// for moon shadow lookups


	float sun_half_angle; // for moon shadow calcs

	//variables mise en cache
	Mat4d mat;
	Mat4d parent_mat;
	Vec3f lightDirection;
	float screen_sz;
	bool isVisible;
	float ang_dist;

	body_flags flags;

	Trail * trail = nullptr;
	Hints * hints = nullptr;
	Axis * axis = nullptr;
	OrbitPlot * orbitPlot = nullptr;
	Halo * halo = nullptr;
	//AtmosphereExt * atmExt = nullptr;

	Mat4f model;
	Mat4f view;
	Mat4f vp;
	Mat4f viewBeforeLookAt;
	Mat4f proj;
	Mat4f matrix;

};

#endif // _BODY_HPP_



