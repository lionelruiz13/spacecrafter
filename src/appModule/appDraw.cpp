/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2018-2020 of Association Sirius
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
#include <sstream>
#include <string>
#include <unistd.h>
#include <sstream>
#include <GL/glew.h>

#include "appModule/appDraw.hpp"
#include "tools/log.hpp"
#include "tools/s_texture.hpp"
#include "tools/utility.hpp"
#include "tools/app_settings.hpp"
#include "tools/OpenGL.hpp"
#include "tools/Renderer.hpp"

AppDraw::AppDraw()
{}


AppDraw::~AppDraw()
{}

void AppDraw::init(unsigned int _width, unsigned int _height)
{
    width=_width;
    height=_height;
	m_radius = std::min(width, height)/2;
	m_decalage_x = (width  - std::min(width, height))/2;
	m_decalage_y = (height - std::min(width, height))/2;
}

void AppDraw::initSplash()
{
	std::unique_ptr<shaderProgram> shaderSplash = std::make_unique<shaderProgram>();
	shaderSplash->init( "splash.vert", "splash.frag");

	float dataPos[]= {-1.0,-1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0};
	float dataTex[]= {0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0 };

	std::unique_ptr<VertexArray> splash = std::make_unique<VertexArray>();
	splash->registerVertexBuffer(BufferType::POS2D, BufferAccess::STATIC);
	splash->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);
	splash->fillVertexBuffer(BufferType::POS2D, 8, dataPos);
	splash->fillVertexBuffer(BufferType::TEXTURE, 8, dataTex);

	int tmp=std::min(width, height);
	Renderer::viewport((width-tmp)/2, (height-tmp)/2, tmp, tmp);

	std::unique_ptr<s_texture> tex_splash = 
			std::make_unique<s_texture>(AppSettings::Instance()->getUserDir()+"textures/splash/spacecrafter.png" , TEX_LOAD_TYPE_PNG_ALPHA);

	StateGL::disable(GL_BLEND);
	StateGL::BlendFunc(GL_ONE, GL_ONE);
	StateGL::bindTexture2D(0,tex_splash->getID());

	Renderer::drawArrays(shaderSplash.get(), splash.get(), GL_TRIANGLE_STRIP,0,4);
}


void AppDraw::createSC_context()
{
	shaderViewportShape= std::make_unique<shaderProgram>();
	shaderViewportShape->init( "viewportShape.vert", "viewportShape.frag");
	shaderViewportShape->setUniformLocation({"radius","decalage_x","decalage_y"});

	shaderColorInverse = std::make_unique<shaderProgram>();
	shaderColorInverse->init( "colorInverse.vert", "colorInverse.frag");

	// point en haut a gauche , en haut a droite, en bas à gauche, en bas à droite
	float points[8] = {-1.f, 1.f, 1.f, 1.f, -1.f, -1.f, 1.f, -1.f};

	m_viewportGL = std::make_unique<VertexArray>();
	m_viewportGL->registerVertexBuffer(BufferType::POS2D, BufferAccess::STATIC);
	m_viewportGL->fillVertexBuffer(BufferType::POS2D, 8, points);
}

//! Fill with black around the circle
void AppDraw::drawViewportShape()
{
	StateGL::disable(GL_BLEND);
	StateGL::BlendFunc(GL_ONE, GL_ONE);

	shaderViewportShape->use();
	shaderViewportShape->setUniform("radius" , m_radius);
	shaderViewportShape->setUniform("decalage_x" , m_decalage_x);
	shaderViewportShape->setUniform("decalage_y" , m_decalage_y);

	Renderer::drawArrays(shaderViewportShape.get(), m_viewportGL.get(), GL_TRIANGLE_STRIP, 0, 4);
}

void AppDraw::drawColorInverse()
{
	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);

	Renderer::drawArrays(shaderColorInverse.get(), m_viewportGL.get(), GL_TRIANGLE_STRIP, 0, 4);
}