//
// bodyTesNight - MESH_TES NIGHT row (row 2, the my_earth class). Port of
// my_earth.frag: (diffuse + specularMap*pow(max(0,N.H),64)) * day, night
// lights max()-blended in where diffuse <= 0.1. NO ambient (my_earth
// parity - the old Earth shader has none). The Gen-1 eclipse-LUT occluder
// loop is REPLACED by the S5 generalized receive (bodyMesh.frag block);
// the old scalar LUT darkening multiplied daycolor only - night lights
// stay unshadowed, preserved here. Old dead cloud code NOT ported
// (commented out in source since before the fork).
// CPU mirror of binding 1: meshFrag (bodyShaderInterface.hpp).
//
#version 450

layout (binding=3) uniform sampler2D mapTexture;   // DayTexture
layout (binding=4) uniform sampler2D NightTexture;
layout (binding=5) uniform sampler2D SpecularTexture;
layout (binding=8) uniform sampler2DArray bodyShadows;

#include <receivedShadowsDecl.glsl>

layout (binding=1) uniform meshFrag {
	vec4 shadowRow0;
	vec4 shadowRow1;
	int nbShadowingBodies;
	ShadowingBody shadowingBodies[MAX_SHADOW_CASTERS];
};

#include <receivedShadows.glsl>

layout (location=0) in vec3 Position;
layout (location=1) in vec2 TexCoord;
layout (location=2) in vec3 Normal;
layout (location=3) in vec3 Light;
layout (location=5) in vec3 ViewDirection;

layout (location=0) out vec4 FragColor;

void main(void)
{
	vec3 daytime = vec3(texture(mapTexture, TexCoord));
	vec3 night = vec3(texture(NightTexture, TexCoord));
	vec3 reflective = vec3(texture(SpecularTexture, TexCoord));
	vec3 halfangle = normalize(Light + ViewDirection);
	const float specularExp = 64.0;
	float NdotH = dot(Normal, halfangle);
	vec3 specular = reflective * vec3(pow(max(0.0, NdotH), specularExp));
	float NdotL = dot(Normal, Light);
	float diffuse = max(0.0, NdotL);
	vec3 daycolor = (diffuse + specular) * daytime;
	vec3 nightcolor = night;
	vec3 shadowing = vec3(1.0);
	if (diffuse != 0.0 && nbShadowingBodies > 0) {
		vec2 shadowPos = vec2(dot(shadowRow0.xyz, Position) + shadowRow0.w,
		                      dot(shadowRow1.xyz, Position) + shadowRow1.w);
		shadowing = computeReceivedShadowing(shadowPos, Position);
	}
	vec3 color = daycolor * shadowing;
	if (diffuse <= 0.1) {
		nightcolor = nightcolor * smoothstep(0.0, 0.1, 0.1 - diffuse);
		color = max(color, nightcolor);
	}
	FragColor = vec4(color, 1.0);
}
