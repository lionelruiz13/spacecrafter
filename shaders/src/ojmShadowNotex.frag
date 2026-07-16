//
// ojmShadowNotex - new-path OJM shadowed COLOR row (texture-less shapes).
// Port of body_artificial_shadow_notex.frag - see ojmShadowTex.frag for the
// port notes (lighting kept exactly, S5/G7 receive convention, selfShadowOn
// gate). The texture-less color term is Material.Ka in place of the texture
// sample - the old shader's own composition, kept as-is.
// CPU mirror of binding 2: ojmShadowBlock (bodyShaderInterface.hpp).
//
#version 450

layout (set = 2, binding=3) uniform sampler2D shadowMap;
layout (set = 2, binding=4) uniform sampler2DArray bodyShadows;

#include <selfShadow.glsl>
#include <receivedShadowsDecl.glsl>

layout (location=0) out vec3 FragColor;

layout (location=0) in vec3 Position;
layout (location=1) in vec2 TexCoord;
layout (location=2) in vec3 Normal;
layout (location=3) in float Ambient;

layout (binding=2, set=2) uniform ojmShadowBlock {
    mat3 ShadowMatrix;      // model -> sun-frame NDC (self-shadow projection)
    mat3 ModelMatrix;       // model -> eye (rotation+scale)
    vec3 ModelPosition;     // eye-space body center
    vec3 lightDirection;    // eye-space, direction light travels
    vec3 LightIntensity;    // A,D,S intensity
    float selfShadowOn;     // 1 = self-shadow depth valid this frame (nominated)
    vec4 shadowRow0;        // model-folded sun-frame rows (receivedShadows.glsl)
    vec4 shadowRow1;
    int nbShadowingBodies;
    ShadowingBody shadowingBodies[MAX_SHADOW_CASTERS];
};

#include <receivedShadows.glsl>

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
    float selfShadowing = (selfShadowOn != 0.0) ? computeEnlightment(ShadowMatrix * Position, sDotN) : 1.0;
    vec2 shadowPos = vec2(dot(shadowRow0.xyz, Position) + shadowRow0.w,
                          dot(shadowRow1.xyz, Position) + shadowRow1.w);
    vec3 shadowing = computeReceivedShadowing(shadowPos, Position) * selfShadowing;
    FragColor = LightIntensity * (Material.Ka * ((Material.Kd * shadowing + Material.Ka) * sDotN) + Material.Ks * (shadowing * specular));
}
