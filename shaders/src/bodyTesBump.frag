//
// bodyTesBump - MESH_TES BUMP row (row 2, the my_moon class: Moon,
// Iapetus). Port of my_moon.frag: tangent-space normal-map diffuse +
// cam_block ambient. The Gen-1 LUT + additive UmbraColor floor are REPLACED
// by the S5 generalized receive: old diffuse*(s + U*(1-s)) equals new
// diffuse*(1 - cov*a) exactly under a = 1-UmbraColor (shadow-paths.md D2,
// measured at S5) - the umbra tint now rides the CASTER's shadowAbsorbtion.
// CPU mirror of binding 1: meshFrag (bodyShaderInterface.hpp).
//
#version 450

layout (binding=3) uniform sampler2D mapTexture;
layout (binding=6) uniform sampler2D normalTexture;
layout (binding=8) uniform sampler2DArray bodyShadows;

#include <cam_block.glsl>

#include <receivedShadowsDecl.glsl>

layout (binding=1) uniform meshFrag {
	int nbShadowingBodies;
	ShadowingBody shadowingBodies[MAX_SHADOW_CASTERS];
};

#include <receivedShadows.glsl>

layout (location=0) in vec3 Position;
layout (location=1) in vec2 TexCoord;
layout (location=4) in vec3 TangentLight;

layout (location=0) out vec4 FragColor;

void main(void)
{
	vec4 color = texture(mapTexture, TexCoord);
	vec3 light_b = normalize(TangentLight);
	vec3 normal_b = 2.0 * vec3(texture(normalTexture, TexCoord)) - vec3(1.0);
	float diffuse = max(dot(normal_b, light_b), 0.0);
	vec3 shadowing = vec3(1.0);
	if (diffuse != 0.0 && nbShadowingBodies > 0) {
		shadowing = computeReceivedShadowing(Position);
	}
	FragColor = vec4(color.rgb * min(diffuse * shadowing + ambient, 1.0), color.a);
}
