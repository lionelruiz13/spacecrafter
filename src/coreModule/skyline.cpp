/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include "bodyModule/body.hpp"
#include "coreModule/skyline.hpp"
#include "tools/s_texture.hpp"
#include "tools/utility.hpp"
#include <string>
#include "navModule/observer.hpp"
#include "tools/OpenGL.hpp"
#include "tools/shader.hpp"
//2346 lignes avant 
//2479 lignes apres
//1560 lignes au final

std::unique_ptr<shaderProgram> SkyLine::shaderSkylineDraw;
std::unique_ptr<VertexArray> SkyLine::m_skylineGL;

SkyLine::SkyLine(double _radius, unsigned int _nb_segment) :
	radius(_radius), nb_segment(_nb_segment), color(0.f, 0.f, 1.f), font(nullptr)
{
}

SkyLine::~SkyLine()
{
	if (font) delete font;
	font = nullptr;
}

void SkyLine::createSC_context()
{
	m_skylineGL = std::make_unique<VertexArray>();
	m_skylineGL->registerVertexBuffer(BufferType::POS2D, BufferAccess::DYNAMIC);
}

void SkyLine::drawSkylineGL(const Vec4f& Color)
{
	m_skylineGL->fillVertexBuffer(BufferType::POS2D, vecDrawPos);
	
	shaderSkylineDraw->use();
	shaderSkylineDraw->setUniform("Color",Color);
	m_skylineGL->bind();
	glDrawArrays(GL_LINES, 0 ,vecDrawPos.size()/2);
	m_skylineGL->unBind();

	shaderSkylineDraw->unuse();
}

void SkyLine::setFont(float font_size, const std::string& font_name)
{
	if (font) {
		delete font;
		font = nullptr;
	}
	font = new s_font(font_size, font_name);
	assert(font);
}

void SkyLine::translateLabels(Translator& trans)
{

	month[1] = trans.translateUTF8("JAN");
	month[2] = trans.translateUTF8("FEB");
	month[3] = trans.translateUTF8("MAR");
	month[4] = trans.translateUTF8("APR");
	month[5] = trans.translateUTF8("MAY");
	month[6] = trans.translateUTF8("JUN");
	month[7] = trans.translateUTF8("JUL");
	month[8] = trans.translateUTF8("AUG");
	month[9] = trans.translateUTF8("SEP");
	month[10] = trans.translateUTF8("OCT");
	month[11] = trans.translateUTF8("NOV");
	month[12] = trans.translateUTF8("DEC");

	if(font) font->clearCache();
}


void SkyLine::createShader()
{
	shaderSkylineDraw = std::make_unique<shaderProgram>();
	shaderSkylineDraw->init( "skylineDraw.vert", "skylineDraw.frag");
	shaderSkylineDraw->setUniformLocation("Color");
}

// -------------------- SKYLINE_POLE ---------------------------------------------

SkyLine_Pole::SkyLine_Pole(SKY_LINE_POLE_TYPE _line_pole_type, double _radius = 1.0f, unsigned int _nb_segment = 10): SkyLine(_radius, _nb_segment)
{
	line_pole_type = _line_pole_type;
	switch (line_pole_type) {
		case POLE:
			proj_func = &Projector::projectEarthEqu;
			break;
		case ECLIPTIC_POLE:
			proj_func = &Projector::projectEarthEcliptic;
			break;
		case GALACTIC_POLE:
			proj_func = &Projector::projectJ2000Galactic;
			break;
		default :
			proj_func = &Projector::projectEarthEqu;
	}
}

SkyLine_Pole::~SkyLine_Pole()
{
}

void SkyLine_Pole::draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory)
{
	if (!fader.getInterstate()) return;

	Vec4f Color (color[0], color[1], color[2], fader.getInterstate());

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	for (unsigned int i=0; i<51; ++i) {
		Utility::spheToRect((float)i/(50)*2.f*M_PI,radius*M_PI/180.f, circlep[i]);
	}
	for (int i=0; i < 50; i++) {
		if ((prj->*proj_func)(circlep[i], pt1) && (prj->*proj_func)(circlep[i+1], pt2) ) {
			insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);
		}
	}
	for (unsigned int i=0; i<51; ++i) {
		Utility::spheToRect((float)i/(50)*2.f*M_PI, -radius*M_PI/180.f,circlep[i]);
	}
	for (int i=0; i < 50; i++) {
		if ((prj->*proj_func)(circlep[i], pt1) && (prj->*proj_func)(circlep[i+1], pt2) ) {
			insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);
		}
	}

	drawSkylineGL(Color);

	vecDrawPos.clear();
}


// -------------------- SKYLINE_ZODIAC ---------------------------------------------

SkyLine_Zodiac::SkyLine_Zodiac(double _radius , unsigned int _nb_segment ):
	SkyLine(_radius, _nb_segment)
{
	zod[10]="     ARIES";
	zod[11]="  TAURUS";
	zod[12]="   GEMINI";
	zod[1]="   CANCER";
	zod[2]="      LEO";
	zod[3]="    VIRGO";
	zod[4]="    LIBRA";
	zod[5]="  SCORPIUS";
	zod[6]="SAGITTARIUS";
	zod[7]="CAPRICORNUS";
	zod[8]=" AQUARIUS";
	zod[9]="    PISCES";
	proj_func = &Projector::projectEarthEqu;
	derivation=0;
}

SkyLine_Zodiac::~SkyLine_Zodiac()
{
}

