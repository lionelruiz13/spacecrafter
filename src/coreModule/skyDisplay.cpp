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
#include "coreModule/skyDisplay.hpp"
#include "tools/s_texture.hpp"
#include "tools/utility.hpp"
#include <string>
#include "tools/log.hpp"
//#include "tools/app_settings.hpp"

#include "tools/s_font.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "tools/tone_reproductor.hpp"
#include "tools/translator.hpp"

#define NB_MAX_POINTS 4194304
const float deg2rad = 3.1415926 / 180.; // Convert deg to radian
const float rad2deg = 180. / 3.1415926; // Converd radian to deg
const float grad2rad = 3.1415926 / 18.; // Convert grind pas to radian
const float pi_div_2 = 1.5707963;		// pi/2

// -------------------- SKYLINE_PERSONAL  ---------------------------------------------

s_font* SkyDisplay::skydisplay_font = nullptr;
shaderProgram* SkyDisplay::shaderSkyDisplay = nullptr;

SkyDisplay::SkyDisplay(PROJECTION_TYPE _ptype)
{
	ptype = _ptype;
	createVao();
	switch (ptype) {
		case AL:
			proj_func = &Projector::projectLocal;
			break;
		case EQ:
			proj_func = &Projector::projectEarthEqu;
			break;
		default:
			proj_func = &Projector::projectLocal;
			break;
	}
	//font = new s_font(12, AppSettings::Instance()->getUserDir() + "fonts/DejaVuSans.ttf");
}

SkyDisplay::~SkyDisplay()
{
	dataSky.clear();
	// if (font)
	// 	delete font;
	deleteVao();
}

void SkyDisplay::createVao()
{
	// shaderSkyDisplay = new shaderProgram();
	// shaderSkyDisplay->init("person.vert", "person.geom", "person.frag");
	// shaderSkyDisplay->setUniformLocation("color");
	// shaderSkyDisplay->setUniformLocation("fader");
	// shaderSkyDisplay->setUniformLocation("Mat");

	glGenVertexArrays(1, &sData.vao);
	glBindVertexArray(sData.vao);
	glGenBuffers(1, &sData.pos);

	glEnableVertexAttribArray(0);
}

void SkyDisplay::deleteVao()
{
	// if (shaderSkyDisplay != nullptr)
	// 	shaderSkyDisplay = nullptr;
	glDeleteBuffers(1, &sData.pos);
	glDeleteVertexArrays(1, &sData.vao);
}

void SkyDisplay::clear()
{
	dataSky.clear();
}

//a optimiser
void SkyDisplay::draw_text(const Projector *prj, const Navigator *nav)
{
	//if (((dataSky.size() / 3) == 198) || ((dataSky.size() / 3) == 396))
		//double alpha = 0.f; // Il me faut la premiere valeur en alpha de personal.txt ou personeq.txt selon

		for (int i = -9; i < 10; i++) {
			std::ostringstream oss;
			//création des positions de points dans pt3,pt4
			Utility::spheToRect(aperson - 0.31415926, (i - 0.0001) * grad2rad, pt3);
			Utility::spheToRect(aperson - 0.31415926 + 0.01, (i - 0.0001) * grad2rad, pt4);
			//test si pt3,pt4 est affichable et transmet à pt1,pt2 sa position
			if (((prj->*proj_func)(pt3, pt1)) && ((prj->*proj_func)(pt4, pt2))) {
				double angle;
				const double dx = pt1[0] - pt2[0];
				const double dy = pt1[1] - pt2[1];
				const double dq = dx * dx + dy * dy;
				const double d = sqrt(dq);
				angle = acos((pt1[1] - pt2[1]) / (d + 0.000001));
				if (pt1[0] < pt2[0])
					angle *= -1;
				if (i == -9)
					angle += 3.1415926;
				Mat4f MVP = prj->getMatProjectionOrtho2D();
				//suite de transformations de position à partir des coordonnées de pt1
				Mat4f TRANSFO = Mat4f::translation(Vec3f(pt1[0], pt1[1], 0));
				TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
				//oss << pt1[0] << " " << pt2[0] << pt1[1] << " " << pt2[1];
				oss << i * 10 << "°";
				skydisplay_font->print(2, -2, oss.str(), color, MVP * TRANSFO, 1);
				oss.clear();
			}
		}
}

///////////////////////////////////////////////////////////////////////
//
//      Classes dérivées
//
//
////////////////////////////////////////////////////////////////////////

SkyPerson::SkyPerson(PROJECTION_TYPE ptype) : SkyDisplay(ptype)
{
}

