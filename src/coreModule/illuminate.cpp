/*
 * Spacecrafter astronomy simulation and visualization
 *
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
#include "coreModule/illuminate.hpp"
#include "tools/s_texture.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "tools/log.hpp"
#include "tools/fmath.hpp"


shaderProgram* Illuminate::shaderIllum=nullptr;

DataGL Illuminate::Illum;

s_texture * Illuminate::illuminateTex= nullptr;


Illuminate::Illuminate()
{
	Name = "";
	illuminateSpecialTex = nullptr;
	specialTex = false;
}

void Illuminate::createShader()
{
	//======raw========
	shaderIllum = new shaderProgram();
	shaderIllum->init( "illuminate.vert", "illuminate.frag");
	shaderIllum->setUniformLocation("Color");

	glGenVertexArrays(1,&Illum.vao);
	glBindVertexArray(Illum.vao);

	glGenBuffers(1,&Illum.pos);
	glGenBuffers(1,&Illum.tex);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}


void Illuminate::deleteShader()
{
	if (shaderIllum) delete shaderIllum;

	glDeleteBuffers(1,&Illum.pos);
	glDeleteBuffers(1,&Illum.tex);
	glDeleteVertexArrays(1,&Illum.vao);
}


Illuminate::~Illuminate()
{
	//deleteShader();
}


// Read Illuminate data passed in and compute x,y and z;
bool Illuminate::createIlluminate(const std::string& filename, double ra, double de, double angular_size, const std::string& name, double r, double g, double b, float tex_rotation)
{
	Name = name;
	texColor.set(r,g,b);

	//std::cout << "createIlluminate "<< name << " f: " << filename << " " << ra << " " << de << " " << angular_size << " " << r << " " << g << " " << b << " rot: " << tex_rotation << std::endl;
	if (filename!="") {
		illuminateSpecialTex = new s_texture(/*true,*/ filename.c_str() ,0);
		if (illuminateSpecialTex != nullptr)
			specialTex = true;
	}

	// Calc the RA and DE from the datas - keep base info for drawing (in radians)
	myRA = ra*C_PI/180.;
	myDe = de*C_PI/180.;

	// Calc the Cartesian coord with RA and DE
	Utility::spheToRect(myRA,myDe,XYZ);

	float tex_size = sin(angular_size/2/60*C_PI/180);

	// Precomputation of the rotation/translation matrix
	Mat4f mat_precomp = Mat4f::translation(XYZ) *
	                    Mat4f::zrotation(myRA) *
	                    Mat4f::yrotation(-myDe) *
	                    Mat4f::xrotation(tex_rotation*C_PI/180.);

	texQuadVertex[0] = mat_precomp * Vec3f(0.,-tex_size,-tex_size);
	texQuadVertex[1] = mat_precomp * Vec3f(0., tex_size,-tex_size);
	texQuadVertex[2] = mat_precomp * Vec3f(0.,-tex_size, tex_size);
	texQuadVertex[3] = mat_precomp * Vec3f(0., tex_size, tex_size);

	return true;
}


// Read Illuminate data from string and compute x,y and z returns false if can't parse record
// bool Illuminate::createIlluminate(const std::string& record)
// {
// 	std::string name, filename;
// 	float ra;
// 	float de;
// 	float tex_angular_size, rotation;
// 	double r, g, b;

// 	std::istringstream istr(record);

// 	if (!(istr >> filename >> ra >> de >> tex_angular_size >> name >> r >> g >> b >> rotation)) return false;

// 	return createIlluminate(filename, ra, de, tex_angular_size, name, r,g,b, rotation);
// }

void Illuminate::drawTex(const Projector* prj, const Navigator* nav)
{
	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Vec3d v;
	float position[12];
	float texPosition[8] = {	1.0,0.0,1.0,1.0,
	                            0.0,0.0,0.0,1.0
	                       };

	//~ glTexCoord2i(1,0);              // Bottom Right
	prj->projectJ2000(texQuadVertex[0],v);
	position[0]=v[0];
	position[1]=v[1];
	position[2]=v[2];

	//~ glTexCoord2i(1,1);              // Top Right
	prj->projectJ2000(texQuadVertex[2],v);
	position[3]=v[0];
	position[4]=v[1];
	position[5]=v[2];

	//~ glTexCoord2i(0,0);              // Bottom Left
	prj->projectJ2000(texQuadVertex[1],v);
	position[6]=v[0];
	position[7]=v[1];
	position[8]=v[2];

	//~ glTexCoord2i(0,1);              // Top Left
	prj->projectJ2000(texQuadVertex[3],v);
	position[9]=v[0];
	position[10]=v[1];
	position[11]=v[2];

	shaderIllum->use();

	if (specialTex)
		glBindTexture(GL_TEXTURE_2D, illuminateSpecialTex->getID());
	else
		glBindTexture(GL_TEXTURE_2D, illuminateTex->getID());

	shaderIllum->setUniform("Color", texColor);

	glBindVertexArray(Illum.vao);
	glBindBuffer(GL_ARRAY_BUFFER,Illum.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*12, &position[0],GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

	glBindBuffer(GL_ARRAY_BUFFER,Illum.tex);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*8, &texPosition[0],GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);

	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	shaderIllum->unuse();
}