void SkyLine_Zodiac::draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory)
{
	if (!fader.getInterstate()) return;
	// TODO changer cette condition
	if (!(observatory->isEarth())) return;

	Vec4f Color (color[0], color[1], color[2], fader.getInterstate());

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	inclination=66.5*M_PI/180.;
	derivation = 0;  //(nav->getJDay()-2451545.0)/(365.2422*71.67)*M_PI/180.0;

	// VERTICAL
	for (int i=0; i<12; i++) {
		for (int j=-4; j<=4; j++) {
			alpha=i*30*M_PI/180.-derivation;
			delta=j*4*M_PI/180.;
			Utility::spheToRect(atan2(sin(alpha), sin(inclination)*cos(alpha)-cos(inclination)*tan(delta)+1.0E-20)+M_PI/2.0, asin(sin(delta)*sin(inclination)+cos(delta)*cos(inclination)*cos(alpha)), punts[j+4]);
		}
		for (int j=0; j<8; j++) {
			if ((prj->*proj_func)(punts[j], pt1) && (prj->*proj_func)(punts[j+1], pt2) ) {

				insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);
			}
		}
	}

	//HAUT
	for (int i=0; i<=48; i++) {

		alpha=i*7.5*M_PI/180.-derivation;
		delta=16*M_PI/180.;
		Utility::spheToRect(atan2(sin(alpha),sin(inclination)*cos(alpha)-cos(inclination)*tan(delta)+1.0E-20)+M_PI/2.0,asin(sin(delta)*sin(inclination)+cos(delta)*cos(inclination)*cos(alpha)),punts[i]);
	}
	for (int i=0; i < 48; i++) {
		if ((prj->*proj_func)(punts[i], pt1) && (prj->*proj_func)(punts[i+1], pt2) ) {

			insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);

			const double dx = pt2[0]-pt1[0];
			const double dy = pt2[1]-pt1[1];
			const double dq = dx*dx+dy*dy;
			const double d = sqrt(dq);
			if ((i%4)==2) {
				double angle = acos((pt1[1]-pt2[1])/d);
				if ( pt1[0] < pt2[0] ) {
					angle *= -1;
				}
				// draw text label
				std::ostringstream oss;

				// TODO: center labels
				oss << zod[((i-2)/4)+1] << "°";

				Mat4f MVP = prj->getMatProjectionOrtho2D();
				TRANSFO= Mat4f::translation( Vec3f(pt2[0],pt2[1],0) );
				TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), M_PI_2-angle );

				font->print(0,-5,oss.str(), Color, MVP*TRANSFO ,1);
			}
		}
	}

	//BAS
	for (int i=0; i<=48; i++) {
		alpha=i*7.5*M_PI/180.-derivation;
		delta=-16*M_PI/180.;
		Utility::spheToRect(atan2(sin(alpha),sin(inclination)*cos(alpha)-cos(inclination)*tan(delta)+1.0E-20)+M_PI/2.0,asin(sin(delta)*sin(inclination)+cos(delta)*cos(inclination)*cos(alpha)),punts[i]);
	}
	for (int i=0; i < 48; i++) {
		if ((prj->*proj_func)(punts[i], pt1) && (prj->*proj_func)(punts[i+1], pt2) ) {

			insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);
		}
	}

	drawSkylineGL(Color);

	vecDrawPos.clear();
}


// -------------------- SKYLINE_CIRCUMPOLAR  ---------------------------------------------

SkyLine_CircumPolar::SkyLine_CircumPolar(double _radius = 1., unsigned int _nb_segment = 48):
	SkyLine(_radius, _nb_segment)
{
	inclination =0;
	proj_func = &Projector::projectEarthEqu;
	Mat4f rotation = Mat4f::xrotation(inclination*M_PI/180.f);

	// Points to draw along the circle
	punts  = new Vec3f[3*nb_segment+3];
	points = new Vec3f[3*nb_segment+3];

	for (unsigned int i=0; i<nb_segment+1; ++i) {
		Utility::spheToRect((float)i/(nb_segment)*2.f*M_PI, 0.f, points[i]);
		points[i] *= radius;
		points[i].transfo4d(rotation);
	}
}

SkyLine_CircumPolar::~SkyLine_CircumPolar()
{
	delete [] points;
	points = nullptr;
	delete [] punts;
	punts = nullptr;
}

void SkyLine_CircumPolar::draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory)
{
	if (!fader.getInterstate()) return;
	Vec4f Color (color[0], color[1], color[2], fader.getInterstate());

	for (double sign=-1; sign<2; sign=sign+2) {

		StateGL::enable(GL_BLEND);

		StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

		inclination=(90.0-abs(observatory->getLatitude()))*M_PI/180.;
		//Vec3f punts[3*nb_segment+3];
		inclination *= sign;

		for (unsigned int i=0; i<nb_segment; i += 2) {
			for (unsigned int j=0; j<nb_segment+1; ++j) {
				Utility::spheToRect((float)j/(nb_segment)*2.f*M_PI, inclination, points[j+nb_segment+1]);
				points[j+nb_segment+1] *= radius;
			}

			if((prj->*proj_func)(points[nb_segment+1+i], pt1) && (prj->*proj_func)(points[nb_segment+1+i+1], pt2)) {

				insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);
			}
			if((prj->*proj_func)(punts[nb_segment+1+i], pt1) && (prj->*proj_func)(punts[nb_segment+1+i+1], pt2)) {

				insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);
			}

		}
	}

	drawSkylineGL(Color);

	vecDrawPos.clear();
}


// -------------------- SKYLINE_ANALEMME  ---------------------------------------------

SkyLine_Analemme::SkyLine_Analemme(SKY_LINE_ANALEMME_TYPE _line_analemme_type = ANALEMMA, double _radius = 1., unsigned int _nb_segment = 48):
	SkyLine(_radius, _nb_segment)
{
	line_analemme_type = _line_analemme_type;
	switch (line_analemme_type) {
		case ANALEMMALINE:
			proj_func = &Projector::projectEarthEqu;
			break;
		case ANALEMMA:
			proj_func = &Projector::projectEarthEquFixed;
			break;
		default :
			proj_func = &Projector::projectEarthEqu;
	}
	float tmp_ana_ad[93] = {
		90.7365,91.1986,91.6318,92.0298,92.3875,92.7002,92.9632,93.173,93.3276,93.4268,93.4721,93.4663,93.4124,
		93.3132,93.1718,92.9917,92.7773,92.534,92.2681,91.9857,91.6923,91.3924,91.0908,90.7921,90.5015,90.2247,
		89.9677,89.7347,89.5293,89.3539,89.2108,89.1019,89.0296,88.9959,89.0015,89.0454,89.1251,89.2373,89.3781,
		89.5438,89.7309,89.9348,90.1497,90.3686,90.5845,90.7901,90.9796,91.1484,91.2924,91.4074,91.4892,91.5341,
		91.5391,91.5028,91.4255,91.3088,91.1544,90.964,90.7395,90.4831,90.1977,89.8879,89.559,89.2164,88.8653,
		88.5103,88.1559,87.8065,87.4679,87.1462,86.8477,86.5782,86.3426,86.1448,85.9884,85.8771,85.8152,85.8065,
		85.8537,85.9584,86.1202,86.3369,86.6061,86.925,87.2902,87.697,88.1392,88.6091,89.0968,89.5927,90.0877,
		90.5737,90.7365
	};
	float tmp_ana_de[93] = {
		-23.0377,-22.6619,-22.1667,-21.5556,-20.8332,-20.0048,-19.0765,-18.0552,-16.9479,-15.7616,-14.5034,-13.1803,
		-11.7995,-10.3686,-8.8953,-7.3871,-5.8511,-4.294,-2.7223,-1.1424,0.4391,2.0152,3.5793,5.125,6.6462,8.1372,
		9.5922,11.0051,12.3698,13.6801,14.93,16.1139,17.2265,18.2626,19.217,20.0846,20.8607,21.541,22.1216,22.5995,
		22.9718,23.2364,23.3918,23.437,23.3722,23.1979,22.9154,22.5266,22.0338,21.4399,20.7487,19.9644,19.0918,18.1355,
		17.1004,15.9913,14.8134,13.5722,12.2737,10.9237,9.5281,8.0923,6.6217,5.1217,3.5981,2.0571,0.505,-1.0522,-2.6083,
		-4.1577,-5.6943,-7.2117,-8.7032,-10.1618,-11.5803,-12.9517,-14.2694,-15.5262,-16.715,-17.8287,-18.8599,-19.8016,
		-20.6471,-21.3905,-22.0259,-22.5484,-22.9536,-23.2378,-23.3987,-23.4346,-23.3451,-23.1308,-23.0377
	};
	for(int i=0; i< 93; i++) {
		ana_ad[i]=tmp_ana_ad[i];
		ana_de[i]=tmp_ana_de[i];
	}
}

