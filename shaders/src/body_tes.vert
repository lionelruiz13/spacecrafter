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

#include <fisheye_noMV.glsl>

layout(location=0) out vec3 PositionOut;
layout(location=1) out vec2 TexCoord;
layout(location=2) out vec3 Normal;
layout(location=3) out vec2 glPosition;
layout(location=4) out int Visible;

void main()
{
	vec3 Position;
	Position.x =position.x * planetScaledRadius;
	Position.y =position.y * planetScaledRadius;
	Position.z =position.z * planetScaledRadius * planetOneMinusOblateness;

	PositionOut = Position;
	glPosition = vec2(fisheyeProject(Position, clipping_fov));
	Visible = int(glPosition.x * glPosition.x + glPosition.y * glPosition.y <= 2.);
	TexCoord = texcoord;
	Normal = normal;
}
