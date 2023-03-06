//
// Moon shadow
//

#version 430
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

#include <cam_block.glsl>

layout (binding=2) uniform sampler2D heightMap;
layout (binding=3) uniform sampler2D normalMap;
layout (binding=4) uniform sampler2D dayTexture;
// layout (binding=6) uniform sampler2DArray bodyShadows;

#define M_PI 3.14159265358979323846

layout (binding=1) uniform globalFrag {
	mat3 ShadowMatrix;
	vec3 lightDirection; // In body-local coordinates
	float sinSunAngle;
	float heightMapDepthLevel; // 0.9
	float heightMapDepth; // 0.1
	float squaredHeightMapDepthLevel; // 0.81
	int nbShadowingBodies;
	vec3 shadowingBodies[4];
	vec3 atmColor; // Colorimetry of the atmosphere
	float sunDeviation; // Deviation of the sun ray
	float atmDeviation; // Deviation of the atmosphere color
};

layout (location=0) in vec3 entryPos;
layout (location=1) in vec3 viewDirection;

layout (location=0) out vec4 fragColor;

// sin(x) = sqrt(1 - cos(x)²)
// sin(x) * (1 - cos(x))
// sqrt(1 - cos(x)²) * (1 - cos(x))

#define UNIT_STEP_COUNT 1024
#define STEP_COUNT 24
#define SHADOW_STEP_FACTOR 1.1
#define SHADOW_STEP_INIT (1.f/8192)

vec3 xyzToLonLatAlt(vec3 pos)
{
	float rq = pos.x*pos.x+pos.y*pos.y;
	float depth = sqrt(rq+pos.z*pos.z);
	float lat = acos(-pos.z/depth) / M_PI;
	float lon = acos(pos.x/sqrt(rq)) / (2 * M_PI);
	if (pos.y < 0)
		lon = 1 - lon;
	return vec3(lon, lat, (depth - heightMapDepthLevel) / heightMapDepth);
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
		vec3 tmp = xyzToLonLatAlt(samplePos);
		if (textureLod(heightMap, tmp.xy, 0).r > tmp.z) {
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
			vec3 tmp = xyzToLonLatAlt(samplePos);
			if (textureLod(heightMap, tmp.xy, 0).r > tmp.z) {
				samplePos -= rayStep; // Ground hit
			} else {
				samplePos += rayStep;
			}
		}
		float rq = samplePos.x*samplePos.x+samplePos.y*samplePos.y;
		float depth = sqrt(rq+samplePos.z*samplePos.z);
		rq = sqrt(rq);
		float lat = acos(-samplePos.z/depth) / M_PI;
		float lon = acos(samplePos.x/rq) / (2 * M_PI);
		if (samplePos.y < 0)
			lon = 1 - lon;
		// samplePos /= depth;
		// rq /= depth;
		vec2 texCoord = vec2(lon, lat);
		vec2 shadowPos = vec2(ShadowMatrix * samplePos); // For shadow projection
		vec3 nSamplePos = samplePos/depth;
		vec3 xAxis = normalize(vec3(-samplePos.y, samplePos.x, 0));
		vec3 yAxis = normalize(cross(xAxis, nSamplePos));
		if (yAxis.z < 0)
			yAxis = -yAxis;
		vec3 normal = normalize(mat3(xAxis,yAxis, nSamplePos) * (texture(normalMap, texCoord).xyz * 2 - 1));
		vec3 sunDirection = normalize(nSamplePos * sunDeviation - lightDirection);
		float NdotL = clamp(dot(sunDirection, normal) + ambient, ambient, 1);
		float atmosphere = clamp(atmDeviation - dot(lightDirection, nSamplePos), 0, 1);
		if (NdotL + atmosphere > ambient) {
			float shadowing = 1;
			// Process shadow of bodies
			for (int i = 0; i < nbShadowingBodies; ++i) {
				vec2 tmp = shadowPos - shadowingBodies[i].xy;
				float sr = shadowingBodies[i].z;
				if (dot(tmp, tmp) < sr) {
					// shadowing += texture(shadowTexture, vec3((tmp + sr) / (sr * 2), i)).r
					shadowing *= dot(tmp, tmp) / sr; // Temporary solution
				}
			}
			// shortly ray trace toward -lightDirection for self-shadowing
			rayLength = SHADOW_STEP_INIT;
			float maxOcclusion = 1;
			vec3 tmp;
			do {
				tmp = xyzToLonLatAlt(samplePos + sunDirection * rayLength);
				maxOcclusion = min(maxOcclusion, (tmp.z - textureLod(heightMap, tmp.xy, 0).r) / rayLength);
				rayLength *= SHADOW_STEP_FACTOR;
			} while (tmp.z < 1);
			NdotL *= clamp(maxOcclusion * heightMapDepth / sinSunAngle + 0.5, 0, 1);

			// Process color
			color = texture(dayTexture, texCoord).xyz * mix(vec3(NdotL), vec3(min(atmosphere + ambient, 1)), atmColor);
		} else {
			color = texture(dayTexture, texCoord).xyz * ambient;
		}
	} else {
		discard;
	}
	// Atmosphere may be traced here
	fragColor = vec4(color, 1);
}
