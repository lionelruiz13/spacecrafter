//
// bodyLayeredDay - MESH_LAYERED base row (row 2). Base-row obligation of
// the family (shader table entry 0); no live body class routes here (plain
// bodies stay on the MESH family). Diffuse computed in-fragment from the
// body_night.vert varying interface + cam_block ambient + S5 receive -
// composition identical to bodyMesh.frag.
// CPU mirror of binding 1: meshFrag (bodyShaderInterface.hpp).
//
#version 450

layout (binding=2) uniform sampler2D mapTexture;
layout (binding=5) uniform sampler2DArray bodyShadows;

#include <cam_block.glsl>

#include <receivedShadowsDecl.glsl>

layout (binding=1) uniform meshFrag {
	vec4 shadowRow0;
	vec4 shadowRow1;
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
	float diffuse = max(dot(Normal, Light), 0.0);
	vec3 color = texture(mapTexture, TexCoord).rgb;
	vec3 shading = vec3(min(diffuse + ambient, 1.0));
	if (diffuse != 0.0 && nbShadowingBodies > 0) {
		vec2 shadowPos = vec2(dot(shadowRow0.xyz, Position) + shadowRow0.w,
		                      dot(shadowRow1.xyz, Position) + shadowRow1.w);
		vec3 shadowing = computeReceivedShadowing(shadowPos, Position);
		shading = min(diffuse * shadowing + ambient, vec3(1.0));
	}
	FragColor = vec4(color * shading, 1.0);
}
