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
#include <memory>

//
#include "tools/vecmath.hpp"

enum SHADER_USE {
	SHADER_SUN = 0,
	SHADER_NORMAL = 1,
	SHADER_NORMAL_TES = 11,
	SHADER_BUMP = 2,
	SHADER_NIGHT = 3,
	SHADER_NIGHT_TES = 31,
	SHADER_RINGED = 4,
	SHADER_MODEL3D = 5,
	SHADER_MOON_NORMAL = 6,
	SHADER_MOON_NORMAL_TES = 61,
	SHADER_MOON_BUMP = 7,
	SHADER_MOON_NIGHT=32,
	SHADER_ARTIFICIAL = 8,
	SHADER_NIGHT_TES_SHADOW = 30,
	SHADER_TES_SHADOW = 12,
	SHADER_SHADOW = 9,
	SHADER_ARTIFICIAL_SHADOW = 10,
	SHADER_UNDEFINED = 127
};

/*struct bodyShaderStatus {
	bool map;
	bool night;
	bool norm;
	bool ring;
};*/
class VertexArray;
class PipelineLayout;
class Pipeline;
class Set;

struct drawState_t {
	PipelineLayout *layout;
	PipelineLayout *layoutShadow = nullptr;
	Pipeline *pipeline;
	Pipeline *pipelineNoDepth = nullptr;
	Pipeline *pipelineShadow = nullptr;
};

class BodyShader {

public:
	BodyShader() {};
	~BodyShader() {};

	static void createShader();

	static drawState_t *getShaderBump() {
		return &shaderBump;
	};

	static drawState_t *getShaderNight() {
		return &shaderNight;
	};

	static drawState_t *getShaderNightTes() {
		return &myEarth;
	};

	static drawState_t *getShaderNightTesShadowed() {
		return &myEarthShadowed;
	};

	static drawState_t *getShaderRinged() {
		return &shaderRinged;
	};

	static drawState_t *getShaderNormal() {
		return &shaderNormal;
	};

	static drawState_t *getShaderNormalTes() {
		return &shaderNormalTes;
	};

	static drawState_t *getShaderMoonNormalTes() {
		return &myMoon;
	};

	static drawState_t *getShaderTesShadowed() {
		return &shaderShadowedTes;
	};

	static drawState_t *getShaderArtificial() {
		return &shaderArtificial;
	};

	static drawState_t *getShaderArtificialShadowed() {
		return &shaderArtificialShadowed;
	};

	// Used when a global depth shape is required
	static drawState_t *getShaderDepthTrace() {
		return &depthTrace;
	}

	// Used when a depth trace without fisheye projection is required
	static drawState_t *getShaderShadowTrace() {
		return &shadowTrace;
	}

	// Used when a stencil shadow shape (without fisheye projection) is required
	static drawState_t *getShaderShadowShape() {
		return &shadowShape;
	}
protected:
	static drawState_t shaderBump;
	static drawState_t shaderNight; //, shaderMoonNight;
	static drawState_t myEarth, shaderNormal, shaderNormalTes;
	static drawState_t shaderRinged;
	static drawState_t myMoon; //, shaderMoonBump, shaderMoonNormal;
	static drawState_t myEarthShadowed; //, shaderMoonBump, shaderMoonNormal;
	static drawState_t shaderArtificial;
	static drawState_t shaderArtificialShadowed;
	static drawState_t shaderShadowedTes;
	static drawState_t depthTrace;
	static drawState_t shadowTrace;
	static drawState_t shadowShape;
};

typedef Mat4f mat4;
typedef Vec3f vec3;
typedef Vec2f vec2;
typedef Vec3i ivec3;

struct depthTraceInfo {
	mat4 ModelViewMatrix;
	vec3 clipping_fov;
	float planetScaledRadius;
	float planetOneMinusOblateness;
};

typedef struct {
	mat4 ModelViewMatrix;
	mat4 NormalMatrix;
	vec3 clipping_fov;
	float planetRadius;
	vec3 LightPosition;
	float planetScaledRadius;
	float planetOneMinusOblateness;
} globalVertProj;

typedef struct {
	mat4 ModelViewMatrix;
	mat4 NormalMatrix;
	vec3 clipping_fov;
	float planetRadius;
	vec3 LightPosition;
} globalProj;

typedef struct {
	float planetScaledRadius;
	float planetOneMinusOblateness;
} globalVertGeom;

typedef struct {
	ivec3 TesParam;
} globalTescGeom;

typedef struct {
	mat4 ViewProjection;
	mat4 Model;
} globalTesc;

typedef struct {
	vec3 MoonPosition1;
	float MoonRadius1;
	vec3 MoonPosition2;
	float MoonRadius2;
	vec3 MoonPosition3;
	float MoonRadius3;
	vec3 MoonPosition4;
	float MoonRadius4;
	float SunHalfAngle;
} globalFrag;

typedef struct {
	float RingInnerRadius;
	float RingOuterRadius;
} ringFrag;

typedef struct {
	vec3 MoonPosition1;
	float MoonRadius1;
	vec3 UmbraColor;
	float SunHalfAngle;
} moonFrag;

typedef struct {
	mat4 ModelViewMatrix;
	vec3 clipping_fov;
} artGeom;

typedef struct {
	float normal[12];
	float radius;
} artVert;

typedef struct {
	vec3 Position;	// Light position in eye coords.
	float fixAlignment; // fix alignment
    vec3 Intensity;	// A,D,S intensity
} LightInfo;

struct ShadowVert {
	mat4 ModelViewMatrix;
	// mat3 ShadowMatrix;
	float WorldToModelMatrix[12];
	// vec3 lightDirection; // Light direction in world coordinates
	float zNear;
	float zRange;
	float fov;
	float radius;
};

struct vec3p {
	vec3 value;
	float padding;
	void operator=(const vec3 &v) {
		value = v;
	}
};

struct UShadowingBody {
	vec3 posRadius;
	int idx;
};

struct ShadowFrag {
	float ShadowMatrix[12];
	vec3 lightDirection; // In body-local coordinates
	float sinSunAngle;
	float heightMapDepthLevel; // 0.9
	float heightMapDepth; // 0.1
	float squaredHeightMapDepthLevel; // 0.81
	float sunDeviation;
	float atmDeviation;
	vec3 _padding;
	vec3 atmColor;
	int nbShadowingBodies;
	UShadowingBody shadowingBodies[];
};

struct OjmShadowFrag {
	float ShadowMatrix[12];
	mat4 ModelMatrix;
	vec3p lightDirection;
	vec3 lightIntensity;
	int nbShadowingBodies;
	UShadowingBody shadowingBodies[];
};

#endif // _BODY_SHADER_HPP_
