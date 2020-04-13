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

#include "coreModule/skydisplay_mgr.hpp"
#include "tools/log.hpp"
#include "tools/s_font.hpp"
#include "tools/shader.hpp"

SkyDisplayMgr::SkyDisplayMgr()
{
	//baseColor=Vec3f(0.f, 0.f, 0.f);
	shaderSkyDisplay = new shaderProgram();
	shaderSkyDisplay->init("person.vert", "person.geom", "person.frag");
	shaderSkyDisplay->setUniformLocation("color");
	shaderSkyDisplay->setUniformLocation("fader");
	shaderSkyDisplay->setUniformLocation("Mat");

	SkyDisplay::setShader(shaderSkyDisplay);
}

void SkyDisplayMgr::draw(const Projector *prj,const Navigator *nav, Vec3d equPos, Vec3d oldEquPos)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		it->second->draw(prj, nav, equPos, oldEquPos);
	}
}

void SkyDisplayMgr::drawPerson(const Projector *prj,const Navigator *nav)
{
	if ((personAL!=nullptr) && (personEQ!=nullptr)) {
		personAL->draw(prj,nav);
		personEQ->draw(prj,nav);
	}
}

SkyDisplayMgr::~SkyDisplayMgr()
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		cLog::get()->write("SkyDisplayMgr : delete " + getSkyName(it->first), LOG_TYPE::L_INFO);
		delete it->second;
	}
	if (shaderSkyDisplay != nullptr)
		delete shaderSkyDisplay;
	if (skyDisplayFont != nullptr)
		delete skyDisplayFont;
}

void SkyDisplayMgr::update(int delta_time)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		it->second->update(delta_time);
	}
}

void SkyDisplayMgr::setFont(float font_size, const std::string& font_name)
{
	skyDisplayFont = new s_font(font_size, font_name);
	SkyDisplay::setFont(skyDisplayFont);
	// for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
	// 	it->second->setFont(font_size, font_name);
	// }
}


void SkyDisplayMgr::clear(SKYDISPLAY_NAME nameObj)
{
	auto it=m_map.find(nameObj);
	if(it!=m_map.end()) {
		it->second->clear();
		return;
	}
	cLog::get()->write("SkyDisplayMgr error : clear not found " + getSkyName(nameObj), LOG_TYPE::L_WARNING);
}

void SkyDisplayMgr::loadData(SKYDISPLAY_NAME nameObj, const std::string& filename)
{
	auto it=m_map.find(nameObj);
	if(it!=m_map.end()) {
		it->second->loadData(filename);
		return;
	}
	cLog::get()->write("SkyDisplayMgr error : loadData not found " + getSkyName(nameObj), LOG_TYPE::L_WARNING);
}

void SkyDisplayMgr::flipFlagShow(SKYDISPLAY_NAME nameObj)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==nameObj) {
			it->second->flipFlagShow();
			return;
		}
	}
	cLog::get()->write("SkyDisplayMgr error : flipFlagShow not found " + getSkyName(nameObj), LOG_TYPE::L_WARNING);
}


void SkyDisplayMgr::setFlagShow(SKYDISPLAY_NAME nameObj, bool a)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==nameObj) {
			it->second->setFlagShow(a);
			return;
		}
	}
	cLog::get()->write("SkyDisplayMgr error : setFlagShow not found " + getSkyName(nameObj), LOG_TYPE::L_WARNING);
}


bool SkyDisplayMgr::getFlagShow(SKYDISPLAY_NAME nameObj)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==nameObj) {
			return it->second->getFlagShow();
		}
	}
	cLog::get()->write("SkyDisplayMgr error : getFlagShow not found " + getSkyName(nameObj), LOG_TYPE::L_WARNING);
	return false;
}


void SkyDisplayMgr::setColor(SKYDISPLAY_NAME nameObj, const Vec3f& c)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==nameObj) {
			it->second->setColor(c);
			break;
		}
	}
}

