//
// my earth tessellation
//
#version 430
#pragma debug(on)
#pragma optimize(off)

//layout
layout (location=0)in vec3 position;
layout (location=1)in vec2 texcoord;
layout (location=2)in vec3 normal;

layout (binding=0) uniform globalProj {
	mat4 ModelViewMatrix;
	mat4 NormalMatrix;
	vec3 clipping_fov;
	float planetRadius;
	vec3 LightPosition;
	float planetScaledRadius;
	float planetOneMinusOblateness;
};

#include <cam_block.glsl>
#include <fisheye2D.glsl>

layout(location=0) out vec3 PositionOut;
layout(location=1) out vec2 TexCoord;
layout(location=2) out vec3 Normal;
layout(location=3) out vec4 glPosition;

layout (binding=2) uniform globalTescGeom {
	uniform ivec3 TesParam;         // [min_tes_lvl, max_tes_lvl, coeff_altimetry]
};

void main()
{
	vec3 Position = position;
	Position.z *= planetOneMinusOblateness;

	PositionOut = Position;
	Position *= planetScaledRadius;
	glPosition = vec4(vec2(fisheye2D(vec4(Position, 1), clipping_fov[2])),
		vec2(fisheye2D(vec4(Position * (1. + TesParam[2]), 1), clipping_fov[2])));
	TexCoord = texcoord;
	Normal = normal;
}
