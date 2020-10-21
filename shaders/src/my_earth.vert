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

#include <cam_block.glsl>

layout (binding=2) uniform globalVertGeom {
	float planetScaledRadius;
	float planetOneMinusOblateness;
};

layout(location=0) out vec3 glPosition;
layout(location=1) out vec2 TexCoord;
layout(location=2) out vec3 Normal;


void main()
{
	/*
	vec3 Position;
	Position.x =position.x * planetScaledRadius;
	Position.y =position.y * planetScaledRadius;
	Position.z =position.z * planetScaledRadius * planetOneMinusOblateness;

	glPosition = Position;
	/*/
	glPosition = position;
	//*/
	TexCoord = texcoord;
	Normal = normal;
}
