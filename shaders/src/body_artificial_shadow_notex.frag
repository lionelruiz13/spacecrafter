//
// body artificial
//
#version 450
#pragma debug(on)
#pragma optimize(off)


layout (set = 2, binding=3) uniform sampler2D shadowMap;
layout (set = 2, binding=4) uniform sampler2DArray bodyShadows;

#include <selfShadow.glsl>

layout (location=0) out vec3 FragColor;

layout (location=0) in vec3 Position;
layout (location=1) in vec2 TexCoord;
layout (location=2) in vec3 Normal;
layout (location=3) in float Ambient;

struct ShadowingBody {
    vec2 pos;
    float size;
    int idx;
};

layout (binding=2, set=2) uniform OjmShadowFrag {
    mat3 ShadowMatrix;
    mat3 ModelMatrix;
    vec3 ModelPosition;
    vec3 lightDirection;	// Light direction
    vec3 LightIntensity;	// A,D,S intensity
    int nbShadowingBodies;
    ShadowingBody shadowingBodies[4];
};

layout (push_constant) uniform MaterialInfo {
    layout (offset=0) vec3 Ka;  	// Ambient reflectivity
    layout (offset=12) float Ns;	// Specular factor
    layout (offset=16) vec3 Kd;		// Diffuse reflectivity
    layout (offset=32) vec3 Ks;		// Specular reflectivity
} Material;

void main()
{
    vec3 v = normalize(ModelMatrix * Position + ModelPosition);
    float sDotN = -dot(lightDirection, Normal);
    float specular = 0;
    if (sDotN < Ambient) {
        sDotN = Ambient;
    } else {
        specular = pow(max(dot(lightDirection + Normal * (2 * sDotN), -v), 0), Material.Ns);
    }
    vec3 shadowPos = ShadowMatrix * Position;
    float shadowing = computeEnlightment(shadowPos, sDotN);
    for (int i = 0; i < nbShadowingBodies; ++i) {
        vec2 tmp = (shadowPos.xy - shadowingBodies[i].pos) / shadowingBodies[i].size;
        if (dot(tmp, tmp) < 1) {
            shadowing *= 1 - texture(bodyShadows, vec3(tmp * 0.5 + 0.5, shadowingBodies[i].idx)).r;
        }
    }
    FragColor = LightIntensity * (Material.Ka * ((Material.Kd * shadowing + Material.Ka) * sDotN) + Material.Ks * (shadowing * specular));
}
