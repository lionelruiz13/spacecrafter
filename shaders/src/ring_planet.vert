//
// ringplanet
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

#include <fisheye.glsl>

//layout
layout (location=0)in vec2 position;
layout (location=1)in vec2 texCoord;

#include <cam_block.glsl>

//externe
uniform mat4 NormalMatrix;
uniform mat4 ModelViewMatrixInverse;
uniform vec3 clipping_fov;

uniform float PlanetRadius;
uniform vec3 PlanetPosition;
uniform vec3 LightDirection;
uniform float SunnySideUp;
uniform float RingScale;

out float PlanetHalfAngle;
out float Separation;
out float SeparationAngle;
out float NdotL;

//out
smooth out vec2 TexCoord;


void main()
{
	vec3 Position = vec3(position, 0);
	TexCoord = texCoord;
	PlanetHalfAngle = atan(PlanetRadius/distance(PlanetPosition, Position));
	Separation = dot(LightDirection, normalize(PlanetPosition-Position));
	SeparationAngle = acos(Separation);

	vec3 modelLight = vec3(ModelViewMatrixInverse * vec4(LightDirection,1.0));

	NdotL = clamp(16.0*dot(vec3(0.0, 0.0, 1.0-2.0*SunnySideUp), modelLight), -1.0, 1.0);

	gl_Position = fisheyeProject( Position*RingScale, clipping_fov);
}
