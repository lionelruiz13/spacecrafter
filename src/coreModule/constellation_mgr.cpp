/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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

// Class used to manage group of constellation

#include <iostream>
#include <fstream>
#include <vector>

#include "coreModule/constellation_mgr.hpp"
#include "coreModule/constellation.hpp"
#include "coreModule/projector.hpp"
#include "starModule/hip_star_mgr.hpp"
#include "starModule/hip_star.hpp"
#include "tools/utility.hpp"
#include "tools/s_font.hpp"
#include "tools/log.hpp"
#include "tools/fmath.hpp"
#include "tools/stateGL.hpp"
#include "tools/translator.hpp"



//! constructor which loads all data from appropriate files
ConstellationMgr::ConstellationMgr(HipStarMgr *_hip_stars) :
	asterFont(nullptr),
	hipStarMgr(_hip_stars),
	flagNames(0),
	flagLines(0),
	flagArt(0),
	flagBoundaries(0)
{
	assert(hipStarMgr);
	isolateSelected = false;

	createShader();
}

void ConstellationMgr::createShader()
{
//ART
	shaderArt = new shaderProgram();
	shaderArt->init("constellationArt.vert", "constellationArt.geom","constellationArt.frag");
	shaderArt->setUniformLocation("Intensity");
	shaderArt->setUniformLocation("Color");

	//BOUNDARY
	shaderBoundary = new shaderProgram();
	shaderBoundary->init("constellationBoundary.vert", "constellationBoundary.frag");
	shaderBoundary->setUniformLocation("Color");

	//LINES
	shaderLines = new shaderProgram();
	shaderLines->init("constellationLines.vert", "constellationLines.frag");

	glGenVertexArrays(1,&constellation.vao);
	glBindVertexArray(constellation.vao);

	glGenBuffers(1,&constellation.tex);
	glGenBuffers(1,&constellation.pos);
	glGenBuffers(1,&constellation.color);
	glGenBuffers(1,&constellation.mag);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
}

void ConstellationMgr::deleteShader()
{
	if(shaderArt) delete shaderArt;
		shaderArt=nullptr;
	if(shaderBoundary) delete shaderBoundary;
		shaderBoundary=nullptr;
	if(shaderLines) delete shaderLines;
		shaderLines=nullptr;

	glDeleteBuffers(1,&constellation.tex);
	glDeleteBuffers(1,&constellation.color);
	glDeleteBuffers(1,&constellation.pos);
	glDeleteVertexArrays(1,&constellation.vao);

}


ConstellationMgr::~ConstellationMgr()
{
	std::vector<Constellation *>::iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); iter++)
		delete(*iter);

	if (asterFont) delete asterFont;
	asterFont = nullptr;

	std::vector<std::vector<Vec3f> *>::iterator iter1;
	for (iter1 = allBoundarySegments.begin(); iter1 != allBoundarySegments.end(); ++iter1) {
		delete (*iter1);
	}
	allBoundarySegments.clear();

	deleteShader();
}


void ConstellationMgr::setLineColor(const Vec3f& c)
{
	lineColor = c;
	std::vector < Constellation * >::iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		(*iter)->setLineColor(c);
	}
}


void ConstellationMgr::setLineColor(const std::string _name, const Vec3f& c)
{
	std::vector < Constellation * >::iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		//~ printf("%s\n", (*iter)->getShortName().c_str());
		if ((*iter)->getShortName()==_name) {
			(*iter)->setLineColor(c);
			return;
		}
	}
	cLog::get()->write("No constellation Color with shortName "+_name,LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT );
}


void ConstellationMgr::setLabelColor(const Vec3f& c)
{
	labelColor = c;
	std::vector < Constellation * >::iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		(*iter)->setLabelColor(c);
	}
}

void ConstellationMgr::setLabelColor(const std::string _name, const Vec3f& c)
{
	std::vector < Constellation * >::iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		//~ printf("%s\n", (*iter)->getShortName().c_str());
		if ((*iter)->getShortName()==_name) {
			(*iter)->setLabelColor(c);
			return;
		}
	}
	cLog::get()->write("No constellation label with shortName "+_name,LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT );
}

