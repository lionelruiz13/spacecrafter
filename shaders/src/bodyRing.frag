//
// bodyRing.frag - new-path RING color row (row 4, 2026-07-18). Port of
// ring_planet.frag with the analytic planet-shadow test
// ((SeparationAngle < PlanetHalfAngle) ? 0 : ...) REPLACED by the
// generalized receive path: the planet's own G1 entry (within-body pair,
// ModularSystem::computeShadows) carries what the angular test carried, plus
// every other caster in the light corridor (moons onto the ring - a case no
// old path had). First non-disc, non-model receiver family.
//
// Composition parity: the old test zeroed ONLY diffuse - `reflected`
// (planet-shine) survived inside the planet's shadow. Kept exactly:
// shadowing applies to the diffuse term alone.
// CPU mirror of binding 1: ringFrag (bodyShaderInterface.hpp).
//
#version 450

layout (binding=2) uniform sampler2D Texture;
layout (binding=3) uniform sampler2DArray bodyShadows;

#include <receivedShadowsDecl.glsl>

layout (binding=1) uniform ringFrag {
	int nbShadowingBodies;
	ShadowingBody shadowingBodies[MAX_SHADOW_CASTERS];
};

#include <receivedShadows.glsl>

layout (location=0) in vec2 TexCoord;
layout (location=1) in vec3 Position;
layout (location=2) in float Separation;
layout (location=3) in float NdotL;
layout (location=4) in float fading;

layout (location=0) out vec4 Color;

void main(void)
{
	vec4 color = vec4(texture(Texture, TexCoord));
	float diffuse = clamp(max(NdotL, -NdotL * 0.2), 0.0, 1.0);
	float reflected = 0.3 * max(-Separation, 0.0);
	vec3 shadowing = (nbShadowingBodies > 0) ? computeReceivedShadowing(Position) : vec3(1.0);
	Color = vec4(color.rgb * (diffuse * shadowing + reflected), color.a * fading);
}