SkyLine_Analemme::~SkyLine_Analemme()
{
}

void SkyLine_Analemme::draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory)
{
	if (!fader.getInterstate()) return;
	if (!(observatory->isEarth()))return;

	Vec3f tmp;

	Vec4f Color(color[0], color[1], color[2], fader.getInterstate());

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	double longitude;
	if(line_analemme_type==ANALEMMALINE) {
		jd = timeMgr->getJDay();
		longitude=0;
		lati = 90;
		T = -79.45+360*((jd - 2451545.0) / 365.2422-int((jd - 2451545.0) / 365.2422));
	} else {
		longitude=observatory->getLongitude();
		lati=90;
		T=0;
	}

	for (int i=0; i < 93; i++) {
		Utility::spheToRect((ana_ad[i]-90+T-longitude)*M_PI/180,(90-lati+ana_de[i])*M_PI/180,analemma[i]);
	}


	for (int i=0; i < 92; i++) {
		if ((prj->*proj_func)(analemma[i], pt1) && (prj->*proj_func)(analemma[i+1], pt2) ) {

			insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);
		}
	}

	drawSkylineGL(Color);

	vecDrawPos.clear();
}


// -------------------- SKYLINE_GALACTIC_CENTER  ---------------------------------------------

SkyLine_Galactic_Center::SkyLine_Galactic_Center( double _radius = 1., unsigned int _nb_segment = 48):
	SkyLine(_radius, _nb_segment)
{
	proj_func = &Projector::projectJ2000Galactic;
	inclination=0*M_PI/180.;
	derivation = 90*M_PI/180.;
}

SkyLine_Galactic_Center::~SkyLine_Galactic_Center()
{
}

void SkyLine_Galactic_Center::draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory)
{
	if (!fader.getInterstate()) return;

	Vec4f Color(color[0], color[1], color[2], fader.getInterstate());

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	for (unsigned int j=0; j<=1; j++) {
		for (unsigned int i=0; i<=48; i++) {

			alpha=i*7.5*M_PI/180.-derivation;
			delta=268.5*M_PI/180.;
			Utility::spheToRect(atan2(sin(alpha),sin(inclination)*cos(alpha)-cos(inclination)*tan(delta)+1.0E-20)+(j*M_PI),asin(sin(delta)*sin(inclination)+cos(delta)*cos(inclination)*cos(alpha)),punts[i]);
		}
		for (int i=0; i < 48; i++) {
			if ((prj->*proj_func)(punts[i], pt1) && (prj->*proj_func)(punts[i+1], pt2) ) {

				insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);
			}
		}
	}

	drawSkylineGL(Color);

	vecDrawPos.clear();
}


// -------------------- SKYLINE_VERNAL  ---------------------------------------------

SkyLine_Vernal::SkyLine_Vernal( double _radius = 1., unsigned int _nb_segment = 48):
	SkyLine(_radius, _nb_segment)
{
	proj_func = &Projector::projectEarthEqu;
	inclination=0*M_PI/180.;
	derivation = 90*M_PI/180.;
}

SkyLine_Vernal::~SkyLine_Vernal()
{
}

void SkyLine_Vernal::draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory)
{
	if (!fader.getInterstate()) return;

	Vec4f Color(color[0], color[1], color[2], fader.getInterstate());

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	for (unsigned int j=0; j<=1; j++) {
		for (unsigned int i=0; i<=48; i++) {

			alpha=i*7.5*M_PI/180.-derivation;
			delta=268.5*M_PI/180.;
			Utility::spheToRect(atan2(sin(alpha),sin(inclination)*cos(alpha)-cos(inclination)*tan(delta)+1.0E-20)+(j*M_PI),asin(sin(delta)*sin(inclination)+cos(delta)*cos(inclination)*cos(alpha)),punts[i]);
		}
		for (int i=0; i < 48; i++) {
			if ((prj->*proj_func)(punts[i], pt1) && (prj->*proj_func)(punts[i+1], pt2) ) {

				insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);
			}
		}
	}
	drawSkylineGL(Color);

	vecDrawPos.clear();
}




// -------------------- SKYLINE_GREENWICH  ---------------------------------------------

SkyLine_Greenwich::SkyLine_Greenwich(double _radius = 1., unsigned int _nb_segment = 48) :
	SkyLine(_radius, _nb_segment)
{
	proj_func = &Projector::projectEarthEquFixed;
}

SkyLine_Greenwich::~SkyLine_Greenwich()
{
}

void SkyLine_Greenwich::draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory)
{
	if (!fader.getInterstate()) return;
	if (!(observatory->isEarth())) return;

	Vec4f Color(color[0], color[1], color[2], fader.getInterstate());

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	latitude=(observatory->getLatitude()*M_PI/180)+M_PI/2;
	double longitude=(observatory->getLongitude()*M_PI/180);
	for (unsigned int i=0; i<60; i++) {
		Utility::spheToRect(-2*longitude,(float)i/59*(2*M_PI),punts[i]);
	}
	for (int i=0; i < 59; i++) {
		if ((prj->*proj_func)(punts[i], pt1) && (prj->*proj_func)(punts[i+1], pt2) ) {

			insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);
		}
	}

	Utility::spheToRect(-2*longitude,(((45.0/59.f)*2*M_PI)+(1*latitude)) ,punt[0]);
	Utility::spheToRect(-2*longitude,(((46.0/59.f)*2*M_PI)+(1*latitude)) ,punt[1]);

	//TODO all this for a single text ?????
	if ((prj->*proj_func)(punt[1],pt1) && (prj->*proj_func)(punt[0],pt2) ) {
		const double dx = pt2[0]-pt1[0];
		const double dy = pt2[1]-pt1[1];
		const double dq = dx*dx+dy*dy;
		double angle;
		const double d = sqrt(dq);
		angle = acos((pt1[1]-pt2[1])/d);
		if ( pt1[0] < pt2[0] ) {
			angle *= -1;
		}

		Mat4f MVP = prj->getMatProjectionOrtho2D();
		TRANSFO= Mat4f::translation( Vec3f(pt1[0],pt1[1],0) );
		TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), -angle );

		if (font) font->print(2,-2,"GM", Color, MVP*TRANSFO ,1);
	}

	drawSkylineGL(Color);

	vecDrawPos.clear();
}


// -------------------- SKYLINE_ARIES  ---------------------------------------------

SkyLine_Aries::SkyLine_Aries(double _radius = 1., unsigned int _nb_segment = 48) :
	SkyLine(_radius, _nb_segment)
{
	proj_func = &Projector::projectEarthEqu;
}

