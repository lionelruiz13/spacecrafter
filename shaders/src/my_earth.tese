//
// body normal tessellation
//

#version 430
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (triangles, equal_spacing) in;

layout (binding=11) uniform sampler2D heightmapTexture;

//externe
layout (binding=0) uniform globalProj {
	mat4 ModelViewMatrix;
	mat4 NormalMatrix;
	vec3 clipping_fov;
	float planetRadius;
	vec3 LightPosition;
	float planetScaledRadius;
	float planetOneMinusOblateness;
};

#include <cam_block.glsl>
#include <fisheye_noMV.glsl>

layout (binding=2) uniform globalTescGeom {
	ivec3 TesParam;         // [min_tes_lvl, max_tes_lvl, coeff_altimetry]
};

layout(location=0) in vec3 glPositionIn[];
layout(location=1) in vec2 TexCoordIn[];
layout(location=2) in vec3 NormalIn[];

layout (location=0) out vec3 PositionOut;
layout (location=1) out vec2 TexCoordOut;
layout (location=2) out vec3 NormalOut;
layout (location=3) out vec3 TangentLightOut;
layout (location=4) out vec3 LightOut;
layout (location=5) out vec3 ViewDirectionOut;

float coeffHeightMap = 0.01 * TesParam[2];

//////////////////// PROJECTION FISHEYE ////////////////////////////////
void main()
{
    vec3 position=(gl_TessCoord.x * glPositionIn[0])+
                  (gl_TessCoord.y * glPositionIn[1])+
                  (gl_TessCoord.z * glPositionIn[2]);
    vec2 TexCoord = TexCoordIn[0]*gl_TessCoord.x+
                    TexCoordIn[1]*gl_TessCoord.y+
                    TexCoordIn[2]*gl_TessCoord.z;
    gl_Position = fisheyeProject(position * planetScaledRadius * (1.0+texture(heightmapTexture,TexCoord).x * coeffHeightMap), clipping_fov);
    position = vec3(ModelViewMatrix * vec4(position * planetRadius, 1));
    PositionOut = position;
    TexCoordOut = TexCoord;

    vec3 Light = normalize(LightPosition - position);
    vec3 Normal = normalize(mat3(NormalMatrix) * (
        NormalIn[0]*gl_TessCoord.x+
        NormalIn[1]*gl_TessCoord.y+
        NormalIn[2]*gl_TessCoord.z));
    vec3 binomial = vec3(0, -Normal.z, Normal.y);
    ViewDirectionOut = normalize(-position);
    LightOut = Light;
    NormalOut = Normal;
    TangentLightOut = vec3(dot(Light, cross(Normal, binomial)), dot(Light, binomial), dot(Light, Normal));
}
