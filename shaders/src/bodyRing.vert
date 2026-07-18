//
// bodyRing.vert - new-path RING color row (row 4, 2026-07-18). Port of
// ring_planet.vert with the analytic planet-shadow inputs REMOVED: shadow
// testing moved to the generalized receive path (bodyRing.frag,
// receivedShadows.glsl include), so PlanetHalfAngle/SeparationAngle die.
// Separation stays - it feeds the planet-shine `reflected` term (lighting,
// not shadow). Eye-space Position exported for the receive projection
// (disc-receiver idiom: entries consumed unfolded in eye space).
// CPU mirror of binding 0: ringVert (bodyShaderInterface.hpp).
//
// Divergence vs old (named): the eye-space Position now includes RingScale -
// the old vert scaled only the projected output, so lighting/shadow geometry
// ignored body scaling (benign at scale 1, inconsistent under `body_scale`);
// identical at scale 1 by construction.
#version 450

layout (binding=0, set=0) uniform ringVert {
	mat4 ModelViewMatrix;
	mat4 ModelViewMatrixInverse;
	vec3 clipping_fov;
	float RingScale;
	vec3 PlanetPosition;
	float SunnySideUp;
	vec3 LightDirection;
	float fadingFactor;
};

#include <custom_project.glsl>

layout (location=0) in vec3 Position3D; // R32G32 input - component 2 reads 0 (Vulkan default fill)
layout (location=1) in vec2 texCoord;   // R32 input - component 1 reads 0

layout (location=0) out vec2 TexCoord;
layout (location=1) out vec3 Position;  // eye space, receive projection input
layout (location=2) out float Separation;
layout (location=3) out float NdotL;
layout (location=4) out float fading;

void main()
{
	Position = vec3(ModelViewMatrix * vec4(Position3D.xy * RingScale, 0, 1));
	TexCoord = texCoord;
	// Planet-shine input: cos of the angle between the sun direction and the
	// ring-point->planet direction (ring_planet.vert verbatim).
	Separation = dot(LightDirection, normalize(PlanetPosition - Position));

	vec3 modelLight = vec3(ModelViewMatrixInverse * vec4(LightDirection, 1.0));
	NdotL = clamp(16.0 * dot(vec3(0.0, 0.0, 1.0 - 2.0 * SunnySideUp), modelLight), -1.0, 1.0);

	vec4 outPos = custom_project(Position3D * RingScale, ModelViewMatrix, clipping_fov);
	// fading depends on how close we are, thus on z value (asteroid-ring
	// cross-fade; without the row-5 asteroid variant fadingFactor is the old
	// else-branch constant 100000 -> fading == 1 everywhere visible).
	fading = min(1, (outPos.z * (clipping_fov[1] - clipping_fov[0]) + clipping_fov[0]) * clipping_fov[2] * fadingFactor);
	gl_Position = outPos;
}
