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


#include "ojmModule/objl.hpp"
#include "bodyModule/trail.hpp"
#include "bodyModule/orbit_plot.hpp"
#include "body_common.hpp"
#include "bodyModule/hints.hpp"
#include "body_tesselation.hpp"
#include "tools/scalable.hpp"
#include "bodyModule/bodyShader.hpp"
#include "rotation_elements.hpp"
#include "tools/scalable.hpp"
#include "atmosphereModule/atmosphere_commun.hpp"

#define JD_MINUTE 0.00069444444444444444444


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
class Observer;
class Set;
class BodyColor;
class AtmExt;


typedef struct body_flags {
	bool flag_trail, flag_hints, flag_axis, flag_orbit, flag_halo;
} body_flags;


enum TURN_AROUND {tANothing = 0, tACenter = 1, tABody = 2};

typedef struct AtmosphereParams {
	bool hasAtmosphere = false;
	ATMOSPHERE_MODEL modelAtmosphere = ATMOSPHERE_MODEL::NONE_MODEL;
	std::string tableAtmosphere;
	float atmosphereRadiusFactor = 1.05; // Coefficient multiplied by the radius to get the atmosphere radius
	float limInf = 0.f;
	float limSup = 0.f;
	float limLandscape = 0.f;
	Vec3f atmColor = Vec3f(0, 0, 0);
	float sunDeviation = 0.f;
	float atmDeviation = 0.f;

} AtmosphereParams;

struct ShadowParams {
	Mat4d lookAt;
	float smoothRadius;
	float mainBodyRadius;
};

// Rendering informations for the center of interest
struct ShadowRenderData {
	Mat4d lookAt;
	float sinSunHalfAngle;
	std::vector<UShadowingBody> shadowingBodies;
};

struct ShadowDescriptor {
    VkCommandBuffer cmd;
    Body *assignment = nullptr; // The body this descriptor is assigned to
};

class Body : public ObjectBase, public std::enable_shared_from_this<Body> {

	friend class Trail;
	friend class Hints;
	friend class Axis;
	friend class Orbit2D;
	friend class Orbit3D;
	friend class OrbitPlot;
	friend class Halo;
	friend class BodyShader;

public:
	Body(std::shared_ptr<Body> parent,
	     const std::string& englishName,
	     BODY_TYPE _typePlanet,
	     bool _flagHalo,
	     double radius,
	     double oblateness,
	     std::unique_ptr<BodyColor> _myColor,
	     float _sol_local_day,
	     float _albedo,
	     std::unique_ptr<Orbit> _orbit,
	     bool close_orbit,
	     ObjL* _currentObj,
	     double orbit_bounding_radius,
	     const BodyTexture &_bodyTexture);
	virtual ~Body();

	double getRadius(void) const {
		return radius;
	}

	std::string getSkyLabel(const Navigator * nav) const;
	std::string getInfoString(const Navigator * nav) const override;
	std::string getTypePlanet(const BODY_TYPE str) const;

	virtual void getAltAz(const Navigator * nav, double *alt, double *az) const override;
	virtual void getRaDeValue(const Navigator *nav,double *ra, double *de) const override;

	std::string getShortInfoString(const Navigator * nav) const;
	std::string getShortInfoNavString(const Navigator * nav, const TimeMgr * timeMgr, const Observer* observatory) const;
	double getCloseFov(const Navigator * nav) const;
	double getSatellitesFov(const Navigator * nav) const;
	double getParentSatellitesFov(const Navigator * nav) const;
	float getMag(const Navigator * nav) const override {
		return computeMagnitude(nav);
	}

	Orbit *getOrbit() {
		return orbit.get();
	}

	const Orbit * getOrbit()const {
		return orbit.get();
	}

	/** Translate Body name using the passed translator */
	void translateName(Translator& trans);