SkyLine_Aries::~SkyLine_Aries()
{
}

void SkyLine_Aries::draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory)
{
	if (!fader.getInterstate()) return;
	if (!(observatory->isEarth())) return;

	Vec4f Color(color[0], color[1], color[2], fader.getInterstate());

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	latitude=(observatory->getLatitude()*M_PI/180)+M_PI/2;
	for (unsigned int i=0; i<60; i++) {
		Utility::spheToRect(0,(float)i/59*(2*M_PI),punts[i]);
	}
	for (int i=0; i < 59; i++) {
		if ((prj->*proj_func)(punts[i], pt1) && (prj->*proj_func)(punts[i+1], pt2) ) {

			insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);
		}
	}
	Utility::spheToRect(0,(((45.0/59.f)*2*M_PI)+(1*latitude)) ,punt[0]);
	Utility::spheToRect(0,(((46.0/59.f)*2*M_PI)+(1*latitude)) ,punt[1]);

	if ((prj->*proj_func)(punt[1],pt1) &&(prj->*proj_func)(punt[0],pt2) ) {
		const double dx = pt2[0]-pt1[0];
		const double dy = pt2[1]-pt1[1];
		const double dq = dx*dx+dy*dy;
		double angle;
		const double d = sqrt(dq);
		angle = acos((pt1[1]-pt2[1])/d);
		if ( pt1[0] < pt2[0] ) {
			angle *= -1;
		}

		Mat4f MVP = prj->getMatProjectionOrtho2D();
		TRANSFO= Mat4f::translation( Vec3f(pt1[0],pt1[1],0) );
		TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), -angle );

		if (font) font->print(2,-2,"Aries", Color, MVP*TRANSFO ,1);
	}

	drawSkylineGL(Color);

	vecDrawPos.clear();
}


// -------------------- SKYLINE_MERIDIAN  ---------------------------------------------

SkyLine_Meridian::SkyLine_Meridian(double _radius = 1., unsigned int _nb_segment = 48):
	SkyLine(_radius, _nb_segment)
{
	proj_func = &Projector::projectLocal;
	inclination = 90;

	Mat4f rotation = Mat4f::xrotation(inclination*M_PI/180.f);

	// Points to draw along the circle
	points = new Vec3f[3*nb_segment+3];
	for (unsigned int i=0; i<nb_segment+1; ++i) {
		Utility::spheToRect((float)i/(nb_segment)*2.f*M_PI, 0.f, points[i]);
		points[i] *= radius;
		points[i].transfo4d(rotation);
	}
}

SkyLine_Meridian::~SkyLine_Meridian()
{
	delete [] points;
	points = nullptr;
}

void SkyLine_Meridian::draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory)
{
	if (!fader.getInterstate()) return;

	Vec4f Color (color[0], color[1], color[2], fader.getInterstate());

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	inclination=70*M_PI/180.;
	for (unsigned int j=0; j<nb_segment+1; ++j) {
		Utility::spheToRect((float)j/(nb_segment)*2.f*M_PI, inclination, points[j+nb_segment+1]);
		points[j+nb_segment+1] *= radius;
	}

	for (unsigned int i=0; i<nb_segment; ++i) {
		if (internalNav) {
			inclination=70*M_PI/180.;

			if((prj->*proj_func)(points[nb_segment+1+i], pt1) && (prj->*proj_func)(points[nb_segment+1+i+1], pt2)) {

				insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);

				// Draw hour ticks
				if ((i+1) % ((nb_segment/36 )*2) == 0) {
					//TODO: Center labels
					const double dx = pt2[0]-pt1[0];
					const double dy = pt2[1]-pt1[1];
					const double dq = dx*dx+dy*dy;
					double angle;
					const double d = sqrt(dq);

					angle = acos((pt1[1]-pt2[1])/d);
					if ( pt1[0] < pt2[0] ) {
						angle *= -1;
					}

					// draw text label
					std::ostringstream oss;
					float tickl = 4.0;

					double res;
					if (i<18*(nb_segment/36)) res = 180-(i+1)/((nb_segment/36.0))*10;
					if (i>18*(nb_segment/36)) res = 540-(i+1)/((nb_segment/36.0))*10;
					if ((i+1)%((nb_segment/36)*2) == 0) {
						oss << res << "°";
						tickl = 4.0;
					} else if ((i+2-5)%((nb_segment/36)*2) == 0) {
						tickl = 2.0;
					} else tickl = 1.0;

					Mat4f MVP = prj->getMatProjectionOrtho2D();
					TRANSFO= Mat4f::translation( Vec3f(pt2[0],pt2[1],0) );
					TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), M_PI-angle );

					TRANSFO= Mat4f::translation( Vec3f(pt2[0],pt2[1],0) );
					TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), M_PI-angle );

					tmp = TRANSFO * Vec4f(-tickl,0.0,0.0,1.0);
					insert_all(vecDrawPos, tmp[0], tmp[1]);

					tmp = TRANSFO * Vec4f(tickl,0.0,0.0,1.0);
					insert_all(vecDrawPos, tmp[0], tmp[1]);

					if (font) font->print(2,-2,oss.str(), Color, MVP*TRANSFO ,1);
				}
			}
		}

		if ((prj->*proj_func)(points[i], pt1) && (prj->*proj_func)(points[i+1], pt2) ) {
			const double dx = pt1[0]-pt2[0];
			const double dy = pt1[1]-pt2[1];
			const double dq = dx*dx+dy*dy;

			double angle;

			insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);

			// Draw text labels and ticks on meridian
			const double d = sqrt(dq);

			angle = acos((pt1[1]-pt2[1])/d);
			if ( pt1[0] < pt2[0] ) {
				angle *= -1;
			}

			// draw text label
			std::ostringstream oss;
			int valdeg = 0;
			float tickl = 5.0;
			if (i<8*(nb_segment/36))
				valdeg = (i+1)*10/(nb_segment/36);
			else {
				valdeg = 180-((i+1)*10/(nb_segment/36));
				if (valdeg<-90) valdeg=-180-valdeg;
				else angle += M_PI;
			}

			if ((i+1)%(nb_segment/36)==0) {
				if (valdeg<=90) oss << valdeg << "°";
				tickl = 5.0;
			} else if ((i+1-5)%(nb_segment/36)== 0) {
				tickl = 3.0;
			} else {
				tickl = 2.0;
			}

			if ( valdeg==90 ) {
				angle += M_PI;
			}

			Mat4f MVP = prj->getMatProjectionOrtho2D();
			TRANSFO= Mat4f::translation( Vec3f(pt2[0],pt2[1],0) );
			TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), M_PI-angle );

			tmp = TRANSFO * Vec4f(-tickl,0.0,0.0,1.0);
			insert_all(vecDrawPos, tmp[0], tmp[1]);

			tmp = TRANSFO * Vec4f(tickl,0.0,0.0,1.0);
			insert_all(vecDrawPos, tmp[0], tmp[1]);

			if (font) font->print(2,-2,oss.str(), Color, MVP*TRANSFO ,1);
		}
	}
	drawSkylineGL(Color);

	vecDrawPos.clear();
}



