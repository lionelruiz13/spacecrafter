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

#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

#define MAX_HINT 1024

NebulaMgr::NebulaMgr() : tex_NEBULA(nullptr),
circleScale(1.f), circleColor(Vec3f(0.2,0.2,1.0)), labelColor(v3fNull),
flagBright(false), displaySpecificHint(false),
dsoPictoSize(6)
{
	nebGrid.subdivise(3);
	if (! initTexPicto())
		cLog::get()->write("DSO: error while loading pictogram texture", LOG_TYPE::L_ERROR);

	//createShaderHint();
	createSC_context();
	Nebula::createSC_context();
}

NebulaMgr::~NebulaMgr()
{
}

void NebulaMgr::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;

	m_hintGL = std::make_unique<VertexArray>(vkmgr);
	m_hintGL->createBindingEntry(7*sizeof(float));
	m_hintGL->addInput(VK_FORMAT_R32G32_SFLOAT);
	m_hintGL->addInput(VK_FORMAT_R32G32_SFLOAT);
	m_hintGL->addInput(VK_FORMAT_R32G32B32_SFLOAT);
	vertexHint = m_hintGL->createBuffer(0, MAX_HINT * 4, context.globalBuffer.get());
	indexHint = context.indexBufferMgr->acquireBuffer(MAX_HINT * 6 * sizeof(uint16_t));
	uint16_t *tmpIndex = (uint16_t *) context.transfer->planCopy(indexHint);
	for (int i = 0; i < MAX_HINT * 4; i += 4) {
		*(tmpIndex++) = i + 0;
		*(tmpIndex++) = i + 1;
		*(tmpIndex++) = i + 2;

		*(tmpIndex++) = i + 2;
		*(tmpIndex++) = i + 1;
		*(tmpIndex++) = i + 3;
	}
	layoutHint = std::make_unique<PipelineLayout>(vkmgr);
	layoutHint->setGlobalPipelineLayout(context.layouts.front().get());
	layoutHint->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
	layoutHint->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4);
	layoutHint->buildLayout();
	layoutHint->build();
	pipelineHint = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_BACKGROUND, layoutHint.get());
	pipelineHint->setDepthStencilMode();
	pipelineHint->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineHint->bindVertex(*m_hintGL);
	pipelineHint->bindShader("nebulaHint.vert.spv");
	pipelineHint->bindShader("nebulaHint.frag.spv");
	pipelineHint->build();
	setHint = std::make_unique<Set>(vkmgr, *context.setMgr, layoutHint.get());
	setHint->bindTexture(tex_NEBULA->getTexture(), 0);
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

// Clear user added nebula
void NebulaMgr::removeNebula(const std::string& name, bool showOriginal=true)
{
	std::string uname = name;
	transform(uname.begin(), uname.end(), uname.begin(), ::toupper);

	for (auto iter = nebGrid.rawBegin(); iter != nebGrid.end(); ++iter) {
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
			nebGrid.erase(iter);
			return;
		}
	}
	cLog::get()->write("DSO: Requested nebula to delete not found " + name, LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
}

// remove all user added nebula and make standard nebulae visible again
// all standard nebulae visible become again selected
void NebulaMgr::removeSupplementalNebulae()
{
	nebGrid.remove_if([](auto &n){
		if (!n->isDeletable()) {
			n->show();
			n->select();
			return false;
		} else {
			return true;
		}
	});
}

