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

#include <memory>
#include "tools/no_copy.hpp"

class VertexArray;
class VertexBuffer;
class PipelineLayout;
class Pipeline;

/**
* \file screenFader.hpp
* \brief Allows the screen to go black and vice versa
* \author Olivier NIVOIX
* \version 1
*
* \class ScreenFader
*
* \brief Allows the screen to go black and vice versa
*
* intensity indicates the quantity of black to display on the screen
*
*
*/

class ScreenFader : public NoCopy {
public:
	ScreenFader();
	~ScreenFader();
	void draw();
	void update(int delta_time);


	//! sets the intensity to a.
	void setIntensity(float a) {
		intensity = a;
	}

	//! fade up:
	//! As value gets closer to max, the intensity increases
	//! If value < min, the intensity is zero
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

	//! fades down:
	//! when value approaches min, the intensity increases
	//! when value approaches max, the intensity decreases
	//! if value > max, the intensity is zero
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

	//! increases the intensity of a
	void upGrade(float a) {
		intensity += a;
		if (intensity >1.0)
			intensity = 1.0;
	}

	//! decreases the intensity of a
	void downGrade(float a) {
		intensity -= a;
		if (intensity <0.0)
			intensity = 0.0;
	}

	void createSC_context();

	// change gradually to a new intensity
	void changeIntensity(float _intensity, double duration)
	{
		flag_change_intensity = 1;

		start_value = intensity;
		end_value = _intensity;

		move_to_coef = 1.0f/(int)(duration*1000);
		move_to_mult = 0;
	}

	void initShader();
private:
	//determines the intensity of the veil on the screen
	float intensity = 0.0;
	bool flag_change_intensity = 0;
	double start_value, end_value;
	float move_to_coef, move_to_mult;
	int cmds[3] {-1, -1, -1};
	std::unique_ptr<VertexArray> m_screenGL;
	std::unique_ptr<VertexBuffer> vertex;
	std::unique_ptr<PipelineLayout> layout;
	std::unique_ptr<Pipeline> pipeline;

	// openGL parameters
	void initShaderParams();
};


#endif //SCREENFADER
