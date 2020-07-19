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

#include <fstream>
#include <algorithm>
#include "coreModule/nebula_mgr.hpp"
#include "coreModule/nebula.hpp"
#include "tools/s_texture.hpp"
#include "tools/s_font.hpp"
#include "navModule/navigator.hpp"
#include "tools/translator.hpp"
#include "tools/log.hpp"
#include "tools/OpenGL.hpp"
#include "tools/shader.hpp"



NebulaMgr::NebulaMgr(void) : tex_NEBULA(nullptr), nebulaFont(nullptr),
circleScale(1.f), circleColor(Vec3f(0.2,0.2,1.0)), labelColor(v3fNull),
flagBright(false), displaySpecificHint(false),
dsoPictoSize(6)
{
	nebZones = new std::vector<Nebula*>[nebGrid.getNbPoints()];
	if (! initTexPicto())
		cLog::get()->write("DSO: error while loading pictogram texture", LOG_TYPE::L_ERROR);

	createShaderHint();
	createSC_context();
	Nebula::createSC_context();
}

NebulaMgr::~NebulaMgr()
{
	std::vector<Nebula *>::iterator iter;
	for (iter=neb_array.begin(); iter!=neb_array.end(); iter++) {
		delete (*iter);
	}

	if (tex_NEBULA) delete tex_NEBULA;
	tex_NEBULA = nullptr;

	if (nebulaFont) delete nebulaFont;
	nebulaFont = nullptr;

	delete[] nebZones;
}


void NebulaMgr::createShaderHint()
{
	shaderNebulaHint = std::make_unique<shaderProgram>();
	shaderNebulaHint->init("nebulaHint.vert","nebulaHint.frag");
	shaderNebulaHint->setUniformLocation("fader");
}


void NebulaMgr::createSC_context()
{
	m_hintGL = std::make_unique<VertexArray>();
	m_hintGL->registerVertexBuffer(BufferType::POS2D, BufferAccess::DYNAMIC);
	m_hintGL->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::DYNAMIC);
	m_hintGL->registerVertexBuffer(BufferType::COLOR, BufferAccess::DYNAMIC);
}


bool NebulaMgr::initTexPicto()
{
	bool succes = true;

	if (!tex_NEBULA)
		tex_NEBULA = new s_texture("tex_nebulaes.png");
	if (tex_NEBULA == nullptr)	succes=false;

	return succes;
}

// read from file
bool NebulaMgr::loadDeepskyObject(const std::string& cat)
{
	return loadDeepskyObjectFromCat(cat);
}

// read from stream
void NebulaMgr::setFont(float font_size, const std::string& font_name)
{
	if (nebulaFont) {
		delete nebulaFont;
		nebulaFont= nullptr;
	}

	nebulaFont = new s_font(font_size, font_name); // load Font
	if (!nebulaFont) {
		cLog::get()->write("Nebula: Can't create nebulaFont\n", LOG_TYPE::L_ERROR);
		assert(nebulaFont);
	}
}

// Clear user added nebula
void NebulaMgr::removeNebula(const std::string& name, bool showOriginal=true)
{
	std::string uname = name;
	transform(uname.begin(), uname.end(), uname.begin(), ::toupper);
	std::vector <Nebula*>::iterator iter;
	std::vector <Nebula*>::iterator iter2;

	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		std::string testName = (*iter)->getEnglishName();
		//std::cout << testName << std::endl;
		transform(testName.begin(), testName.end(), testName.begin(), ::toupper);

		// if(testName != "" ) cout << ">" << testName << "< " << endl;
		if (testName==uname) {
			//std::cout << testName << "=" << uname << std::endl;
			if(!(*iter)->isDeletable()) {
				if(showOriginal) (*iter)->show(); // make sure original is now visible
				return;
			}

			// erase from locator grid
			int zone = nebGrid.GetNearest((*iter)->XYZ_);
			for (iter2 = nebZones[zone].begin(); iter2!=nebZones[zone].end(); ++iter2) {
				if(*iter2 == *iter) {
//					cerr << "Deleting nebula from zone " << zone << " with name " << (*iter2)->englishName << endl;
					//std::cout << testName << " delete from iter2" << std::endl;
					nebZones[zone].erase(iter2);
					break;
				}
			}

			// Delete nebula
			delete *iter;
//			std::cout << testName << " delete" << std::endl;
			neb_array.erase(iter);
			return;
			//cerr << "Erased nebula " << uname << endl;
			//return "";
		}
	}
	cLog::get()->write("DSO: Requested nebula to delete not found " + name, LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
}

