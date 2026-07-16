//
// bodyRayMarch - MESH_RAYMARCH base row (row 2, the my_moon_shadow class:
// Moon/Mars/Iapetus close-range). Port of my_moon_shadow.frag: per-pixel
// heightmap ray-march relief, normal-map lighting with cam_block ambient
// floor, terrain self-shadow march. Deltas vs source, both S5:
// - mat3 ShadowMatrix -> two sun-frame rows PRE-FOLDED through the model
//   matrix (CPU side): shadowPos lands in eye-space AU, caster posRadius
//   consumed unchanged (no initialRadius normalization - it existed because
//   the old frames mixed unit systems, shadow-paths.md E "Units");
// - scalar darkening -> per-channel caster shadowAbsorbtion (vec3).
// Bindings re-homed to the family contract "bodyRayMarch" (bodyShadows 7).
// CPU mirror of binding 1: rayMarchFrag (bodyShaderInterface.hpp).
//
#version 450

#include <cam_block.glsl>

layout (binding=2) uniform sampler2D heightMap;
layout (binding=3) uniform sampler2D normalMap;
layout (binding=4) uniform sampler2D dayTexture;
layout (binding=7) uniform sampler2DArray bodyShadows;

#define M_PI 3.14159265358979323846

#define MAX_SHADOW_CASTERS 8

struct ShadowingBody {
	vec4 posRadius;      // xy = caster center in sun-frame (rel. receiver), z = disc radius
	vec4 absorbtionIdx;  // rgb = caster shadow absorption, w = layer index
	vec4 clip;           // half-space gate: apply iff dot(P, xyz) + w <= 0 ((0,0,0,-1) = always; planar casters)
};

layout (binding=1) uniform rayMarchFrag {
	vec4 shadowRow0;     // pre-folded: dot(row.xyz, samplePosUnit) + row.w = eye-space shadowPos
	vec4 shadowRow1;
	vec3 lightDirection; // In body-local coordinates
	float sinSunAngle;
	float heightMapDepthLevel; // 0.9
	float heightMapDepth; // 0.1
	float squaredHeightMapDepthLevel; // 0.81
	float sunDeviation; // Deviation of the sun ray
	vec3 atmColor; // Colorimetry of the atmosphere
	float atmDeviation; // Deviation of the atmosphere color
	int nbShadowingBodies;
	ShadowingBody shadowingBodies[MAX_SHADOW_CASTERS];
};

layout (location=0) in vec3 entryPos;
layout (location=1) in vec3 viewDirection;
layout (location=2) in flat float side;

layout (location=0) out vec4 fragColor;

#define UNIT_STEP_COUNT 1024
#define STEP_COUNT 24
#define SHADOW_STEP_FACTOR 1.1
#define SHADOW_STEP_INIT (1.f/8192)
#define SHADOW_MIN_STEP (1.f/65536)

float xyzToHeight(vec3 pos)
{
	float depth = length(pos);
	float tmp = atan(pos.y, pos.x) / (2 * M_PI);
	tmp += mix(0.5, 1.5, tmp < side);
	return (depth - heightMapDepthLevel) / heightMapDepth - textureLod(heightMap, vec2(
		tmp,
		acos(-pos.z/depth) / M_PI
	), 0).r;
}

