//
// ringplanet
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (binding=0, set=1) uniform ubo {
	mat4 ModelViewMatrix;
	mat4 ModelViewMatrixInverse;
	vec3 clipping_fov;
	float RingScale;
	vec3 PlanetPosition;
	float PlanetRadius;
	vec3 LightDirection;
	float SunnySideUp;
};

#include <fisheye_noMV.glsl>

//layout
layout (location=0)in vec3 Position; // Missing component 2 is set to 0 (see Vulkan spec)
layout (location=1)in vec2 texCoord;

#include <cam_block_only.glsl>

layout (location=1) out float PlanetHalfAngle;
layout (location=2) out float Separation;
layout (location=3) out float SeparationAngle;
layout (location=4) out float NdotL;

//out
layout (location=0) out vec2 TexCoord;


void main()
{
	TexCoord = texCoord;
	PlanetHalfAngle = atan(PlanetRadius/distance(PlanetPosition, Position));
	Separation = dot(LightDirection, normalize(PlanetPosition-Position));
	SeparationAngle = acos(Separation);

	vec3 modelLight = vec3(ModelViewMatrixInverse * vec4(LightDirection,1.0));

	NdotL = clamp(16.0*dot(vec3(0.0, 0.0, 1.0-2.0*SunnySideUp), modelLight), -1.0, 1.0);

	gl_Position = fisheyeProject( Position*RingScale, clipping_fov);
}