void ConstellationMgr::setArtColor(const Vec3f& c)
{
	Constellation::artColor = c;
}

Vec3f ConstellationMgr::getArtColor() const
{
	return Constellation::artColor;
}

void ConstellationMgr::setFont(float font_size, const std::string& ttfFileName)
{
	if (asterFont) {
		delete asterFont;
		asterFont=nullptr;
	}
	asterFont = new s_font(font_size, ttfFileName);
	if (!asterFont)
		cLog::get()->write("ConstellationMgr: no font usable",  LOG_TYPE::L_ERROR);
	assert(asterFont);
}

// Load line and art data from files
int ConstellationMgr::loadLinesAndArt(const std::string &skyCultureDir)
{

	std::string fileName = skyCultureDir + "/constellationship.fab";
	std::string artfileName = skyCultureDir + "/constellationsart.fab";
	std::string boundaryfileName = skyCultureDir + "/boundaries.dat";

	std::ifstream inf(fileName.c_str());

	if (!inf.is_open()) {
		cLog::get()->write("Constellation_Mgr::loadLinesAndArt can't open constellation data file "+ fileName, LOG_TYPE::L_ERROR);
		return -1;
	}

	std::vector < Constellation * >::iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		delete(*iter);
	}
	asterisms.clear();
	selected.clear();

	Constellation *cons = nullptr;

	std::string record;
	int line=0;
	while (!inf.eof() && std::getline(inf, record)) {
		line++;
		if (record.size()!=0 && record[0]=='#')
			continue;
		cons = new Constellation;

		if (cons->read(record, hipStarMgr)) {
			asterisms.push_back(cons);
		} else {
			cLog::get()->write("ConstellationMgr::loadLinesAndArt on line " + Utility::intToString(line) + " of " + fileName, LOG_TYPE::L_ERROR);
			delete cons;
		}
	}
	inf.close();

	// Set current states
	setCurrentStates();

	FILE *fic = fopen(artfileName.c_str(), "r");
	if (!fic) {
		cLog::get()->write("ConstellationMgr::loadLinesAndArt Can't open " + artfileName, LOG_TYPE::L_ERROR);
		return 0; // no art, but still loaded constellation data
	}
	fclose(fic);

	// Read the constellation art file with the following format :
	// ShortName texture_file x1 y1 hp1 x2 y2 hp2
	// Where :
	// shortname is the international short name (i.e "Lep" for Lepus)
	// texture_file is the graphic file of the art texture
	// x1 y1 are the x and y texture coordinates in pixels of the star of hipparcos number hp1
	// x2 y2 are the x and y texture coordinates in pixels of the star of hipparcos number hp2
	// The coordinate are taken with (0,0) at the top left corner of the image file
	std::string shortname;
	std::string texfile;
	unsigned int x1, y1, x2, y2, x3, y3, hp1, hp2, hp3;
	float fx1, fy1, fx2, fy2, fx3, fy3; // read floats to allow proportional image points to allow image sizes to vary as needed
	int texW, texH; // art texture dimensions

	// Read in constellation art information
	// Note: Stellarium 0.10.3 allows more than 3 alignment points, here only first 3 are used.

	std::ifstream artFile(artfileName.c_str());
	if (!artFile.is_open()) {
		//cerr << "Can't open file" << artFile << endl;
		cLog::get()->write("ConstellationMgr::loadLinesAndArt Can't open " + artfileName, LOG_TYPE::L_ERROR);
		return 0;
	}

	while (!artFile.eof() && std::getline(artFile, record)) {

		if ( record != "") {
			std::stringstream in(record);
			in >> shortname >> texfile;

			if(in.fail()) {
				//cerr << "Error parsing constellation art record:\n" << record << endl;
				cLog::get()->write("ConstellationMgr::loadLinesAndArt Error parsing constellation art record " + record, LOG_TYPE::L_ERROR);
				continue;
			}

			// TODO add better error checking
			if(shortname!="" && shortname[0]!='#') {
				in >> fx1 >> fy1 >> hp1;
				in >> fx2 >> fy2 >> hp2;
				in >> fx3 >> fy3 >> hp3;
			} else {
				continue;
			}

			cons = nullptr;
			cons = findFromAbbreviation(shortname);
			if (!cons) {
				// save on common error delay
				cLog::get()->write("ConstellationMgr::loadLinesAndArt can't find constellation called : " + shortname, LOG_TYPE::L_ERROR);
			} else {

				// Try local sky culture directory for texture images first //TODO use FILEPATH
				std::string localFile = skyCultureDir + "/" + texfile;
				FILE *ftest = fopen(localFile.c_str(), "r");
				if (!ftest) {
					// Load from application texture directory
					cons->art_tex = new s_texture(texfile, TEX_LOAD_TYPE_PNG_SOLID, true);  // use mipmaps
				} else {
					// Load from local directory
					fclose(ftest);
					cons->art_tex = new s_texture(/*true,*/ localFile, TEX_LOAD_TYPE_PNG_SOLID, true);  // use mipmaps
				}

				if(cons->art_tex->getID() == 0) continue;  // otherwise no texture
				cons->art_tex->getDimensions(texW, texH);

				// support absolute and proportional image coordinates
				(fx1>1) ? x1=(unsigned int)fx1 : x1=(unsigned int)(texW*fx1);
				(fy1>1) ? y1=(unsigned int)fy1 : y1=(unsigned int)(texH*fy1);

				(fx2>1) ? x2=(unsigned int)fx2 : x2=(unsigned int)(texW*fx2);
				(fy2>1) ? y2=(unsigned int)fy2 : y2=(unsigned int)(texH*fy2);

				(fx3>1) ? x3=(unsigned int)fx3 : x3=(unsigned int)(texW*fx3);
				(fy3>1) ? y3=(unsigned int)fy3 : y3=(unsigned int)(texH*fy3);

				Vec3f s1 = hipStarMgr->searchHP(hp1)->getObsJ2000Pos(0);
				Vec3f s2 = hipStarMgr->searchHP(hp2)->getObsJ2000Pos(0);
				Vec3f s3 = hipStarMgr->searchHP(hp3)->getObsJ2000Pos(0);

				// To transform from texture coordinate to 2d coordinate we need to find X with XA = B
				// A formed of 4 points in texture coordinate, B formed with 4 points in 3d coordinate
				// We need 3 stars and the 4th point is deduced from the other to get an normal base
				// X = B inv(A)
				Vec3f s4 = s1 + ((s2 - s1) ^ (s3 - s1));
				Mat4f B(s1[0], s1[1], s1[2], 1, s2[0], s2[1], s2[2], 1, s3[0], s3[1], s3[2], 1, s4[0], s4[1], s4[2], 1);
				Mat4f A(x1, texH - y1, 0.f, 1.f, x2, texH - y2, 0.f, 1.f, x3, texH - y3, 0.f, 1.f, x1, texH - y1, texW, 1.f);
				Mat4f X = B * A.inverse();

				cons->art_vertex[0] = Vec3f(X * v3fNull);
				cons->art_vertex[1] = Vec3f(X * Vec3f(texW / 2, 0, 0));
				cons->art_vertex[2] = Vec3f(X * Vec3f(texW / 2, texH / 2, 0));
				cons->art_vertex[3] = Vec3f(X * Vec3f(0, texH / 2, 0));
				cons->art_vertex[4] = Vec3f(X * Vec3f(texW, 0, 0));
				cons->art_vertex[5] = Vec3f(X * Vec3f(texW, texH / 2, 0));
				cons->art_vertex[6] = Vec3f(X * Vec3f(texW, texH, 0));
				cons->art_vertex[7] = Vec3f(X * Vec3f(texW / 2 + 0, texH, 0));
				cons->art_vertex[8] = Vec3f(X * Vec3f(0, texH, 0));

			}
		}
	}
	artFile.close();

	loadBoundaries(boundaryfileName);

	return 0;
}