void main(void)
{
	vec3 view = normalize(viewDirection);
	vec3 samplePos = normalize(entryPos);
	float rayLength = -dot(view, samplePos);
	float delta = squaredHeightMapDepthLevel - (1 - rayLength*rayLength);
	bool hitBody = (delta > 0);
	if (hitBody)
		rayLength = rayLength - sqrt(delta);
	int stepCount = int(rayLength * UNIT_STEP_COUNT);
	vec3 rayStep = view * (rayLength / stepCount);
	for (int i = 0; i < stepCount; ++i) {
		samplePos += rayStep;
		if (xyzToHeight(samplePos) < 0) {
			hitBody = true;
			rayStep /= 2;
			samplePos -= rayStep;
			break;
		}
	}
	vec3 color = vec3(0);
	if (hitBody) {
		for (int i = 0; i < STEP_COUNT; ++i) {
			rayStep /= 2;
			if (xyzToHeight(samplePos) < 0) {
				samplePos -= rayStep; // Ground hit
			} else {
				samplePos += rayStep;
			}
		}
		float tmp = atan(samplePos.y, samplePos.x) / (2 * M_PI);
		tmp += mix(0.5, 1.5, tmp < side);
		float depth = length(samplePos);
		vec2 texCoord = vec2(tmp, acos(-samplePos.z/depth) / M_PI);
		vec2 shadowPos = vec2(dot(shadowRow0.xyz, samplePos) + shadowRow0.w,
		                      dot(shadowRow1.xyz, samplePos) + shadowRow1.w); // For shadow projection
		vec3 shadowSample = samplePos; // clip planes are folded through the same map as the rows
		vec3 xAxis = normalize(vec3(-samplePos.y, samplePos.x, 0));
		samplePos /= depth;
		vec3 yAxis = normalize(cross(xAxis, samplePos));
		if (yAxis.z < 0)
			yAxis = -yAxis;
		vec3 normal = normalize(mat3(xAxis,yAxis, samplePos) * (texture(normalMap, texCoord).xyz * 2 - 1));
		vec3 sunDirection = normalize(samplePos * sunDeviation - lightDirection);
		float NdotL = clamp(dot(sunDirection, normal) + ambient, ambient, 1);
		float atmosphere = clamp(atmDeviation - dot(lightDirection, samplePos), 0, 1);
		samplePos *= textureLod(heightMap, texCoord, 0).r * heightMapDepth + heightMapDepthLevel;
		if (NdotL + atmosphere > ambient) {
			vec3 shadowing = vec3(1);
			// Process shadow of bodies
			for (int i = 0; i < nbShadowingBodies; ++i) {
				vec2 tmp = (shadowPos.xy - shadowingBodies[i].posRadius.xy) / shadowingBodies[i].posRadius.z;
				if (dot(tmp, tmp) < 1 && dot(shadowSample, shadowingBodies[i].clip.xyz) + shadowingBodies[i].clip.w <= 0.0) {
					float coverage = textureLod(bodyShadows, vec3(tmp * 0.5 + 0.5, shadowingBodies[i].absorbtionIdx.w), 0.0).r; // explicit LOD: implicit derivatives are UNDEFINED in this non-uniform flow (zero reads on NVIDIA; layer is single-mip)
					shadowing *= vec3(1) - coverage * shadowingBodies[i].absorbtionIdx.rgb;
				}
			}
			// shortly ray trace toward -lightDirection for self-shadowing
			rayLength = SHADOW_STEP_INIT;
			float maxOcclusion = 1;
			float tmp;
			float prev = 0;
			bool approaching = false; // Is previously approaching
			do {
				tmp = xyzToHeight(samplePos + sunDirection * rayLength);
				if (tmp < prev) {
					approaching = true;
				} else if (approaching) {
					prev = rayLength / SHADOW_STEP_FACTOR;
					float tmpR = rayLength - prev;
					do {
						tmpR /= 2;
						if (xyzToHeight(samplePos + sunDirection * (prev - tmpR)) < xyzToHeight(samplePos + sunDirection * (prev + tmpR))) {
							prev -= tmpR;
						} else {
							prev += tmpR;
						}
					} while (tmpR > SHADOW_MIN_STEP);
					maxOcclusion = min(maxOcclusion, xyzToHeight(samplePos + sunDirection * prev) / prev);
					approaching = false;
				}
				prev = tmp;
				rayLength *= SHADOW_STEP_FACTOR;
			} while (tmp < 0.8);
			NdotL *= clamp(maxOcclusion * heightMapDepth / sinSunAngle + 0.5, 0, 1);

			// Process color
			color = texture(dayTexture, texCoord).xyz * min(mix(vec3(NdotL), vec3(atmosphere), atmColor) * shadowing + ambient, 1);
		} else {
			color = texture(dayTexture, texCoord).xyz * ambient;
		}
	} else {
		discard;
	}
	// Atmosphere may be traced here
	fragColor = vec4(color, 1);
}
