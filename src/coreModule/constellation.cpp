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

#include <iostream>
#include <algorithm>
#include <vector>
#include "coreModule/constellation.hpp"
#include "starModule/hip_star_mgr.hpp"
#include "navModule/navigator.hpp"
#include "coreModule/projector.hpp"
#include "tools/s_font.hpp"
#include "tools/log.hpp"

Vec3f Constellation::artColor = Vec3f(1.0,1.0,1.0);
bool Constellation::singleSelected = false;


Constellation::Constellation() : asterism(nullptr), art_tex(nullptr)
{
}

Constellation::~Constellation()
{
	if (asterism) delete[] asterism;
	asterism = nullptr;

	if (art_tex) delete art_tex;
	art_tex = nullptr;

	while(!vecPos.empty()) {
		vecPos.pop_back();
	}

	while(!vecTex.empty()) {
		vecTex.pop_back();
	}
}

//! Read Constellation data record and grab cartesian positions of stars
//! @return false if can't parse record
bool Constellation::read(const std::string& record, HipStarMgr * _VouteCeleste)
{
	unsigned int HP;

	abbreviation.clear();
	nb_segments = 0;

	std::istringstream istr(record);
	if (!(istr >> abbreviation >> nb_segments)) {
		cLog::get()->write("Error parsing constellation record: " + record, LOG_TYPE::L_ERROR);
		return false;
	}

	// make short_name uppercase for case insensitive searches
	transform(abbreviation.begin(),abbreviation.end(), abbreviation.begin(), ::toupper);

	asterism = new ObjectBaseP[nb_segments*2];
	for (unsigned int i=0; i<nb_segments*2; ++i) {
		HP = 0;
		istr >> HP;

		if (HP == 0) {
			delete [] asterism;
			asterism = nullptr;
			return false;
		}

		asterism[i]=_VouteCeleste->searchHP(HP);
		if (!asterism[i]) {
			cLog::get()->write("Error in Constellation " + abbreviation + " asterism : can't find star HP= " + Utility::intToString(HP), LOG_TYPE::L_ERROR);
			delete [] asterism;
			asterism = nullptr;
			return false;
		}

	}

	for (unsigned int ii=0; ii<nb_segments*2; ++ii) {
		XYZname+= asterism[ii]->getObsJ2000Pos(0);
	}
	XYZname*=1./(nb_segments*2);

	return true;
}


//! Draw the lines for the Constellation using the coords of the stars
//! (optimized for use thru the class ConstellationMgr only)
void Constellation::drawLines(const Projector* prj, std::vector<float> &vLinesPos, std::vector<float> &vLinesColor)
{
	if (!line_fader.getInterstate()) return;

	Vec3d gettemp1,gettemp2,startemp1,startemp2;
	double ra1,de1,ra2,de2,rat,det;

	for (unsigned int i=0; i<nb_segments; ++i) {
		// orthodromy on line
		Utility::rectToSphe(&ra1,&de1,asterism[2*i]->getObsJ2000Pos(0));
		Utility::rectToSphe(&ra2,&de2,asterism[2*i+1]->getObsJ2000Pos(0));
		if ((abs(ra2-ra1)>0.000001) && (abs(de2-de1)>0.000001)) {
		  if ((ra2-ra1)>C_PI) ra1+=2*C_PI; 
		  if ((ra1-ra2)>C_PI) ra2+=2*C_PI; 
		  Utility::spheToRect(ra1,de1, gettemp1);
		  int npoints=11;
		  float delta=(ra1-ra2)/(npoints-1);
		  for(int i=0; i<npoints ; i++) {
			rat=ra1-delta*i;
			det=atan(((tan(de2)*sin(rat-ra1))/sin(ra2-ra1+0.00001))+(tan(de1)*sin(ra2-rat))/sin(ra2-ra1+0.00001));
			Utility::spheToRect(rat,det, gettemp2);
			if (prj->projectJ2000LineCheck( gettemp1,startemp1, gettemp2,startemp2)) {
			
				vLinesPos.push_back(startemp1[0]);
				vLinesPos.push_back(startemp1[1]);
				vLinesColor.push_back(lineColor[0]);
				vLinesColor.push_back(lineColor[1]);
				vLinesColor.push_back(lineColor[2]);
				vLinesColor.push_back(line_fader.getInterstate());
				vLinesPos.push_back(startemp2[0]);
				vLinesPos.push_back(startemp2[1]);
				vLinesColor.push_back(lineColor[0]);
				vLinesColor.push_back(lineColor[1]);
				vLinesColor.push_back(lineColor[2]);
				vLinesColor.push_back(line_fader.getInterstate());
			}
			gettemp1=gettemp2;
			startemp1=startemp2;
		  }
	    }
	}
}


