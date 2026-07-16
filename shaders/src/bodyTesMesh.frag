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

#define MAX_SHADOW_CASTERS 8

struct ShadowingBody {
	vec4 posRadius;      // xy = caster center in sun-frame (rel. receiver), z = disc radius
	vec4 absorbtionIdx;  // rgb = caster shadow absorption, w = layer index
	vec4 clip;           // half-space gate: apply iff dot(P, xyz) + w <= 0 ((0,0,0,-1) = always; planar casters)
};

layout (binding=1) uniform meshFrag {
	vec4 shadowRow0;     // xyz = sun-frame x row, w = -dot(x, receiverCenter)
	vec4 shadowRow1;
	int nbShadowingBodies;
	ShadowingBody shadowingBodies[MAX_SHADOW_CASTERS];
};

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
		vec2 shadowPos = vec2(dot(shadowRow0.xyz, Position) + shadowRow0.w,
		                      dot(shadowRow1.xyz, Position) + shadowRow1.w);
		vec3 shadowing = vec3(1.0);
		for (int i = 0; i < nbShadowingBodies; ++i) {
			vec2 tmp = (shadowPos - shadowingBodies[i].posRadius.xy) / shadowingBodies[i].posRadius.z;
			if (dot(tmp, tmp) < 1.0 && dot(Position, shadowingBodies[i].clip.xyz) + shadowingBodies[i].clip.w <= 0.0) {
				float coverage = textureLod(bodyShadows, vec3(tmp * 0.5 + 0.5, shadowingBodies[i].absorbtionIdx.w), 0.0).r; // explicit LOD: implicit derivatives are UNDEFINED in this non-uniform flow (zero reads on NVIDIA; layer is single-mip)
				shadowing *= vec3(1.0) - coverage * shadowingBodies[i].absorbtionIdx.rgb;
			}
		}
		shading = min(diffuse * shadowing + ambient, vec3(1.0));
	}
	FragColor = vec4(color * shading, 1.0);
}
