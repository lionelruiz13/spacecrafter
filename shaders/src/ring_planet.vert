//
// ringplanet
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (binding=0, set=0) uniform ubo {
	mat4 ModelViewMatrix;
	mat4 ModelViewMatrixInverse;
	vec3 clipping_fov;
	float RingScale;
	vec3 PlanetPosition;
	float PlanetRadius;
	vec3 LightDirection;
	float SunnySideUp;
	float fadingFactor;
};

#include <fisheye_noMV.glsl>

//layout
layout (location=0)in vec3 Position3D; // Missing component 2 is set to 0 (see Vulkan spec)
layout (location=1)in vec2 texCoord;

layout (location=1) out float PlanetHalfAngle;
layout (location=2) out float Separation;
layout (location=3) out float SeparationAngle;
layout (location=4) out float NdotL;
layout (location=5) out float fading;

//out
layout (location=0) out vec2 TexCoord;


void main()
{
	vec3 Position = vec3(ModelViewMatrix * vec4(Position3D.x, Position3D.y, 0, 1));
	TexCoord = texCoord;
	PlanetHalfAngle = atan(PlanetRadius/distance(PlanetPosition, Position));
	Separation = dot(LightDirection, normalize(PlanetPosition-Position));
	SeparationAngle = acos(Separation);

	vec3 modelLight = vec3(ModelViewMatrixInverse * vec4(LightDirection,1.0));

	NdotL = clamp(16.0*dot(vec3(0.0, 0.0, 1.0-2.0*SunnySideUp), modelLight), -1.0, 1.0);
	
	vec4 outPos = fisheyeProject(Position3D*RingScale, clipping_fov);
	// fading depend on how close we are, thus on z value
	fading = min(1, (outPos.z * (clipping_fov[1] - clipping_fov[0]) + clipping_fov[0]) * clipping_fov[2] * fadingFactor);
	gl_Position = outPos;
}
