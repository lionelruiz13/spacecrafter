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

#include "coreModule/skyline_mgr.hpp"
#include "tools/log.hpp"


SkyLineMgr::SkyLineMgr()
{
	baseColor=Vec3f(0.f, 0.f, 0.f);
	SkyLine::createShader();
}

void SkyLineMgr::draw(const Projector* prj, const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		it->second->draw(prj, nav, timeMgr, observatory);
	}
}

SkyLineMgr::~SkyLineMgr()
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		//~ cout << "suppression de " << it->first << endl;
		cLog::get()->write("SkyLineMgr : delete " + typeToString(it->first) , LOG_TYPE::L_INFO);
		delete it->second;
	}

	SkyLine::deleteShader();
}

void SkyLineMgr::update(int delta_time)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		it->second->update(delta_time);
	}
}

void SkyLineMgr::translateLabels(Translator& trans)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		it->second->translateLabels(trans);
	}
}

void SkyLineMgr::setInternalNav(bool a)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		it->second->setInternalNav(a);
	}
}


bool SkyLineMgr::isExist(SKYLINE_TYPE type_obj)
{
	auto it=m_map.find(type_obj);
	//si l'itérateur ne vaut pas map.end(), cela signifie que que la clé à été trouvée
	if(it!=m_map.end())
		return true;
	else
		return false;
}

void SkyLineMgr::setFont(float font_size, const std::string& font_name)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		it->second->setFont(font_size, font_name);
	}
}


void SkyLineMgr::flipFlagShow(SKYLINE_TYPE typeObj)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==typeObj) {
			it->second->flipFlagShow();
			return;
		}
	}
	//~ std::cout << "error SkyGridMgr::flipFlagShow : " << typeObj << " not found" << std::endl;
	cLog::get()->write("SkyLineMgr error : flipFlagShow not found " + typeToString(typeObj) , LOG_TYPE::L_WARNING);
}


void SkyLineMgr::setFlagShow(SKYLINE_TYPE typeObj, bool a)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==typeObj) {
			it->second->setFlagshow(a);
			return;
		}
	}
	//~ std::cout << "error SkyGridMgr::setFlagShow : " << typeObj << " not found" << std::endl;
	cLog::get()->write("SkyLineMgr error : setFlagShow not found " + typeToString(typeObj) , LOG_TYPE::L_WARNING);
}


bool SkyLineMgr::getFlagShow(SKYLINE_TYPE typeObj)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==typeObj) {
			return it->second->getFlagshow();
		}
	}
	//~ std::cout << "error SkyGridMgr::getFlagShow : " << typeObj << " not found" << std::endl;
	cLog::get()->write("SkyLineMgr error : getFlagShow not found " + typeToString(typeObj) , LOG_TYPE::L_WARNING);
	return false;
}


void SkyLineMgr::setColor(SKYLINE_TYPE typeObj, const Vec3f& c)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==typeObj) {
			it->second->setColor(c);
			return;
		}
	}
	//~ std::cout << "error SkyGridMgr::setColor : " << typeObj << " not found" << std::endl;
	cLog::get()->write("SkyLineMgr error : setColor not found " + typeToString(typeObj) , LOG_TYPE::L_WARNING);
}

const Vec3f& SkyLineMgr::getColor(SKYLINE_TYPE typeObj)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==typeObj) {
			return it->second->getColor();
		}
	}
	//~ std::cout << "error SkyGridMgr::getColor : " << typeObj << " not found" << std::endl;
	cLog::get()->write("SkyLineMgr error : getColor not found " + typeToString(typeObj) , LOG_TYPE::L_WARNING);
	//~ Vec3f tmp(0.f, 0.f, 0.f);
	return baseColor;
}