	// Compute the z rotation to use from equatorial to geographic coordinates
	double getSiderealTime(double jd) const;
	Mat4d getRotEquatorialToVsop87(void) const;
	void setRotEquatorialToVsop87(const Mat4d &m);

	// Compute the position in the parent Body coordinate system
	//void computePositionWithoutOrbits(double date);
	void compute_position(double date);

	// Compute the transformation matrix from the local Body coordinate to the parent Body coordinate
	void compute_trans_matrix(double date);

	// Get the phase angle for an observer at pos obs_pos in the heliocentric coordinate (in AU)
	double get_phase(Vec3d obs_pos) const;

	// Get the magnitude for an observer at pos obs_pos in the heliocentric coordinate (in AU)
	virtual float computeMagnitude(const Vec3d obs_pos) const;
	float computeMagnitude(const Navigator * nav) const;

	// calculates all the elements necessary to prepare the draw
	virtual void computeDraw(const Projector* prj, const Navigator* nav);

	// Draw the Planet, if hint_ON is != 0 draw a circle and the name as well
	// Return the squared distance in pixels between the current and the  previous position this Body was drawn at.
	virtual bool drawGL(Projector* prj, const Navigator* nav, const Observer* observatory, const ToneReproductor* eye, bool depthTest, bool drawHomePlanet, bool needClearDepthBuffer);

	// Draw the Planet trace in the depth buffer
	virtual void drawOrbit(VkCommandBuffer cmdBodyDepth, VkCommandBuffer cmdOrbit, const Observer* observatory, const Navigator* nav, const Projector* prj);
	// // draw the tail of the comet
	// void drawTail(StelCore* core, StelProjector::ModelViewTranformP transfo, bool gas);

	// Vec2f getComaDiameterAndTailLengthAU() const;

	// void computeComa(const float diameter);

	// void computeParabola(const float parameter, const float radius, const float zshift,
	// 					  QVector<Vec3d>& vertexArr, QVector<Vec2f>& texCoordArr,
	// 					  QVector<unsigned short> &indices, const float xOffset);

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
	// double compute_distance(const Vec3d& obs_helio_pos);
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

	const std::shared_ptr<Body> get_parent(void) const {
		return parent;
	}

	inline Body *getParent(void) {
		return parent.get();
	}

	static void setFont(s_font* f) {
		planet_name_font = f;
	}

	//it talks about magnitude with the stars
	static void setScale(float s) {
		object_scale = s;
	}
	//it talks about magnitude with the stars
	static float getScale(void) {
		return object_scale;
	}
	//fonction qui modifie multiplicativement par s le rayon du corps celeste
	// in order to know if we should keep this display size in memory, we use the bool
	virtual void setSphereScale(float s, bool initial_scale =  false);

	// returns the ratio radius_actual/initial radius
	float getSphereScale(void) {
		return (radius/initialRadius);
	}

	static void setSizeLimit(float s) {
		object_size_limit = s;
	}
	static float getSizeLimit(void) {
		return object_size_limit;
	}



	// set a color
	void setColor(const std::string& colorName,  const Vec3f& oc);

	// retrieves a color from a parameter
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

	// for depth buffering of orbits
	void updateBoundingRadii();
	virtual double calculateBoundingRadius();

	double getBoundingRadius() const {
		return boundingRadius;
	}

	// double getBoundingRadiusWithOrbit() const {
	// 	return boundingRadiusWithOrbit;
	// }

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