// -------------------- SKYLINE_EQUATOR  ---------------------------------------------

SkyLine_Equator::SkyLine_Equator(SKY_LINE_EQUATOR_LINE _line_equator_type, double _radius = 1., unsigned int _nb_segment = 48):
	SkyLine(_radius, _nb_segment)
{
	line_equator_type = _line_equator_type;

	switch (line_equator_type) {
		case EQUATOR :
			proj_func = &Projector::projectEarthEqu;
			break;
		case GALACTIC_EQUATOR :
			proj_func = &Projector::projectJ2000Galactic;
			break;
		default :
			proj_func = &Projector::projectEarthEqu;
	}

	inclination=0.;
	Mat4f rotation = Mat4f::xrotation(inclination*M_PI/180.f);

	// Points to draw along the circle
	points = new Vec3f[3*nb_segment+3];
	for (unsigned int i=0; i<nb_segment+1; ++i) {
		Utility::spheToRect((float)i/(nb_segment)*2.f*M_PI, 0.f, points[i]);
		points[i] *= radius;
		points[i].transfo4d(rotation);
	}
}

SkyLine_Equator::~SkyLine_Equator()
{
	delete [] points;
	points = nullptr;
}

void SkyLine_Equator::draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory)
{
	if (!fader.getInterstate()) return;

	Vec4f Color(color[0], color[1], color[2], fader.getInterstate());

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	if (line_equator_type == EQUATOR) {
		inclination=70*M_PI/180.;
		for (unsigned int j=0; j<nb_segment+1; ++j) {
			Utility::spheToRect((float)j/(nb_segment)*2.f*M_PI, inclination, points[j+nb_segment+1]);
			points[j+nb_segment+1] *= radius;
		}
	}
	for (unsigned int i=0; i<nb_segment; ++i) {
		if (internalNav) {
			inclination=70*M_PI/180.;

			if((prj->*proj_func)(points[nb_segment+1+i], pt1) && (prj->*proj_func)(points[nb_segment+1+i+1], pt2)) {

				insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);

				// Draw hour ticks
				if ((i+1) % ((nb_segment/48)*2) == 0) {

					//TODO: Center labels
					const double dx = pt2[0]-pt1[0];
					const double dy = pt2[1]-pt1[1];
					const double dq = dx*dx+dy*dy;
					double angle;
					const double d = sqrt(dq);

					angle = acos((pt1[1]-pt2[1])/d);
					if ( pt1[0] < pt2[0] ) {
						angle *= -1;
					}

					// draw text label
					std::ostringstream oss;
					float tickl = 4.0;
					if (((i)/(nb_segment/24)+1)%24>9)
						oss << ((i)/(nb_segment/24)+1)%24 << "h   " << (24-((i)/(nb_segment/24)+1)%24)*15 << "°";
					else
						oss << " " << ((i)/(nb_segment/24)+1)%24 << "h   " << (24-((i)/(nb_segment/24)+1)%24)*15 << "°";

					Mat4f MVP = prj->getMatProjectionOrtho2D();
					TRANSFO= Mat4f::translation( Vec3f(pt2[0],pt2[1],0) );
					TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), M_PI-angle );

					tmp = TRANSFO * Vec4f(-tickl,0.0,0.0,1.0);
					insert_all(vecDrawPos, tmp[0], tmp[1]);

					tmp = TRANSFO * Vec4f( tickl,0.0,0.0,1.0);
					insert_all(vecDrawPos, tmp[0], tmp[1]);

					if (font) font->print(-24,-2,oss.str(), Color, MVP*TRANSFO ,1);
				}
			}
		}

		if ((prj->*proj_func)(points[i], pt1) && (prj->*proj_func)(points[i+1], pt2) ) {
			const double dx = pt1[0]-pt2[0];
			const double dy = pt1[1]-pt2[1];
			const double dq = dx*dx+dy*dy;

			double angle;

			// TODO: allow for other numbers of meridians and parallels without
			// screwing up labels?

			insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);

			// Draw text labels and ticks on equator
			if ((i+1) % ((nb_segment/48)*2) == 0) {
				const double d = sqrt(dq);
				angle = acos((pt1[1]-pt2[1])/d);
				if ( pt1[0] < pt2[0] ) {
					angle *= -1;
				}
				// draw text label
				std::ostringstream oss;
				int tickl = 3;

				if ((internalNav) && (line_equator_type != GALACTIC_EQUATOR)) {

					double num = 360.0f/(nb_segment/2.f)*(nb_segment/2.f-(i+1)/2.f);
					if (fmod(num,15) == 0) {
						tickl = 8;
						if ((i+1)/2 == 24*4) oss << " 0h   ";
						else {
							if ((i+1)/(2*4)<10) oss << " ";
							oss << (i+1)/(2*4) << "h   ";
						}
						oss << num << "°";
					} else if (fmod(num,7.5) == 0) {
						oss << num << "°";
						tickl = 4;
					} else tickl = 2;
				} else {
					if (line_equator_type == GALACTIC_EQUATOR)
						oss << ((i+37)%72)*5 << "°";
					else if ((i+1)/2 == 24*4) oss << "0h";
					else oss << (i+1)/(2*4) << "h";
				}

				Mat4f MVP = prj->getMatProjectionOrtho2D();
				TRANSFO= Mat4f::translation( Vec3f(pt2[0],pt2[1],0) );
				TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), M_PI-angle );

				if ((internalNav) && (line_equator_type != GALACTIC_EQUATOR)) {
					if ((i+1) % (2*4) == 0) {

						tmp = TRANSFO * Vec4f(-tickl,0.0,0.0,1.0);
						insert_all(vecDrawPos, tmp[0], tmp[1]);

						tmp = TRANSFO * Vec4f( tickl,0.0,0.0,1.0);
						insert_all(vecDrawPos, tmp[0], tmp[1]);
					}
				} else {

					tmp = TRANSFO * Vec4f(-tickl,0.0,0.0,1.0);
					insert_all(vecDrawPos, tmp[0], tmp[1]);

					tmp = TRANSFO * Vec4f( tickl,0.0,0.0,1.0);
					insert_all(vecDrawPos, tmp[0], tmp[1]);
				}

				if (((i+1)%2==0) && font && ((internalNav) && (line_equator_type != GALACTIC_EQUATOR)))
					font->print(-26,-2,oss.str(), Color, MVP*TRANSFO ,1);
				if (((i+1)%2==0) && font && !((internalNav) && (line_equator_type != GALACTIC_EQUATOR)))
					font->print(2,-2,oss.str(), Color, MVP*TRANSFO ,1);
			}
		}
	}

	drawSkylineGL(Color);

	vecDrawPos.clear();
}


// -------------------- SKYLINE_TROPIC  ---------------------------------------------

