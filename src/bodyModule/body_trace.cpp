/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
 * Copyright (C) 2014-2020 of the LSS Team & Association Sirius
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

#include "bodyModule/body_trace.hpp"
#include "tools/utility.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "renderGL/OpenGL.hpp"
#include "renderGL/shader.hpp"


BodyTrace::BodyTrace()
{
	for(int i= 0; i<NB_MAX_LIST; i++) {
		bodyData[i].size=0;
		bodyData[i].old_punt[0]=0.0;
		bodyData[i].old_punt[1]=0.0;
		bodyData[i].hide=false;
	}
	bodyData[0].color= Vec3f(1.0,0.0,0.0);
	bodyData[1].color= Vec3f(0.2f, 0.2f, 0.7f);
	bodyData[2].color= Vec3f(0.5f, 0.9f, 0.4f);
	bodyData[3].color= Vec3f(0.3f, 0.6f, 0.1f);
	bodyData[4].color= Vec3f(0.1f, 0.8f, 0.5f);
	bodyData[5].color= Vec3f(0.f, 1.0f, 0.2f);
	bodyData[6].color= Vec3f(0.2f, 1.0f, 0.4f);
	bodyData[7].color= Vec3f(0.4f, 1.0f, 0.6f);
	is_tracing=true;
	currentUsedList=0;
	createSC_context();
}

BodyTrace::~BodyTrace()
{}

void BodyTrace::hide(int numberList)
{
	if ( numberList > (NB_MAX_LIST-1)) return;
	if (numberList==-1) {
		//-1 mean all the lists
		for(int i=0; i<NB_MAX_LIST; i++)
			bodyData[i].hide= !bodyData[i].hide;
	}
	else
		bodyData[numberList].hide= !bodyData[numberList].hide;
}

void BodyTrace::createSC_context()
{
	shaderTrace = std::make_unique<shaderProgram>();
	shaderTrace->init("body_trace.vert","body_trace.geom","body_trace.frag");
	shaderTrace->setUniformLocation({"Color", "Mat"});

	m_dataGL = std::make_unique<VertexArray>();
	m_dataGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::DYNAMIC);
}


void BodyTrace::draw(const Projector *prj,const Navigator *nav)
{
	if (!fader.getInterstate()) return;

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	shaderTrace->use();

	for(int l=0; l<currentUsedList+1; l++) {

		if (bodyData[l].size>2 && !bodyData[l].hide) {
			for (int i=0; i < bodyData[l].size; i=i+1) {
				insert_vec3(vecVertex, bodyData[l].punts[i]);
			}

			//tracé en direct de la courbe de ci dessus
			if( vecVertex.size() >=6 ) { // il faut au moins deux points pour le tracé

				shaderTrace->setUniform("Color", bodyData[l].color );
				shaderTrace->setUniform("Mat", prj->getMatLocalToEye() );

				m_dataGL->fillVertexBuffer(BufferType::POS3D,vecVertex);

				// m_dataGL->bind();
				// glDrawArrays(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, 0, vecVertex.size()/3);
				// m_dataGL->unBind();
				Renderer::drawArraysWithoutShader(m_dataGL.get(), VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, 0, vecVertex.size()/3);
			}
			//suppression du contenu des data
			vecVertex.clear();
		}
	}
	shaderTrace->unuse();
}

void BodyTrace::addData(const Navigator *nav, double alt, double az)
{
	if (!fader.getInterstate()) return;
	if(!is_tracing) return;

	if (bodyData[currentUsedList].size==0) {
		bodyData[currentUsedList].old_punt[0]=alt;
		bodyData[currentUsedList].old_punt[1]=az;
		Utility::spheToRect(-(az+M_PI),alt,bodyData[currentUsedList].punts[0]);
		bodyData[currentUsedList].size=1;
	}
	else {
		if ( (abs(alt-bodyData[currentUsedList].old_punt[0])< 0.001) && (abs(az-bodyData[currentUsedList].old_punt[1])< 0.001) ) return;
		if (bodyData[currentUsedList].size==(MAX_POINTS-1)) return;
		bodyData[currentUsedList].old_punt[0]=alt;
		bodyData[currentUsedList].old_punt[1]=az;
		bodyData[currentUsedList].size+=1;
		Utility::spheToRect(-(az+M_PI),alt,bodyData[currentUsedList].punts[bodyData[currentUsedList].size]);
	}
}
