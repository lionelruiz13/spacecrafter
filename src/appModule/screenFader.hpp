/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2018 Association Sirius
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

#ifndef SCREENFADER_HPP
#define SCREENFADER_HPP

#include "tools/no_copy.hpp"
#include "tools/shader.hpp"
#include "tools/stateGL.hpp"

class VertexArray;

class ScreenFader : public NoCopy {
public:
	ScreenFader();
	~ScreenFader();
	void draw();


	//! fixe l'intensité à a.
	void setIntensity(float a) {
		intensity = a;
	}

	//! opère un fondu vers le haut:
	//! Plus value se rapproche de max, et plus l'intensité augmente
	//! Si value < min, l'intensité est nulle
	void fixIntensityUp(float min, float max, float value) {
		if (value<min) {
			intensity = 0.0;
			//~ return;
		}
		else {
			if (value> max)
				intensity = 1.0;
			else
				intensity = (value - min)/(max-min);
		}
	}

	//! opère un fondu vers le bas:
	//! quand  value se rapproche de min, l'intensité augmente
	//! quand value se rapproche de max, l'intensité diminue
	//! si value > max, l'intensité est nulle
	void fixIntensityDown(float min, float max, float value) {
			if (value> max)
				intensity = 0.0;
			else {
				if (value < min)
					intensity =  1.0;
				else
					intensity = 1.0- (value - min)/(max-min);
			}
	}

	//! augmente l'intensité de a
	void upGrade(float a) {
		intensity += a;
		if (intensity >1.0)
			intensity = 1.0;
	}

	//! diminue l'intensité de a
	void downGrade(float a) {
		intensity -= a;
		if (intensity <0.0)
			intensity = 0.0;
	}

	void initShader();
private:
	//détermine l'intensité du voile sur l'écran
	float intensity =0.0;
	// paramètres openGL
	void initShaderParams();
	//DataGL screen;
	VertexArray* m_screenGL;
	shaderProgram* shaderScreen;
};


#endif //SCREENFADER
