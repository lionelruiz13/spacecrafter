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

layout(location=0) out VS_OUT{
	vec3 glPosition;
    vec2 TexCoord;
    vec3 Normal;
} vs_out;


void main()
{
	vec3 Position;
	Position.x =position.x * planetScaledRadius;
	Position.y =position.y * planetScaledRadius;
	Position.z =position.z * planetScaledRadius * planetOneMinusOblateness;

	vs_out.glPosition = Position;
	vs_out.TexCoord = texcoord;
	vs_out.Normal = normal;
}
