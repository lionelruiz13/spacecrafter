/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014-2020 LSS team & AssociationSirius
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


Constellation::Constellation() : asterism(nullptr), art_tex(nullptr)
{}

Constellation::~Constellation()
{
	HpStars.clear();

	if (asterism) delete[] asterism;
	asterism = nullptr;

	if (art_tex) delete art_tex;
	art_tex = nullptr;
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
			cLog::get()->write("Error in Constellation " + abbreviation + " asterism : can't find star HP= " + std::to_string(HP), LOG_TYPE::L_ERROR);
			delete [] asterism;
			asterism = nullptr;
			return false;
		}
		HpStars.push_back(HP);
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
		  if ((ra2-ra1)>M_PI) ra1+=2*M_PI;
		  if ((ra1-ra2)>M_PI) ra2+=2*M_PI;
		  Utility::spheToRect(ra1,de1, gettemp1);
		  int npoints=10;
		  float delta=(ra1-ra2)/(npoints);
		  for(int i=0; i<=npoints ; i++) {
			rat=ra1-delta*i;
			if (ra2 - ra1 != 0)
				det=atan(((tan(de2)*sin(rat-ra1))/sin(ra2-ra1))+(tan(de1)*sin(ra2-rat))/sin(ra2-ra1));
			else
				det=M_PI/2.;
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
	if (!name_fader.getInterstate())
		return;
	Vec4f Color(labelColor[0], labelColor[1], labelColor[2], name_fader.getInterstate());
	// prj->printGravity180(constfont, XYname[0], XYname[1], nameI18, Color, -constfont->getStrLen(nameI18)/2, 0);
	float az, alt;
	Utility::rectToSphe(&az, &alt, prj->getMatJ2000ToEye() * XYZname);
	alt += M_PI_2;
	alt *= (180*180/M_PI)/prj->getFov();
	if (alt < 90.f)
		constfont->printHorizontal(prj, std::min(90.f - alt, 80.f), az*180.f/M_PI-90.f, nameI18, Color, TEXT_ALIGN::CENTER, true);
}


