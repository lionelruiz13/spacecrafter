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

#include <iostream>
#include "coreModule/nebula.hpp"
#include "tools/s_texture.hpp"
#include "tools/s_font.hpp"
#include "navModule/navigator.hpp"
#include "tools/utility.hpp"
#include "tools/log.hpp"
#include "tools/fmath.hpp"



s_texture * Nebula::tex_NEBULA = nullptr;
shaderProgram * Nebula::shaderNebulaTex = nullptr;
s_font* Nebula::nebulaFont = nullptr;
float Nebula::circleScale = 1.f;
float Nebula::hintsBrightness = 0;
float Nebula::textBrightness = 0;
float Nebula::nebulaBrightness = 1;
Vec3f Nebula::labelColor = v3fNull;
Vec3f Nebula::circleColor = Vec3f(0.2,0.2,1.0);
bool Nebula::flagBright = false;
bool Nebula::displaySpecificHint = false;
float Nebula::dsoRadius = 1.f;
int Nebula::dsoPictoSize = 6;
DataGL Nebula::nebulaTex;


// Provide the luminance in cd/m^2 from the magnitude and the surface in arcmin^2
static float magToLuminance(float mag, float surface)
{
	return expf(-0.4f * 2.3025851f * (mag - (-2.5f * log10f(surface)))) * 108064.73f;
}

//todo path est inutile
Nebula::Nebula(std::string _englishName, std::string _mtype, std::string _constellation, float _ra, float _de, float _mag, float _size, std::string _classe,
               float _distance, std::string tex_name, bool path, float tex_angular_size, float tex_rotation, std::string tex_credit, float _luminance, bool _deletable, bool _hidden) :
	neb_tex(nullptr)
{
	neb_color=Vec3f(0.2,0.2,1.);
	tex_circle = nullptr;
	englishName = _englishName;
	nameI18 = englishName;
	myDistance = _distance;
	DSOclass = _classe;
	DSOstringType = _mtype;
	m_deletable = _deletable;
	constellation= _constellation;
	m_hidden = _hidden;
	m_selected = true;
	texLuminanceAdjust= _luminance;
	if (tex_credit  == "none")
		credit = "";
	else
		credit = std::string("Credit: ") + tex_credit;
	for (std::string::size_type i=0; i<credit.length(); ++i) {
		if (credit[i]=='_') credit[i]=' ';
	}

	// - keep base info for drawing (in radians)
	myRA = _ra*C_PI/180.;
	myDe = _de*C_PI/180.;
	texAngularSize = tex_angular_size/60*C_PI/180;
	myRotation = tex_rotation*C_PI/180;

	// Calc the Cartesian coord with RA and DE
	Utility::spheToRect(myRA,myDe,XYZ);
	XYZ*=dsoRadius;

	mag= _mag;
	if ((mag!=-30) && (mag < 1.0)) mag = 99.0;

	if (tex_angular_size < 0.0)
		tex_angular_size = 1.0;
	if (tex_angular_size > 150.0)
		tex_angular_size = 150.0;

	// Calc the angular size in radian
	m_angular_size = tex_angular_size/2/60*C_PI/180;

	neb_tex = new s_texture(tex_name, TEX_LOAD_TYPE_PNG_ALPHA, true);  // use mipmaps

	luminance = magToLuminance(mag, tex_angular_size*tex_angular_size*3600);

	// this is a huge performance drag if called every frame, so cache here
	tex_avg_luminance = neb_tex->getAverageLuminance();

	Vec3d imagev = Mat4d::zrotation(myRA-C_PI_2) * Mat4d::xrotation(myDe) * Vec3d(0,1,0);
	Vec3d ortho1 = Mat4d::zrotation(myRA-C_PI_2) * Vec3d(1,0,0);
	Vec3d ortho2 = imagev^ortho1;

	Vec3d grdpt;
	Vec3f grdptf;

	for(int i=0; i<2; i++)
		for(int j=0; j<2; j++) {
			grdpt = Mat4d::rotation( imagev, myRotation+C_PI) *
				Mat4d::rotation( ortho1, texAngularSize*(i+-1./2.)) *
				Mat4d::rotation( ortho2, texAngularSize*(j+-1./2.)) * imagev;

			grdptf = grdpt.convert();
			for(int k=0; k<3; k++)
				sDataPos.push_back(grdptf[k]);
	}

	sDataTex[0]=0.f;
	sDataTex[1]=0.f;
	sDataTex[2]=1.f;
	sDataTex[3]=0.f;
	sDataTex[4]=0.f;
	sDataTex[5]=1.f;
	sDataTex[6]=1.f;
	sDataTex[7]=1.f;

	//what sort of circle should we draw ?
	DSOType = getDsoType(_mtype);

	switch (DSOType) {
		case GALXY:
			neb_color= Vec3f(0.3,0.3,1.);
			posTex = Vec2f(.25, .5); //Donne le points bas gauche de la la grande texture a prendre
			break;
		case OPNCL:
			neb_color= Vec3f(0.,1.0,1.0);
			posTex = Vec2f(.5, .5);
			break;
		case GLOCL:
			neb_color= Vec3f(1.,1.0,0.0);
			posTex = Vec2f(.75, .75);
			break;
		case BRTNB:
			neb_color= Vec3f(1.,0.2,0.2);
			posTex = Vec2f(.0, .75);
			break;
		case PLNNB:
			neb_color= Vec3f(0.0,1.0,0.0);
			posTex = Vec2f(.75, .5);
			break;
		case DRKNB:
			neb_color= Vec3f(0.4,0.4,0.4);
			posTex = Vec2f(.0, .5);
			break;
		case CLNEB:
			neb_color= Vec3f(1.0,0.5,0.5);
			posTex = Vec2f(.25, .75);
			break;
		case STARS:
			neb_color= Vec3f(1.0,1.0,0.5);
			posTex = Vec2f(.5, .75);
			break;
		case GALCL:
			neb_color= Vec3f(0.5,0.5,1.0);
			posTex = Vec2f(.5, .75);
			break;
		case QUASR:
			neb_color= Vec3f(0.5,0.,0.8);
			posTex = Vec2f(.0, .25);
			break;
		case SNREM:
			neb_color= Vec3f(0.8,0.,0.5);
			posTex = Vec2f(.25, .25);
			break;
		default: //case GENRC
			neb_color = getCircleColor();
			posTex = Vec2f(.75, .0);
			break;
	}
}

