/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014 of the LSS Team & Association Sirius
 * Copyright (C) 2017 Immersive Adventure
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

#ifndef _BODY_SHADER_HPP_
#define _BODY_SHADER_HPP_

#pragma once


#include <list>
#include <string>
#include "tools/shader.hpp"


enum SHADER_USE {SHADER_SUN = 0, SHADER_NORMAL = 1,  SHADER_NORMAL_TES = 11,  SHADER_BUMP = 2, SHADER_NIGHT = 3,SHADER_NIGHT_TES = 31,  SHADER_RINGED = 4, SHADER_MODEL3D = 5, SHADER_MOON_NORMAL = 6, SHADER_MOON_BUMP = 7, SHADER_ARTIFICIAL = 8};

/*struct bodyShaderStatus {
	bool map;
	bool night;
	bool norm;
	bool ring;
};*/

class BodyShader {

public:
	BodyShader() {};
	~BodyShader() {};

	static void createShader();
	static void deleteShader();

	static shaderProgram * getShaderBump() {
		return shaderBump;
	};

	static shaderProgram * getShaderNight() {
		return shaderNight;
	};

	static shaderProgram * getShaderNightTes() {
		return shaderNightTes;
	};

	static shaderProgram * getShaderRinged() {
		return shaderRinged;
	};

	static shaderProgram * getShaderNormal() {
		return shaderNormal;
	};

	static shaderProgram * getShaderNormalTes() {
		return shaderNormalTes;
	};

	static shaderProgram * getShaderMoonNormal() {
		return shaderMoonNormal;
	};

	static shaderProgram * getShaderMoonBump() {
		return shaderMoonBump;
	};

	static shaderProgram * getShaderArtificial() {
		return shaderArtificial;
	};

protected:
	static shaderProgram *shaderBump;
	static shaderProgram *shaderNight;
	static shaderProgram *shaderNightTes;
	static shaderProgram *shaderRinged;
	static shaderProgram *shaderNormal;
	static shaderProgram *shaderNormalTes;
	static shaderProgram *shaderMoonNormal;
	static shaderProgram *shaderMoonBump;
	static shaderProgram *shaderArtificial;
};

#endif // _BODY_SHADER_HPP_



