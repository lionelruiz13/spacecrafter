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

#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

unsigned int SkyGrid::nbPointsToDraw = -1;
VertexArray *SkyGrid::m_dataGL;
Pipeline *SkyGrid::pipeline;
PipelineLayout *SkyGrid::layout;
Set *SkyGrid::set;
int SkyGrid::vUniformID0;
int SkyGrid::vUniformID1;
s_font* SkyGrid::font = nullptr;
std::weak_ptr<VertexBuffer> SkyGrid::pVertex;

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
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Context &context = *Context::instance;

	m_dataGL = new VertexArray(vkmgr);
    m_dataGL->createBindingEntry(4 * sizeof(float));
    m_dataGL->addInput(VK_FORMAT_R32G32B32_SFLOAT); // POS3D
    m_dataGL->addInput(VK_FORMAT_R32_SFLOAT); // MAG
    layout = new PipelineLayout(vkmgr);
    layout->setGlobalPipelineLayout(context.layouts.front().get());
    layout->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT, 0, 1, true);
    layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1, true);
    layout->buildLayout();
    layout->build();
    pipeline = new Pipeline(vkmgr, *context.render, PASS_BACKGROUND, layout);
    pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    pipeline->setDepthStencilMode();
    pipeline->bindVertex(*m_dataGL);
    pipeline->bindShader("skygrid.vert.spv");
    pipeline->bindShader("skygrid.geom.spv");
    pipeline->bindShader("skygrid.frag.spv");
    pipeline->build();
    set = new Set(vkmgr, *context.setMgr, layout);
    vUniformID0 = set->bindVirtualUniform(context.uniformMgr->getBuffer(), 0, sizeof(Mat4f));
    vUniformID1 = set->bindVirtualUniform(context.uniformMgr->getBuffer(), 1, sizeof(frag));
}

void SkyGrid::destroyShader()
{
    delete m_dataGL;
    delete pipeline;
    delete layout;
    delete set;
}

void SkyGrid::createBuffer()
{
    Context &context = *Context::instance;

    nbPointsToDraw = (nb_meridian * nb_alt_segment + nb_parallel * nb_azi_segment) * 2;
    vertex = m_dataGL->createBuffer(0, nbPointsToDraw, context.globalBuffer.get());
    pVertex = vertex;
	float *dataSky = (float *) context.transfer->planCopy(vertex->get());

	// Draw meridians
	for (unsigned int nm=0; nm<nb_meridian; ++nm) {
        *(dataSky++) = alt_points[nm][0][0];
        *(dataSky++) = alt_points[nm][0][1];
        *(dataSky++) = alt_points[nm][0][2];
        *(dataSky++) = 0.f;

        *(dataSky++) = alt_points[nm][1][0];
        *(dataSky++) = alt_points[nm][1][1];
        *(dataSky++) = alt_points[nm][1][2];
        *(dataSky++) = 1.f;

		for (unsigned int i=1; i<nb_alt_segment-1; ++i) {
            *(dataSky++) = alt_points[nm][i][0];
            *(dataSky++) = alt_points[nm][i][1];
            *(dataSky++) = alt_points[nm][i][2];
            *(dataSky++) = 1.f;

            *(dataSky++) = alt_points[nm][i+1][0];
            *(dataSky++) = alt_points[nm][i+1][1];
            *(dataSky++) = alt_points[nm][i+1][2];
            *(dataSky++) = 1.f;
		}

        *(dataSky++) = alt_points[nm][nb_alt_segment - 1][0];
        *(dataSky++) = alt_points[nm][nb_alt_segment - 1][1];
        *(dataSky++) = alt_points[nm][nb_alt_segment - 1][2];
        *(dataSky++) = 1.f;

        *(dataSky++) = alt_points[nm][nb_alt_segment][0];
        *(dataSky++) = alt_points[nm][nb_alt_segment][1];
        *(dataSky++) = alt_points[nm][nb_alt_segment][2];
        *(dataSky++) = 0.f;
	}

	// Draw parallels
	for (unsigned int np=0; np<nb_parallel; ++np) {
		for (unsigned int i=0; i<nb_azi_segment; ++i) {
            *(dataSky++) = azi_points[np][i][0];
            *(dataSky++) = azi_points[np][i][1];
            *(dataSky++) = azi_points[np][i][2];
            *(dataSky++) = 1.f;

            *(dataSky++) = azi_points[np][i+1][0];
            *(dataSky++) = azi_points[np][i+1][1];
            *(dataSky++) = azi_points[np][i+1][2];
            *(dataSky++) = 1.f;
		}
	}
}

void SkyGrid::recordDraw()
{
    Context &context = *Context::instance;
    uMat = std::make_unique<SharedBuffer<Mat4f>>(*context.uniformMgr);
    uFrag = std::make_unique<SharedBuffer<frag>>(*context.uniformMgr);
    set->setVirtualUniform(uMat->getOffset(), vUniformID0);
    set->setVirtualUniform(uFrag->getOffset(), vUniformID1);

    context.cmdInfo.commandBufferCount = 3;
    vkAllocateCommandBuffers(VulkanMgr::instance->refDevice, &context.cmdInfo, cmds);
    for (int i = 0; i < 3; ++i) {
		auto cmd = cmds[i];
		context.frame[i]->begin(cmd, PASS_BACKGROUND);
		pipeline->bind(cmd);
		layout->bindSets(cmd, {*context.uboSet, *set}, *set);
		vertex->bind(cmd);
		vkCmdDraw(cmd, nbPointsToDraw, 1, 0, 0);
		context.frame[i]->compile(cmd);
        context.frame[i]->setName(cmd, "SkyGrid " + std::to_string(i));
	}
}

void SkyGrid::draw(const Projector* prj)
{
	if (!fader.getInterstate()) return;

    if (!vertex) {
        vertex = pVertex.lock();
        if (!vertex)
            createBuffer();
    }
    if (!cmds[0])
        recordDraw();

    Vec3d pt1;
	Vec3d pt2;

    uFrag->get().color = color;
    uFrag->get().fader = fader.getInterstate();

	switch (gtype) {
		case EQUATORIAL:
			*uMat = prj->getMatEarthEquToEye();
			break;
		case ECLIPTIC :
			*uMat = prj->getMatEarthEquToEye()* Mat4f::xrotation(23.4392803055555555556*(M_PI/180));
			break;
		case GALACTIC :
			*uMat = prj->getMatJ2000ToEye()* Mat4f::zrotation(14.8595*(M_PI/180))*Mat4f::yrotation(-61.8717*(M_PI/180))*Mat4f::zrotation(55.5*(M_PI/180));
			break;
		case ALTAZIMUTAL :
			*uMat = prj->getMatLocalToEye();
			break;
		default:
			return; //for GCC
	}

    Context::instance->frame[Context::instance->frameIdx]->toExecute(cmds[Context::instance->frameIdx], PASS_BACKGROUND);

	// text plot.
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
