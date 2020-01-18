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
#include "coreModule/skygrid.hpp"
#include "tools/s_texture.hpp"
#include "tools/utility.hpp"
#include <string>


#include "tools/fmath.hpp"



SkyGrid::SkyGrid(unsigned int _nb_meridian, unsigned int _nb_parallel,
                 double _radius, unsigned int _nb_alt_segment, unsigned int _nb_azi_segment) :
	nb_meridian(_nb_meridian), nb_parallel(_nb_parallel), 	radius(_radius), nb_alt_segment(_nb_alt_segment), nb_azi_segment(_nb_azi_segment), color(0.2,0.2,0.2)
{
	// Alt points are the points to draw along the meridian
	alt_points = new Vec3f*[nb_meridian];
	for (unsigned int nm=0; nm<nb_meridian; ++nm) {
		alt_points[nm] = new Vec3f[nb_alt_segment+1];
		for (unsigned int i=0; i<nb_alt_segment+1; ++i) {
			Utility::spheToRect((float)nm/(nb_meridian)*2.f*C_PI,(float)i/nb_alt_segment*C_PI-C_PI_2, alt_points[nm][i]);
			alt_points[nm][i] *= radius;
		}
	}

	// Alt points are the points to draw along the meridian
	azi_points = new Vec3f*[nb_parallel];
	for (unsigned int np=0; np<nb_parallel; ++np) {
		azi_points[np] = new Vec3f[nb_azi_segment+1];
		for (unsigned int i=0; i<nb_azi_segment+1; ++i) {
			Utility::spheToRect((float)i/(nb_azi_segment)*2.f*C_PI, (float)(np+1)/(nb_parallel+1)*C_PI-C_PI_2, azi_points[np][i]);
			azi_points[np][i] *= radius;
		}
	}

	createShader();
}

SkyGrid::~SkyGrid()
{
	for (unsigned int nm=0; nm<nb_meridian; ++nm) {
		delete [] alt_points[nm];
	}
	delete [] alt_points;

	for (unsigned int np=0; np<nb_parallel; ++np) {
		delete [] azi_points[np];
	}
	delete [] azi_points;

	if (font) delete font;
	font = nullptr;

	dataSky.clear();
	dataColor.clear();
	deleteShader();
}

void SkyGrid::createShader()
{
	shaderSkyGrid = new shaderProgram();
	shaderSkyGrid->init("skygrid.vert","skygrid.geom","skygrid.frag");
	shaderSkyGrid->setUniformLocation("color");
	shaderSkyGrid->setUniformLocation("fader");
	shaderSkyGrid->setUniformLocation("Mat");

	glGenVertexArrays(1,&sData.vao);
	glBindVertexArray(sData.vao);
	glGenBuffers(1,&sData.pos);
	glGenBuffers(1,&sData.color); //en fait c'est plutôt l'intensité de la couleur du point

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	createBuffer();
}

void SkyGrid::deleteShader()
{
	if(shaderSkyGrid) shaderSkyGrid=nullptr;

	glDeleteBuffers(1,&sData.color);
	glDeleteBuffers(1,&sData.pos);
	glDeleteVertexArrays(1,&sData.vao);
}

void SkyGrid::setFont(float font_size, const std::string& font_name)
{
	font = new s_font(font_size, font_name);
	assert(font);
}

void SkyGrid::createBuffer()
{
	dataSky.clear();
	dataColor.clear();

	// Draw meridians
	for (unsigned int nm=0; nm<nb_meridian; ++nm) {
		dataSky.push_back(alt_points[nm][0][0]);
		dataSky.push_back(alt_points[nm][0][1]);
		dataSky.push_back(alt_points[nm][0][2]);
		dataColor.push_back(0.f);

		dataSky.push_back(alt_points[nm][1][0]);
		dataSky.push_back(alt_points[nm][1][1]);
		dataSky.push_back(alt_points[nm][1][2]);
		dataColor.push_back(1.f);

		for (unsigned int i=1; i<nb_alt_segment-1; ++i) {
			dataSky.push_back(alt_points[nm][i][0]);
			dataSky.push_back(alt_points[nm][i][1]);
			dataSky.push_back(alt_points[nm][i][2]);
			dataColor.push_back(1.f);

			dataSky.push_back(alt_points[nm][i+1][0]);
			dataSky.push_back(alt_points[nm][i+1][1]);
			dataSky.push_back(alt_points[nm][i+1][2]);
			dataColor.push_back(1.f);
		}

		dataSky.push_back(alt_points[nm][nb_alt_segment-1][0]);
		dataSky.push_back(alt_points[nm][nb_alt_segment-1][1]);
		dataSky.push_back(alt_points[nm][nb_alt_segment-1][2]);
		dataColor.push_back(1.f);

		dataSky.push_back(alt_points[nm][nb_alt_segment][0]);
		dataSky.push_back(alt_points[nm][nb_alt_segment][1]);
		dataSky.push_back(alt_points[nm][nb_alt_segment][2]);
		dataColor.push_back(0.f);
	}

	// Draw parallels
	for (unsigned int np=0; np<nb_parallel; ++np) {
		for (unsigned int i=0; i<nb_azi_segment; ++i) {
			dataSky.push_back(azi_points[np][i][0]);
			dataSky.push_back(azi_points[np][i][1]);
			dataSky.push_back(azi_points[np][i][2]);
			dataColor.push_back(1.f);

			dataSky.push_back(azi_points[np][i+1][0]);
			dataSky.push_back(azi_points[np][i+1][1]);
			dataSky.push_back(azi_points[np][i+1][2]);
			dataColor.push_back(1.f);
		}
	}

	//on charge les points dans un vbo
	glBindVertexArray(sData.vao);

	glBindBuffer(GL_ARRAY_BUFFER,sData.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*dataSky.size(),dataSky.data(),GL_STATIC_DRAW);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

	glBindBuffer(GL_ARRAY_BUFFER,sData.color);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*dataColor.size(),dataColor.data(),GL_STATIC_DRAW);
	glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,0,NULL);
}


