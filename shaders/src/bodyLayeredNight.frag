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

#define MAX_SHADOW_CASTERS 8

struct ShadowingBody {
	vec4 posRadius;
	vec4 absorbtionIdx;
	vec4 clip;           // half-space gate: apply iff dot(P, xyz) + w <= 0 ((0,0,0,-1) = always; planar casters)
};

layout (binding=1) uniform meshFrag {
	vec4 shadowRow0;
	vec4 shadowRow1;
	int nbShadowingBodies;
	ShadowingBody shadowingBodies[MAX_SHADOW_CASTERS];
};

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
		vec2 shadowPos = vec2(dot(shadowRow0.xyz, Position) + shadowRow0.w,
		                      dot(shadowRow1.xyz, Position) + shadowRow1.w);
		for (int i = 0; i < nbShadowingBodies; ++i) {
			vec2 tmp = (shadowPos - shadowingBodies[i].posRadius.xy) / shadowingBodies[i].posRadius.z;
			if (dot(tmp, tmp) < 1.0 && dot(Position, shadowingBodies[i].clip.xyz) + shadowingBodies[i].clip.w <= 0.0) {
				float coverage = textureLod(bodyShadows, vec3(tmp * 0.5 + 0.5, shadowingBodies[i].absorbtionIdx.w), 0.0).r; // explicit LOD: implicit derivatives are UNDEFINED in this non-uniform flow (zero reads on NVIDIA; layer is single-mip)
				shadowing *= vec3(1.0) - coverage * shadowingBodies[i].absorbtionIdx.rgb;
			}
		}
	}
	vec3 color = daycolor * shadowing;
	if (diffuse <= 0.1) {
		nightcolor = nightcolor * smoothstep(0.0, 0.1, 0.1 - diffuse);
		color = max(color, nightcolor);
	}
	FragColor = vec4(color, 1.0);
}
