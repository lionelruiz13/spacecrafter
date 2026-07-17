//
// bodyTesMesh - MESH_TES base row tese (port of body_normal_tes.tese;
// heightmap re-homed to binding 7, Ambient dropped - the frag reads
// cam_block directly). Interface = exactly bodyTesMesh.frag inputs.
// One-tese-per-row (not a superset): unconsumed tese outputs fire
// WARNING-Shader-OutputNotConsumed - the zero-validation-messages bar
// (S5) forces exact interfaces, same structure as the three old teses.
//
#version 450

layout (triangles, equal_spacing) in;

layout (binding=7) uniform sampler2D heightmapTexture;

layout (binding=0) uniform globalVertProj {
	mat4 ModelViewMatrix;
	mat4 NormalMatrix;
	vec3 clipping_fov;
	float planetRadius;
	vec3 LightPosition;
	float planetScaledRadius;
	float planetOneMinusOblateness;
};

#include <cam_block.glsl>
#include <custom_project.glsl> // multi-mode (spec-const 8), INTENT 11.33 - mainline my_earth.tese template

layout (binding=2) uniform meshTescGeom {
	ivec3 TesParam;         // [min_tes_lvl, max_tes_lvl, altimetry_level]
};

layout(location=0) in vec3 glPositionIn[];
layout(location=1) in vec2 TexCoordIn[];
layout(location=2) in vec3 NormalIn[];

layout (location=0) out vec3 PositionOut;
layout (location=1) out vec2 TexCoordOut;
layout (location=6) out float NdotLOut;

float coeffHeightMap = 0.01 * TesParam[2];

void main()
{
    vec3 position=(gl_TessCoord.x * glPositionIn[0])+
                  (gl_TessCoord.y * glPositionIn[1])+
                  (gl_TessCoord.z * glPositionIn[2]);
    vec2 TexCoord = TexCoordIn[0]*gl_TessCoord.x+
                    TexCoordIn[1]*gl_TessCoord.y+
                    TexCoordIn[2]*gl_TessCoord.z;
    gl_Position = custom_project(position * planetScaledRadius * (1.0+texture(heightmapTexture,TexCoord).x * coeffHeightMap), ModelViewMatrix, clipping_fov);
    position = vec3(ModelViewMatrix * vec4(position * planetRadius, 1));

    vec3 Light = normalize(LightPosition - position);
    vec3 Normal = normalize(mat3(NormalMatrix) * (
        NormalIn[0]*gl_TessCoord.x+
        NormalIn[1]*gl_TessCoord.y+
        NormalIn[2]*gl_TessCoord.z));
    vec3 binomial = vec3(0, -Normal.z, Normal.y);
    PositionOut = position;
    TexCoordOut = TexCoord;
    NdotLOut = dot(Normal, Light);
}