// remove all user added nebula and make standard nebulae visible again
// all standard nebulae visible become again selected
void NebulaMgr::removeSupplementalNebulae()
{

	std::vector<Nebula *>::iterator iter;
	std::vector<Nebula *>::iterator iter2;

	for (iter=neb_array.begin(); iter!=neb_array.end(); /*iter++*/) {

		if (!(*iter)->isDeletable()) {
			(*iter)->show();
			(*iter)->select();
			iter++;
		} else {

			// erase from locator grid
			int zone = nebGrid.GetNearest((*iter)->XYZ_);

			for (iter2 = nebZones[zone].begin(); iter2!=nebZones[zone].end(); ++iter2) {
				if(*iter2 == *iter) {
//					cerr << "Deleting nebula from zone " << zone << " with name " << (*iter2)->englishName << endl;
					nebZones[zone].erase(iter2);
					break;
				}
			}

			// Delete nebula
			delete *iter;
			iter=neb_array.erase(iter);
			//iter--;
			// cerr << "Erased nebula " << uname << endl;
		}
	}

	// return "";
}

// Draw all the Nebulae
void NebulaMgr::draw(const Projector* prj, const Navigator * nav, ToneReproductor* eye, double sky_brightness)
{
	if(!showFader) return;

	Nebula::setHintsBrightness(hintsFader.getInterstate());
	Nebula::setNebulaBrightness(showFader.getInterstate());
	Nebula::setTextBrightness(textFader.getInterstate());

	//cout << "Draw Nebulaes" << endl;

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_ONE, GL_ONE);

	Vec3f pXYZ;

	// Find the star zones which are in the screen
	int nbZones=0;
	// FOV is currently measured vertically, so need to adjust for wide screens
	// TODO: projector should probably use largest measurement itself
	float max_fov = std::max( prj->getFov(), prj->getFov()*prj->getViewportWidth()/prj->getViewportHeight());
	nbZones = nebGrid.Intersect(nav->getPrecEquVision(), max_fov*M_PI/180.f*1.2f);
	static int * zoneList = nebGrid.getResult();

	//~ prj->set_orthographic_projection();	// set 2D coordinate

	// Print all the stars of all the selected zones
	static std::vector<Nebula *>::iterator end;
	static std::vector<Nebula *>::iterator iter;
	Nebula* n;

	// speed up the computation of n->getOnScreenSize(prj, nav)>5:
	const float size_limit = 5.0 * (M_PI/180.0) * (prj->getFov()/prj->getViewportHeight());
	Vec3d win;

	for (int i=0; i<nbZones; ++i) {
		end = nebZones[zoneList[i]].end();
		for (iter = nebZones[zoneList[i]].begin(); iter!=end; ++iter) {

			n = *iter;

			// improve performance by skipping if too small to see
			if ( n->getAngularSize()>size_limit|| (hintsFader.getInterstate()>0.0001 && n->getMag() <= getMaxMagHints())) {
				// Refactor this by refactoring projectJ2000 and his dependencies
				//prj->projectJ2000(n->XYZ_, win);
				n->setXY(prj);

				if (n->getAngularSize()>size_limit) {
					n->drawTex(prj, nav, eye, sky_brightness, flagBright);
				}

				if (textFader) {
					n->drawName(prj, labelColor, nebulaFont);
				}

				//~ cout << "drawhint " << n->getEnglishName() << endl;
				if ( n->getAngularSize()<size_limit)
					n->drawHint(prj, nav, vecHintPos, vecHintTex, vecHintColor, displaySpecificHint, circleColor, getPictoSize());
			}
		}
	}
	drawAllHint(prj);
}

void NebulaMgr::drawAllHint(const Projector* prj)
{
	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode


	glBindTexture (GL_TEXTURE_2D, tex_NEBULA->getID());

	if(vecHintPos.size()==0)
		return;

	shaderNebulaHint->use();
	shaderNebulaHint->setUniform("fader", hintsFader.getInterstate());

	m_hintGL->fillVertexBuffer(BufferType::POS2D, vecHintPos);
	m_hintGL->fillVertexBuffer(BufferType::TEXTURE, vecHintTex);
	m_hintGL->fillVertexBuffer(BufferType::COLOR, vecHintColor);

	// m_hintGL->bind();
	// for(unsigned int i=0; i < (vecHintPos.size()/8) ; i++)
	// 	glDrawArrays(GL_TRIANGLE_STRIP, 4*i, 4);
	// m_hintGL->unBind();
	// shaderNebulaHint->unuse();
	Renderer::drawMultiArrays(shaderNebulaHint.get(), m_hintGL.get(), GL_TRIANGLE_STRIP, vecHintPos.size()/8, 4);

	vecHintPos.clear();
	vecHintTex.clear();
	vecHintColor.clear();
}

