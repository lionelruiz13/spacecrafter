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
#include "tools/object.hpp"
#include "tools/fader.hpp"
#include "coreModule/grid.hpp"
#include "coreModule/nebula.hpp"
#include "tools/no_copy.hpp"

class VertexArray;

/*! \class NebulaMgr
  * \brief NebulaMgr handles all deepsky_objects DSO.
  */

class NebulaMgr : public NoCopy {
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

	//!Update Fader from DSO
	void update(int delta_time) {
		hintsFader.update(delta_time);
		showFader.update(delta_time);
		textFader.update(delta_time);
	}

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
		Nebula::circleScale = scale;
	}

	//! get the scale of the Nebula circle
	float getNebulaCircleScale(void) const {
		return Nebula::circleScale;
	}

	//!set the fade duration from Hints DSO
	void setHintsFadeDuration(float duration) {
		hintsFader.setDuration((int) (duration * 1000.f));
	}

	//!set the Flag Hints for DSO
	void setFlagHints(bool b) {
		hintsFader=b;
	}
	//!get the Flag Hints value for DSO
	bool getFlagHints(void) const {
		return hintsFader;
	}

	//!set the showFader value
	void setFlagShow(bool b) {
		showFader = b;
	}

	//!get the showFader value
	bool getFlagShow(void) const {
		return showFader;
	}

	//! Define the default Label Color for DSO font
	void setLabelColor(const Vec3f& c) {
		Nebula::labelColor = c;
	}

	//! get the actual Label Color for DSO font
	const Vec3f &getLabelColor(void) const {
		return Nebula::labelColor;
	}

	//! set flag for display specific or generic Hint
	void setDisplaySpecificHint(const bool& b ) {
		Nebula::displaySpecificHint = b;
	}

	//! get flag for display specific or generic Hint
	const bool &getDisplaySpecificHint(void) const {
		return Nebula::displaySpecificHint;
	}

	//! set commun DSO circle color
	void setCircleColor(const Vec3f& c) {
		Nebula::circleColor = c;
	}

	//! get commun DSO circle color
	const Vec3f &getCircleColor(void) const {
		return Nebula::circleColor;
	}

	//! Defined half-size pictogram representing DSO
	void setPictoSize(int radius) const {
		Nebula::dsoPictoSize = radius;
	}

	//! Return a stl vector containing the nebulas located inside the lim_fov circle around position v
	std::vector<Object> searchAround(Vec3d v, double lim_fov) const;

	//! @brief Update i18 names from english names according to passed translator
	//! The translation is done using gettext with translated strings defined in translations.h
	void translateNames(Translator& trans);

	//! Set flag for displaying Nebulae as bright
	void setFlagBright(bool b) {
		Nebula::flagBright = b;
	}

	//! Get flag for displaying Nebulae as bright
	bool getFlagBright(void) const {
		return Nebula::flagBright;
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
		return textFader;
	}

	//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
	//! @param objPrefix the case insensitive first letters of the searched object
	//! @param maxNbItem the maximum number of returned object names
	//! @return a vector of matching object name by order of relevance, or an empty vector if nothing match
	std::vector<std::string> listMatchingObjectsI18n(const std::string& objPrefix, unsigned int maxNbItem=5) const;

	//! Return the matching Nebula object's pointer if exists or NULL
	//! @param nameI18n The case sensistive nebula name or NGC M catalog name : format can be M31, M 31, NGC31 NGC 31
	Object searchByNameI18n(const std::string& nameI18n) const;

	//! load the commun font for all DSO
	//! \return return true if font is correctly loaded false owerwise
	void setFont(float font_size, const std::string& font_name);

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
	void createShaderHint();
	void createShaderTex();
	void deleteShaderTex();
	//void deleteShaderHint();
	void createGL_context();
	void drawAllHint(const Projector* prj);

private:
	bool loadDeepskyObjectFromCat(const std::string& cat); //!< load DSO with reading file cat

	std::vector<Nebula*> neb_array;		//!< The nebulas list
	LinearFader hintsFader;			//!< Hint about position and number of dso
	LinearFader showFader;			//!< For display all DSO fonctionnalities
	LinearFader textFader;			//!< Display names smoothly

	std::vector<Nebula*>* nebZones;		//!< array of nebula vector with the grid id as array rank
	LittleGrid nebGrid;				//! Grid for opimisation

	float maxMagHints;				//!< Define maximum magnitude at which nebulae hints are displayed

	//shaderProgram *shaderNebulaHint;
	std::unique_ptr<shaderProgram> shaderNebulaHint;
	//DataGL nebulaHint;
	std::unique_ptr<VertexArray> m_hintGL;

	std::vector<float> vecHintPos;		//!< array of coordinates of the nebula's position
	std::vector<float> vecHintTex;		//!< array of coordinates of the nebula's texture
	std::vector<float> vecHintColor;		//!< array of the nebula's color
};

#endif // _NEBULA_MGR_H_