void ConstellationMgr::setCurrentStates()
{
	setFlagArt(flagArt);
	setFlagLines(flagLines);
	setFlagNames(flagNames);
	setFlagBoundaries(flagBoundaries);
	setLineColor(lineColor);
	setBoundaryColor(boundaryColor);
	setLabelColor(labelColor);
}

void ConstellationMgr::draw(const Projector * prj,const Navigator * nav)
{
	drawLines(prj);
	drawNames(prj);
	drawArt(prj, nav);
	drawBoundaries(prj);
}

//! Draw constellations art textures
void ConstellationMgr::drawArt(const Projector * prj, const Navigator * nav)
{
	StateGL::BlendFunc(GL_ONE, GL_ONE);
	StateGL::enable(GL_BLEND);
	StateGL::enable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	//~ StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	std::vector < Constellation * >::const_iterator iter;

	shaderArt->use();

	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		(*iter)->drawArt(prj, nav, shaderArt, constellation);
	}

	shaderArt->unuse();
	glFrontFace(GL_CCW);
	StateGL::disable(GL_CULL_FACE);
}

//! Draw constellations lines
void ConstellationMgr::drawLines(const Projector * prj)
{
	std::vector<float> vLinesPos;
	std::vector<float> vLinesColor;

	std::vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		(*iter)->drawLines(prj, vLinesPos, vLinesColor);
	}

	if (vLinesPos.size()==0)
		return;

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	shaderLines->use();

	glBindVertexArray(constellation.vao);

	glBindBuffer(GL_ARRAY_BUFFER,constellation.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vLinesPos.size(),vLinesPos.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);

	glBindBuffer(GL_ARRAY_BUFFER,constellation.color);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vLinesColor.size(),vLinesColor.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(2,4,GL_FLOAT,GL_FALSE,0,NULL);

	glDrawArrays(GL_LINES, 0, vLinesPos.size()/2);

	shaderLines->unuse();
}

