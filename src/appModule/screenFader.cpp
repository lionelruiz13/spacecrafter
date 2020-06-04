/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2018 of Association Sirius
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

#include "appModule/screenFader.hpp"

ScreenFader::ScreenFader()
{
	//~ initShader();
	intensity = 0.0;
}

ScreenFader::~ScreenFader()
{
	
}

void ScreenFader::initShader()
{
	shaderScreen =  nullptr;
	shaderScreen= new shaderProgram();
	shaderScreen->init( "screenFader.vert","screenFader.frag");
	shaderScreen->setUniformLocation("intensity");
	initShaderParams();
}

void ScreenFader::initShaderParams()
{
	float points[8];

	points[0]= -1.0; // point en haut a gauche
	points[1]= 1.0;

	points[2]= 1.0;  // point en haut a droite
	points[3]= 1.0;

	points[4]= -1.0; // point en bas à gauche
	points[5]= -1.0;

	points[6]= 1.0;  // point en bas à droite
	points[7]= -1.0;

	//~ float points[]={-1.f, 1.f, 1.f, 1.f, -1.f, -1.f, 1.f, -1.f };

	glGenBuffers(1,&screen.pos);
	glBindBuffer(GL_ARRAY_BUFFER,screen.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*8, points,GL_STATIC_DRAW);

	glGenVertexArrays(1,&screen.vao);
	glBindVertexArray(screen.vao);

	glBindBuffer (GL_ARRAY_BUFFER, screen.pos);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);

	glEnableVertexAttribArray(0);
}

void ScreenFader::draw()
{
	if (intensity==0)
		return;

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	shaderScreen->use();
	shaderScreen->setUniform("intensity" , intensity);

	glBindVertexArray(screen.vao);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	glBindVertexArray(0);
	shaderScreen->unuse();
}