std::string SkyLineMgr::typeToString(SKYLINE_TYPE typeObj)
{
	switch ( typeObj) {
		case SKYLINE_TYPE::LINE_CIRCLE_POLAR : return "LINE_CIRCLE_POLAR"; break;
		case SKYLINE_TYPE::LINE_POINT_POLAR : return "LINE_POINT_POLAR"; break;
		case SKYLINE_TYPE::LINE_ECLIPTIC_POLE : return "LINE_ECLIPTIC_POLE"; break;
		case SKYLINE_TYPE::LINE_GALACTIC_POLE : return "LINE_GALACTIC_POLE"; break;
		case SKYLINE_TYPE::LINE_ANALEMMA : return "LINE_ANALEMMA"; break;
		case SKYLINE_TYPE::LINE_ANALEMMALINE : return "LINE_ANALEMMALINE"; break;
		case SKYLINE_TYPE::LINE_CIRCUMPOLAR : return "LINE_CIRCUMPOLAR"; break;
		case SKYLINE_TYPE::LINE_GALACTIC_CENTER : return "LINE_GALACTIC_CENTER"; break;
		case SKYLINE_TYPE::LINE_VERNAL : return "LINE_VERNAL"; break;
		case SKYLINE_TYPE::LINE_GREENWICH : return "LINE_GREENWICH"; break;
		case SKYLINE_TYPE::LINE_ARIES : return "LINE_ARIES"; break;
		case SKYLINE_TYPE::LINE_EQUATOR : return "LINE_EQUATOR"; break;
		case SKYLINE_TYPE::LINE_GALACTIC_EQUATOR : return "LINE_GALACTIC_EQUATOR"; break;
		case SKYLINE_TYPE::LINE_MERIDIAN : return "LINE_MERIDIAN"; break;
		case SKYLINE_TYPE::LINE_TROPIC : return "LINE_TROPIC"; break;
		case SKYLINE_TYPE::LINE_ECLIPTIC : return "LINE_ECLIPTIC"; break;
		case SKYLINE_TYPE::LINE_PRECESSION : return "LINE_PRECESSION"; break;
		case SKYLINE_TYPE::LINE_VERTICAL : return "LINE_VERTICAL"; break;
		case SKYLINE_TYPE::LINE_ZODIAC : return "LINE_ZODIAC"; break;
		case SKYLINE_TYPE::LINE_ZENITH : return "LINE_ZENITH"; break;
		case SKYLINE_TYPE::LINE_UNKNOWN : return "LINE_UNKNOWN"; break;
		default : return "LINE_UNKNOWN"; break;
	}
}

SKYLINE_TYPE SkyLineMgr::stringToType(const std::string& typeObj)
{
	if (typeObj == "LINE_CIRCLE_POLAR")
		return SKYLINE_TYPE::LINE_CIRCLE_POLAR;

	if (typeObj == "LINE_POINT_POLAR")
		return SKYLINE_TYPE::LINE_POINT_POLAR;

	if (typeObj == "LINE_ECLIPTIC_POLE")
		return SKYLINE_TYPE::LINE_ECLIPTIC_POLE;

	if (typeObj == "LINE_GALACTIC_POLE")
		return SKYLINE_TYPE::LINE_GALACTIC_POLE;

	if (typeObj == "LINE_ANALEMMA")
		return SKYLINE_TYPE::LINE_ANALEMMA;

	if (typeObj == "LINE_ANALEMMALINE")
		return SKYLINE_TYPE::LINE_ANALEMMALINE;

	if (typeObj == "LINE_CIRCUMPOLAR")
		return SKYLINE_TYPE::LINE_CIRCUMPOLAR;

	if (typeObj == "LINE_GALACTIC_CENTER")
		return SKYLINE_TYPE::LINE_GALACTIC_CENTER;

	if (typeObj == "LINE_GALACTIC_EQUATOR")
		return SKYLINE_TYPE::LINE_GALACTIC_EQUATOR;

	if (typeObj == "LINE_VERNAL")
		return SKYLINE_TYPE::LINE_VERNAL;

	if (typeObj == "LINE_ANALEMMA")
		return SKYLINE_TYPE::LINE_ANALEMMA;

	if (typeObj == "LINE_ANALEMMALINE")
		return SKYLINE_TYPE::LINE_ANALEMMALINE;

	if (typeObj == "LINE_GREENWICH")
		return SKYLINE_TYPE::LINE_GREENWICH;

	if (typeObj == "LINE_ARIES")
		return SKYLINE_TYPE::LINE_ARIES;

	if (typeObj == "LINE_MERIDIAN")
		return SKYLINE_TYPE::LINE_MERIDIAN;

	if (typeObj == "LINE_TROPIC")
		return SKYLINE_TYPE::LINE_TROPIC;

	if (typeObj == "LINE_EQUATOR")
		return SKYLINE_TYPE::LINE_EQUATOR;

	if (typeObj == "LINE_ECLIPTIC")
		return SKYLINE_TYPE::LINE_ECLIPTIC;

	if (typeObj == "LINE_PRECESSION")
		return SKYLINE_TYPE::LINE_PRECESSION;

	if (typeObj == "LINE_VERTICAL")
		return SKYLINE_TYPE::LINE_VERTICAL;

	if (typeObj == "LINE_ZENITH")
		return SKYLINE_TYPE::LINE_ZENITH;

	if (typeObj == "LINE_ZODIAC")
		return SKYLINE_TYPE::LINE_ZODIAC;

	return SKYLINE_TYPE::LINE_UNKNOWN;
}