//! Draw the name
void Constellation::drawName(s_font *constfont, const Projector* prj) const
{
	if (!name_fader.getInterstate()) return;
	Vec4f Color(labelColor[0], labelColor[1], labelColor[2], name_fader.getInterstate());
	prj->printGravity180(constfont, XYname[0], XYname[1], nameI18, Color, 1, -constfont->getStrLen(nameI18)/2);
}

//! Draw the art texture, optimized function to be called thru a constellation manager only
void Constellation::drawArt(const Projector* prj, const Navigator* nav, shaderProgram* &shaderArt, const DataGL &constellation)
{
	float intensity = art_fader.getInterstate();

	vecPos.clear();
	vecTex.clear();

	if (art_tex && intensity) {
		int tailleTab = 9;
		// bool tabB[tailleTab];
		std::vector<bool> tabB (tailleTab, false);

		// Vec3d tabV[tailleTab];
		std::vector<Vec3d> tabV (tailleTab, v3dNull);
		bool onScreen = false;

		for (int i = 0; i < tailleTab; i++) {
			if( (tabB[i] = prj->projectJ2000Check(art_vertex[i],tabV[i]) || (nav->getPrecEquVision().dot(art_vertex[i])>0.9)) )
				onScreen = true;
		}

		if(onScreen) {

			if ((tabB[0] || tabB[1] || tabB[2] || tabB[3]) && (tabV[0][2]<1 && tabV[1][2]<1 && tabV[2][2]<1 && tabV[3][2]<1)) {

				vecTex.push_back(0.5);
				vecTex.push_back(0);
				vecTex.push_back(0.5);
				vecTex.push_back(0.5);
				vecTex.push_back(0);
				vecTex.push_back(0);
				vecTex.push_back(0);
				vecTex.push_back(0.5);

				vecPos.push_back(tabV[1][0]);
				vecPos.push_back(tabV[1][1]);
				vecPos.push_back(tabV[2][0]);
				vecPos.push_back(tabV[2][1]);
				vecPos.push_back(tabV[0][0]);
				vecPos.push_back(tabV[0][1]);
				vecPos.push_back(tabV[3][0]);
				vecPos.push_back(tabV[3][1]);
			}

			if ((tabB[1] || tabB[4] || tabB[5] || tabB[2]) && (tabV[1][2]<1 && tabV[4][2]<1 && tabV[5][2]<1 && tabV[2][2]<1)) {

				vecTex.push_back(1);
				vecTex.push_back(0);
				vecTex.push_back(1);
				vecTex.push_back(0.5);
				vecTex.push_back(0.5);
				vecTex.push_back(0);
				vecTex.push_back(0.5);
				vecTex.push_back(0.5);

				vecPos.push_back(tabV[4][0]);
				vecPos.push_back(tabV[4][1]);
				vecPos.push_back(tabV[5][0]);
				vecPos.push_back(tabV[5][1]);
				vecPos.push_back(tabV[1][0]);
				vecPos.push_back(tabV[1][1]);
				vecPos.push_back(tabV[2][0]);
				vecPos.push_back(tabV[2][1]);
			}

			if ((tabB[2] || tabB[5] || tabB[6] || tabB[7]) && (tabV[2][2]<1 && tabV[5][2]<1 && tabV[6][2]<1 && tabV[7][2]<1)) {

				vecTex.push_back(1);
				vecTex.push_back(0.5);
				vecTex.push_back(1);
				vecTex.push_back(1);
				vecTex.push_back(0.5);
				vecTex.push_back(0.5);
				vecTex.push_back(0.5);
				vecTex.push_back(1);

				vecPos.push_back(tabV[5][0]);
				vecPos.push_back(tabV[5][1]);
				vecPos.push_back(tabV[6][0]);
				vecPos.push_back(tabV[6][1]);
				vecPos.push_back(tabV[2][0]);
				vecPos.push_back(tabV[2][1]);
				vecPos.push_back(tabV[7][0]);
				vecPos.push_back(tabV[7][1]);

			}
			if ((tabB[3] || tabB[2] || tabB[7] || tabB[8]) && (tabV[3][2]<1 && tabV[2][2]<1 && tabV[7][2]<1 && tabV[8][2]<1)) {

				vecTex.push_back(0.5);
				vecTex.push_back(0.5);
				vecTex.push_back(0.5);
				vecTex.push_back(1);
				vecTex.push_back(0);
				vecTex.push_back(0.5);
				vecTex.push_back(0);
				vecTex.push_back(1);

				vecPos.push_back(tabV[2][0]);
				vecPos.push_back(tabV[2][1]);
				vecPos.push_back(tabV[7][0]);
				vecPos.push_back(tabV[7][1]);
				vecPos.push_back(tabV[3][0]);
				vecPos.push_back(tabV[3][1]);
				vecPos.push_back(tabV[8][0]);
				vecPos.push_back(tabV[8][1]);
			}
		}
	}


	if (vecPos.size()==0)
		return;

	glBindVertexArray(constellation.vao);

	glBindBuffer(GL_ARRAY_BUFFER,constellation.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecPos.size(),vecPos.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);

	glBindBuffer(GL_ARRAY_BUFFER,constellation.tex);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecTex.size(),vecTex.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);

	glBindTexture(GL_TEXTURE_2D, art_tex->getID());

	shaderArt->setUniform("Intensity", intensity);
	shaderArt->setUniform("Color", artColor);

	glDrawArrays(GL_LINES_ADJACENCY, 0, vecPos.size()/2);
}