const Vec3f& SkyDisplayMgr::getColor(SKYDISPLAY_NAME nameObj)
{
	for (auto it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==nameObj) {
			return it->second->getColor();
		}
	}
	cLog::get()->write("SkyDisplayMgr error : getColor not found " + getSkyName(nameObj), LOG_TYPE::L_WARNING);
	return baseColor;
}

void SkyDisplayMgr::Create(SKYDISPLAY_NAME nameObj)
{
	SkyDisplay* tmp=nullptr;
	auto it=m_map.find(nameObj);

	//si l'itérateur ne vaut pas map.end(), cela signifie que que la clé à été trouvée
	if(it!=m_map.end()) {
		cLog::get()->write("SkyDisplayMgr SkyGrid already create " + getSkyName(nameObj), LOG_TYPE::L_ERROR);
		return;
	}
	cLog::get()->write("SkyDisplayMgr creating "+ getSkyName(nameObj), LOG_TYPE::L_INFO);

	//switch (nameObj) {
	switch (nameObj) {
		case SKYDISPLAY_NAME::SKY_PERSONAL :
			tmp= new SkyPerson(SkyDisplay::AL);
			m_map[nameObj]= tmp;
			personAL = tmp;
			return;
			break;

		case SKYDISPLAY_NAME::SKY_PERSONEQ :
			tmp= new SkyPerson(SkyDisplay::EQ);
			m_map[nameObj]= tmp;
			personEQ = tmp;
			return;
			break;

		case SKYDISPLAY_NAME::SKY_NAUTICAL :
			tmp= new SkyNautic(SkyDisplay::AL);
			m_map[nameObj]= tmp;
			return;
			break;

		case SKYDISPLAY_NAME::SKY_NAUTICEQ :
			tmp= new SkyNautic(SkyDisplay::EQ);
			m_map[nameObj]= tmp;
			return;
			break;

		case SKYDISPLAY_NAME::SKY_OBJCOORDS :
			tmp= new SkyCoords();
			m_map[nameObj]= tmp;
			return;
			break;

		case SKYDISPLAY_NAME::SKY_MOUSECOORDS :
			tmp= new SkyCoords();
			m_map[nameObj]= tmp;
			return;
			break;

		case SKYDISPLAY_NAME::SKY_ANGDIST :
			tmp= new SkyAngDist();
			m_map[nameObj]= tmp;
			return;
			break;

		case SKYDISPLAY_NAME::SKY_LOXODROMY :
			tmp= new SkyLoxodromy();
			m_map[nameObj]= tmp;
			return;
			break;

		case SKYDISPLAY_NAME::SKY_ORTHODROMY :
			tmp= new SkyOrthodromy();
			m_map[nameObj]= tmp;
			return;
			break;

		default: // inconnue
			break;
	}
}

std::string SkyDisplayMgr::getSkyName(SKYDISPLAY_NAME nameObj)
{
	switch (nameObj) {
		case SKYDISPLAY_NAME::SKY_PERSONAL :
			return "SkyPersonAL";
			break;
		case SKYDISPLAY_NAME::SKY_PERSONEQ :
			return "SkyPersonEQ";
			break;
		case SKYDISPLAY_NAME::SKY_NAUTICAL :
			return "SkyNauticAL";
			break;
		case SKYDISPLAY_NAME::SKY_NAUTICEQ :
			return "SkyNauticEQ";
			break;
		case SKYDISPLAY_NAME::SKY_ORTHODROMY:
			return "SkyOrthodromy";
			break;
		case SKYDISPLAY_NAME::SKY_LOXODROMY :
			return "SkyLoxodromy";
			break;
		case SKYDISPLAY_NAME::SKY_OBJCOORDS:
			return "SkyObjCoords";
			break;
		case SKYDISPLAY_NAME::SKY_MOUSECOORDS:
			return "SkyMouseCoords";
			break;
		case SKYDISPLAY_NAME::SKY_ANGDIST:
			return "SkyAngDist";
			break;
		default:
			return "None";
			break;
	}
}
