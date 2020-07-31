/*
 * Spacecrafter
 * Copyright (C) 2002 Fabien Chereau
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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
 */

#ifndef _STAR_MGR_H_
#define _STAR_MGR_H_
#define NBR_MAX_STARS 8000

#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <tuple>
#include <memory>

#include "tools/fader.hpp"
#include "tools/object_type.hpp"
#include "tools/object.hpp"
#include "tools/shader.hpp"
#include "tools/stateGL.hpp"
#include "tools/no_copy.hpp"
#include "tools/ScModule.hpp"

class Translator;
class InitParser;
class s_texture;
class Object;
class ToneReproductor;
class Projector;
class Navigator;
class TimeMgr;
class s_font;
class HipStarMgr;
class GeodesicGrid;
class VertexArray;

typedef std::tuple<double, double, const std::string , const Vec4f > starDBtoDraw;

namespace BigStarCatalog {
class ZoneArray;
class HipIndexStruct;
}


class MagConverter {
public:
	MagConverter(const HipStarMgr &mgr) : mgr(mgr) {
		setMaxFov(180.f);
		setMinFov(0.1f);
		setFov(180.f);
		setMagShift(0.f);
		setMaxMag(30.f);
		min_rmag = 0.01f;
	}
	void setMaxFov(float fov) {
		max_fov = (fov < 60.f) ? 60.f : fov;
	}
	void setMinFov(float fov) {
		min_fov = (fov > 60.f) ? 60.f : fov;
	}
	void setMagShift(float d) {
		mag_shift = d;
	}
	void setMaxMag(float mag) {
		max_mag = mag;
	}
	void setMaxScaled60DegMag(float mag) {
		max_scaled_60deg_mag = mag;
	}
	float getMaxFov(void) const {
		return max_fov;
	}
	float getMinFov(void) const {
		return min_fov;
	}
	float getMagShift(void) const {
		return mag_shift;
	}
	float getMaxMag(void) const {
		return max_mag;
	}
	float getMaxScaled60DegMag(void) const {
		return max_scaled_60deg_mag;
	}
	void setFov(float fov);
	void setEye(const ToneReproductor *eye);
	int computeRCMag(float mag, const ToneReproductor *eye, float rc_mag[2]) const;
private:
	const HipStarMgr &mgr;
	float max_fov, min_fov, mag_shift, max_mag, max_scaled_60deg_mag, min_rmag, fov_factor;
};


//! @class StarMgr
//! Stores the star catalogue data.
//! Used to render the stars themselves, as well as determine the color table
//! and render the labels of those stars with names for a given SkyCulture.
//!
//! The celestial sphere is split into zones, which correspond to the
//! triangular faces of a geodesic sphere. The number of zones (faces)
//! depends on the level of sub-division of this sphere. The lowest
//! level, 0, is an icosahedron (20 faces), subsequent levels, L,
//! of sub-division give the number of zones, n as:
//!
//! n=20 x 4^L
//!
//! Stellarium uses levels 0 to 7 in the existing star catalogues.
//! Star Data Records contain the position of a star as an offset from
//! the central position of the zone in which that star is located,
//! thus it is necessary to determine the vector from the observer
//! to the centre of a zone, and add the star's offsets to find the
//! absolute position of the star on the celestial sphere.
//!
//! This position for a star is expressed as a 3-dimensional vector
//! which points from the observer (at the centre of the geodesic sphere)
//! to the position of the star as observed on the celestial sphere.

class HipStarMgr: public NoCopy , public ModuleFont, public ModuleFader<LinearFader> {
public:
	HipStarMgr(int width,int height);
	virtual ~HipStarMgr(void);

	//!/////////////////////////////////////////////////////////////////////////
	//! Methods defined in the StelModule class
	//! Initialize the StarMgr.
	//! - Loads the star catalogue data into memory
	//! - Sets up the star color table
	//! - Loads the star texture
	//! - Loads the star font (for labels on named stars)
	//! - Loads the texture of the sar selection indicator
	//! - Lets various display flags from the ini parser object
	//!
	//! @param conf The ini parser object containing relevant settings.
	virtual void init(const InitParser &conf);