// search by name
Object NebulaMgr::search(const std::string& name)
{
	Nebula *n = searchNebula(name, false);
	return Object(n);
}

// search by name
Nebula *NebulaMgr::searchNebula(const std::string& name, bool search_hidden=false)
{

	std::string uname = name;
	transform(uname.begin(), uname.end(), uname.begin(), ::toupper);
	std::vector <Nebula*>::const_iterator iter;

	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		std::string testName = (*iter)->getEnglishName();
		transform(testName.begin(), testName.end(), testName.begin(), ::toupper);
		//		if(testName != "" ) cout << ">" << testName << "< " << endl;
		if (testName==uname  && ((*iter)->isHidden()==false || search_hidden)) return *iter;
	}
	return nullptr;
}

void NebulaMgr::showAll()
{
	std::vector <Nebula*>::const_iterator iter;
	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		(*iter)->select();
	}
}

void NebulaMgr::hideAll()
{
	std::vector <Nebula*>::const_iterator iter;
	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		(*iter)->unselect();
	}
}

void NebulaMgr::selectConstellation(bool hide, std::string constellationName)
{
	std::vector <Nebula*>::const_iterator iter;
	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		if ((*iter)->getConstellation() == constellationName)
			hide ? (*iter)-> unselect() : (*iter)->select();
	}
}

void NebulaMgr::selectName(bool hide, std::string Name)
{
	std::vector <Nebula*>::const_iterator iter;
	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		if ((*iter)->getEnglishName() == Name)
			hide ? (*iter)-> unselect() : (*iter)->select();
	}
}

void NebulaMgr::selectType(bool hide, std::string dsoType)
{
	std::vector <Nebula*>::const_iterator iter;
	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		if ((*iter)->getStringType() == dsoType)
			hide ? (*iter)-> unselect() : (*iter)->select();
	}
}

// Look for a nebulae by XYZ coords
Object NebulaMgr::search(Vec3f Pos)
{
	Pos.normalize();
	std::vector<Nebula *>::iterator iter;
	Nebula * plusProche=nullptr;
	float anglePlusProche=0.;
	for (iter=neb_array.begin(); iter!=neb_array.end(); iter++) {
		if((*iter)->isHidden()==true) continue;
		if ((*iter)->XYZ_[0]*Pos[0]+(*iter)->XYZ_[1]*Pos[1]+(*iter)->XYZ_[2]*Pos[2]>anglePlusProche) {
			anglePlusProche=(*iter)->XYZ_[0]*Pos[0]+(*iter)->XYZ_[1]*Pos[1]+(*iter)->XYZ_[2]*Pos[2];
			plusProche=(*iter);
		}
	}
	if (anglePlusProche>Nebula::dsoRadius*0.999) {
		return plusProche;
	} else return nullptr;
}

// Return a stl vector containing the nebulas located inside the lim_fov circle around position v
std::vector<Object> NebulaMgr::searchAround(Vec3d v, double lim_fov) const
{
	std::vector<Object> result;
	v.normalize();
	double cos_lim_fov = cos(lim_fov * M_PI/180.);
	static Vec3d equPos;

	std::vector<Nebula*>::const_iterator iter = neb_array.begin();
	while (iter != neb_array.end()) {
		equPos = (*iter)->XYZ_;
		equPos.normalize();
		if (equPos[0]*v[0] + equPos[1]*v[1] + equPos[2]*v[2]>=cos_lim_fov) {

			// NOTE: non-labeled nebulas are not returned!
			// Otherwise cursor select gets invisible nebulas - Rob
			if ((*iter)->getNameI18n() != "" && (*iter)->isHidden()==false) result.push_back(*iter);
		}
		iter++;
	}
	return result;
}

