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

#ifndef APP_DRAW_HPP
#define APP_DRAW_HPP

#include <SDL2/SDL_thread.h>
#include <memory>
#include <queue>

#include "tools/vecmath.hpp"
#include "tools/no_copy.hpp"
#include "EntityCore/SubBuffer.hpp"

class VertexBuffer;
class VertexArray;
class Set;
class Pipeline;
class PipelineLayout;
class Texture;
/**
@author AssociationSirius
*/

class AppDraw : public NoCopy{

public:
	AppDraw();
	~AppDraw();

	//! Initialize application drawer
	void init(unsigned int _width, unsigned int _height);

	//! draws the splash at startup
    void initSplash();

	//! filled in black the outside of the dome
	void drawViewportShape();
	//! draws the final rendering of the software by inverting the colors
	void drawColorInverse();

	void setLineWidth(float w);
	float getLineWidth() const {
		return m_lineWidth;
	}

	//! Set rendering flag of antialiased lines
	void setFlagAntialiasLines(bool b) {
		antialiasLines = b;
	}
	//! Get display flag of constellation lines
	bool getFlagAntialiasLines(void) {
		return antialiasLines;
	}
	void flipFlagAntialiasLines() {
		setFlagAntialiasLines(!antialiasLines);
	}

	//! creation of shaders
	void createSC_context();
private:
	std::unique_ptr<VertexArray> m_viewportGL;
	std::unique_ptr<PipelineLayout> layoutEmpty;
	std::unique_ptr<Pipeline> pipelineViewportShape, pipelineColorInverse;
	std::unique_ptr<PipelineLayout> layout; // splash screen
	std::unique_ptr<Pipeline> pipeline; // splash screen
	std::unique_ptr<Texture> texture; // splash screen
	SubBuffer staging;
	std::unique_ptr<VertexBuffer> vertexBuffer; // 0-4 colorInverse, 4-16 viewportShape
	// 0-3 viewport shape, 3-6 color inverse
	std::vector<VkCommandBuffer> cmds;

	float m_lineWidth;							//!< openGL line thickness
	bool antialiasLines;						//!< using GL_LINE_SMOOTH
    Uint16 width, height;  						//!< Contains the resolution w and h of the SDL window
	Uint16 m_radius, m_decalage_x, m_decalage_y;	//!< for calculation optimization
};

#endif
