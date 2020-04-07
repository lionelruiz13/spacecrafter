/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2016 of the LSS Team & Association Sirius
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include <iostream>
#include <iterator>

#include "coreModule/skygrid_mgr.hpp"
#include "tools/log.hpp"

SkyGridMgr::SkyGridMgr()
{
	baseColor=Vec3f(0.f, 0.f, 0.f);
}

void SkyGridMgr::draw(const Projector* prj)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		it->second->draw(prj);
	}
}

SkyGridMgr::~SkyGridMgr()
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		cLog::get()->write("SkyGridMgr : delete " + it->first , LOG_TYPE::L_INFO);
		delete it->second;
	}
}

void SkyGridMgr::update(int delta_time)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		it->second->update(delta_time);
	}
}


void SkyGridMgr::setInternalNav(bool a)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		it->second->setInternalNav(a);
	}
}


void SkyGridMgr::setFont(float font_size, const std::string& font_name)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		it->second->setFont(font_size, font_name);
	}
}


void SkyGridMgr::flipFlagShow(SKYGRID_TYPE typeObj)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==typeObj) {
			it->second->flipFlagShow();
			return;
		}
	}
	cLog::get()->write("SkyGridMgr error : flipFlagShow not found " + typeObj , LOG_TYPE::L_WARNING);
}


void SkyGridMgr::setFlagShow(SKYGRID_TYPE typeObj, bool a)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==typeObj) {
			it->second->setFlagShow(a);
			return;
		}
	}
	cLog::get()->write("SkyGridMgr error : setFlagShow not found " + typeObj , LOG_TYPE::L_WARNING);
}


bool SkyGridMgr::getFlagShow(SKYGRID_TYPE typeObj)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==typeObj) {
			return it->second->getFlagShow();
		}
	}
	cLog::get()->write("SkyGridMgr error : getFlagShow not found " + typeObj , LOG_TYPE::L_WARNING);
	return false;
}


void SkyGridMgr::setColor(SKYGRID_TYPE typeObj, const Vec3f& c)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==typeObj) {
			it->second->setColor(c);
			return;
		}
	}
	cLog::get()->write("SkyGridMgr error : setColor not found " + typeObj , LOG_TYPE::L_WARNING);
}

const Vec3f& SkyGridMgr::getColor(SKYGRID_TYPE typeObj)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==typeObj) {
			return it->second->getColor();
		}
	}
	cLog::get()->write("SkyGridMgr error : getColor not found " + typeObj , LOG_TYPE::L_WARNING);
	return baseColor;
}


SKYGRID_TYPE SkyGridMgr::stringToType(const std::string& typeObj)
{
	if (typeObj == "GRID_EQUATORIAL")
		return SKYGRID_TYPE::GRID_EQUATORIAL;

	if (typeObj == "GRID_ALTAZIMUTAL")
		return SKYGRID_TYPE::GRID_ALTAZIMUTAL;

	if (typeObj == "GRID_ECLIPTIC")
		return SKYGRID_TYPE::GRID_ECLIPTIC;

	if (typeObj == "GRID_GALACTIC")
		return SKYGRID_TYPE::GRID_GALACTIC;

	return SKYGRID_TYPE::GRID_UNKNOWN;
}

void SkyGridMgr::Create(SKYGRID_TYPE type_obj)
{
	SkyGrid* tmp=nullptr;
	auto it=m_map.find(type_obj);

	//si l'itérateur ne vaut pas map.end(), cela signifie que que la clé à été trouvée
	if(it!=m_map.end()) {
		cLog::get()->write("SkyGridMgr SkyGrid already create " + type_obj , LOG_TYPE::L_ERROR);
		return;
	}

	//SKYGRID_TYPE typeObj=stringToType(type_obj);

	//switch (typeObj) {
	switch (type_obj) {
		case GRID_EQUATORIAL :
			cLog::get()->write("SkyGridMgr creating GRID EQUATORIAL" , LOG_TYPE::L_INFO);
			tmp=new GridEquatorial();
			m_map[type_obj]= tmp;
			return;
			break;

		case GRID_ALTAZIMUTAL:
			cLog::get()->write("SkyGridMgr creating GRID ALTAZIMUTAL" , LOG_TYPE::L_INFO);
			tmp=new GridAltAzimutal();
			m_map[type_obj]= tmp;
			return;
			break;

		case GRID_ECLIPTIC :
			cLog::get()->write("SkyGridMgr creating GRID ECLIPTIC" , LOG_TYPE::L_INFO);
			tmp=new GridEcliptic();
			m_map[type_obj]= tmp;
			return;
			break;

		case GRID_GALACTIC :
			cLog::get()->write("SkyGridMgr creating GRID GALACTIC" , LOG_TYPE::L_INFO);
			tmp=new GridGalactic();
			m_map[type_obj]= tmp;
			return;
			break;

		default: // grille inconnue
			cLog::get()->write("SkyGridMgr SkyGrid unknown " + type_obj , LOG_TYPE::L_ERROR);
			break;
	}
}