//! Draw the art texture, optimized function to be called thru a constellation manager only
void Constellation::drawArt(const Projector* prj, const Navigator* nav, std::vector<float> &vecPos, std::vector<float> &vecTex)
{
	float intensity = art_fader.getInterstate();

	if (art_tex && intensity) {
		{
		struct ArtPatch {
				int topLeft;
				int topRight;
				int bottomLeft;
				int bottomRight;
				float u0;
				float v0;
				float u1;
				float v1;
			};

			const ArtPatch patches[] = {
				{0, 1, 3, 2, 0.0f, 0.0f, 0.5f, 0.5f},
				{1, 4, 2, 5, 0.5f, 0.0f, 1.0f, 0.5f},
				{3, 2, 8, 7, 0.0f, 0.5f, 0.5f, 1.0f},
				{2, 5, 7, 6, 0.5f, 0.5f, 1.0f, 1.0f}
			};
			constexpr int subdivisions = 2;

			auto interpolate = [](const Vec3d& topLeft, const Vec3d& topRight,
				                    const Vec3d& bottomLeft, const Vec3d& bottomRight,
			                    float u, float v) {
				return topLeft * ((1.0f - u) * (1.0f - v))
				     + topRight * (u * (1.0f - v))
				     + bottomLeft * ((1.0f - u) * v)
				     + bottomRight * (u * v);
			};

			for (const ArtPatch& patch : patches) {
				for (int y = 0; y < subdivisions; ++y) {
					for (int x = 0; x < subdivisions; ++x) {
						const float u0 = static_cast<float>(x) / subdivisions;
						const float u1 = static_cast<float>(x + 1) / subdivisions;
						const float v0 = static_cast<float>(y) / subdivisions;
						const float v1 = static_cast<float>(y + 1) / subdivisions;

						const Vec3d skyTopLeft = interpolate(art_vertex[patch.topLeft], art_vertex[patch.topRight], art_vertex[patch.bottomLeft], art_vertex[patch.bottomRight], u0, v0);
						const Vec3d skyTopRight = interpolate(art_vertex[patch.topLeft], art_vertex[patch.topRight], art_vertex[patch.bottomLeft], art_vertex[patch.bottomRight], u1, v0);
						const Vec3d skyBottomLeft = interpolate(art_vertex[patch.topLeft], art_vertex[patch.topRight], art_vertex[patch.bottomLeft], art_vertex[patch.bottomRight], u0, v1);
						const Vec3d skyBottomRight = interpolate(art_vertex[patch.topLeft], art_vertex[patch.topRight], art_vertex[patch.bottomLeft], art_vertex[patch.bottomRight], u1, v1);


						Vec3d screenTopLeft, screenTopRight;
						Vec3d screenBottomLeft, screenBottomRight;
						prj->projectJ2000(skyTopLeft, screenTopLeft);
						prj->projectJ2000(skyTopRight, screenTopRight);
						prj->projectJ2000(skyBottomLeft, screenBottomLeft);
						prj->projectJ2000(skyBottomRight, screenBottomRight);
						const Vec3d skyCenter = interpolate(art_vertex[patch.topLeft], art_vertex[patch.topRight], art_vertex[patch.bottomLeft], art_vertex[patch.bottomRight], (u0 + u1) * 0.5f, (v0 + v1) * 0.5f);
						Vec3d screenCenter;
						const bool centerVisible = prj->projectJ2000Check(skyCenter, screenCenter)
							|| nav->getPrecEquVision().dot(skyCenter) > 0.9;
						const double minX = std::min({screenTopLeft[0], screenTopRight[0], screenBottomLeft[0], screenBottomRight[0], screenCenter[0]});
						const double maxX = std::max({screenTopLeft[0], screenTopRight[0], screenBottomLeft[0], screenBottomRight[0], screenCenter[0]});
						const double minY = std::min({screenTopLeft[1], screenTopRight[1], screenBottomLeft[1], screenBottomRight[1], screenCenter[1]});
						const double maxY = std::max({screenTopLeft[1], screenTopRight[1], screenBottomLeft[1], screenBottomRight[1], screenCenter[1]});
						const Vec4i& viewport = prj->getViewport();
						const bool overlapsViewport = maxX >= viewport[0]
							&& minX <= viewport[0] + viewport[2]
							&& maxY >= viewport[1]
							&& minY <= viewport[1] + viewport[3];
						const bool allInFront = screenTopLeft[2] < 1.0 && screenTopRight[2] < 1.0
							&& screenBottomLeft[2] < 1.0 && screenBottomRight[2] < 1.0
							&& screenCenter[2] < 1.0;
						if ((!overlapsViewport && !centerVisible) || !allInFront)
							continue;

						const float texU0 = patch.u0 + (patch.u1 - patch.u0) * u0;
						const float texU1 = patch.u0 + (patch.u1 - patch.u0) * u1;
						const float texV0 = patch.v0 + (patch.v1 - patch.v0) * v0;
						const float texV1 = patch.v0 + (patch.v1 - patch.v0) * v1;

						// The geometry shader expects top-right, bottom-right, top-left, bottom-left.
						vecPos.push_back(static_cast<float>(screenTopRight[0]));
						vecPos.push_back(static_cast<float>(screenTopRight[1]));
						vecTex.push_back(texU1);
						vecTex.push_back(texV0);
						vecPos.push_back(static_cast<float>(screenBottomRight[0]));
						vecPos.push_back(static_cast<float>(screenBottomRight[1]));
						vecTex.push_back(texU1);
						vecTex.push_back(texV1);
						vecPos.push_back(static_cast<float>(screenTopLeft[0]));
						vecPos.push_back(static_cast<float>(screenTopLeft[1]));
						vecTex.push_back(texU0);
						vecTex.push_back(texV0);
						vecPos.push_back(static_cast<float>(screenBottomLeft[0]));
						vecPos.push_back(static_cast<float>(screenBottomLeft[1]));
						vecTex.push_back(texU0);
						vecTex.push_back(texV1);
					}
				}
			}
		}
	}
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
void Constellation::drawBoundary(const Projector* prj, std::vector<float> &vBoundariesPos, std::vector<float> &vBoundariesIntensity, bool singleSelected)
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

void Constellation::getHPStarsFromAsterim(std::vector<unsigned int>& starsFromAsterim) const
{
	for(auto i : HpStars) {
		starsFromAsterim.push_back(i);
	}
}
