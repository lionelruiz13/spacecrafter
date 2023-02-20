/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014 of the LSS Team & Association Sirius
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

#ifndef _NEBULA_MGR_H_
#define _NEBULA_MGR_H_

#include <vector>
#include <memory>
#include <set>
#include "tools/object.hpp"
#include "tools/auto_fader.hpp"
#include "tools/SphereGrid.hpp"
#include "coreModule/nebula.hpp"
#include "tools/no_copy.hpp"
#include "tools/ScModule.hpp"

#include "EntityCore/SubBuffer.hpp"

class VertexArray;
class VertexBuffer;
class Pipeline;
class PipelineLayout;
class Set;

/*! \class NebulaMgr
  * \brief NebulaMgr handles all deepsky_objects DSO.
  */

class NebulaMgr : public NoCopy, public ModuleFont, public AModuleFader<ALinearFader> {
public:
	NebulaMgr();
	virtual ~NebulaMgr();

	//! Read the Nebulas data from a unique file cat
	bool loadDeepskyObject(const std::string& cat);

	//! Create the Nebulas data from variables
	bool loadDeepskyObject(std::string _englishName, std::string _DSOType, std::string _constellation, float _ra, float _de, float _mag, float _size, std::string _classe,
	                       float _distance, std::string tex_name, bool path, float tex_angular_size, float _rotation, std::string _credit, float _luminance, bool deletable = true);

	//! remove user added nebula and optionally unhide the original of the same name
	void removeNebula(const std::string& name, bool showOriginal);

	//! remove all user added nebula
	void removeSupplementalNebulae();

	//! Draw all the DSO
	void draw(const Projector *prj, const Navigator *nav, ToneReproductor *eye, double sky_brightness);

	//! search deepskyObject by name M83, NGC 1123, IC1234 ...
	//! \return a object
	Object search(const std::string& name);

	//! \brief search deepskyObject by name
	//! \return a nebula object
	Nebula *searchNebula(const std::string& name, bool search_hidden);

	//! \brief search the Nebulae by position
	//! \return a object
	Object search(Vec3f Pos);

	//! Define the scale of the Nebula circle
	void setNebulaCircleScale(float scale) {
		circleScale = scale;
	}

	//! get the scale of the Nebula circle
	float getNebulaCircleScale(void) const {
		return circleScale;
	}

	//!set the fade duration from Hints DSO
	void setHintsFadeDuration(float duration) {
		hintsFader.setDuration(duration);
	}

	//!set the Flag Hints for DSO
	void setFlagHints(bool b) {
		hintsFader=b;
	}
	//!get the Flag Hints value for DSO
	bool getFlagHints(void) const {
		return hintsFader.finalState();
	}

	//! Define the default Label Color for DSO font
	void setLabelColor(const Vec3f& c) {
		labelColor = c;
	}

	//! get the actual Label Color for DSO font
	const Vec3f &getLabelColor(void) const {
		return labelColor;
	}

	//! set flag for display specific or generic Hint
	void setDisplaySpecificHint(const bool& b ) {
		displaySpecificHint = b;
	}

	//! get flag for display specific or generic Hint
	const bool &getDisplaySpecificHint(void) const {
		return displaySpecificHint;
	}

	//! set commun DSO circle color
	void setCircleColor(const Vec3f& c) {
		circleColor = c;
	}

	//! get commun DSO circle color
	const Vec3f &getCircleColor(void) const {
		return circleColor;
	}

	int getPictoSize() const {
		return dsoPictoSize;
	}

	//! Defined half-size pictogram representing DSO
	void setPictoSize(int radius) {
		dsoPictoSize = radius;
	}

	//! Return a stl vector containing the nebulas located inside the lim_fov circle around position v
	std::vector<Object> searchAround(Vec3d v, double lim_fov) const;

	//! @brief Update i18 names from english names according to passed translator
	//! The translation is done using gettext with translated strings defined in translations.h
	void translateNames(Translator& trans);

	//! Set flag for displaying Nebulae as bright
	void setFlagBright(bool b) {
		flagBright = b;
	}

