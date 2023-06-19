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
	float rq = pos.x*pos.x + pos.y*pos.y;
	float depth = sqrt(rq + pos.z*pos.z);
	rq = sqrt(rq);
	float f = asin(min(rq/depth, 1)); // min patch a driver bug were rq/depth > 1
	if (pos.z > 0)
		f = M_PI - f;

	// shadowViewDirection = ShadowMatrix * worldToNormalMatrix;

	viewDirection = normalize(WorldToModelMatrix * pos.xyz);
	// float tmp = sqrt(1 - normal.z);
	// float tmp2 = normal.z / tmp;
	// dir = mat3(
	// 	normal.y, normal.x*tmp2, normal.x,
	// 	-normal.x, normal.y*tmp2, normal.y,
	// 	0, tmp, normal.z
	// ) * viewDirection;
	// viewTexRay = dir.xy / (heightmapDepth * max(0.01, dir.z));
	// outLightDirection = normalize(worldToNormalMatrix * lightDirection);
	// outNormal = normal;

	f /= rq * fov;
	// depth = (depth - zNear) / zRange;
	side = mix(0.5, -0.5, (texcoord < 0.5));
	gl_Position = vec4(pos.x*f, pos.y*f, 0, 1);
}