Nebula::~Nebula()
{
	if (neb_tex) delete neb_tex;
	neb_tex = nullptr;
}

nebula_type Nebula::getDsoType( std::string type)
{
	if (type == "GALXY")
		return GALXY;
	else if (type == "OPNCL")
		return OPNCL;
	else if (type == "GLOCL")
		return GLOCL;
	else if (type == "BRTNB")
		return BRTNB;
	else if (type == "PLNNB")
		return PLNNB;
	else if (type == "DRKNB")
		return DRKNB;
	else if (type == "CLNEB")
		return CLNEB;
	else if (type == "STARS")
		return STARS;
	else if (type == "GALCL")
		return GALCL;
	else if (type == "QUASR")
		return QUASR;
	else if (type == "SNREM")
		return SNREM;
	else
		return GENRC;
}

std::string Nebula::getInfoString(const Navigator* nav) const
{
	float tempDE, tempRA;

	Vec3d equPos = nav->j2000ToEarthEqu(XYZ);
	Utility::rectToSphe(&tempRA,&tempDE,equPos);

	std::ostringstream oss;

	if (nameI18!="") {
		oss << nameI18 << std::endl;
	} else
		oss << englishName << std::endl;

	oss.setf(std::ios::fixed);
	oss.precision(2);

	if(mag < 99) oss << _("Magnitude: ") << mag << std::endl;

	oss << _("RA/DE: ") << Utility::printAngleHMS(tempRA) << " / " << Utility::printAngleDMS(tempDE) << std::endl;

	// calculate alt az
	Vec3d localPos = nav->earthEquToLocal(equPos);
	Utility::rectToSphe(&tempRA,&tempDE,localPos);
	tempRA = 3*C_PI - tempRA;  // N is zero, E is 90 degrees
	if (tempRA > C_PI*2) tempRA -= C_PI*2;

	oss << _("Alt/Az: ") << Utility::printAngleDMS(tempDE) << " / " << Utility::printAngleDMS(tempRA) << std::endl;
	oss << _("Type: ") << getTypeToString() << std::endl;
	oss << _("Size: ") << Utility::printAngleDMS(m_angular_size*C_PI/180.) << std::endl;
	return oss.str();
}

