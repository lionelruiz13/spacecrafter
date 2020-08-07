/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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

#ifndef _FOG_HPP_
#define _FOG_HPP_

#include <string>
#include <vector>
#include <memory>

#include "tools/vecmath.hpp"
#include "tools/fader.hpp"
#include "tools/utility.hpp"
#include "renderGL/stateGL.hpp"
#include "tools/no_copy.hpp"
#include "tools/ScModule.hpp"

class s_texture;
class Navigator;
class Projector;
class VertexArray;
class shaderProgram;

class Fog : public NoCopy, public ModuleFader<LinearFader> {
public:
	Fog(float _radius);
	~Fog();

	void update(int delta_time) {
		fader.update(delta_time);
	}

	void setAltAngle(float _value) {
		alt_angle= _value;
	}

	void setAngleShift(float _value) {
		angle_shift = _value;
	}

	void setSkyBrightness(float b) {
		sky_brightness = b;
	}

	static void createSC_context();

	void draw(const Projector* prj, const Navigator* nav) const;

	void initShader();

private:

	void createFogMesh(GLdouble radius, GLdouble height, GLint slices, GLint stacks, std::vector<float>* dataTex, std::vector<float>* dataPos);

	static std::unique_ptr<shaderProgram> shaderFog;
	std::unique_ptr<VertexArray> m_fogGL;
	unsigned int nbVertex;			//nombre de vertex pour le fog
	static s_texture* fog_tex;			// allways the same
	float radius;
	float alt_angle;
	float angle_shift;
	float sky_brightness;
};


#endif // _LANDSCAPE_H_