//! Draw the names of all the constellations
void ConstellationMgr::drawNames(const Projector * prj)
{
	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_ONE, GL_ONE);

	std::vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); iter++) {
		// Check if in the field of view
		if (prj->projectJ2000Check((*iter)->XYZname, (*iter)->XYname))
			(*iter)->drawName(asterFont, prj);
	}
}

Constellation *ConstellationMgr::isStarIn(const Object &s) const
{
	std::vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		// Check if the star is in one of the constellation
		if ((*iter)->isStarIn(s))
			return (*iter);
	}
	return nullptr;
}

Constellation *ConstellationMgr::findFromAbbreviation(const std::string & abbreviation) const
{
	// search in uppercase only
	std::string tname = abbreviation;
	transform(tname.begin(), tname.end(), tname.begin(),::toupper);

	std::vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		if ((*iter)->abbreviation == tname)
			return (*iter);
	}
	return nullptr;
}


/**
 * @brief Read constellation names from the given file
 * @param namesFile Name of the file containing the constellation names in english
 */
void ConstellationMgr::loadNames(const std::string& namesFile)
{
	// Constellation not loaded yet
	if (asterisms.empty()) return;

	std::vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
		(*iter)->englishName.clear();

	// read in translated common names from file
	std::ifstream commonNameFile(namesFile.c_str());
	if (!commonNameFile.is_open()) {
		//cerr << "Can't open file" << namesFile << endl;
		cLog::get()->write("ConstellationMgr::loadNames :: Can't open file " + namesFile, LOG_TYPE::L_ERROR);
		return;
	}

	// find matching constellation and update name
	std::string record, tmp, tmp2, ename;
	bool quotes;
	bool done=false;
	std::string tmpShortName;
	Constellation *aster;
	while (!commonNameFile.eof() && std::getline(commonNameFile, record)) {

		if ( record != "") {
			std::stringstream in(record);
			in >> tmpShortName;

			aster = findFromAbbreviation(tmpShortName);
			if (aster != nullptr) {
				// Read the names in english
				// Whitespace delimited with optional quotes to be mostly compatible with Stellarium 0.10.3 format
				in >> tmp;

				done = false;
				if(tmp[0]=='"') {
					quotes=true;

					if(tmp[tmp.length()-1]=='"') {
						ename = tmp.substr(1,tmp.length()-2);
						done=true;
					} else ename = tmp.substr(1,tmp.length()-1);
				} else {
					quotes=false;
					ename = tmp;
				}

				while(!done && in >> tmp) {

					if(quotes && tmp[tmp.length()-1]=='"') {
						tmp2 = tmp.substr(0, tmp.length()-2);
						ename += " " + tmp2;
						done = true;
					} else {
						ename += " " + tmp;
					}
				}
				aster->englishName = ename;
			}
		}
	}

	commonNameFile.close();
}

