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

#include "spacecrafter.hpp"
#include "appModule/fps.hpp"
#include "appModule/space_date.hpp"
#include "tools/app_settings.hpp"
#include "tools/shader.hpp"
#include "tools/stateGL.hpp"
#include "tools/no_copy.hpp"

class VertexArray;
/**
@author AssociationSirius
*/

class AppDraw : public NoCopy{

public:
	AppDraw();
	~AppDraw();

	//! Initialize application drawer
	void init(unsigned int _width, unsigned int _height);

	//! dessine le splash au démarrage
    void initSplash();

    //! clean screen with black color
	void drawFirstLayer();
	//! rempli en noir l'extérieur du dôme
	void drawViewportShape();
	//! dessine le rendu final du logiciel en inversant les couleurs
	void drawColorInverse();

	void setLineWidth(float w) {
		if (abs(m_lineWidth-w)<0.5f) {
			glLineWidth(m_lineWidth);
		}
		m_lineWidth = w;
	}
	float getLineWidth() const {
		return m_lineWidth;
	}

	//! Set rendering flag of antialiased lines
	void setFlagAntialiasLines(bool b) {
		antialiasLines = b;

		if(b) glEnable(GL_LINE_SMOOTH);
		else glDisable(GL_LINE_SMOOTH);
	}
	//! Get display flag of constellation lines
	bool getFlagAntialiasLines(void) {
		return antialiasLines;
	}
	void flipFlagAntialiasLines() {
		setFlagAntialiasLines(!antialiasLines);
	}

	//! création des shaders
	void createSC_context();
private:
	//! suppression des shaders
	void deleteShader();

	std::unique_ptr<shaderProgram> shaderViewportShape, shaderColorInverse;
	std::unique_ptr<VertexArray> m_viewportGL;

	float m_lineWidth;							//!< épaisseur du tracé des lignes openGL
	bool antialiasLines;						//!< using GL_LINE_SMOOTH
    Uint16 width, height;  						//!< Contient la résolution w et h de la fenetre SDL
	Uint16 m_radius, m_decalage_x, m_decalage_y;	//!< pour optimisation des calculs  
};

#endif
