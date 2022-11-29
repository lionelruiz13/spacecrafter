/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2021 Jérémy Calvo
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

#include "tools/object.hpp"
#include "bodyModule/body_common.hpp"

class ProtoSystem;

/**
 * \file solarsystem_selected.hpp
 * \brief Handle solar system select functions
 * \author Jérémy Calvo
 * \version 1
 *
 * \class SolarSystemSelected
 *
 * \brief Set the selected object in solar system
 *
 * Acts on select flags
 *
*/

class SolarSystemSelected {
public:
    SolarSystemSelected(ProtoSystem * _ssystem);
    ~SolarSystemSelected();

	void changeSystem(ProtoSystem * _ssystem) {
        const bool flagPlanetsOrbits_ = flagPlanetsOrbits;
        const bool flagSatellitesOrbits_ = flagSatellitesOrbits;
        setFlagPlanetsOrbits(false);
        setFlagSatellitesOrbits(false);
		ssystem = _ssystem;
        setFlagPlanetsOrbits(flagPlanetsOrbits_);
    	setFlagSatellitesOrbits(flagSatellitesOrbits_);
	}

	//! Get selected object's pointer
	Object getSelected(void) const {
		return selected;
	}

    //! Set selected object from its pointer
	void setSelected(const Object &obj);

	//! Set selected planet by english name or "" to select none
	void setSelected(const std::string& englishName);

	//! set flag for Activate/Deactivate planets trails display
	void setFlagTrails(bool b);

	//! set flag for Activate/Deactivate planets hints display
	void setFlagHints(bool b);

    //! Get flag for Activate/Deactivate planets orbits display
	bool getFlagPlanetsOrbits(void) const {
		return flagPlanetsOrbits;
    }

	//! Set flag for Activate/Deactivate planets orbits display
	void setFlagPlanetsOrbits(bool b);

	//! Set flag for Activate/Deactivate planet _name orbit display
	void setFlagPlanetsOrbits(const std::string &_name, bool b);

	//! Activate/Deactivate planet&&satellites orbits display
	void setFlagOrbits(bool b) {
		setFlagPlanetsOrbits(b);
		setFlagSatellitesOrbits(b);
	}

	//! getflag for Activate/Deactivate satellites orbits display
	bool getFlagSatellitesOrbits(void) const {
		return flagSatellitesOrbits;
	}

	//! Set flag for Activate/Deactivate satellites orbits display
	void setFlagSatellitesOrbits(bool b);

	bool getFlag(BODY_FLAG name);

	void setFlagIsolateSelected(bool b);

    bool getFlagIsolateSelected() {return flagIsolateSelected;}

private:
    ProtoSystem * ssystem;
    //! The currently selected planet
	Object selected;

	bool flagTrails= false;
	bool flagHints= false;
	bool flagPlanetsOrbits= false;
	bool flagSatellitesOrbits= false;
	bool flagIsolateSelected= false;
};