void SkyLineMgr::Create(SKYLINE_TYPE type_obj)
{
	SkyLine* tmp=nullptr;
	auto it=m_map.find(type_obj);

	//si l'itérateur ne vaut pas map.end(), cela signifie que que la clé à été trouvée
	if(it!=m_map.end()) {
		cLog::get()->write("SkyLineMgr already create " + typeToString(type_obj) , LOG_TYPE::L_ERROR);
		return;
	}

	//SKYLINE_TYPE typeObj = stringToType(type_obj);

	//switch (typeObj) {
	switch (type_obj) {
		case SKYLINE_TYPE::LINE_CIRCLE_POLAR :
			cLog::get()->write("SkyLineMgr creating LINE_CIRCLE_POLAR" , LOG_TYPE::L_INFO);
			tmp=new SkyLine_Pole(SkyLine_Pole::POLE,66.5,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case SKYLINE_TYPE::LINE_POINT_POLAR :
			cLog::get()->write("SkyLineMgr creating LINE_POINT_POLAR" , LOG_TYPE::L_INFO);
			tmp=new SkyLine_Pole(SkyLine_Pole::POLE,89,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case SKYLINE_TYPE::LINE_ECLIPTIC_POLE:
			cLog::get()->write("SkyLineMgr creating LINE LINE_ECLIPTIC_POLE" , LOG_TYPE::L_INFO);
			tmp=new SkyLine_Pole(SkyLine_Pole::ECLIPTIC_POLE,89,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case SKYLINE_TYPE::LINE_GALACTIC_POLE :
			cLog::get()->write("SkyLineMgr creating LINE LINE_GALACTIC_POLE" , LOG_TYPE::L_INFO);
			tmp=new SkyLine_Pole(SkyLine_Pole::GALACTIC_POLE,89,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case SKYLINE_TYPE::LINE_ANALEMMA :
			cLog::get()->write("SkyLineMgr creating LINE_ANALEMMA" , LOG_TYPE::L_INFO);
			tmp=new SkyLine_Analemme(SkyLine_Analemme::ANALEMMA,1,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case SKYLINE_TYPE::LINE_ANALEMMALINE :
			cLog::get()->write("SkyLineMgr creating LINE_ANALEMMALINE" , LOG_TYPE::L_INFO);
			tmp=new SkyLine_Analemme(SkyLine_Analemme::ANALEMMALINE,1,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case SKYLINE_TYPE::LINE_CIRCUMPOLAR :
			cLog::get()->write("SkyLineMgr creating LINE_CIRCUMPOLAR" , LOG_TYPE::L_INFO);
			tmp=new SkyLine_CircumPolar(1.0,144);
			m_map[type_obj]= tmp;
			return;
			break;

		case SKYLINE_TYPE::LINE_GALACTIC_CENTER :
			cLog::get()->write("SkyLineMgr creating LINE LINE_GALACTIC_CENTER" , LOG_TYPE::L_INFO);
			tmp=new SkyLine_Galactic_Center(1,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case SKYLINE_TYPE::LINE_VERNAL :
			cLog::get()->write("SkyLineMgr creating LINE_VERNAL" , LOG_TYPE::L_INFO);
			tmp=new SkyLine_Vernal(1,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case SKYLINE_TYPE::LINE_GREENWICH :
			cLog::get()->write("SkyLineMgr creating LINE_GREENWICH" , LOG_TYPE::L_INFO);
			tmp=new SkyLine_Greenwich(1,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case SKYLINE_TYPE::LINE_ARIES :
			cLog::get()->write("SkyLineMgr creating LINE_ARIES" , LOG_TYPE::L_INFO);
			tmp=new SkyLine_Aries(1,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case SKYLINE_TYPE::LINE_EQUATOR :
			cLog::get()->write("SkyLineMgr creating LINE_EQUATOR" , LOG_TYPE::L_INFO);
			tmp=new SkyLine_Equator(SkyLine_Equator::EQUATOR,1,192);
			m_map[type_obj]= tmp;
			return;
			break;

		case SKYLINE_TYPE::LINE_GALACTIC_EQUATOR :
			cLog::get()->write("SkyLineMgr creating LINE_GALACTIC_EQUATOR" , LOG_TYPE::L_INFO);
			tmp=new SkyLine_Equator(SkyLine_Equator::GALACTIC_EQUATOR,1,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case SKYLINE_TYPE::LINE_MERIDIAN :
			cLog::get()->write("SkyLineMgr creating LINE_MERIDIAN" , LOG_TYPE::L_INFO);
			tmp=new SkyLine_Meridian(1, 360);
			m_map[type_obj]= tmp;
			return;
			break;

		case SKYLINE_TYPE::LINE_TROPIC :
			cLog::get()->write("SkyLineMgr creating LINE_TROPIC" , LOG_TYPE::L_INFO);
			tmp=new SkyLine_Tropic(1.0,96);
			m_map[type_obj]= tmp;
			return;
			break;

		case SKYLINE_TYPE::LINE_PRECESSION :
			cLog::get()->write("SkyLineMgr creating LINE_PRECESSION" , LOG_TYPE::L_INFO);
			tmp=new SkyLine_Precession(1.0,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case SKYLINE_TYPE::LINE_VERTICAL :
			cLog::get()->write("SkyLineMgr creating LINE_VERTICAL" , LOG_TYPE::L_INFO);
			tmp=new SkyLine_Vertical(1,180); //always multiples of 18
			m_map[type_obj]= tmp;
			return;
			break;

		case SKYLINE_TYPE::LINE_ZENITH :
			cLog::get()->write("SkyLineMgr creating LINE_ZENITH" , LOG_TYPE::L_INFO);
			tmp=new SkyLine_Zenith(1.0,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case SKYLINE_TYPE::LINE_ZODIAC :
			cLog::get()->write("SkyLineMgr creating LINE_ZODIAC" , LOG_TYPE::L_INFO);
			tmp=new SkyLine_Zodiac(1.0,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case SKYLINE_TYPE::LINE_ECLIPTIC :
			cLog::get()->write("SkyLineMgr creating LINE_MERIDIAN" , LOG_TYPE::L_INFO);
			tmp=new SkyLine_Ecliptic(1.0,72);
			m_map[type_obj]= tmp;
			return;
			break;

		default:
			cLog::get()->write("SkyLineMgr SkyGrid unknown " + typeToString(type_obj) , LOG_TYPE::L_ERROR);
			break;
	}
}
