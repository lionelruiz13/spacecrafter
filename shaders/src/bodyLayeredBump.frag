//
// bodyLayeredBump - MESH_LAYERED BUMP row (row 2, the body_bump class:
// Mars mid-range). Port of body_bump.frag: tangent-space normal-map diffuse
// + ambient. Interface ADAPTED to the body_night.vert varyings (the old
// pairing was body_bump.vert, whose Ambient varying = cam_block.ambient -
// read here directly instead). Gen-1 LUT + UmbraColor REPLACED by the S5
// receive (umbra tint = caster shadowAbsorbtion, shadow-paths.md D2).
// CPU mirror of binding 1: meshFrag (bodyShaderInterface.hpp).
//
#version 450

layout (binding=2) uniform sampler2D mapTexture;
layout (binding=4) uniform sampler2D normalTexture;
layout (binding=5) uniform sampler2DArray bodyShadows;

#include <cam_block.glsl>

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
