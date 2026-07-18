//
// bodyLayeredNight - MESH_LAYERED NIGHT row (row 2, the body_night class:
// Io). Port of body_night.frag: diffuse * day, night lights max()-blended
// where diffuse <= 0.1. NO specular (commented out in the old source - not
// ported, parity) and NO ambient (body_night has none). Gen-1 LUT loop
// REPLACED by the S5 generalized receive; the old scalar darkening
// multiplied daycolor only - night lights stay unshadowed, preserved.
// CPU mirror of binding 1: meshFrag (bodyShaderInterface.hpp).
//
#version 450

layout (binding=2) uniform sampler2D mapTexture;   // DayTexture
layout (binding=3) uniform sampler2D NightTexture;
layout (binding=5) uniform sampler2DArray bodyShadows;

#include <receivedShadowsDecl.glsl>

layout (binding=1) uniform meshFrag {
	int nbShadowingBodies;
	ShadowingBody shadowingBodies[MAX_SHADOW_CASTERS];
};

#include <receivedShadows.glsl>

layout (location=0) in vec2 TexCoord;
layout (location=1) in vec3 Normal;
layout (location=2) in vec3 Position;
layout (location=3) in vec3 TangentLight;
layout (location=4) in vec3 Light;
layout (location=5) in vec3 ViewDirection;
// (full body_night.vert interface declared - unconsumed inputs are legal and
//  keep the pipeline free of OutputNotConsumed validation warnings; the old
//  frags did the same)

layout (location=0) out vec4 FragColor;

void main(void)
{
	vec3 daytime = vec3(texture(mapTexture, TexCoord));
	vec3 night = vec3(texture(NightTexture, TexCoord));
	float NdotL = dot(Normal, Light);
	float diffuse = max(0.0, NdotL);
	vec3 daycolor = diffuse * daytime;
	vec3 nightcolor = night;
	vec3 shadowing = vec3(1.0);
	if (diffuse != 0.0 && nbShadowingBodies > 0) {
		shadowing = computeReceivedShadowing(Position);
	}
	vec3 color = daycolor * shadowing;
	if (diffuse <= 0.1) {
		nightcolor = nightcolor * smoothstep(0.0, 0.1, 0.1 - diffuse);
		color = max(color, nightcolor);
	}
	FragColor = vec4(color, 1.0);
}