	//! draw the stars and the star selection indicator if necessary
	virtual double draw(GeodesicGrid* grid, ToneReproductor* eye, Projector* prj, TimeMgr* timeMgr, float altitude);

	//! compute the stars and the star selection indicator if necessary in buffer.
	virtual double preDraw(GeodesicGrid* grid, ToneReproductor* eye, Projector* prj, Navigator* nav, TimeMgr* timeMgr, float altitude, bool atmosphere);

	//! Update any time-dependent features.
	//! Includes fading in and out stars and labels when they are turned on and off.
	virtual void update(double deltaTime) {
		names_fader.update(deltaTime);
		fader.update(deltaTime);
	}

	//! Translate text.
	virtual void updateI18n(Translator& trans);

	//!/////////////////////////////////////////////////////////////////////////
	//! Methods defined in StelObjectManager class
	//! Return a stl vector containing the stars located inside the lim_fov circle around position v
	virtual std::vector<ObjectBaseP > searchAround(const Vec3d& v, double limitFov, const GeodesicGrid* grid) const;

	//! Return the matching Stars object's pointer if exists or NULL
	//! @param nameI18n The case sensistive star common name or HP
	//! catalog name (format can be HP1234 or HP 1234) or sci name
	virtual ObjectBaseP searchByNameI18n(const std::string& nameI18n) const;

	//! Return the matching star if exists or NULL
	//! @param name The case sensistive standard program planet name
	virtual ObjectBaseP searchByName(const std::string& name) const;

	//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name.
	//! @param objPrefix the case insensitive first letters of the searched object
	//! @param maxNbItem the maximum number of returned object names
	//! @return a vector of matching object name by order of relevance, or an empty vector if nothing match
	virtual std::vector<std::string> listMatchingObjectsI18n(const std::string& objPrefix, unsigned int maxNbItem=5) const;

	//!/////////////////////////////////////////////////////////////////////////
	//! Properties setters and getters
	//! Get the maximum level of the geodesic sphere used.
	//! See the class description for a short introduction to the meaning of this value.
	int getMaxGridLevel(void) const {
		return max_geodesic_grid_level;
	}

	//! Initializes each triangular face of the geodesic grid.
	void setGrid(class GeodesicGrid* grid);

	//! Gets the maximum search level.
	//! @todo: add a non-lame description - what is the purpose of the max search level?
	int getMaxSearchLevel(const ToneReproductor *eye, const Projector *prj) const;

	//! Sets the time it takes for star names to fade and off.
	//! @param duration the time in seconds.
	void setNamesFadeDuration(float duration) {
		names_fader.setDuration((int) (duration * 1000.f));
	}

	//! Loads common names for stars from a file.
	//! Called when the SkyCulture is updated.
	//! @param the path to a file containing the common names for bright stars.
	int loadCommonNames(const std::string& commonNameFile);

	//! Loads scientific names for stars from a file.
	//! Called when the SkyCulture is updated.
	//! @param the path to a file containing the scientific names for bright stars.
	void loadSciNames(const std::string& sciNameFile);

	//! Search for the nearest star to some position.
	//! @param Pos the 3d vector representing the direction to search.
	//! @return the nearest star from the specified position, or an
	//! empty StelObjectP if none were found close by.
	ObjectBaseP search(Vec3d Pos) const;

	//! Search for a star by catalogue number (including catalogue prefix).
	//! @param id the catalogue identifier for the required star.
	//! @return the requested StelObjectP or an empty objecy if the requested
	//! one was not found.
	ObjectBaseP search(const std::string& id) const;

	//! Search bu Hipparcos catalogue number.
	//! @param num the Hipparcos catalogue number of the star which is required.
	//! @return the requested StelObjectP or an empty objecy if the requested
	//! one was not found.
	ObjectBaseP searchHP(int num) const;

	//! Set display flag for Star names (labels).
	void setFlagNames(bool b) {
		names_fader=b;
	}

	//! Get display flag for Star names (labels).
	bool getFlagNames(void) const {
		return names_fader==true;
	}

	void setSelected(Object star);

	std::vector<int> getSelected() {
		return selected_stars;
	}

	void deselect() {
		selected_star.clear();
		selected_stars.clear();
	}