// Draw all the Nebulae
void NebulaMgr::draw(const Projector* prj, const Navigator * nav, ToneReproductor* eye, double sky_brightness)
{
	if(!fader) return;

	Nebula::setHintsBrightness(hintsFader.getInterstate());
	Nebula::setNebulaBrightness(fader.getInterstate());
	Nebula::setTextBrightness(textFader.getInterstate());

	//cout << "Draw Nebulaes" << endl;

	// StateGL::enable(GL_BLEND);
	// StateGL::BlendFunc(GL_ONE, GL_ONE);

	Vec3f pXYZ;

	// Find the star zones which are in the screen
	// FOV is currently measured vertically, so need to adjust for wide screens
	// TODO: projector should probably use largest measurement itself
	float max_fov = std::max( prj->getFov(), prj->getFov()*prj->getViewportWidth()/prj->getViewportHeight());
	nebGrid.intersect(nav->getPrecEquVision(), max_fov*M_PI/180.f*1.2f);

	//~ prj->set_orthographic_projection();	// set 2D coordinate

	// Print all the stars of all the selected zones
	// speed up the computation of n->getOnScreenSize(prj, nav)>5:
	const float size_limit = 5.0 * (M_PI/180.0) * (prj->getFov()/prj->getViewportHeight());
	Vec3d win;

	float *data = (float *) Context::instance->transfer->beginPlanCopy(vertexHint->get().size);
	nbDraw = 0;
	Nebula::beginDraw(prj);
	for (const auto &n : nebGrid) {
		// improve performance by skipping if too small to see
		if ( n->getAngularSize()>size_limit|| (hintsFader.getInterstate()>0.0001 && n->getMag() <= getMaxMagHints())) {
			// Refactor this by refactoring projectJ2000 and his dependencies
			//prj->projectJ2000(n->XYZ_, win);
			n->setXY(prj);

			if (n->getAngularSize()>size_limit) {
				n->drawTex(prj, nav, eye, sky_brightness, flagBright);
			}

			if (textFader) {
				if (isolateSelected) {
					const std::string nebula_name = n->getNameI18n();
					if (selected_nebulas.find(nebula_name) != selected_nebulas.end()) {
						n->drawName(prj, labelColor, font);
					}
				} else
					n->drawName(prj, labelColor, font);
			}

			//~ cout << "drawhint " << n->getEnglishName() << endl;
			if ( n->getAngularSize()<size_limit && nbDraw < MAX_HINT) {
				if (!displaySpecificHint)
					n->drawHint(prj, nav, data, nbDraw, displaySpecificHint, circleColor, getPictoSize());
				else {
					bool displayPicto = false;
					if (isolateSelected) {
						const std::string nebula_name = n->getNameI18n();
						if (selected_nebulas.find(nebula_name) != selected_nebulas.end())
							displayPicto = true;
					}
					else
						displayPicto = true;
					n->drawHint(prj, nav, data, nbDraw, displayPicto, circleColor, getPictoSize());
				}
			}
		}
	}
	Nebula::endDraw();
	drawAllHint(prj);
}

void NebulaMgr::drawAllHint(const Projector* prj)
{
	Context &context = *Context::instance;
	context.transfer->endPlanCopy(vertexHint->get(), nbDraw * 4 * (7 * sizeof(float)));
	if(nbDraw==0)
		return;

	if (cmds[context.frameIdx] == -1) {
		cmds[context.frameIdx] = context.frame[context.frameIdx]->create(1);
		context.frame[context.frameIdx]->setName(cmds[context.frameIdx], "NebulaHint " + std::to_string(context.frameIdx));
	}
	VkCommandBuffer cmd = context.frame[context.frameIdx]->begin(cmds[context.frameIdx], PASS_BACKGROUND);

	pipelineHint->bind(cmd);
	layoutHint->bindSets(cmd, {*context.uboSet, *setHint});
	float fader = hintsFader.getInterstate();
	layoutHint->pushConstant(cmd, 0, &fader);
	vertexHint->bind(cmd);
	vkCmdBindIndexBuffer(cmd, indexHint.buffer, indexHint.offset, VK_INDEX_TYPE_UINT16);
	vkCmdDrawIndexed(cmd, nbDraw * 6, 1, 0, 0, 0);
	context.frame[context.frameIdx]->compile(cmd);
	context.frame[context.frameIdx]->toExecute(cmd, PASS_BACKGROUND);
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

	auto iter = std::find_if(nebGrid.rawBegin(), nebGrid.end(), [uname, search_hidden](const auto &n){
		std::string testName = n->getEnglishName();
		transform(testName.begin(), testName.end(), testName.begin(), ::toupper);
		return (testName==uname && (n->isHidden()==false || search_hidden));
	});

	if (iter != nebGrid.end())
		return (*iter).get();
	return nullptr;
}

void NebulaMgr::showAll()
{
	for (auto iter = nebGrid.rawBegin(); iter != nebGrid.end(); ++iter) {
		(*iter)->select();
	}
}

void NebulaMgr::hideAll()
{
	for (auto iter = nebGrid.rawBegin(); iter != nebGrid.end(); ++iter) {
		(*iter)->unselect();
	}
}

void NebulaMgr::selectConstellation(bool hide, std::string constellationName)
{
	std::for_each(nebGrid.rawBegin(), nebGrid.end(), [hide, constellationName](const auto &n){
		if (n->getConstellation() == constellationName)
			hide ? n->unselect() : n->select();
	});
}