SkyLine_Tropic::SkyLine_Tropic(double _radius = 1., unsigned int _nb_segment = 48):
	SkyLine(_radius, _nb_segment)
{
	proj_func = &Projector::projectEarthEqu;
	points = new Vec3f[3*nb_segment+3];
	inclination=0;
	Mat4f rotation = Mat4f::xrotation(inclination*M_PI/180.f);
	for (unsigned int i=0; i<nb_segment+1; ++i) {
		Utility::spheToRect((float)i/(nb_segment)*2.f*M_PI, 0.f, points[i]);
		points[i] *= radius;
		points[i].transfo4d(rotation);
	}
}

SkyLine_Tropic::~SkyLine_Tropic()
{
	delete [] points;
	points = nullptr;
}

void SkyLine_Tropic::draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory)
{
	if (!fader.getInterstate()) return;

	// Not valid in Space
	if (!observatory->isOnBody())
		return;

	// Not valid on non-planets
	if ( (observatory->getHomeBody()->isSatellite()) || observatory->isSun()) return;

	Vec4f Color(color[0], color[1], color[2], fader.getInterstate());

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	for (unsigned int i=0; i<nb_segment; ++i) {
		inclination=observatory->getHomeBody()->getAxialTilt()*M_PI/180.;

		for (unsigned int j=0; j<nb_segment+1; ++j) {
			Utility::spheToRect((float)j/(nb_segment)*2.f*M_PI, inclination, points[j+nb_segment+1]);
			points[j+nb_segment+1] *= radius;
			Utility::spheToRect((float)j/(nb_segment)*2.f*M_PI, -inclination, points[j+2*nb_segment+2]);
			points[j+2*nb_segment+2] *= radius;
		}

		// Draw equator
		if ((prj->*proj_func)(points[i], pt1) && (prj->*proj_func)(points[i+1], pt2) ) {

			insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);

			if((i+1) % 4 == 0) {
				const double dx = pt1[0]-pt2[0];
				const double dy = pt1[1]-pt2[1];
				const double dq = dx*dx+dy*dy;
				double angle;
				const double d = sqrt(dq);

				angle = acos((pt1[1]-pt2[1])/d);
				if( pt1[0] < pt2[0] ) {
					angle *= -1;
				}

				TRANSFO= Mat4f::translation( Vec3f(pt2[0],pt2[1],0) );
				TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), M_PI-angle );

				tmp = TRANSFO * Vec4f(-3.0,0.0,0.0,1.0);
				insert_all(vecDrawPos, tmp[0], tmp[1]);

				tmp = TRANSFO * Vec4f( 3.0,0.0,0.0,1.0);
				insert_all(vecDrawPos, tmp[0], tmp[1]);
			}
		}

		if((prj->*proj_func)(points[nb_segment+1+i], pt1) && (prj->*proj_func)(points[nb_segment+1+i+1], pt2)) {

			insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);

			if((i+1) % 4 == 0) {
				const double dx = pt1[0]-pt2[0];
				const double dy = pt1[1]-pt2[1];
				const double dq = dx*dx+dy*dy;
				double angle;
				const double d = sqrt(dq);

				angle = acos((pt1[1]-pt2[1])/d);
				if( pt1[0] < pt2[0] ) {
					angle *= -1;
				}

				TRANSFO= Mat4f::translation( Vec3f(pt2[0],pt2[1],0) );
				TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), M_PI-angle );

				tmp = TRANSFO * Vec4f(-3.0,0.0,0.0,1.0);
				insert_all(vecDrawPos, tmp[0], tmp[1]);

				tmp = TRANSFO * Vec4f( 3.0,0.0,0.0,1.0);
				insert_all(vecDrawPos, tmp[0], tmp[1]);
			}

			if( (prj->*proj_func)(points[2*nb_segment+2+i], pt1) && (prj->*proj_func)(points[2*nb_segment+2+i+1], pt2)) {

				insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);
			}

			// Draw hour ticks
			if ((i+1) % 4 == 0) {
				const double dx = pt1[0]-pt2[0];
				const double dy = pt1[1]-pt2[1];
				const double dq = dx*dx+dy*dy;

				double angle;

				const double d = sqrt(dq);

				angle = acos((pt1[1]-pt2[1])/d);
				if ( pt1[0] < pt2[0] ) {
					angle *= -1;
				}

				TRANSFO= Mat4f::translation( Vec3f(pt2[0],pt2[1],0) );
				TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), M_PI-angle );
				
				tmp = TRANSFO * Vec4f(-3.0,0.0,0.0,1.0);
				insert_all(vecDrawPos, tmp[0], tmp[1]);
				
				tmp = TRANSFO * Vec4f( 3.0,0.0,0.0,1.0);
				insert_all(vecDrawPos, tmp[0], tmp[1]);
			}
		}
	}

	drawSkylineGL(Color);

	vecDrawPos.clear();
}



// -------------------- SKYLINE_ECLIPTIC  ---------------------------------------------

SkyLine_Ecliptic::SkyLine_Ecliptic(double _radius = 1., unsigned int _nb_segment = 48):
	SkyLine(_radius, _nb_segment)
{
	proj_func = &Projector::projectJ2000;
	inclination = 23.4392803055555555556;  //inutile ?
}

SkyLine_Ecliptic::~SkyLine_Ecliptic()
{
}