	//! Set whether selected stars must be displayed alone
	void setFlagIsolateSelected(bool s) {
		isolateSelected = s;
	}

	//! Get whether selected stars are displayed alone
	bool getFlagIsolateSelected(void) const {
		return isolateSelected;
	}

	// //! Set display flag for Star Scientific names.
	// void setFlagStarsSciNames(bool b) {
	// 	flagStarSciName=b;
	// }
	// //! Get display flag for Star Scientific names.
	// bool getFlagStarsSciNames(void) const {
	// 	return flagStarSciName;
	// }

	//! Set flag for Star twinkling.
	void setFlagTwinkle(bool b) {
		flagStarTwinkle=b;
	}
	//! Get flag for Star twinkling.
	bool getFlagTwinkle(void) const {
		return flagStarTwinkle;
	}

	//! Set flag for Star spinning.
	void setFlagTrace(bool b) {
		starTrace=b;
	}
	//! Get flag for Star spinning.
	bool getFlagTrace(void) const {
		return starTrace;
	}


	//! Set maximum magnitude at which stars names are displayed.
	void setMaxMagName(float b) {
		maxMagStarName=b;
	}
	//! Get maximum magnitude at which stars names are displayed.
	float getMaxMagName(void) const {
		return maxMagStarName;
	}

	//! Set base stars display scaling factor.
	void setScale(float b) {
		starScale=b;
	}
	//! Get base stars display scaling factor.
	float getScale(void) const {
		return starScale;
	}

	float getStarSizeLimit(void) const {
		return starSizeLimit;
	}
	void setStarSizeLimit(float f) {
		starSizeLimit = f;
	}

	//! This is the marginal planet size limit set here
	//! for use rendering stars.  Master location is in ssystem.
	void setObjectSizeLimit(float f) {
		objectSizeLimit = f;
	}

	//! Set stars display scaling factor wrt magnitude.
	void setMagScale(float b) {
		starMagScale=b;
	}
	//! Get base stars display scaling factor wrt magnitude.
	float getMagScale(void) const {
		return starMagScale;
	}

	//! Set stars twinkle amount.
	void setTwinkleAmount(float b) {
		twinkleAmount=b;
	}
	//! Get stars twinkle amount.
	float getTwinkleAmount(void) const {
		return twinkleAmount;
	}


	//! Set MagConverter maximum FOV.
	//! Usually stars/planet halos are drawn fainter when FOV gets larger,
	//! but when FOV gets larger than this value, the stars do not become
	//! fainter any more. Must be >= 60.0.
	void setMagConverterMaxFov(float x) {
		mag_converter->setMaxFov(x);
	}

	//! Set MagConverter minimum FOV.
	//! Usually stars/planet halos are drawn brighter when FOV gets smaller.
	//! But when FOV gets smaller than this value, the stars do not become
	//! brighter any more. Must be <= 60.0.
	void setMagConverterMinFov(float x) {
		mag_converter->setMinFov(x);
	}

	//! Set MagConverter magnitude shift.
	//! draw the stars/planet halos as if they were brighter of fainter
	//! by this amount of magnitude
	void setMagConverterMagShift(float x) {
		mag_converter->setMagShift(x);
	}

	//! Set MagConverter maximum magnitude.
	//! stars/planet halos, whose original (unshifted) magnitude is greater
	//! than this value will not be drawn.
	void setMagConverterMaxMag(float mag) {
		mag_converter->setMaxMag(mag);
	}

	//! Set MagConverter maximum scaled magnitude wrt 60 degree FOV.
	//! Stars/planet halos, whose original (unshifted) magnitude is greater
	//! than this value will not be drawn at 60 degree FOV.
	void setMagConverterMaxScaled60DegMag(float mag) {
		mag_converter->setMaxScaled60DegMag(mag);
	}

	//! Get MagConverter maximum FOV.
	float getMagConverterMaxFov(void) const {
		return mag_converter->getMaxFov();
	}
	//! Get MagConverter minimum FOV.
	float getMagConverterMinFov(void) const {
		return mag_converter->getMinFov();
	}
	//! Get MagConverter magnitude shift.
	float getMagConverterMagShift(void) const {
		return mag_converter->getMagShift();
	}
	//! Get MagConverter maximum magnitude.
	float getMagConverterMaxMag(void) const {
		return mag_converter->getMaxMag();
	}
	//! Get MagConverter maximum scaled magnitude wrt 60 degree FOV.
	float getMagConverterMaxScaled60DegMag(void) const {
		return mag_converter->getMaxScaled60DegMag();
	}

