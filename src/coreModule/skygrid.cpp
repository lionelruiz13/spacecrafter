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

#include <string>

#include "bodyModule/body.hpp"
#include "coreModule/skygrid.hpp"
#include "tools/s_texture.hpp"
#include "tools/utility.hpp"
#include "renderGL/OpenGL.hpp"
#include "renderGL/shader.hpp"

std::unique_ptr<VertexArray> SkyGrid::m_dataGL;
std::unique_ptr<shaderProgram> SkyGrid::shaderSkyGrid;
unsigned int SkyGrid::nbPointsToDraw;


SkyGrid::SkyGrid(unsigned int _nb_meridian, unsigned int _nb_parallel,
                 double _radius, unsigned int _nb_alt_segment, unsigned int _nb_azi_segment) :
	nb_meridian(_nb_meridian), nb_parallel(_nb_parallel), 	radius(_radius), nb_alt_segment(_nb_alt_segment), nb_azi_segment(_nb_azi_segment), color(0.2,0.2,0.2)
{
	// Alt points are the points to draw along the meridian
	alt_points = new Vec3f*[nb_meridian];
	for (unsigned int nm=0; nm<nb_meridian; ++nm) {
		alt_points[nm] = new Vec3f[nb_alt_segment+1];
		for (unsigned int i=0; i<nb_alt_segment+1; ++i) {
			Utility::spheToRect((float)nm/(nb_meridian)*2.f*M_PI,(float)i/nb_alt_segment*M_PI-M_PI_2, alt_points[nm][i]);
			alt_points[nm][i] *= radius;
		}
	}

	// Alt points are the points to draw along the meridian
	azi_points = new Vec3f*[nb_parallel];
	for (unsigned int np=0; np<nb_parallel; ++np) {
		azi_points[np] = new Vec3f[nb_azi_segment+1];
		for (unsigned int i=0; i<nb_azi_segment+1; ++i) {
			Utility::spheToRect((float)i/(nb_azi_segment)*2.f*M_PI, (float)(np+1)/(nb_parallel+1)*M_PI-M_PI_2, azi_points[np][i]);
			azi_points[np][i] *= radius;
		}
	}
	createBuffer();
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

	// if (font) delete font;
	// font = nullptr;
}

void SkyGrid::createShader()
{
	shaderSkyGrid = std::make_unique<shaderProgram>();
	shaderSkyGrid->init("skygrid.vert","skygrid.geom","skygrid.frag");
	shaderSkyGrid->setUniformLocation({"color","fader","Mat"});

	m_dataGL = std::make_unique<VertexArray>();
	m_dataGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
	m_dataGL->registerVertexBuffer(BufferType::MAG, BufferAccess::STATIC);
}


void SkyGrid::createBuffer()
{
	std::vector<float> dataSky;
	std::vector<float> dataColor;

	// Draw meridians
	for (unsigned int nm=0; nm<nb_meridian; ++nm) {
		insert_vec3(dataSky,alt_points[nm][0]);
		dataColor.push_back(0.f);

		insert_vec3(dataSky,alt_points[nm][1]);		
		dataColor.push_back(1.f);

		for (unsigned int i=1; i<nb_alt_segment-1; ++i) {
			insert_vec3(dataSky,alt_points[nm][i]);
			dataColor.push_back(1.f);

			insert_vec3(dataSky,alt_points[nm][i+1]);
			dataColor.push_back(1.f);
		}

		insert_vec3(dataSky,alt_points[nm][nb_alt_segment-1]);
		dataColor.push_back(1.f);

		insert_vec3(dataSky,alt_points[nm][nb_alt_segment]);
		dataColor.push_back(0.f);
	}

	// Draw parallels
	for (unsigned int np=0; np<nb_parallel; ++np) {
		for (unsigned int i=0; i<nb_azi_segment; ++i) {
			insert_vec3(dataSky,azi_points[np][i]);
			dataColor.push_back(1.f);

			insert_vec3(dataSky,azi_points[np][i+1]);
			dataColor.push_back(1.f);
		}
	}

	nbPointsToDraw = dataSky.size()/3;
	m_dataGL->fillVertexBuffer(BufferType::POS3D, dataSky);
	m_dataGL->fillVertexBuffer(BufferType::MAG, dataColor);
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
			shaderSkyGrid->setUniform("Mat", prj->getMatEarthEquToEye()* Mat4f::xrotation(23.4392803055555555556*(M_PI/180)) );
			break;
		case GALACTIC :
			shaderSkyGrid->setUniform("Mat", prj->getMatJ2000ToEye()* Mat4f::zrotation(14.8595*(M_PI/180))*Mat4f::yrotation(-61.8717*(M_PI/180))*Mat4f::zrotation(55.5*(M_PI/180)) );
			break;
		case ALTAZIMUTAL :
			shaderSkyGrid->setUniform("Mat", prj->getMatLocalToEye() );
			break;
		default:
			return; //pour GCC
	}

	// m_dataGL->bind();
	// glDrawArrays(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, 0, nbPointsToDraw); //un point est représenté par 3 points
	// m_dataGL->unBind();
	// shaderSkyGrid->unuse();
	Renderer::drawArrays(shaderSkyGrid.get(), m_dataGL.get(), VK_PRIMITIVE_TOPOLOGY_LINE_LIST, 0, nbPointsToDraw); //un point est représenté par 3 points


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
					} else if ( gtype == ALTAZIMUTAL ) {
						sprintf( str, "%d°", nm<12 ? ((12-nm)*15) : ((36-nm)*15) );					
					} else {
						sprintf( str, "%d°", nm<12 ? (360-(12-nm)*15) : (360-(36-nm)*15) );
					}

					Mat4f TRANSFO= Mat4f::translation( Vec3f(pt2[0],pt2[1],0) );
					TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), -M_PI_2-angle );

					if ( gtype == EQUATORIAL ) {
						font->print(2,-2,str, Color, MVP*TRANSFO ,1);
					} else {
						font->print(6,-2,str, Color, MVP*TRANSFO ,1);
					}

				} 
				if (nm % 8 == 0 && i != 16) {

					const double d = sqrt(dq);

					angle = acos((pt1[1]-pt2[1])/d);
					if ( pt1[0] < pt2[0] ) {
						angle *= -1;
					}

					sprintf( str, "%d°", (i-8)*10);

					if ( gtype == GALACTIC || gtype == ALTAZIMUTAL || (gtype == EQUATORIAL && i >= 8)) {
						angle += M_PI;
					}

					Mat4f TRANSFO= Mat4f::translation( Vec3f(pt2[0],pt2[1],0) );
					TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), -angle);

					font->print(2,-2,str, Color, MVP*TRANSFO ,1);
				}
			}
		}
	}
}