void SkyLine_Ecliptic::draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory)
{
	if (!fader.getInterstate()) return;

	if (!observatory->isOnBody())
		return;

	Vec4f Color(color[0], color[1], color[2], fader.getInterstate());

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode
	Mat4d m = observatory->getRotEquatorialToVsop87().transpose();

	bool draw_labels = (observatory->isEarth() && font);

	// start labeling from the vernal equinox
	const double corr = draw_labels ? (atan2(m.r[4],m.r[0]) - 2.68*M_PI/6) : 0.0;
	Vec3d point(radius*cos(corr),radius*sin(corr),0.0);
	point.transfo4d(m);
	bool prev_on_screen = prj->projectEarthEqu(point,pt1);

	for (unsigned int i=1; i<365+1; ++i) {
		const double phi = corr+2*i*M_PI/365;
		Vec3d point(radius*cos(phi),radius*sin(phi),0.0);
		point.transfo4d(m);
		const bool on_screen = prj->projectEarthEqu(point,pt2);
		if (on_screen && prev_on_screen) {
			const double dx = pt2[0]-pt1[0];
			const double dy = pt2[1]-pt1[1];
			const double dq = dx*dx+dy*dy;

			insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);

			const double d = sqrt(dq);
			double angle;

			angle = acos((pt1[1]-pt2[1])/d);
			if ( pt1[0] < pt2[0] ) {
				angle *= -1;
			}

			TRANSFO= Mat4f::translation( Vec3f(pt2[0],pt2[1],0) );
			TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), M_PI-angle );

			if (draw_labels) {
				//                31 	     28 	31 	   30 	       31 	  30 	     31 	31 	   30 	      31         30
				if ((i==1) || (i==32) || (i==60) || (i==91) || (i==121) || (i==152) || (i==182) || (i==213) || (i==244) || (i==274) || (i==305) || (i==335)) {
					//~ printf("9\n");
					tmp = TRANSFO * Vec4f(-9.0,0.0,0.0,1.0);
					vecDrawPos.push_back( tmp[0] );
					vecDrawPos.push_back( tmp[1] );

					tmp = TRANSFO * Vec4f(9.0,0.0,0.0,1.0);
					vecDrawPos.push_back( tmp[0] );
					vecDrawPos.push_back( tmp[1] );
				} else if ((i==6)|| (i==11) || (i==16) || (i==21) || (i==26)
				           || (i==37) || (i==42) || (i==47) || (i==52) || (i==57)
				           || (i==65) || (i==70) || (i==75) || (i==80) || (i==85)
				           || (i==96) || (i==101) || (i==106) || (i==111) || (i==116)
				           || (i==126) || (i==131) || (i==136) || (i==141) || (i==146)
				           || (i==157) || (i==162) || (i==167) || (i==172) || (i==177)
				           || (i==187) || (i==192) || (i==197) || (i==202) || (i==207)
				           || (i==218) || (i==223) || (i==228) || (i==233) || (i==238)
				           || (i==249) || (i==254) || (i==259) || (i==264) || (i==269)
				           || (i==279) || (i==284) || (i==289) || (i==294) || (i==299)
				           || (i==310) || (i==315) || (i==320) || (i==325) || (i==330)
				           || (i==340) || (i==345) || (i==350) || (i==355) || (i==360)) {

					//~ printf("6\n");
					tmp = TRANSFO * Vec4f(-6.0,0.0,0.0,1.0);
					insert_all(vecDrawPos, tmp[0], tmp[1]);

					tmp = TRANSFO * Vec4f(6.0,0.0,0.0,1.0);
					insert_all(vecDrawPos, tmp[0], tmp[1]);
				} else {
					//~ printf("3\n");
					tmp = TRANSFO * Vec4f(-3.0,0.0,0.0,1.0);
					insert_all(vecDrawPos, tmp[0], tmp[1]);

					tmp = TRANSFO * Vec4f(3.0,0.0,0.0,1.0);
					insert_all(vecDrawPos, tmp[0], tmp[1]);
				}
			} // End draw ticks - To do: graduate in ° from vernal point

			if (draw_labels && (i+15) % 30 == 3) {

				const double d = sqrt(dq);
				double angle = acos((pt1[1]-pt2[1])/d);
				if ( pt1[0] < pt2[0] ) {
					angle *= -1;
				}

				// draw text label
				std::ostringstream oss;

				// TODO: center labels

				if (observatory->isEarth()) {
					float degree = i-84.5;
					if (degree < 0) degree += 360;
					if (internalNav)
						oss <<  month[ (i+15)/30 ] << " " << degree << "°";
					else
						oss << month[ (i+15)/30 ];
				}

				Mat4f MVP = prj->getMatProjectionOrtho2D();
				TRANSFO= Mat4f::translation( Vec3f(pt2[0],pt2[1],0) );
				TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), M_PI_2-angle );

				font->print(0,-10,oss.str(), Color, MVP*TRANSFO ,1);
			}
		}
		prev_on_screen = on_screen;
		pt1 = pt2;
	}

	drawSkylineGL(Color);

	vecDrawPos.clear();
}




// -------------------- SKYLINE_PRECESSION  ---------------------------------------------

SkyLine_Precession::SkyLine_Precession(double _radius = 1., unsigned int _nb_segment = 48):
	SkyLine(_radius, _nb_segment)
{
	proj_func = &Projector::projectJ2000;
}

SkyLine_Precession::~SkyLine_Precession()
{
}

void SkyLine_Precession::draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory)
{
	if (!fader.getInterstate()) return;
	if (!observatory->isOnBody())
		return;
	
	if(!(observatory->isEarth())) return;

	Vec4f Color(color[0], color[1], color[2], fader.getInterstate());

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	Mat4d m = observatory->getRotEquatorialToVsop87().transpose();
	draw_labels = (font != nullptr);

	const double corr = draw_labels ? (atan2(m.r[4],m.r[0]) - 2.68*M_PI/6) : 0.0;

	bool prev_on_screen;

	for(int pole=1; pole>=-1; pole-=2) {

		Vec3d point(radius*cos(corr),radius*sin(corr),pole*radius*2.3213f);
		point.transfo4d(m);
		prev_on_screen = prj->projectEarthEqu(point,pt1);

		for (unsigned int i=0; i<104+1; ++i) {
			const double phi = corr+2*(i-0.5)*M_PI/104;
			Vec3d point(radius*cos(phi),radius*sin(phi),pole*radius*2.3213f);
			point.transfo4d(m);
			const bool on_screen = prj->projectEarthEqu(point,pt2);
			if (on_screen && prev_on_screen) {
				const double dx = pt2[0]-pt1[0];
				const double dy = pt2[1]-pt1[1];
				const double dq = dx*dx+dy*dy;

				insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);

				if((i+2) % 4 == 0) {
					const double d = sqrt(dq);
					double angle;

					angle = acos((pt1[1]-pt2[1])/d);
					if ( pt1[0] < pt2[0] ) {
						angle *= -1;
					}

					// draw text label
					std::ostringstream oss;

					// TODO: center labels
					if (pole==1) {
						if ((i>52) && (i<100)) {
							oss << "-" << ((i-48) / 4)*1000;
						}
						if (i<48) {
							oss << "+" << ((52-i) / 4)*1000;
						}
						if (i>100) {
							oss << "+13000";
						}
					} else {
						if ((i>=48) && (i<100)) {
							oss << "+" << 13000-((i-48) / 4)*1000;
						}
						if (i<48) {
							oss << "-" << 13000-((52-i) / 4)*1000;
						}
					}

					Mat4f MVP = prj->getMatProjectionOrtho2D();
					TRANSFO= Mat4f::translation( Vec3f(pt2[0],pt2[1],0) );
					if (pole==1) TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), M_PI-angle );
					if (pole==-1) TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), -angle );

					font->print(0,-2,oss.str(), Color, MVP*TRANSFO ,1);

					tmp = TRANSFO * Vec4f(-3.0,0.0,0.0,1.0);
					insert_all(vecDrawPos, tmp[0], tmp[1]);

					tmp = TRANSFO * Vec4f(3.0,0.0,0.0,1.0);
					insert_all(vecDrawPos, tmp[0], tmp[1]);
				}
			}
			prev_on_screen = on_screen;
			pt1 = pt2;
		}
	}

	drawSkylineGL(Color);

	vecDrawPos.clear();
}




// -------------------- SKYLINE_VERTICAL  ---------------------------------------------

SkyLine_Vertical::SkyLine_Vertical(double _radius = 1., unsigned int _nb_segment = 48):
	SkyLine(_radius, _nb_segment)
{
	circlep = new Vec3f[nb_segment+1];
	proj_func = &Projector::projectLocal;
}