std::string Nebula::getShortInfoString(const Navigator*) const
{
	std::ostringstream oss;

	if (nameI18 != "")
		oss << nameI18 << "  ";
	else
		oss << englishName << "  ";

	if (mag < 99) oss << _("Magnitude: ") << mag;

	oss << " " <<  _("Type: ") << getTypeToString() << " " ;

	if( myDistance > 0 ) {
		std::string units = _("ly");
		double distance = myDistance;

		if(distance >= 1000) {
			distance /= 1000;
			if(distance < 1000) units = _("kly");
			else {
				distance /= 1000;
				units = _("Mly");
			}
		}
		oss.precision(4);
		oss << "  " <<  _("Distance: ") << distance << " " << _(units);
	}

	return oss.str();
}


std::string Nebula::getShortInfoNavString(const Navigator*, const TimeMgr * timeMgr, const Observer* observatory) const
{
	return "";
}

double Nebula::getCloseFov(const Navigator*) const
{
	return m_angular_size * 180./C_PI * 4;
}

void Nebula::drawTex(const Projector* prj, const Navigator* nav, ToneReproductor* eye, double sky_brightness)
{
	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (!neb_tex || m_hidden || !m_selected) return;

	// daylight hackery
	float ad_lum=eye->adaptLuminance(luminance);

	float color = 1;
	float nebulaScreenSize = getOnScreenSize(prj,nav);
	float minZoom = prj->getViewportHeight()/75.;
	float maxZoom = prj->getViewportHeight()/60.;


	if (flagBright && sky_brightness < 0.011 && ( nebulaScreenSize < maxZoom) && (nebulaScreenSize > minZoom)) {
	       // fade the nebula while zooming
	       color = (nebulaScreenSize - minZoom) / ( maxZoom - minZoom );
	       color *= nebulaBrightness;
	} else if (flagBright && sky_brightness < 0.011 && (getOnScreenSize(prj, nav) > maxZoom)) {

		//cout << "Bright nebula drawn for" << getEnglishName() << endl;
		color *= nebulaBrightness;
	} else {
		// TODO this should be revisited to be less ad hoc
		// 3 is a fudge factor since only about 1/3 of a texture is not black background
		float cmag = 3 * ad_lum / tex_avg_luminance * texLuminanceAdjust;
		color = color * cmag * nebulaBrightness;
		//cout << "No bright nebula drawn for" << getEnglishName() << endl;
	}

	shaderNebulaTex->use();
	shaderNebulaTex->setUniform("Mat", prj->getMatJ2000ToEye());
	shaderNebulaTex->setUniform("fader", color);

	glBindTexture (GL_TEXTURE_2D, neb_tex->getID());
	glBindVertexArray(nebulaTex.vao);

	glBindBuffer(GL_ARRAY_BUFFER,nebulaTex.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*sDataPos.size(),sDataPos.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);
	glEnableVertexAttribArray(0); //layout 0

	glBindBuffer(GL_ARRAY_BUFFER,nebulaTex.tex);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*8,sDataTex,GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);
	glEnableVertexAttribArray(1); //layout 1

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	shaderNebulaTex->unuse();
}