bool NebulaMgr::loadDeepskyObject(std::string _englishName, std::string _DSOType, std::string _constellation, float _ra, float _de, float _mag, float _size, std::string _classe,
                                  float _distance, std::string tex_name, bool path, float tex_angular_size, float _rotation, std::string _credit, float _luminance, bool deletable)
{
	Nebula *e = searchNebula(_englishName, false);
	if(e) {
		if(e->isDeletable()) {
			//~ cout << "Warning: replacing user added nebula with name " << name << ".\n";
			cLog::get()->write("nebula: replacing user added nebula with name " + _englishName, LOG_TYPE::L_WARNING);
			removeNebula(_englishName, false);
		} else {
			e->hide();
			cLog::get()->write("dso: hide nebula with name " + _englishName, LOG_TYPE::L_WARNING);
		}
	}
	e = new Nebula(_englishName, _DSOType, _constellation, _ra, _de, _mag, _size, _classe, _distance, tex_name, path,
	               tex_angular_size, _rotation, _credit, _luminance, deletable, false);

	if (e != nullptr) {
		neb_array.push_back(e);
		nebZones[nebGrid.GetNearest(e->XYZ_)].push_back(e);
		return true;
	} else
		return false;
}


// read from file
bool NebulaMgr::loadDeepskyObjectFromCat(const std::string& cat)
{
	std::string recordstr;
	unsigned int i=0;
	unsigned int data_drop=0;

	//~ cout << "Loading NGC data... ";
	cLog::get()->write("Loading NGC data... ",LOG_TYPE::L_INFO);
	std::ifstream  ngcFile(cat);
	if (!ngcFile) {
		//~ cout << "NGC data file " << catNGC << " not found" << endl;
		cLog::get()->write("NGC data file " + cat + " not found""Loading NGC data... ",LOG_TYPE::L_ERROR);
		return false;
	}

	std::string name, type, constellation, deep_class, credits, tex_name;
	float ra, de, mag, tex_angular_size, distance, scale, tex_rotation, texLuminanceAdjust;

	// Read the cat entries
	while ( getline (ngcFile, recordstr )) {

		std::istringstream istr(recordstr);
		if (!(istr >> name >> type >> constellation >> ra >> de >> mag >> scale >> deep_class
		        >>  distance >> tex_name >> tex_angular_size >> tex_rotation >> credits >> texLuminanceAdjust )) {
			data_drop++;
		} else {
			if ( ! loadDeepskyObject(name, type, constellation, ra, de, mag, scale, deep_class, distance,
			                         tex_name, false, tex_angular_size, tex_rotation, credits, texLuminanceAdjust,false)) {
				//printf("error creating nebula\n");
				data_drop++;
			}
			i++;
		}
	}
	ngcFile.close();
	cLog::get()->write("Nebula: "+ std::to_string(i) + " items loaded, " + std::to_string(data_drop) + " dropped", LOG_TYPE::L_INFO);
	return true;
}


//! @brief Update i18 names from english names according to passed translator
//! The translation is done using gettext with translated strings defined in translations.h
void NebulaMgr::translateNames(Translator& trans)
{
	std::vector<Nebula*>::iterator iter;
	for ( iter = neb_array.begin(); iter < neb_array.end(); iter++ ) {
		(*iter)->translateName(trans);
	}
	if(nebulaFont) nebulaFont->clearCache();
}


//! Return the matching Nebula object's pointer if exists or NULL
Object NebulaMgr::searchByNameI18n(const std::string& nameI18n) const
{
	std::string objw = nameI18n;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);
	std::vector <Nebula*>::const_iterator iter;

	// Search by common names
	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		if((*iter)->isHidden()==true) continue;
		std::string objwcap = (*iter)->getNameI18n();
		transform(objwcap.begin(), objwcap.end(), objwcap.begin(), ::toupper);
		if (objwcap==objw) return *iter;
	}

	return nullptr;
}

//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
std::vector<std::string> NebulaMgr::listMatchingObjectsI18n(const std::string& objPrefix, unsigned int maxNbItem) const
{
	std::vector<std::string> result;
	if (maxNbItem==0) return result;

	std::string objw = objPrefix;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);

	std::vector <Nebula*>::const_iterator iter;

	// Search by common names
	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		if((*iter)->isHidden()==true) continue;
		std::string constw = (*iter)->getNameI18n().substr(0, objw.size());
		transform(constw.begin(), constw.end(), constw.begin(), ::toupper);
		if (constw==objw) {
			result.push_back((*iter)->getNameI18n());
		}
	}

	sort(result.begin(), result.end());
	if (result.size()>maxNbItem) result.erase(result.begin()+maxNbItem, result.end());

	return result;
}
