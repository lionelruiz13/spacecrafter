//
// my earth tessellation
//
#version 430
#pragma debug(on)
#pragma optimize(off)

// Specialized for sphere
layout (location=0) in vec3 position;
layout (location=1) in float texcoord;
// layout (location=2) in vec3 normal;

layout (binding=0) uniform globalProj {
	mat4 ModelViewMatrix;
	// mat3 ShadowMatrix;
	mat3 WorldToModelMatrix;
	// vec3 lightDirection; // Light direction in world coordinates
	float zNear;
	float zRange;
	float fov;
	float radius;
	// float heightmapDepth;
};

#include <custom_project.glsl>

layout (location=0) out vec3 entryPos;
layout (location=1) out vec3 viewDirection;
layout (location=2) out float side;

// layout(location=0) out vec3 outNormal; // entry point
// layout(location=0) out vec2 viewTexRay; // View direction in the texture
// layout(location=1) out vec3 outLightDirection; // Light direction in the normal
// layout(location=2) out vec2 TexCoord;
// layout(location=3) out vec3 shadowPos;
// layout(location=4) out vec3 shadowViewDirection;

#define M_PI 3.14159265358979323846

void main()
{
	entryPos = position;
	vec4 pos = ModelViewMatrix * vec4(position * radius, 1);

	viewDirection = normalize(WorldToModelMatrix * pos.xyz);

	// Use custom projection with specialization constant
	vec4 projected = custom_projectNoMV(pos.xyz, vec3(zNear, zNear + zRange, fov));
	side = mix(-1, -0.2, (texcoord < 0.5));
	gl_Position = projected;
}