void SkyPerson::loadData(const std::string& filename)
{
	double alpha, delta, x, y, z;
	int nblines;
	Vec3f punts;
	clear();

	std::ifstream fichier(filename, std::ios::in);
	if (fichier) {
		fichier >> nblines;
		if (nblines > NB_MAX_POINTS)
			nblines = NB_MAX_POINTS;

		for (int i = 0; i < nblines; i++) {
			fichier >> alpha >> delta;
			Utility::spheToRect(alpha, delta, punts);
			// On Earth or AL
			x = punts[0];
			y = punts[1];
			z = punts[2];
			// Elsewhere
			//x=punts[0];
			//y=punts[1]*cos(-23.43928*3.1415926/180.0)-punts[2]*sin(-23.43928*3.1415926/180.0);
			//z=punts[1]*sin(-23.43928*3.1415926/180.0)+punts[2]*cos(-23.43928*3.1415926/180.0);
			// End of test
			dataSky.push_back(x);
			dataSky.push_back(y);
			dataSky.push_back(z);
		}
		aperson = alpha;
		fichier.close();
	}

	//on charge les points dans un vbo
	glBindVertexArray(sData.vao);

	glBindBuffer(GL_ARRAY_BUFFER, sData.pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * dataSky.size(), dataSky.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	//~ glEnableVertexAttribArray(0);
}


void SkyPerson::loadString(const std::string& message)
{
	//on récupère d'abord les 2 nombres et ensuite on les converit...
    std::string delimiter = ";";
    float ftemp;
	std::string txt = message;
	// std::cout << message << std::endl;
    //Vérifie la présence d'une lettre
    for(std::string::size_type i = 0; i < txt.length(); i++)
    {
        char c = txt[i];
        if(!(isdigit(c)||c==';'||c=='.')){ //check si le caractère est une lettre
			cLog::get()->write("Skyperson error loading dataStr, check dataStr", LOG_TYPE::L_WARNING);
			// std::cout << "   " << c << std::endl;
            txt.erase(i, 1);
        }
    }
	// std::cout << txt << std::endl;

    size_t pos = 0;
    std::string token;

	std::vector<float> dataTmp;
    //Décompose la chaine de caractère
    while ((pos = txt.find(delimiter)) != std::string::npos) {
        token = txt.substr(0, pos);
		// std::cout << "   " << token << " | " ;
	    //fonction plus résistante aux erreurs
		std::istringstream dstr( token );
		dstr >> ftemp;
		//ftemp = std::stof(token);

        dataTmp.push_back(ftemp);
        txt.erase(0, pos + delimiter.length());
		// std::cout << txt  << std::endl;
    }
	// on vérifie quand même que le contenu est bien un multiple de 2
	// sinon on supprime les dernières valeurs.
	if (dataTmp.size()%2!=0) {
		dataTmp.pop_back();
	 	cLog::get()->write("Skyperson loading incomplete data", LOG_TYPE::L_WARNING);
	}
	
	// std::cout << "dataTmp a " << dataTmp.size()  << std::endl;

	Vec3f punts;
	for (auto it =dataTmp.begin(); it!=dataTmp.end(); it++) {
			Utility::spheToRect(*it, *++it, punts);
			// std::cout << punts[0] << "|"<< punts[1] << "|"<< punts[2] << std::endl;
			for(int i=0; i<3; i++)
				dataSky.push_back(punts[i]);
	}

	//on charge les points dans un vbo
	glBindVertexArray(sData.vao);
	glBindBuffer(GL_ARRAY_BUFFER, sData.pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * dataSky.size(), dataSky.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}


void SkyPerson::draw(const Projector *prj, const Navigator *nav, Vec3d equPos, Vec3d oldEquPos)
{
	if (!fader.getInterstate())
		return;

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	shaderSkyDisplay->use();
	shaderSkyDisplay->setUniform("color", color);
	shaderSkyDisplay->setUniform("fader", fader.getInterstate());

	if (ptype == AL)
		shaderSkyDisplay->setUniform("Mat", prj->getMatLocalToEye());
	else
		shaderSkyDisplay->setUniform("Mat", prj->getMatEarthEquToEye());

	glBindVertexArray(sData.vao);

	glDrawArrays(GL_LINES, 0, dataSky.size() / 3); //un point est représenté par 3 points

	shaderSkyDisplay->unuse();

	//draw_text(prj, nav);
}

SkyNautic::SkyNautic(PROJECTION_TYPE ptype) : SkyDisplay(ptype)
{
}

void SkyNautic::draw(const Projector *prj, const Navigator *nav, Vec3d equPos, Vec3d oldEquPos)
{
	if (!fader.getInterstate())
		return;

	Vec3f punts;
	clear();
	float tick;
	double direction;

	// calculate alt az
	double tempDE, tempRA;

	//Vec3d equPos = selected_object.getEarthEquPos(nav);
	Utility::rectToSphe(&tempRA, &tempDE, equPos);
	direction = tempRA * rad2deg;
	if (ptype == AL) {
		// calculate alt az position
		Vec3d localPos = nav->earthEquToLocal(equPos);
		Utility::rectToSphe(&tempRA, &tempDE, localPos);
		tempRA = 3 * M_PI - tempRA; // N is zero, E is 90 degrees
		if (tempRA > M_PI * 2)
			tempRA -= M_PI * 2;
		direction = (M_PI - tempRA) * rad2deg;
	}
	for (int j = -9; j < 9; j++) {
		Utility::spheToRect(direction * deg2rad, j * grad2rad, punts);
		dataSky.push_back(punts[0]); // punts[0] represent x coordinate
		dataSky.push_back(punts[1]); // punts[1] represent y coordinate
		dataSky.push_back(punts[2]); // punts[2] represent z coordinate
		Utility::spheToRect(direction * deg2rad, (j + 1) * grad2rad, punts);
		dataSky.push_back(punts[0]); // punts[0] represent x coordinate
		dataSky.push_back(punts[1]); // punts[1] represent y coordinate
		dataSky.push_back(punts[2]); // punts[2] represent z coordinate
		for (int i = 0; i < 10; i++) {
			if (i == 1)
				tick = 0.6 * 90 / (90 - (j * 10 + i));
			else if (i == 5)
				tick = 0.4 * 90 / (90 - (j * 10 + i));
			else
				tick = 0.2 * 90 / (90 - (j * 10 + i));
			Utility::spheToRect((direction - tick) * deg2rad, (j * 10 + i) * deg2rad, punts);
			dataSky.push_back(punts[0]); // punts[0] represent x coordinate
			dataSky.push_back(punts[1]); // punts[1] represent y coordinate
			dataSky.push_back(punts[2]); // punts[2] represent z coordinate
			Utility::spheToRect((direction + tick) * deg2rad, (j * 10 + i) * deg2rad, punts);
			dataSky.push_back(punts[0]); // punts[0] represent x coordinate
			dataSky.push_back(punts[1]); // punts[1] represent y coordinate
			dataSky.push_back(punts[2]); // punts[2] represent z coordinate
		}
	}
	aperson = (direction + tick) * deg2rad;
	//on charge les points dans un vbo
	glBindVertexArray(sData.vao);

	glBindBuffer(GL_ARRAY_BUFFER, sData.pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * dataSky.size(), dataSky.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	//~ glEnableVertexAttribArray(0);

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	shaderSkyDisplay->use();
	shaderSkyDisplay->setUniform("color", color);
	shaderSkyDisplay->setUniform("fader", fader.getInterstate());

	if (ptype == AL)
		shaderSkyDisplay->setUniform("Mat", prj->getMatLocalToEye());
	else
		shaderSkyDisplay->setUniform("Mat", prj->getMatEarthEquToEye());

	glBindVertexArray(sData.vao);

	glDrawArrays(GL_LINES, 0, dataSky.size() / 3); //un point est représenté par 3 points

	shaderSkyDisplay->unuse();

	draw_text(prj, nav);
}

SkyCoords::SkyCoords() : SkyDisplay(PROJECTION_TYPE::AL)
{
}

void SkyCoords::draw(const Projector *prj, const Navigator *nav, Vec3d equPos, Vec3d oldPos)
{
	if (!fader.getInterstate())
		return;

	double tempDE, tempRA;
	float alt, az, aza, alta, ra, dec, mn;
	double fov = prj->getFov() / 360.f;
	Utility::rectToSphe(&tempRA, &tempDE, equPos);
	// calculate ra dec
	ra = tempRA * rad2deg;
	if (ra < 0)
		ra = 360. + ra;
	ra = ra * 24. / 360.;
	dec = tempDE * rad2deg;
	// calculate alt az position
	Vec3d localPos = nav->earthEquToLocal(equPos);
	Utility::rectToSphe(&tempRA, &tempDE, localPos);
	tempRA = 3 * M_PI - tempRA; // N is zero, E is 90 degrees
	if (tempRA > M_PI * 2)
		tempRA -= M_PI * 2;
	az = (tempRA)*rad2deg;
	alt = tempDE * rad2deg;
	aza = (M_PI - tempRA - (0.1 * fov)) * rad2deg;
	// ALT
	alta = (tempDE - (0.05 * fov * 2)) * rad2deg;
	Utility::spheToRect(aza * deg2rad, alta * deg2rad, pt3);
	Utility::spheToRect(aza * deg2rad + (0.001 * fov), alta * deg2rad, pt4);
	if (((prj->*proj_func)(pt3, pt1)) && ((prj->*proj_func)(pt4, pt2))) {
		double angle;
		const double dx = pt1[0] - pt2[0];
		const double dy = pt1[1] - pt2[1];
		const double dq = dx * dx + dy * dy;
		const double d = sqrt(dq);
		angle = acos((pt1[1] - pt2[1]) / (d + 0.000001 * fov));
		if (pt1[0] < pt2[0])
			angle *= -1;
		std::ostringstream oss;
		Mat4f MVP = prj->getMatProjectionOrtho2D();
		//suite de transformations de position à partir des coordonnées de punts
		Mat4f TRANSFO = Mat4f::translation(Vec3f(pt1[0], pt1[1], 0));
		TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
		oss << "alt:";
		if (alt < 0.) {
			alt = -alt;
			oss << "-";
		}
		else
			oss << "+";
		if (alt < 10.)
			oss << "0";
		oss << truncf(alt) << "°";
		mn = truncf((alt - truncf(alt)) * 60);
		if (mn < 10)
			oss << "0";
		oss << mn << "'";
		skydisplay_font->print(2, -2, oss.str(), color, MVP * TRANSFO, 1);
		oss.clear();
	}
	// AZ
	alta = tempDE * rad2deg;
	Utility::spheToRect(aza * deg2rad, alta * deg2rad, pt3);
	Utility::spheToRect(aza * deg2rad + (0.001 * fov), alta * deg2rad, pt4);
	if (((prj->*proj_func)(pt3, pt1)) && ((prj->*proj_func)(pt4, pt2))) {
		double angle;
		const double dx = pt1[0] - pt2[0];
		const double dy = pt1[1] - pt2[1];
		const double dq = dx * dx + dy * dy;
		const double d = sqrt(dq);
		angle = acos((pt1[1] - pt2[1]) / (d + 0.000001 * fov));
		if (pt1[0] < pt2[0])
			angle *= -1;
		std::ostringstream oss;
		Mat4f MVP = prj->getMatProjectionOrtho2D();
		//suite de transformations de position à partir des coordonnées de punts
		Mat4f TRANSFO = Mat4f::translation(Vec3f(pt1[0], pt1[1], 0));
		TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
		oss << "az :";
		if (az < 0.) {
			az = -az;
			oss << "-";
		}
		else
			oss << "+";
		if (az < 100.)
			oss << "0";
		if (az < 10.)
			oss << "0";
		oss << truncf(az) << "°";
		mn = truncf((az - truncf(az)) * 60);
		if (mn < 10)
			oss << "0";
		oss << mn << "'";
		skydisplay_font->print(2, -2, oss.str(), color, MVP * TRANSFO, 1);
		oss.clear();
	}
	// RA
	alta = (tempDE + (0.1 * fov * 2)) * rad2deg;
	Utility::spheToRect(aza * deg2rad, alta * deg2rad, pt3);
	Utility::spheToRect(aza * deg2rad + (0.001 * fov), alta * deg2rad, pt4);
	if (((prj->*proj_func)(pt3, pt1)) && ((prj->*proj_func)(pt4, pt2))) {
		double angle;
		const double dx = pt1[0] - pt2[0];
		const double dy = pt1[1] - pt2[1];
		const double dq = dx * dx + dy * dy;
		const double d = sqrt(dq);
		angle = acos((pt1[1] - pt2[1]) / (d + 0.000001 * fov));
		if (pt1[0] < pt2[0])
			angle *= -1;
		std::ostringstream oss;
		Mat4f MVP = prj->getMatProjectionOrtho2D();
		//suite de transformations de position à partir des coordonnées de punts
		Mat4f TRANSFO = Mat4f::translation(Vec3f(pt1[0], pt1[1], 0));
		TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
		oss << "ra :";
		if (ra < 10.)
			oss << "0";
		oss << truncf(ra) << "h";
		mn = truncf((ra - truncf(ra)) * 60);
		if (mn < 10)
			oss << "0";
		oss << mn << "m";
		skydisplay_font->print(2, -2, oss.str(), color, MVP * TRANSFO, 1);
		oss.clear();
	}
	// DEC
	alta = (tempDE + (0.05 * fov * 2)) * rad2deg;
	Utility::spheToRect(aza * deg2rad, alta * deg2rad, pt3);
	Utility::spheToRect(aza * deg2rad + (0.001 * fov), alta * deg2rad, pt4);

	if (((prj->*proj_func)(pt3, pt1)) && ((prj->*proj_func)(pt4, pt2))) {
		double angle;
		const double dx = pt1[0] - pt2[0];
		const double dy = pt1[1] - pt2[1];
		const double dq = dx * dx + dy * dy;
		const double d = sqrt(dq);
		angle = acos((pt1[1] - pt2[1]) / (d + 0.000001 * fov));
		if (pt1[0] < pt2[0])
			angle *= -1;
		std::ostringstream oss;
		Mat4f MVP = prj->getMatProjectionOrtho2D();
		//suite de transformations de position à partir des coordonnées de punts
		Mat4f TRANSFO = Mat4f::translation(Vec3f(pt1[0], pt1[1], 0));
		TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
		oss << "dec:";
		if (dec < 0.) {
			dec = -dec;
			oss << "-";
		}
		else
			oss << "+";
		if (dec < 10.)
			oss << "0";
		oss << truncf(dec) << "°";
		mn = truncf((dec - truncf(dec)) * 60);
		if (mn < 10)
			oss << "0";
		oss << mn << "'";
		skydisplay_font->print(2, -2, oss.str(), color, MVP * TRANSFO, 1);
		oss.clear();
	}
}


SkyMouse::SkyMouse() : SkyDisplay(PROJECTION_TYPE::AL)
{
}

void SkyMouse::draw(const Projector *prj, const Navigator *nav, Vec3d _equPos, Vec3d _oldPos)
{
	if (!fader.getInterstate())
		return;

	int x,y;
	SDL_GetMouseState(&x,&y);
	Vec3d equPos = prj->getCursorPosEqu(x, y);


	double tempDE, tempRA;
	float alt, az, aza, alta, ra, dec, mn;
	double fov = prj->getFov() / 360.f;
	Utility::rectToSphe(&tempRA, &tempDE, equPos);
	// calculate ra dec
	ra = tempRA * rad2deg;
	if (ra < 0)
		ra = 360. + ra;
	ra = ra * 24. / 360.;
	dec = tempDE * rad2deg;
	// calculate alt az position
	Vec3d localPos = nav->earthEquToLocal(equPos);
	Utility::rectToSphe(&tempRA, &tempDE, localPos);
	tempRA = 3 * M_PI - tempRA; // N is zero, E is 90 degrees
	if (tempRA > M_PI * 2)
		tempRA -= M_PI * 2;
	az = (tempRA)*rad2deg;
	alt = tempDE * rad2deg;
	aza = (M_PI - tempRA - (0.1 * fov)) * rad2deg;
	// ALT
	alta = (tempDE - (0.05 * fov * 2)) * rad2deg;
	Utility::spheToRect(aza * deg2rad, alta * deg2rad, pt3);
	Utility::spheToRect(aza * deg2rad + (0.001 * fov), alta * deg2rad, pt4);
	if (((prj->*proj_func)(pt3, pt1)) && ((prj->*proj_func)(pt4, pt2))) {
		double angle;
		const double dx = pt1[0] - pt2[0];
		const double dy = pt1[1] - pt2[1];
		const double dq = dx * dx + dy * dy;
		const double d = sqrt(dq);
		angle = acos((pt1[1] - pt2[1]) / (d + 0.000001 * fov));
		if (pt1[0] < pt2[0])
			angle *= -1;
		std::ostringstream oss;
		Mat4f MVP = prj->getMatProjectionOrtho2D();
		//suite de transformations de position à partir des coordonnées de punts
		Mat4f TRANSFO = Mat4f::translation(Vec3f(pt1[0], pt1[1], 0));
		TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
		oss << "alt:";
		if (alt < 0.) {
			alt = -alt;
			oss << "-";
		}
		else
			oss << "+";
		if (alt < 10.)
			oss << "0";
		oss << truncf(alt) << "°";
		mn = truncf((alt - truncf(alt)) * 60);
		if (mn < 10)
			oss << "0";
		oss << mn << "'";
		skydisplay_font->print(2, -2, oss.str(), color, MVP * TRANSFO, 1);
		oss.clear();
	}
	// AZ
	alta = tempDE * rad2deg;
	Utility::spheToRect(aza * deg2rad, alta * deg2rad, pt3);
	Utility::spheToRect(aza * deg2rad + (0.001 * fov), alta * deg2rad, pt4);
	if (((prj->*proj_func)(pt3, pt1)) && ((prj->*proj_func)(pt4, pt2))) {
		double angle;
		const double dx = pt1[0] - pt2[0];
		const double dy = pt1[1] - pt2[1];
		const double dq = dx * dx + dy * dy;
		const double d = sqrt(dq);
		angle = acos((pt1[1] - pt2[1]) / (d + 0.000001 * fov));
		if (pt1[0] < pt2[0])
			angle *= -1;
		std::ostringstream oss;
		Mat4f MVP = prj->getMatProjectionOrtho2D();
		//suite de transformations de position à partir des coordonnées de punts
		Mat4f TRANSFO = Mat4f::translation(Vec3f(pt1[0], pt1[1], 0));
		TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
		oss << "az :";
		if (az < 0.) {
			az = -az;
			oss << "-";
		}
		else
			oss << "+";
		if (az < 100.)
			oss << "0";
		if (az < 10.)
			oss << "0";
		oss << truncf(az) << "°";
		mn = truncf((az - truncf(az)) * 60);
		if (mn < 10)
			oss << "0";
		oss << mn << "'";
		skydisplay_font->print(2, -2, oss.str(), color, MVP * TRANSFO, 1);
		oss.clear();
	}
	// RA
	alta = (tempDE + (0.1 * fov * 2)) * rad2deg;
	Utility::spheToRect(aza * deg2rad, alta * deg2rad, pt3);
	Utility::spheToRect(aza * deg2rad + (0.001 * fov), alta * deg2rad, pt4);
	if (((prj->*proj_func)(pt3, pt1)) && ((prj->*proj_func)(pt4, pt2))) {
		double angle;
		const double dx = pt1[0] - pt2[0];
		const double dy = pt1[1] - pt2[1];
		const double dq = dx * dx + dy * dy;
		const double d = sqrt(dq);
		angle = acos((pt1[1] - pt2[1]) / (d + 0.000001 * fov));
		if (pt1[0] < pt2[0])
			angle *= -1;
		std::ostringstream oss;
		Mat4f MVP = prj->getMatProjectionOrtho2D();
		//suite de transformations de position à partir des coordonnées de punts
		Mat4f TRANSFO = Mat4f::translation(Vec3f(pt1[0], pt1[1], 0));
		TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
		oss << "ra :";
		if (ra < 10.)
			oss << "0";
		oss << truncf(ra) << "h";
		mn = truncf((ra - truncf(ra)) * 60);
		if (mn < 10)
			oss << "0";
		oss << mn << "m";
		skydisplay_font->print(2, -2, oss.str(), color, MVP * TRANSFO, 1);
		oss.clear();
	}
	// DEC
	alta = (tempDE + (0.05 * fov * 2)) * rad2deg;
	Utility::spheToRect(aza * deg2rad, alta * deg2rad, pt3);
	Utility::spheToRect(aza * deg2rad + (0.001 * fov), alta * deg2rad, pt4);

	if (((prj->*proj_func)(pt3, pt1)) && ((prj->*proj_func)(pt4, pt2))) {
		double angle;
		const double dx = pt1[0] - pt2[0];
		const double dy = pt1[1] - pt2[1];
		const double dq = dx * dx + dy * dy;
		const double d = sqrt(dq);
		angle = acos((pt1[1] - pt2[1]) / (d + 0.000001 * fov));
		if (pt1[0] < pt2[0])
			angle *= -1;
		std::ostringstream oss;
		Mat4f MVP = prj->getMatProjectionOrtho2D();
		//suite de transformations de position à partir des coordonnées de punts
		Mat4f TRANSFO = Mat4f::translation(Vec3f(pt1[0], pt1[1], 0));
		TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
		oss << "dec:";
		if (dec < 0.) {
			dec = -dec;
			oss << "-";
		}
		else
			oss << "+";
		if (dec < 10.)
			oss << "0";
		oss << truncf(dec) << "°";
		mn = truncf((dec - truncf(dec)) * 60);
		if (mn < 10)
			oss << "0";
		oss << mn << "'";
		skydisplay_font->print(2, -2, oss.str(), color, MVP * TRANSFO, 1);
		oss.clear();
	}
}


SkyAngDist::SkyAngDist() : SkyDisplay(PROJECTION_TYPE::AL)
{
}

void SkyAngDist::draw(const Projector *prj, const Navigator *nav, Vec3d equPos, Vec3d oldEquPos)
{
	if (!fader.getInterstate())
		return;
	double tempDE, tempRA, azt, altt, alt1, alt2, az1, az2;
	// for Selected position
	// calculate alt az position
	Vec3d localPos = nav->earthEquToLocal(equPos);
	Utility::rectToSphe(&tempRA, &tempDE, localPos);
	tempRA = 3 * M_PI - tempRA; // N is zero, E is 90 degrees
	if (tempRA > M_PI * 2)
		tempRA -= M_PI * 2;
	alt1 = tempDE;
	az1 = M_PI - tempRA;
	// end of calculate alt az position

	// for Old position
	// calculate alt az position
	localPos = nav->earthEquToLocal(oldEquPos);
	Utility::rectToSphe(&tempRA, &tempDE, localPos);
	tempRA = 3 * M_PI - tempRA; // N is zero, E is 90 degrees
	if (tempRA > M_PI * 2)
		tempRA -= M_PI * 2;
	alt2 = tempDE;
	az2 = M_PI - tempRA;
	// end of calculate alt az position
	if ((az2 - az1) > M_PI)
		az1 += 2 * M_PI;
	if ((az1 - az2) > M_PI)
		az2 += 2 * M_PI;

	// Draw orthodromy
	clear();
	Utility::spheToRect(az1, alt1, pt1);
	int npoints = 21;
	float delta = (az1 - az2) / (npoints - 1);
	for (int i = 0; i < npoints; i++) {
		dataSky.push_back(pt1[0]); // punts[0] represent x coordinate
		dataSky.push_back(pt1[1]); // punts[1] represent y coordinate
		dataSky.push_back(pt1[2]); // punts[2] represent z coordinate
		azt = az1 - delta * i;
		altt = atan(((tan(alt2) * sin(azt - az1)) / sin(az2 - az1 + 0.00001)) + (tan(alt1) * sin(az2 - azt)) / sin(az2 - az1 + 0.00001));
		Utility::spheToRect(azt, altt, pt1);
		if (i == 12)
			pt5 = pt1;
		dataSky.push_back(pt1[0]); // punts[0] represent x coordinate
		dataSky.push_back(pt1[1]); // punts[1] represent y coordinate
		dataSky.push_back(pt1[2]); // punts[2] represent z coordinate
	}
	glBindVertexArray(sData.vao);
	glBindBuffer(GL_ARRAY_BUFFER, sData.pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * dataSky.size(), dataSky.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode
	shaderSkyDisplay->use();
	shaderSkyDisplay->setUniform("color", color);
	shaderSkyDisplay->setUniform("fader", fader.getInterstate());
	shaderSkyDisplay->setUniform("Mat", prj->getMatLocalToEye());
	glBindVertexArray(sData.vao);
	glDrawArrays(GL_LINES, 0, dataSky.size() / 3); //un point est représenté par 3 points
	shaderSkyDisplay->unuse();

	// Text
	float ang, mn;
	ang = acos(sin(alt1) * sin(alt2) + cos(alt1) * cos(alt2) * cos(az2 - az1)) * rad2deg;
	Utility::spheToRect(az1, alt1, pt3);
	Utility::spheToRect(az2, alt2, pt4);
	//Utility::spheToRect(az, alt, pt5);
	(prj->*proj_func)(pt3, pt1);
	(prj->*proj_func)(pt4, pt2);
	double angle;
	const double dx = pt1[0] - pt2[0];
	const double dy = pt1[1] - pt2[1];
	const double dq = dx * dx + dy * dy;
	const double d = sqrt(dq);
	angle = acos((pt1[1] - pt2[1]) / (d + 0.000001));
	if (pt1[0] < pt2[0])
		angle *= -1;
	std::ostringstream oss;
	Mat4f MVP = prj->getMatProjectionOrtho2D();
	//localPos = nav->earthEquToLocal(pt5);
	(prj->*proj_func)(pt5, pt0);
	Mat4f TRANSFO = Mat4f::translation(Vec3f(pt0[0], pt0[1], 0));
	TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
	oss << truncf(ang) << "°";
	mn = truncf((ang - truncf(ang)) * 60);
	if (mn < 10)
		oss << "0";
	oss << mn << "'";
	skydisplay_font->print(2, -2, oss.str(), color, MVP * TRANSFO, 1);
	oss.clear();
}

SkyLoxodromy::SkyLoxodromy() : SkyDisplay(PROJECTION_TYPE::EQ)
{
}

void SkyLoxodromy::draw(const Projector *prj, const Navigator *nav, Vec3d equPos, Vec3d oldEquPos)
{
	if (!fader.getInterstate() or (equPos == oldEquPos))
		return;
	double de1, ra1, de2, ra2, dem, ram;
	// for Old position
	Utility::rectToSphe(&ra1, &de1, oldEquPos);
	// for Selected position
	Utility::rectToSphe(&ra2, &de2, equPos);
	if ((ra2 - ra1) > M_PI)
		ra1 += 2 * M_PI;
	if ((ra1 - ra2) > M_PI)
		ra2 += 2 * M_PI;
	float distM, Rv;
	ram = (ra1 + ra2) / 2;
	dem = (de1 + de2) / 2;
	Rv = atan((ra2 - ra1) * cos(dem) / (de2 - de1 + 0.00001));
	distM = fabs((de2 - de1) / cos(Rv) * rad2deg * 60);
	Utility::spheToRect(ra1, de1, pt3);
	Utility::spheToRect(ra2, de2, pt4);
	Utility::spheToRect(ram, dem, pt5);
	(prj->*proj_func)(pt3, pt1);
	(prj->*proj_func)(pt4, pt2);
	(prj->*proj_func)(pt5, pt0);
	double angle;
	const double dx = pt1[0] - pt2[0];
	const double dy = pt1[1] - pt2[1];
	const double dq = dx * dx + dy * dy;
	const double d = sqrt(dq);
	angle = acos((pt1[1] - pt2[1]) / (d + 0.000001));
	if (pt1[0] < pt2[0])
		angle *= -1;
	clear();
	for (int j = 0; (pi_div_2 - fabs(de1 * (10 - j) / 10 + de2 * j / 10)) > 0.001; j++) {
		Utility::spheToRect((ra1 * (10 - j) / 10 + ra2 * j / 10), (de1 * (10 - j) / 10 + de2 * j / 10), pt1);
		dataSky.push_back(pt1[0]); // punts[0] represent x coordinate
		dataSky.push_back(pt1[1]); // punts[1] represent y coordinate
		dataSky.push_back(pt1[2]); // punts[2] represent z coordinate
		Utility::spheToRect((ra1 * (9 - j) / 10 + ra2 * (j + 1) / 10), (de1 * (9 - j) / 10 + de2 * (j + 1) / 10), pt2);
		dataSky.push_back(pt2[0]); // punts[0] represent x coordinate
		dataSky.push_back(pt2[1]); // punts[1] represent y coordinate
		dataSky.push_back(pt2[2]); // punts[2] represent z coordinate
	}
	glBindVertexArray(sData.vao);
	glBindBuffer(GL_ARRAY_BUFFER, sData.pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * dataSky.size(), dataSky.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode
	shaderSkyDisplay->use();
	shaderSkyDisplay->setUniform("color", color);
	shaderSkyDisplay->setUniform("fader", fader.getInterstate());
	shaderSkyDisplay->setUniform("Mat", prj->getMatEarthEquToEye());
	glBindVertexArray(sData.vao);
	glDrawArrays(GL_LINES, 0, dataSky.size() / 3); //un point est représenté par 3 points
	shaderSkyDisplay->unuse();
	//draw_text(prj, nav);
	std::ostringstream oss;
	Mat4f MVP = prj->getMatProjectionOrtho2D();
	Mat4f TRANSFO = Mat4f::translation(Vec3f(pt0[0], pt0[1], 0));
	TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), 3 * pi_div_2 - angle);
	oss << truncf(distM) << " nmi"; // for km *1.85185
	skydisplay_font->print(2, -2, oss.str(), color, MVP * TRANSFO, 1);
	oss.clear();
}

SkyOrthodromy::SkyOrthodromy() : SkyDisplay(PROJECTION_TYPE::AL)
{
}

void SkyOrthodromy::draw(const Projector *prj, const Navigator *nav, Vec3d equPos, Vec3d oldEquPos)
{
	if (!fader.getInterstate())
		return;
	// for Selected position
	double ra1, de1, ra2, de2, rat, det, ang, alt1, az1, alt2, az2;
	Utility::rectToSphe(&ra1, &de1, oldEquPos);
	Utility::rectToSphe(&ra2, &de2, equPos);
	if ((ra2 - ra1) > M_PI)
		ra1 += 2 * M_PI;
	if ((ra1 - ra2) > M_PI)
		ra2 += 2 * M_PI;
	ang = acos(sin(de1) * sin(de2) + cos(de1) * cos(de2) * cos(ra2 - ra1)) * rad2deg;
	clear();
	// Draw orthodromy
	Utility::spheToRect(ra1, de1, pt1);
	int npoints = 11;
	float delta = (ra1 - ra2) / (npoints - 1);
	for (int i = 0; i < npoints; i++) {
		dataSky.push_back(pt1[0]); // punts[0] represent x coordinate
		dataSky.push_back(pt1[1]); // punts[1] represent y coordinate
		dataSky.push_back(pt1[2]); // punts[2] represent z coordinate
		rat = ra1 - delta * i;
		det = atan(((tan(de2) * sin(rat - ra1)) / sin(ra2 - ra1 + 0.00001)) + (tan(de1) * sin(ra2 - rat)) / sin(ra2 - ra1 + 0.00001));
		Utility::spheToRect(rat, det, pt1);
		if (i == 5)
			pt5 = pt1;
		dataSky.push_back(pt1[0]); // punts[0] represent x coordinate
		dataSky.push_back(pt1[1]); // punts[1] represent y coordinate
		dataSky.push_back(pt1[2]); // punts[2] represent z coordinate
	}
	glBindVertexArray(sData.vao);
	glBindBuffer(GL_ARRAY_BUFFER, sData.pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * dataSky.size(), dataSky.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode
	shaderSkyDisplay->use();
	shaderSkyDisplay->setUniform("color", color);
	shaderSkyDisplay->setUniform("fader", fader.getInterstate());
	shaderSkyDisplay->setUniform("Mat", prj->getMatEarthEquToEye());
	glBindVertexArray(sData.vao);
	glDrawArrays(GL_LINES, 0, dataSky.size() / 3); //un point est représenté par 3 points
	shaderSkyDisplay->unuse();

	// Text
	Vec3d localPos = nav->earthEquToLocal(oldEquPos);
	Utility::rectToSphe(&rat, &det, localPos);
	rat = 3 * M_PI - rat; // N is zero, E is 90 degrees
	if (rat > M_PI * 2)
		rat -= M_PI * 2;
	alt1 = det;
	az1 = M_PI - rat;
	Utility::spheToRect(az1, alt1, pt3);
	(prj->*proj_func)(pt3, pt1);

	localPos = nav->earthEquToLocal(equPos);
	Utility::rectToSphe(&rat, &det, localPos);
	rat = 3 * M_PI - rat; // N is zero, E is 90 degrees
	if (rat > M_PI * 2)
		rat -= M_PI * 2;
	alt2 = det;
	az2 = M_PI - rat;
	Utility::spheToRect(az2, alt2, pt4);
	(prj->*proj_func)(pt4, pt2);

	double angle;
	const double dx = pt1[0] - pt2[0];
	const double dy = pt1[1] - pt2[1];
	const double dq = dx * dx + dy * dy;
	const double d = sqrt(dq);
	angle = acos((pt1[1] - pt2[1]) / (d + 0.000001));
	if (pt1[0] > pt2[0])
		angle *= -1;
	Mat4f MVP = prj->getMatProjectionOrtho2D();
	localPos = nav->earthEquToLocal(pt5);
	(prj->*proj_func)(localPos, pt0);
	Mat4f TRANSFO = Mat4f::translation(Vec3f(pt0[0], pt0[1], 0));
	TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), M_PI - pi_div_2 + angle);
	std::ostringstream oss;
	oss << truncf(ang * 60) << " nmi"; // for km *1.85185
	skydisplay_font->print(2, -2, oss.str(), color, MVP * TRANSFO, 1);
	oss.clear();
}