const Constellation* Constellation::isStarIn(const Object &s) const
{
	for (unsigned int i=0; i<nb_segments*2; ++i) {
		// if (asterism[i]==s) return this; WAS NOT WORKING
		if (asterism[i]->getEnglishName()==s.getEnglishName()) {
			//cout << "Const matched. " << getEnglishName() << endl;
			return this;
		}
	}
	return nullptr;
}

void Constellation::update(int delta_time)
{
	line_fader.update(delta_time);
	name_fader.update(delta_time);
	art_fader.update(delta_time);
	boundary_fader.update(delta_time);
}

//! Draw the Constellation lines
void Constellation::drawBoundary(const Projector* prj, std::vector<float> &vBoundariesPos, std::vector<float> &vBoundariesIntensity)
{
	if (!boundary_fader.getInterstate()) return;

	unsigned int i, j, size;
	Vec3d pt1, pt2;
	std::vector<Vec3f> *points;

	if (singleSelected) size = isolatedBoundarySegments.size();
	else size = sharedBoundarySegments.size();

	for (i=0; i<size; i++) {
		if (singleSelected) points = isolatedBoundarySegments[i];
		else points = sharedBoundarySegments[i];

		for (j=0; j<points->size()-1; j += 2) {
			if (prj->projectJ2000LineCheck(points->at(j),pt1,points->at(j+1),pt2)) {

				vBoundariesPos.push_back(pt1[0]);
				vBoundariesPos.push_back(pt1[1]);
				vBoundariesIntensity.push_back(boundary_fader.getInterstate());
				vBoundariesPos.push_back(pt2[0]);
				vBoundariesPos.push_back(pt2[1]);
				vBoundariesIntensity.push_back(boundary_fader.getInterstate());
			}
		}
	}
}

ObjectBaseP Constellation::getBrightestStarInConstellation(void) const
{
	float maxMag = 99.f;
	ObjectBaseP brightest;
	// maybe the brightest star has always odd index, so check all segment endpoints:
	for (int i=2*nb_segments-1; i>=0; i--) {
		const float Mag = asterism[i]->getMag(0);
		if (Mag < maxMag) {
			brightest = asterism[i];
			maxMag = Mag;
		}
	}
	return brightest;
}