	//! Get flag for displaying Nebulae as bright
	bool getFlagBright(void) const {
		return flagBright;
	}

	//! Set maximum magnitude at which nebulae hints are displayed
	void setMaxMagHints(float f) {
		maxMagHints = f;
	}

	//! Get maximum magnitude at which nebulae hints are displayed
	float getMaxMagHints(void) const {
		return maxMagHints;
	}

	//! set if Nebulas Names are drawing or not
	void setNebulaNames(bool value) {
		textFader=value;
	}

	//! get current status of Nebulas Names
	bool getNebulaNames() {
		return textFader.finalState();
	}

	void setSelected(Object ojb);

	void deselect() {
		selected_nebulas.clear();
	}

	//! Set whether selected Nebulas must be displayed alone
	void setFlagIsolateSelected(bool s) {
		isolateSelected = s;
	}

	//! Get whether selected Nebulas are displayed alone
	bool getFlagIsolateSelected(void) const {
		return isolateSelected;
	}

	//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
	//! @param objPrefix the case insensitive first letters of the searched object
	//! @param maxNbItem the maximum number of returned object names
	//! @return a vector of matching object name by order of relevance, or an empty vector if nothing match
	std::vector<std::string> listMatchingObjectsI18n(const std::string& objPrefix, unsigned int maxNbItem=5) const;

	//! Return the matching Nebula object's pointer if exists or NULL
	//! @param nameI18n The case sensistive nebula name or NGC M catalog name : format can be M31, M 31, NGC31 NGC 31
	Object searchByNameI18n(const std::string& nameI18n) const;

	//! Hide all DSO for display
	void hideAll();

	//! Show all DSO for display
	void showAll();

	//! select DSO according to their constellation ratachement
	void selectConstellation(bool hide, std::string constellationName);

	//! sort DSO according to their type ratachement
	void selectType(bool hide, std::string constellationName);

	//! select DSO with name DSOName
	void selectName(bool hide, std::string DSOName);

protected:

	//! load all texture pictograms for all DSO
	//! \return return true if all texture pictograms are correctly loaded false  owerwise
	bool initTexPicto();
	//void createShaderHint();
	void createSC_context();
	void drawAllHint(const Projector* prj);

private:
	bool loadDeepskyObjectFromCat(const std::string& cat); //!< load DSO with reading file cat

	ALinearFader hintsFader;			//!< Hint about position and number of dso
	ALinearFader textFader;			//!< Display names smoothly

	#ifdef _MSC_VER // MSVC is not C++11 compliant, using copy for moving in resize and reserve
	typedef SphereGrid<std::shared_ptr<Nebula>> nebGrid_t;
	#else
	typedef SphereGrid<std::unique_ptr<Nebula>> nebGrid_t;
	#endif
	nebGrid_t nebGrid;

	float maxMagHints;				//!< Define maximum magnitude at which nebulae hints are displayed

	int cmds[3] = {-1, -1, -1};
	std::unique_ptr<VertexArray> m_hintGL;
	std::unique_ptr<VertexBuffer> vertexHint;
	SubBuffer indexHint;
	std::unique_ptr<Pipeline> pipelineHint;
	std::unique_ptr<PipelineLayout> layoutHint;
	std::unique_ptr<Set> setHint;

	// std::vector<float> vecHintPos;		//!< array of coordinates of the nebula's position
	// std::vector<float> vecHintTex;		//!< array of coordinates of the nebula's texture
	// std::vector<float> vecHintColor;	//!< array of the nebula's color
	int nbDraw;

	s_texture * tex_NEBULA;

	float circleScale;			// Define the sclaing of the hints circle
	Vec3f circleColor;
	Vec3f labelColor;
	bool flagBright;			// Define if nebulae must be drawn in bright mode
	bool displaySpecificHint;	// Define if specific or generic Hints are to be displayed
	int dsoPictoSize;			// Define the size/2 from picto tex

	bool isolateSelected=false;
	std::set<std::string> selected_nebulas;
};

#endif // _NEBULA_MGR_H_