void Nebula::drawHint(const Projector* prj, const Navigator * nav, std::vector<float> &vecHintPos, std::vector<float> &vecHintTex, std::vector<float> &vecHintColor)
{
	if (m_hidden || !m_selected) return;
	if (2.f/getOnScreenSize(prj, nav)<0.1) return;
	float r = dsoPictoSize;

	if (displaySpecificHint) {
		//~ printf("color specific: %f %f %f\n", neb_color[0] , neb_color[1] , neb_color[2]);
		vecHintColor.push_back( neb_color[0] );
		vecHintColor.push_back(  neb_color[1] );
		vecHintColor.push_back( neb_color[2] );
		//~ vecHintColor.push_back( hintsBrightness );
		vecHintColor.push_back( neb_color[0] );
		vecHintColor.push_back(  neb_color[1] );
		vecHintColor.push_back( neb_color[2] );
		//~ vecHintColor.push_back( hintsBrightness );
		vecHintColor.push_back( neb_color[0] );
		vecHintColor.push_back(  neb_color[1] );
		vecHintColor.push_back( neb_color[2] );
		//~ vecHintColor.push_back( hintsBrightness );
		vecHintColor.push_back( neb_color[0] );
		vecHintColor.push_back(  neb_color[1] );
		vecHintColor.push_back( neb_color[2] );
		//~ vecHintColor.push_back( hintsBrightness );

		vecHintTex.push_back( posTex[0] );
		vecHintTex.push_back( posTex[1] );
		vecHintTex.push_back( posTex[0]+.25 );
		vecHintTex.push_back( posTex[1] );
		vecHintTex.push_back( posTex[0] );
		vecHintTex.push_back( posTex[1]+.25 );
		vecHintTex.push_back( posTex[0]+.25 );
		vecHintTex.push_back( posTex[1]+.25 );
	} else {
		//~ printf("color circle: %f %f %f\n",circleColor[0], circleColor[1], circleColor[2]);
		vecHintColor.push_back( circleColor[0] );
		vecHintColor.push_back(  circleColor[1] );
		vecHintColor.push_back( circleColor[2] );
		//~ vecHintColor.push_back( hintsBrightness );
		vecHintColor.push_back( circleColor[0] );
		vecHintColor.push_back(  circleColor[1] );
		vecHintColor.push_back( circleColor[2] );
		//~ vecHintColor.push_back( hintsBrightness );
		vecHintColor.push_back( circleColor[0] );
		vecHintColor.push_back(  circleColor[1] );
		vecHintColor.push_back( circleColor[2] );
		//~ vecHintColor.push_back( hintsBrightness );
		vecHintColor.push_back( circleColor[0] );
		vecHintColor.push_back(  circleColor[1] );
		vecHintColor.push_back( circleColor[2] );
		//~ vecHintColor.push_back( hintsBrightness );

		vecHintTex.push_back(0.75);
		vecHintTex.push_back(0.);
		vecHintTex.push_back(1.0);
		vecHintTex.push_back(0.);
		vecHintTex.push_back(0.75);
		vecHintTex.push_back(0.25);
		vecHintTex.push_back(1.0);
		vecHintTex.push_back(0.25);
	}

	vecHintPos.push_back( (float) (XY[0] + r) );
	vecHintPos.push_back( (float) (XY[1] - r) );
	vecHintPos.push_back( (float) (XY[0] - r) );
	vecHintPos.push_back( (float) (XY[1] - r) );
	vecHintPos.push_back( (float) (XY[0] + r) );
	vecHintPos.push_back( (float) (XY[1] + r) );
	vecHintPos.push_back( (float) (XY[0] - r) );
	vecHintPos.push_back( (float) (XY[1] + r) );
}

void Nebula::drawName(const Projector* prj)
{
	if (m_hidden || !m_selected) return;

	Vec4f Color(labelColor[0], labelColor[1], labelColor[2], hintsBrightness);
	float size = getOnScreenSize(prj);
	float shift = 8.f + size/2.f;

	std::string nebulaname = getNameI18n();

	prj->printGravity180(nebulaFont, XY[0], XY[1], nebulaname, Color, 1, shift, shift);

	// draw image credit, if it fits easily
	if (credit != "" && size > nebulaFont->getStrLen(credit, 1)) {
		prj->printGravity180(nebulaFont, XY[0]-shift-40, XY[1]+-shift-40, credit, Color, 1, 0, 0);
	}
}


std::string Nebula::getTypeToString(void) const
{
	switch (DSOType) {
		case GALXY:
			return _("Galaxy");
			break;
		case OPNCL:
			return _("Open Cluster");
			break;
		case GLOCL:
			return _("Globular Cluster");
			break;
		case DRKNB:
			return _("Dark Nebula");
			break;
		case PLNNB:
			return _("Planetary Nebula");
			break;
		case BRTNB:
			return _("Bright Nebula");
			break;
		case CLNEB:
			return _("Cluster + Nebula");
			break;
		case STARS:
			return _("Stars");
			break;
		case GALCL:
			return _("Galaxies Cluster");
			break;
		case QUASR:
			return _("Quasar");
			break;
		case SNREM:
			return _("Supernova Remnant");
			break;
		default:
			return _("Unknown");
	}
	return "";
}

void Nebula::translateName(Translator& trans)
{
	nameI18 = trans.translateUTF8(englishName);
}