void NebulaMgr::selectName(bool hide, std::string Name)
{
	std::for_each(nebGrid.rawBegin(), nebGrid.end(), [hide, Name](const auto &n){
		if (n->getEnglishName() == Name)
			hide ? n->unselect() : n->select();
	});
}

void NebulaMgr::selectType(bool hide, std::string dsoType)
{
	std::for_each(nebGrid.rawBegin(), nebGrid.end(), [hide, dsoType](const auto &n){
		if (n->getStringType() == dsoType)
			hide ? n->unselect() : n->select();
	});
}

// Look for a nebulae by XYZ coords
Object NebulaMgr::search(Vec3f Pos)
{
	Pos.normalize();
	Nebula * plusProche=nullptr;
	float anglePlusProche=0.;
	for (auto iter = nebGrid.rawBegin(); iter != nebGrid.end(); ++iter) {
		if((*iter)->isHidden()==true) continue;
		if ((*iter)->XYZ_[0]*Pos[0]+(*iter)->XYZ_[1]*Pos[1]+(*iter)->XYZ_[2]*Pos[2]>anglePlusProche) {
			anglePlusProche=(*iter)->XYZ_[0]*Pos[0]+(*iter)->XYZ_[1]*Pos[1]+(*iter)->XYZ_[2]*Pos[2];
			plusProche=(*iter).get();
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

	for (auto &n : nebGrid) {
		equPos = n->XYZ_;
		equPos.normalize();
		if (equPos[0]*v[0] + equPos[1]*v[1] + equPos[2]*v[2]>=cos_lim_fov) {
			// NOTE: non-labeled nebulas are not returned!
			// Otherwise cursor select gets invisible nebulas - Rob
			if (n->getNameI18n() != "" && n->isHidden()==false) result.push_back(n.get());
		}
	}
	return result;
}

void NebulaMgr::setSelected(Object obj) {
	auto it = selected_nebulas.find(obj.getNameI18n());
	if (it != selected_nebulas.end()) {
		selected_nebulas.erase(it);
	} else {
		selected_nebulas.insert(std::pair<std::string, bool>(obj.getNameI18n(), true));
	}
}

bool NebulaMgr::loadDeepskyObject(std::string _englishName, std::string _DSOType, std::string _constellation, float _ra, float _de, float _mag, float _size, std::string _classe,
                                  float _distance, std::string tex_name, bool path, float tex_angular_size, float _rotation, std::string _credit, float _luminance, bool deletable)
{
	Nebula *e = searchNebula(_englishName, false);
	if (e) {
		if(e->isDeletable()) {
			//~ cout << "Warning: replacing user added nebula with name " << name << ".\n";
			cLog::get()->write("nebula: replacing user added nebula with name " + _englishName, LOG_TYPE::L_WARNING);
			removeNebula(_englishName, false);
		} else {
			e->hide();
			cLog::get()->write("dso: hide nebula with name " + _englishName, LOG_TYPE::L_WARNING);
		}
	}
	auto neb = std::make_unique<Nebula>(_englishName, _DSOType, _constellation, _ra, _de, _mag, _size, _classe, _distance, tex_name, path,
	               tex_angular_size, _rotation, _credit, _luminance, deletable, false);

	if (neb != nullptr) {
		nebGrid.insert(std::move(neb), neb->XYZ_, neb->getAngularSize()/2.f);
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
	for (auto iter = nebGrid.rawBegin(); iter != nebGrid.end(); ++iter) {
		(*iter)->translateName(trans);
	}
	if(font) font->clearCache();
}


//! Return the matching Nebula object's pointer if exists or NULL
Object NebulaMgr::searchByNameI18n(const std::string& nameI18n) const
{
	std::string objw = nameI18n;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);

	// Search by common names
	auto iter = std::find_if(nebGrid.rawBegin(), nebGrid.end(), [objw](const auto &n){
		if(n->isHidden()==true) return false;
		std::string objwcap = n->getNameI18n();
		transform(objwcap.begin(), objwcap.end(), objwcap.begin(), ::toupper);
		if (objwcap==objw) return true;
		return false;
	});
	if (iter != nebGrid.end())
		return (*iter).get();
	return nullptr;
}

//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
std::vector<std::string> NebulaMgr::listMatchingObjectsI18n(const std::string& objPrefix, unsigned int maxNbItem) const
{
	std::vector<std::string> result;
	if (maxNbItem==0) return result;

	std::string objw = objPrefix;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);

	// Search by common names
	for (auto iter = nebGrid.rawBegin(); iter != nebGrid.end(); ++iter) {
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