SkyLine_Vertical::~SkyLine_Vertical()
{
	delete [] circlep;
	circlep = nullptr;
}

void SkyLine_Vertical::draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory)
{
	if (!fader.getInterstate()) return;

	Vec4f Color(color[0], color[1], color[2], fader.getInterstate());

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	for (unsigned int i=0; i<nb_segment+1; ++i) {
		Utility::spheToRect(M_PI_2, ((float)i/nb_segment*M_PI),circlep[i]);
	}

	for (unsigned int i=0; i < nb_segment; i++) {
		if ((prj->*proj_func)(circlep[i], pt1) && (prj->*proj_func)(circlep[i+1], pt2) ) {

			insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);

			const double dx = pt2[0]-pt1[0];
			const double dy = pt2[1]-pt1[1];
			const double dq = dx*dx+dy*dy;
			double angle;
			const double d = sqrt(dq);
			int tickl = 4;

			angle = acos((pt1[1]-pt2[1])/d);
			if ( pt1[0] < pt2[0] ) {
				angle *= -1;
			}
			std::ostringstream oss;
			double val;
			if ((i+1)<=8*(nb_segment/18)) val =  (i+1)*10/(nb_segment/18);
			else if ((i+1) > 8*(nb_segment/18)) {
				val = 180-((i+1)*10/(nb_segment/18));
				angle += M_PI;
			} else val=0; //uninitialised var
			if ((i+1)%(nb_segment/18)==0) {
				oss << val << "°";
				tickl = 5;
			} else if ((i+1-5)%(nb_segment/18)== 0) {
				tickl = 4;
			} else {
				tickl = 2;
			}
			angle *= -1;

			Mat4f MVP = prj->getMatProjectionOrtho2D();
			TRANSFO= Mat4f::translation( Vec3f(pt2[0],pt2[1],0) );
			TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), M_PI+angle );

			tmp = TRANSFO * Vec4f(-tickl,0.0,0.0,1.0);
			insert_all(vecDrawPos, tmp[0], tmp[1]);
	
			tmp = TRANSFO * Vec4f(tickl,0.0,0.0,1.0);
			insert_all(vecDrawPos, tmp[0], tmp[1]);

			if (font) font->print(2,-2,oss.str(), Color, MVP*TRANSFO ,1);
		}
	}

	drawSkylineGL(Color);

	vecDrawPos.clear();
}


// -------------------- SKYLINE_ZENITH  ---------------------------------------------

SkyLine_Zenith::SkyLine_Zenith(double _radius = 1., unsigned int _nb_segment = 48):
	SkyLine(_radius, _nb_segment)
{
	proj_func = &Projector::projectLocal;
}

SkyLine_Zenith::~SkyLine_Zenith()
{

}

void SkyLine_Zenith::draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory)
{
	if (!fader.getInterstate()) return;

	Vec4f Color(color[0], color[1], color[2], fader.getInterstate());

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	for (unsigned int i=0; i<51; ++i) {
		Utility::spheToRect((float)i/(50)*2.f*M_PI, (0.993f*M_PI-M_PI_2),circlep[i]);
	}
	for (unsigned int i=0; i<51; ++i) {
		Utility::spheToRect((float)i/(50)*2.f*M_PI, -(0.993f*M_PI-M_PI_2),circlen[i]);
	}

	for (int i=0; i < 50; i++) {
		if ((prj->*proj_func)(circlep[i], pt1) && (prj->*proj_func)(circlep[i+1], pt2) ) {

			insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);

		}
		if ((prj->*proj_func)(circlen[i], pt1) && (prj->*proj_func)(circlen[i+1], pt2) ) {

			insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);
		}
	}

	Utility::spheToRect((float)12.5/(50)*2.f*M_PI, 0.993f*M_PI-M_PI_2,punts[0]);
	Utility::spheToRect((float)37.5/(50)*2.f*M_PI, 0.993f*M_PI-M_PI_2,punts[1]);
	Utility::spheToRect(0,0.992f*M_PI-M_PI_2,punts[2]);

	if ((prj->*proj_func)(punts[0],pt1) && (prj->*proj_func)(punts[1],pt2) ) {

		insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);
	}
	if ((prj->*proj_func)(circlep[25],pt1) && (prj->*proj_func)(circlep[0],pt2) ) {

		insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);
	}

	Utility::spheToRect(0.98*M_PI, M_PI_2,punts[0]);
	Utility::spheToRect(M_PI, M_PI_2,punts[1]);

	if ((prj->*proj_func)(punts[0],pt1) && (prj->*proj_func)(punts[1],pt2) ) {
		const double dx = pt2[0]-pt1[0];
		const double dy = pt2[1]-pt1[1];
		const double dq = dx*dx+dy*dy;
		double angle;
		const double d = sqrt(dq);
		angle = acos((pt1[1]-pt2[1])/d);
		if ( pt1[0] < pt2[0] ) {
			angle *= -1;
		}
		std::ostringstream oss;
		oss << "Z";

		Mat4f MVP = prj->getMatProjectionOrtho2D();
		TRANSFO= Mat4f::translation( Vec3f(pt1[0],pt1[1],0) );
		TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), 0 );

		if (font) font->print(10,-10,oss.str(), Color, MVP*TRANSFO ,1);
	}

	Utility::spheToRect((float)12.5/(50)*2.f*M_PI, -(0.993f*M_PI-M_PI_2),punts[0]);
	Utility::spheToRect((float)37.5/(50)*2.f*M_PI, -(0.993f*M_PI-M_PI_2),punts[1]);
	Utility::spheToRect(0, -(0.992f*M_PI-M_PI_2),punts[2]);

	if ((prj->*proj_func)(punts[0],pt1) && (prj->*proj_func)(punts[1],pt2) ) {

		insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);
	}
	if ((prj->*proj_func)(circlen[25],pt1) && (prj->*proj_func)(circlen[0],pt2) ) {

		insert_all(vecDrawPos, pt1[0], pt1[1], pt2[0], pt2[1]);
	}

	Utility::spheToRect(0.98*M_PI, -M_PI_2,punts[0]);
	Utility::spheToRect(M_PI, -M_PI_2,punts[1]);

	if ((prj->*proj_func)(punts[0],pt1) && (prj->*proj_func)(punts[1],pt2) ) {
		const double dx = pt2[0]-pt1[0];
		const double dy = pt2[1]-pt1[1];
		const double dq = dx*dx+dy*dy;
		double angle;
		const double d = sqrt(dq);
		angle = acos((pt1[1]-pt2[1])/d);
		if ( pt1[0] < pt2[0] ) {
			angle *= -1;
		}
		std::ostringstream oss;
		oss << "N";

		Mat4f MVP = prj->getMatProjectionOrtho2D();
		TRANSFO= Mat4f::translation( Vec3f(pt1[0],pt1[1],0) );
		TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), 0 );

		if (font) font->print(10,-10,oss.str(), Color, MVP*TRANSFO ,1);
	}

	drawSkylineGL(Color);

	vecDrawPos.clear();
}