void SkyGrid::draw(const Projector* prj) const
{
	if (!fader.getInterstate()) return;

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	Vec3d pt1;
	Vec3d pt2;

	shaderSkyGrid->use();
	shaderSkyGrid->setUniform("color", color);
	shaderSkyGrid->setUniform("fader", fader.getInterstate());

	switch (gtype) {
		case EQUATORIAL:
			shaderSkyGrid->setUniform("Mat", prj->getMatEarthEquToEye() );
			break;
		case ECLIPTIC :
			shaderSkyGrid->setUniform("Mat", prj->getMatEarthEquToEye()* Mat4f::xrotation(23.4392803055555555556*(C_PI/180)) );
			break;
		case GALACTIC :
			shaderSkyGrid->setUniform("Mat", prj->getMatEarthEquToEye()* Mat4f::zrotation(14.8595*(C_PI/180))*Mat4f::yrotation(-61.8717*(C_PI/180))*Mat4f::zrotation(55.5*(C_PI/180)) );
			break;
		case ALTAZIMUTAL :
			shaderSkyGrid->setUniform("Mat", prj->getMatLocalToEye() );
			break;
		default:
			return; //pour GCC
	}

	glBindVertexArray(sData.vao);
	glDrawArrays(GL_LINES, 0, dataSky.size()/3 ); //un point est représenté par 3 points
	shaderSkyGrid->unuse();

	// tracé de texte.
	for (unsigned int nm=0; nm<nb_meridian; ++nm) {

		Vec4f Color (color[0],color[1],color[2],fader.getInterstate());
		Mat4f MVP = prj->getMatProjectionOrtho2D();

		for (unsigned int i=1; i<nb_alt_segment-1; ++i) {
			if ((prj->*proj_func)(alt_points[nm][i], pt1) && (prj->*proj_func)(alt_points[nm][i+1], pt2) ) {

				static char str[255];	// TODO use c++ string

				double angle;
				const double dx = pt1[0]-pt2[0];
				const double dy = pt1[1]-pt2[1];
				const double dq = dx*dx+dy*dy;

				// TODO: allow for other numbers of meridians and parallels without screwing up labels?
				if ( i == 8 ) {
					// draw labels along equator for RA
					const double d = sqrt(dq);

					angle = acos((pt1[1]-pt2[1])/d);
					if ( pt1[0] < pt2[0] ) {
						angle *= -1;
					}

					if ( gtype == EQUATORIAL ) {
						if (internalNav)
							sprintf( str, "%d°E", (24-nm)*15);
						else
							sprintf( str, "%dh", nm);
					} else {
						sprintf( str, "%d°", nm<12 ? (360-(12-nm)*15) : (360-(36-nm)*15) );
					}

					Mat4f TRANSFO= Mat4f::translation( Vec3f(pt2[0],pt2[1],0) );
					TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), -C_PI_2-angle );

					if ( gtype == EQUATORIAL ) {
						font->print(2,-2,str, Color, MVP*TRANSFO ,1,1);
					} else {
						font->print(6,-2,str, Color, MVP*TRANSFO ,1,1);
					}

				} else if (nm % 8 == 0 && i != 16) {

					const double d = sqrt(dq);

					angle = acos((pt1[1]-pt2[1])/d);
					if ( pt1[0] < pt2[0] ) {
						angle *= -1;
					}

					sprintf( str, "%d°", (i-8)*10);

					if ( gtype == GALACTIC || gtype == ALTAZIMUTAL || (gtype == EQUATORIAL && i > 8)) {
						angle += C_PI;
					}

					Mat4f TRANSFO= Mat4f::translation( Vec3f(pt2[0],pt2[1],0) );
					TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), -angle);

					font->print(2,-2,str, Color, MVP*TRANSFO ,1,1);
				}
			}
		}
	}
}