//! @brief Update i18 names from english names according to current locale
//! The translation is done using gettext with translated strings defined in translations.h
void ConstellationMgr::translateNames(Translator& trans)
{
	std::vector < Constellation * >::const_iterator iter;

	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
		(*iter)->nameI18 = trans.translateUTF8((*iter)->englishName.c_str());

	if(asterFont) asterFont->clearCache();  // remove cached strings
}

//! update faders
void ConstellationMgr::update(int delta_time)
{
	std::vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		(*iter)->update(delta_time);
	}
}


void ConstellationMgr::setArtIntensity(float _max)
{
	artMaxIntensity = _max;
	std::vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
		(*iter)->art_fader.setMaxValue(_max);
}

void ConstellationMgr::setArtIntensity(const std::string &_name, float _max)
{
	std::vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
		if ((*iter)->getShortName()==_name) {
			(*iter)->art_fader.setMaxValue(_max);
			return;
		}
	cLog::get()->write("No constellation ArtIntensity with shortName "+_name,LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT );
}


void ConstellationMgr::setArtFadeDuration(float duration)
{
	artFadeDuration = duration;
	std::vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
		(*iter)->art_fader.setDuration((int) (duration * 1000.f));
}


void ConstellationMgr::setFlagLines(bool b)
{
	flagLines = b;

	if (selected.begin() != selected.end()  ) {
		std::vector < Constellation * >::const_iterator iter;
		for (iter = selected.begin(); iter != selected.end(); ++iter)
			(*iter)->setFlagLines(b);
	} else if (isolateSelected) {
		std::vector < Constellation * >::const_iterator iter;
		for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
			(*iter)->setFlagLines(b);
	}
}

void ConstellationMgr::setFlagBoundaries(bool b)
{
	flagBoundaries = b;

	if (selected.begin() != selected.end() ) {
		std::vector < Constellation * >::const_iterator iter;
		for (iter = selected.begin(); iter != selected.end(); ++iter)
			(*iter)->setFlagBoundaries(b);
	} else if (isolateSelected) {
		std::vector < Constellation * >::const_iterator iter;
		for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
			(*iter)->setFlagBoundaries(b);
	}
}

void ConstellationMgr::setFlagArt(bool b)
{
	flagArt = b;

	if (selected.begin() != selected.end() ) {
		std::vector < Constellation * >::const_iterator iter;
		for (iter = selected.begin(); iter != selected.end(); ++iter)
			(*iter)->setFlagArt(b);
	} else if (isolateSelected) {
		std::vector < Constellation * >::const_iterator iter;
		for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
			(*iter)->setFlagArt(b);
	}
}


void ConstellationMgr::setFlagNames(bool b)
{
	flagNames = b;

	if (selected.begin() != selected.end() ) {
		std::vector < Constellation * >::const_iterator iter;
		for (iter = selected.begin(); iter != selected.end(); ++iter)
			(*iter)->setFlagName(b);
	} else if (isolateSelected) {
		std::vector < Constellation * >::const_iterator iter;
		for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
			(*iter)->setFlagName(b);
	}
}

Object ConstellationMgr::getSelected(void) const
{
	return *selected.begin();  // TODO return all or just remove this method
}

//! Define which constellation is selected from its abbreviation
void ConstellationMgr::setSelected(const std::string& abbreviation)
{
	Constellation * c = findFromAbbreviation(abbreviation);

	if (c != nullptr) setSelectedConst(c);

}

//! Define which constellation to unselect from its abbreviation
void ConstellationMgr::unsetSelected(const std::string& abbreviation)
{
	Constellation * c = findFromAbbreviation(abbreviation);

	if (c != nullptr) unsetSelectedConst(c);
}


//! Define which constellation is selected and return brightest star
ObjectBaseP ConstellationMgr::setSelectedStar(const std::string& abbreviation)
{
	Constellation * c = findFromAbbreviation(abbreviation);

	if (c != nullptr) {
		setSelectedConst(c);
		return c->getBrightestStarInConstellation();
	}
	return nullptr;
}

