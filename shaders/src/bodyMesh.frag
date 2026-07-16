//
// bodyMesh - new-path MESH fragment (S5/G7). Port of body_normal.frag with
// the Gen-1 eclipse-LUT occluder loop REPLACED by generalized projected
// shadows (shadow-paths.md B4): the receiver projects its eye-space surface
// point into its sun frame (two folded rows - ShadowProjection.hpp) and
// samples each caster's blurred silhouette layer; application is modulated
// per channel by the CASTER's shadowAbsorbtion (Earth {0,1,1} -> red umbra,
// the atmosphere-diffraction emulation [vixy: 2026-07-12]).
// CPU mirror of binding 1: meshFrag (bodyShaderInterface.hpp) - field order
// and types are the GPU-visible layout.
//
#version 450

layout (binding=2) uniform sampler2D mapTexture;
layout (binding=3) uniform sampler2DArray bodyShadows;

#include <receivedShadowsDecl.glsl>

layout (binding=1) uniform meshFrag {
	vec4 shadowRow0;     // xyz = sun-frame x row, w = -dot(x, receiverCenter)
	vec4 shadowRow1;
	int nbShadowingBodies;
	ShadowingBody shadowingBodies[MAX_SHADOW_CASTERS];
};

#include <receivedShadows.glsl>

layout (location=0) in vec2 TexCoord;
layout (location=1) in float Ambient;
layout (location=2) in vec3 Position;
layout (location=3) in float NdotL;
layout (location=0) out vec4 FragColor;

void main(void)
{
	float diffuse = max(NdotL, 0.0);
	vec3 color = texture(mapTexture, TexCoord).rgb;
	vec3 shading = vec3(min(diffuse + Ambient, 1.0));
	if (diffuse != 0.0 && nbShadowingBodies > 0) {
		vec2 shadowPos = vec2(dot(shadowRow0.xyz, Position) + shadowRow0.w,
		                      dot(shadowRow1.xyz, Position) + shadowRow1.w);
		vec3 shadowing = computeReceivedShadowing(shadowPos, Position);
		shading = min(diffuse * shadowing + Ambient, vec3(1.0));
	}
	FragColor = vec4(color * shading, 1.0);
}
