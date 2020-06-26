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
#include "tools/OpenGL.hpp"
#include "tools/stateGL.hpp"
#include "tools/shader.hpp"

ScreenFader::ScreenFader()
{
	intensity = 0.0;
	move_to_mult = 0;
	start_value = 0;
	end_value = 0;
	move_to_mult = 0;
}

ScreenFader::~ScreenFader()
{
}

void ScreenFader::createGL_context()
{
	shaderScreen = std::make_unique<shaderProgram>();
	shaderScreen->init( "screenFader.vert","screenFader.frag");
	shaderScreen->setUniformLocation("intensity");

	// point en haut a gauche
	// point en haut a droite
	// point en bas à gauche
	// point en bas à droite
	float points[8] = {-1.f, 1.f, 1.f, 1.f, -1.f, -1.f, 1.f, -1.f};

	m_screenGL = std::make_unique<VertexArray>();
	m_screenGL->registerVertexBuffer(BufferType::POS2D, BufferAccess::STATIC);
	m_screenGL->fillVertexBuffer(BufferType::POS2D, 8, points);
}

void ScreenFader::update(int delta_time)
{
	if (flag_change_intensity) {
		move_to_mult += move_to_coef*delta_time;

		if ( move_to_mult >= 1) {
			move_to_mult = 1;
			flag_change_intensity = 0;
		}
		intensity = start_value - move_to_mult*(start_value-end_value);
	}
}
void ScreenFader::draw()
{
	if (intensity==0)
		return;

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	shaderScreen->use();
	shaderScreen->setUniform("intensity" , intensity);

	m_screenGL->bind();
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	m_screenGL->unBind();
	shaderScreen->unuse();
}