std::string ConstellationMgr::getSelectedShortName() const
{
	std::string result="";
	std::vector < Constellation * >::const_iterator iter;
	for (iter = selected.begin(); iter != selected.end(); ++iter)
		result = result+(*iter)->getShortName();
	if (result.empty())
		result = "EOL"; //end of line
	return result;
}


void ConstellationMgr::setSelectedConst(Constellation * c)
{
	// update states for other constellations to fade them out
	if (c != nullptr) {

		bool matchc = 0;
		std::vector < Constellation * >::const_iterator c_iter;
		for (c_iter = selected.begin(); c_iter != selected.end(); ++c_iter)
			if ( (c)==(*c_iter) ) {
				matchc=true; // this is an already selected constellation
				break;
			}

		if (!matchc)  {
			selected.push_back(c);
			// Not selected constellation
			c->setFlagLines(getFlagLines());
			c->setFlagName(getFlagNames());
			c->setFlagArt(getFlagArt());
			c->setFlagBoundaries(getFlagBoundaries());
		} else if (!isolateSelected) {
			// selected constellation
			c->setFlagLines(false);
			c->setFlagName(false);
			c->setFlagArt(false);
			c->setFlagBoundaries(false);
			selected.erase(c_iter);
		}

		std::vector < Constellation * >::const_iterator iter;
		for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {

			bool match = 0;
			std::vector < Constellation * >::const_iterator s_iter;
			for (s_iter = selected.begin(); s_iter != selected.end(); ++s_iter)
				if ( (*iter)==(*s_iter) ) {
					match=true; // this is a selected constellation
					break;
				}

			if (!match) {
				// Not selected constellation
				(*iter)->setFlagLines(false);
				(*iter)->setFlagName(false);
				(*iter)->setFlagArt(false);
				(*iter)->setFlagBoundaries(false);
			}
		}
		Constellation::singleSelected = true;  // For boundaries

	} else {
		if (isolateSelected) {
			if (selected.begin() == selected.end()) return;

			// Otherwise apply standard flags to all constellations
			std::vector < Constellation * >::const_iterator iter;
			for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
				(*iter)->setFlagLines(getFlagLines());
				(*iter)->setFlagName(getFlagNames());
				(*iter)->setFlagArt(getFlagArt());
				(*iter)->setFlagBoundaries(getFlagBoundaries());
			}

			// And remove all selections
			selected.clear();
		}

	}
}

//! Remove constellation from selected list
void ConstellationMgr::unsetSelectedConst(Constellation * c)
{
	std::vector < Constellation * >::iterator iter;
	for (iter = selected.begin(); iter != selected.end(); ++iter) {
		if (c == (*iter)) {
			//	  cout << "matched constellation to remove from selected list\n";
			selected.erase(iter);
			// stay at same location next time through
			iter--;
		}
	}
	// if nothing is selected now, send out current settings to all constellations
	if ((selected.begin() == selected.end()) && (isolateSelected)) {
		// Otherwise apply standard flags to all constellations
		std::vector < Constellation * >::const_iterator iter;
		for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
			(*iter)->setFlagLines(getFlagLines());
			(*iter)->setFlagName(getFlagNames());
			(*iter)->setFlagArt(getFlagArt());
			(*iter)->setFlagBoundaries(getFlagBoundaries());
		}
	} else {

		// just this one constellation was unselected so reset flags
		c->setFlagLines(false);
		c->setFlagName(false);
		c->setFlagArt(false);
		c->setFlagBoundaries(false);
	}
}