	// TODO selects the appropriate shader for the Body
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
		tex_eclipse_map = std::make_shared<s_texture>(texMap, TEX_LOAD_TYPE_PNG_SOLID);
		if (tex_eclipse_map != nullptr)
			return true;
		else
			return false;
	}

	static bool setTexDefaultMap(const std::string &texMap) {
		defaultTexMap = std::make_shared<s_texture>(texMap, TEX_LOAD_TYPE_PNG_SOLID_REPEAT, true, true);
		if (defaultTexMap != nullptr)
			return true;
		else
			return false;
	}

	static void createDefaultAtmosphereParams() {
		defaultAtmosphereParams =  new(AtmosphereParams);
		assert(defaultAtmosphereParams != nullptr);
		defaultAtmosphereParams->hasAtmosphere = false;
		defaultAtmosphereParams->modelAtmosphere = ATMOSPHERE_MODEL::NONE_MODEL;
		defaultAtmosphereParams->limInf = 0.f;
		defaultAtmosphereParams->limSup = 0.f;
		defaultAtmosphereParams->limLandscape = 10000.f;
	}

	static void deleteDefaultatmosphereParams() {
		if (defaultAtmosphereParams)
			delete defaultAtmosphereParams;
		defaultAtmosphereParams = nullptr;
	}

	static void setTesselation(std::shared_ptr<BodyTesselation> _bodyTesselation) {
		Body::bodyTesselation = _bodyTesselation;
	}

	static bool setTexHaloMap(const std::string &texMap);

	static void deleteDefaultTexMap();

	inline const std::list<Body *> &getSatellites() const {
		return satellites;
	}

	Vec3d getPositionAtDate(double jDate) const;

	const AtmosphereParams* getAtmosphereParams() const {
		return this->atmosphereParams;
	}

	void setAtmosphereParams(AtmosphereParams* tmp ) {
		atmosphereParams = tmp;
		hasAtmosphere = tmp && !tmp->tableAtmosphere.empty();
	}

	void switchMapSkin(bool a);

	bool getSwitchMapSkin() {
		return tex_current!=tex_map;
	}

	void createTexSkin(const std::string &texName);

	double getAxisAngle() const;

	TURN_AROUND getTurnAround() {
		return tAround;
	}

	BODY_TYPE getBodyType() {
		return typePlanet;
	}

	//! Ask this body to preload his textures
	//! @param keepFrames number of frames from which the big texture can be destroyed if it was still unused
	virtual void preload(int keepFrames);

	// Return the screen_size if visible, 0 otherwise
	inline float getImportance() {
		return (isRelevant) ? screen_sz/distance : 0;
	}

	inline bool isVisibleOnScreen() {
		return screen_sz > 2 && isVisible;
	}

	//! Return true if this body need orbit occlusion
	inline bool needOrbitDepth() {
		return isVisible && screen_sz > 10;
	}

	virtual bool needBucket(const Observer *obs);

	// This body is now the center of interest
	virtual void gainInterest() {
		isCenterOfInterest = true;
		changed = true;
	}

	// This body is no longer the center of interest
	virtual void looseInterest() {
		isCenterOfInterest = false;
		changed = true;
	}

	inline bool isCoI() const {
		return isCenterOfInterest;
	}

	virtual void bindShadows(const ShadowRenderData &renderData) {}

	// Draw a shadow, return a Vec2f center and float radius
	UShadowingBody drawShadow(const ShadowParams &params);

	// Record the self-shadow draw
	virtual void drawShadow(VkCommandBuffer drawCmd) {}

	// Record the shadow draw, return a normalized float describing the sun radius in the shadow
	virtual void drawShadow(VkCommandBuffer drawCmd, int idx);

	// Get sun direction in body-local coordinates
	Vec3f getLocalSunDirection() const {
		return mat.inverseUntranslated().multiplyWithoutTranslation(lightDirection);
	}
