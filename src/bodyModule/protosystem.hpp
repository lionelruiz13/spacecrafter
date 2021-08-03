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

#include "ojmModule/objl_mgr.hpp"
#include "bodyModule/body_tesselation.hpp"
#include "bodyModule/body_common.hpp"
#include "bodyModule/body_sun.hpp"
#include "bodyModule/body_moon.hpp"
#include "bodyModule/body_bigbody.hpp"
#include "bodyModule/body_smallbody.hpp"
#include "bodyModule/body_artificial.hpp"

class ThreadContext;
class OrbitCreator;


class ProtoSystem {
public:

    ProtoSystem(ThreadContext *_context, ObjLMgr *_objLMgr);
    ~ProtoSystem(){};
	
	//! Load the bodies data from a file
	void load(const std::string& planetfile);

	// load one object from a hash
	virtual void addBody(stringHash_t & param, bool deletable) = 0;

	//! Return the matching planet pointer if exists or nullptr
	virtual Body* searchByEnglishName(const std::string &planetEnglishName) const = 0;

protected:

	ThreadContext *context;
	ObjLMgr* objLMgr=nullptr;					// représente  les objets légers du ss
	Body* bodyTrace=nullptr; //retourne le body qui est sélectionné par bodyTrace
	OrbitCreator * orbitCreator = nullptr;

};