//! Load from file
bool ConstellationMgr::loadBoundaries(const std::string& boundaryFile)
{
	Constellation *cons = nullptr;
	unsigned int i, j;

	std::vector<std::vector<Vec3f> *>::iterator iter;
	for (iter = allBoundarySegments.begin(); iter != allBoundarySegments.end(); ++iter) {
		delete (*iter);
	}
	allBoundarySegments.clear();

	std::ifstream dataFile;
	dataFile.open(boundaryFile.c_str());
	if (!dataFile.is_open()) {
		cLog::get()->write("Boundary file " + boundaryFile + " not found", LOG_TYPE::L_ERROR);
		return false;
	}

	cLog::get()->write("Loading Constellation boundary data...", LOG_TYPE::L_INFO);

	float DE, RA;
	Vec3f XYZ;
	unsigned num, numc;
	std::vector<Vec3f> *points = nullptr;
	std::string consname;
	i = 0;
	while (!dataFile.eof()) {
		points = new std::vector<Vec3f>;

		num = 0;
		dataFile >> num;
		if (num == 0) continue; // empty line

		for (j=0; j<num; j++) {
			dataFile >> RA >> DE;

			RA*=M_PI/12.;     // Convert from hours to rad
			DE*=M_PI/180.;    // Convert from deg to rad

			// Calc the Cartesian coord with RA and DE
			Utility::spheToRect(RA,DE,XYZ);
			points->push_back(XYZ);
		}

		// this list is for the de-allocation
		allBoundarySegments.push_back(points);

		dataFile >> numc;
		// there are 2 constellations per boundary

		for (j=0; j<numc; j++) {
			dataFile >> consname;
			// not used?
			if (consname == "SER1" || consname == "SER2") consname = "SER";

			cons = findFromAbbreviation(consname);
			if (cons) {
				cons->isolatedBoundarySegments.push_back(points);
			}
		}

		if (cons) cons->sharedBoundarySegments.push_back(points);
		i++;

	}
	dataFile.close();
	cLog::get()->write("(" + Utility::intToString(i) + " segments loaded)", LOG_TYPE::L_INFO);
	delete points;

	return true;
}

//! Draw constellations lines
void ConstellationMgr::drawBoundaries(const Projector * prj)
{
	std::vector<float> vBoundariesPos;
	std::vector<float> vBoundariesIntensity;

	std::vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		(*iter)->drawBoundary(prj, vBoundariesPos,vBoundariesIntensity);
	}

	if (vBoundariesPos.size()==0)
		return;

	//~ StateGL::disable(GL_BLEND);
	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	shaderBoundary->use();

	shaderBoundary->setUniform("Color", boundaryColor);
	glBindVertexArray(constellation.vao);

	glBindBuffer(GL_ARRAY_BUFFER,constellation.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vBoundariesPos.size(),vBoundariesPos.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);

	glBindBuffer(GL_ARRAY_BUFFER,constellation.mag);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vBoundariesIntensity.size(),vBoundariesIntensity.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,0,NULL);

	glDrawArrays(GL_LINES, 0, vBoundariesPos.size()/2);

	shaderBoundary->unuse();
}

//! Return the matching constellation object's pointer if exists or NULL
//! @param nameI18n The case sensistive constellation name
Object ConstellationMgr::searchByNameI18n(const std::string& nameI18n) const
{
	std::string objw = nameI18n;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);

	std::vector <Constellation*>::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		std::string objwcap = (*iter)->nameI18;
		transform(objwcap.begin(), objwcap.end(), objwcap.begin(), ::toupper);
		if (objwcap==objw) return *iter;
	}
	return nullptr;
}

//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
std::vector<std::string> ConstellationMgr::listMatchingObjectsI18n(const std::string& objPrefix, unsigned int maxNbItem) const
{
	std::vector<std::string> result;
	if (maxNbItem==0) return result;

	std::string objw = objPrefix;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);

	std::vector < Constellation * >::const_iterator iter;
	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		std::string constw = (*iter)->getNameI18n().substr(0, objw.size());
		transform(constw.begin(), constw.end(), constw.begin(), ::toupper);
		if (constw==objw) {
			result.push_back((*iter)->getNameI18n());
			if (result.size()==maxNbItem)
				return result;
		}
	}
	return result;
}


void ConstellationMgr::getHPStarsFromAbbreviation(const std::string& abbreviation, std::vector<unsigned int>& HpStarsFromAsterim) const
{
	Constellation* target = findFromAbbreviation(abbreviation);
	if (target ==nullptr)
		return;
	target->getHPStarsFromAsterim(HpStarsFromAsterim);
}

void ConstellationMgr::getHPStarsFromAll(std::vector<unsigned int>& HpStarsFromAsterim)
{
	for (auto iter = asterisms.begin(); iter != asterisms.end(); ++iter) {
		(*iter)->getHPStarsFromAsterim(HpStarsFromAsterim);
	}
}