protected:
	bool useParentPrecession(double jd) {
		return getOrbit()->useParentPrecession(jd);
	}

	//! Return set to bind, may change at every frame
	virtual Set &getSet(float screen_sz) = 0;

	virtual bool skipDrawingThisBody(const Observer* observatory, bool drawHomePlanet);

	// Return true if only the halo need to be drawn
	virtual bool canSkip(const Navigator* nav, const Projector* prj);

	virtual bool hasRings() {
		return false;
	}

	virtual void drawRings(VkCommandBuffer cmd, const Projector* prj, const Observer *obs,const Mat4d& mat,double screen_sz, Vec3f& _lightDirection, Vec3f& _planetPosition, float planetRadius) {
		return;
	}

	virtual void drawTrail(VkCommandBuffer cmd, const Navigator* nav, const Projector* prj);

	virtual void drawHints(const Navigator* nav, const Projector* prj);

	virtual void handleVisibilityFader(const Observer* observatory, const Projector* prj, const Navigator * nav) {
		return;
	}


	virtual void drawAxis(VkCommandBuffer cmd, const Projector* prj, const Mat4d& mat);

	void drawAtmExt(VkCommandBuffer cmd, const Projector *prj, const Navigator *nav, const Mat4f &mat, float screen_sz, bool depthTest);

	// Draw the 3D body: pshere or model3d
	virtual void drawBody(VkCommandBuffer cmd, const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz, bool depthTest) = 0;

	virtual void drawHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye);

	std::string englishName; 			// english Body name
	std::string nameI18;					// International translated name
	RotationElements re;			// Rotation param
	Scalable<double> radius;					// actual Body radius in UA
	double initialRadius;			// real Body radius in UA
	double initialScale; 			// return the initial scale between actual radius et real radius
	double one_minus_oblateness;    // (polar radius)/(equatorial radius)
	Vec3d ecliptic_pos; 			// Position in UA in the rectangular ecliptic coordinate system centered on the parent Planet
	std::pair<float, float> screenPos;			// Used to store temporarily the 2D position on screen
	BODY_TYPE typePlanet;			//get the type of Body in univers: real planet, moon, dwarf ...

	std::shared_ptr<BodyColor> myColor=nullptr;
	AtmosphereParams* atmosphereParams=nullptr;

	static AtmosphereParams *defaultAtmosphereParams;
	static std::shared_ptr<BodyTesselation> bodyTesselation; 	// all global parameters with shader tesselaiton
	float sol_local_day;			//time of a sideral day in this planet
	float albedo;					// Body albedo
	Mat4d rot_local_to_parent;
	Mat4d rot_local_to_parent_unprecessed;  // currently used for moons (Moon elliptical orbit required this)
	Mat4d mat_local_to_parent;		// Transfo matrix from local ecliptique to parent ecliptic
	float axis_rotation;			// Rotation angle of the Body on it's axis
	std::shared_ptr<s_texture> tex_map=nullptr;			// Body map texture
	std::shared_ptr<s_texture> tex_skin=nullptr;			// Body skin texture
	std::shared_ptr<s_texture> tex_norm=nullptr;			// Body normal map
	std::shared_ptr<s_texture> tex_heightmap=nullptr;		// Body height map for Tessellation
	std::shared_ptr<s_texture> tex_current=nullptr;		// current body texture to display
	std::unique_ptr<Set> bigSet;

	Vec3f eye_sun;
	Vec3f eye_planet;
	SHADER_USE myShader = SHADER_UNDEFINED;  			// the name of the shader used for his display
	drawState_t *drawState;		// State for draw, include Pipeline and PipelineLayout
	ShadowDescriptor *shadow = nullptr;
	int cmds[3] = {-1, -1, -1};
	bool changed = true;
	bool hasAtmosphere = false;
	bool isCenterOfInterest = false;
	bool isRelevant = false;

	ObjL *currentObj = nullptr;

	double distance;				// Temporary variable used to store the distance to a given point it is used for sorting while drawing

	double lastJD;
	double deltaJD;

	std::unique_ptr<Orbit> orbit=nullptr;            // orbit object for this body
	Vec3f orbit_position;    // position of the planet

	std::shared_ptr<Body> parent;				// Body parent i.e. sun for earth
	std::list<Body *> satellites;		// satellites of the Planet

	static s_font* planet_name_font;// Font for names
	static float object_scale;
	static float object_size_limit;  // in pixels


	static LinearFader flagClouds;

	//LinearFader trail_fader;
	LinearFader visibilityFader;  // allows related lines and labels to fade in/out

	bool close_orbit; // whether to close orbit loop
	bool is_satellite;  // whether has a Body as a parent

	TURN_AROUND tAround;

	double orbit_bounding_radius; // AU calculated at load time for elliptical orbits at least DIGITALIS
	double boundingRadius;  // Cached AU value for use with depth buffer
	// double boundingRadiusWithOrbit;  // Cached AU value for use with depth buffer

	static std::shared_ptr<s_texture> defaultTexMap;  		// Default texture map for bodies if none supplied
	static std::shared_ptr<s_texture> tex_eclipse_map;  	// for moon shadow lookups


	float sun_half_angle; // for moon shadow calcs

	//variables caching
	Mat4d mat;
	Mat4d parent_mat;
	Vec3f lightDirection;
	float screen_sz;
	float angularSize;
	float ang_dist;
	bool isVisible;

	body_flags flags;

	std::unique_ptr<Trail> trail;
	std::shared_ptr<Hints> hints;
	std::shared_ptr<Axis> axis;
	std::unique_ptr<OrbitPlot> orbitPlot;
	std::shared_ptr<Halo> halo;
	std::unique_ptr<AtmExt> atmExt;

	Mat4d model;
	// Mat4f view;
	// Mat4f vp;
	// Mat4f proj;

	// //GZ Tail additions
	// Vec2f tailFactors; // result of latest call to getComaDiameterAndTailLengthAU(); Results cached here for infostring. [0]=Coma diameter, [1] gas tail length.
	// bool tailActive;		//! true if there is a tail long enough to be worth drawing. Drawing tails is quite costly.
	// bool tailBright;		//! true if tail is bright enough to draw.
	// double deltaJDEtail;            //! like deltaJDE, but time difference between tail geometry updates.
	// double lastJDEtail;             //! like lastJDE, but time of last tail geometry update.
	// Mat4d gasTailRot;		//! rotation matrix for gas tail parabola
	// Mat4d dustTailRot;		//! rotation matrix for the skewed dust tail parabola
	// float dustTailWidthFactor;      //!< empirical individual broadening of the dust tail end, compared to the gas tail end. Actually, dust tail width=2*comaWidth*dustTailWidthFactor. Default 1.5
	// float dustTailLengthFactor;     //!< empirical individual length of dust tail relative to gas tail. Taken from ssystem.ini, typical value 0.3..0.5, default 0.4
	// float dustTailBrightnessFactor; //!< empirical individual brightness of dust tail relative to gas tail. Taken from ssystem.ini, default 1.5

	// // adapt to vulkan ???
	// QVector<Vec3d> comaVertexArr;
	// QVector<Vec2f> comaTexCoordArr; //  --> 2014-08: could also be declared static, but it is filled by StelPainter...

	// // These are static to avoid having index arrays for each comet when all are equal.
	// static bool createTailIndices;
	// static bool createTailTextureCoords;

	// // adapt to vulkan ???
	// QVector<Vec3d> gastailVertexArr;  // computed frequently, describes parabolic shape (along z axis) of gas tail.
	// QVector<Vec3d> dusttailVertexArr; // computed frequently, describes parabolic shape (along z axis) of dust tail.
	// QVector<Vec3f> gastailColorArr;    // NEW computed for every 5 mins, modulates gas tail brightness for extinction
	// QVector<Vec3f> dusttailColorArr;   // NEW computed for every 5 mins, modulates dust tail brightness for extinction
	// static QVector<Vec2f> tailTexCoordArr; // computed only once for all comets!
	// static QVector<unsigned short> tailIndices; // computed only once for all comets!
	// static StelTextureSP comaTexture;
	// static StelTextureSP tailTexture;      // it seems not really necessary to have different textures. gas tail is just painted blue.

};

#endif // _BODY_HPP_