	//! Compute RMag and CMag from magnitude.
	//! Useful for conststent drawing of Planet halos.
	int computeRCMag(float mag, float fov, const ToneReproductor *eye, float rc_mag[2]) const {
		mag_converter->setFov(fov);
		mag_converter->setEye(eye);
		return mag_converter->computeRCMag(mag,eye,rc_mag);
	}

	//! Show scientific or catalog names on stars without common names.
	static void setFlagSciNames(bool f) {
		flagSciNames = f;
	}
	static bool getFlagSciNames(void) {
		return flagSciNames;
	}

	//! Draw a star of specified position, magnitude and color.
	int drawStar(const Projector *prj, const Vec3d &XY, const float rc_mag[2], const Vec3f &color) const;

	//! Get the (translated) common name for a star with a specified
	//! Hipparcos catalogue number.
	static std::string getCommonName(int hip);

	//! Get the (translated) scientifc name for a star with a specified
	//! Hipparcos catalogue number.
	static std::string getSciName(int hip);

	static Vec3f color_table[128];

	static double getCurrentJDay(void) {
		return current_JDay;
	}
	static std::string convertToSpectralType(int index);
	static std::string convertToComponentIds(int index);

	void iniColorTable();
	void readColorTable ();
	void setColorStarTable(int p, Vec3f a);

private:
	//! Load all the stars from the files.
	void load_data(const InitParser &conf);

	void drawStarName( Projector* prj );
	int getHPFromStarName(const std::string& name) const;

	LinearFader names_fader;

	float starSizeLimit;
	float objectSizeLimit;
	float starScale;
	float starMagScale;
	bool flagStarName;
	// bool flagStarSciName;
	float maxMagStarName;
	bool flagStarTwinkle;
	float twinkleAmount;
	bool gravityLabel;
	bool isolateSelected;
	std::map<std::string, bool> selected_star;
	std::vector<int> selected_stars;

	s_texture* starTexture; //! star texture

	int max_geodesic_grid_level;
	int last_max_search_level;
	typedef std::map<int,BigStarCatalog::ZoneArray*> ZoneArrayMap;
	ZoneArrayMap zone_arrays; //! index is the grid level
	static void initTriangleFunc(int lev, int index, const Vec3d &c0, const Vec3d &c1, const Vec3d &c2, void *context) {
		reinterpret_cast<HipStarMgr*>(context)->initTriangle(lev, index, c0, c1, c2);
	}

	void initTriangle(int lev, int index, const Vec3d &c0, const Vec3d &c1, const Vec3d &c2);

	BigStarCatalog::HipIndexStruct *hip_index; //! array of hiparcos stars

	MagConverter *mag_converter;

	static std::map<int, std::string> common_names_map;
	static std::map<int, std::string> common_names_map_i18n;
	static std::map<std::string, int> common_names_index;
	static std::map<std::string, int> common_names_index_i18n;

	static std::map<int, std::string> sci_names_map_i18n;
	static std::map<std::string, int> sci_names_index_i18n;

	static double current_JDay;

	double fontSize;
	static bool flagSciNames;
	float twinkle_amount;

	std::vector<starDBtoDraw> starNameToDraw;

	s_texture* texPointer;		//! The selection pointer texture

	mutable int nbStarsToDraw;
	void createShaderParams(int width,int height);
	// void deleteShader();
	std::unique_ptr<shaderProgram> shaderStars, shaderFBO;
	mutable std::vector<float> dataPos;
	mutable std::vector<float> dataMag;
	mutable std::vector<float> dataColor;
	std::unique_ptr<VertexArray> m_starsGL, m_drawFBO_GL;
	int sizeTexFbo;
	bool starTrace = false;
	
	//FBO and render buffer object ID
	GLuint fboID, rbID;
	//offscreen render texture ID
	GLuint renderTextureID;
};


#endif // _STAR_MGR_H_
