//
// bodyTesMesh - MESH_TES base row (row 2). Port of body_normal_tes.frag:
// plain diffuse + cam_block ambient on the tessellated, heightmap-displaced
// sphere, with the Gen-1 eclipse-LUT occluder loop REPLACED by the S5
// generalized projected-shadow receive (bodyMesh.frag block, verbatim).
// CPU mirror of binding 1: meshFrag (bodyShaderInterface.hpp).
//
#version 450

layout (binding=3) uniform sampler2D mapTexture;
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
layout (location=6) in float NdotL;

layout (location=0) out vec4 FragColor;

void main(void)
{
	float diffuse = max(NdotL, 0.0);
	vec3 color = texture(mapTexture, TexCoord).rgb;
	vec3 shading = vec3(min(diffuse + ambient, 1.0));
	if (diffuse != 0.0 && nbShadowingBodies > 0) {
		vec3 shadowing = computeReceivedShadowing(Position);
		shading = min(diffuse * shadowing + ambient, vec3(1.0));
	}
	FragColor = vec4(color * shading, 1.0);
}
