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
#include <queue>

#include "spacecrafter.hpp"
#include "appModule/fps.hpp"
#include "appModule/space_date.hpp"
#include "tools/app_settings.hpp"
#include "tools/shader.hpp"
#include "tools/stateGL.hpp"
#include "appModule/appModule.hpp"


/**
@author AssociationSirius
*/

class AppDraw : public AppModule{

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
		m_lineWidth = w;
	}
	float getLineWidth() const {
		return m_lineWidth;
	}

	//! création des shaders
	void createShader();
private:
	//! suppression des shaders
	void deleteShader();

	DataGL layer;
	shaderProgram* shaderViewportShape = nullptr;
	shaderProgram* shaderColorInverse = nullptr;
	DataGL dataGL;

	float m_lineWidth;							//!< épaisseur du tracé des lignes openGL

    Uint16 width, height;  						//! Contient la résolution w et h de la fenetre SDL
};

#endif
