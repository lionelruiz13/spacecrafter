#version 450

layout (triangles, equal_spacing) in;

layout (location=0) in vec3 position[];
layout (location=0) out vec3 eyePos;

// All positions are relative to the camera (camPos is at origin) - without projection
layout (binding=0) uniform ubo {
    mat4 ModelViewMatrix;
    vec3 sunPos;
    float planetRadius; // 1.0  ;
    vec3 bodyPos;
    float planetOneMinusOblateness;
    vec3 clipping_fov;
    float atmRadius;    // 1.05 ;
    ivec2 TesParam;     // [min_tes_lvl, max_tes_lvl]
    float atmAlpha;     // 1.0  ; // this value is a scale for atmosphere transparency
};

#include <fisheyeNoMV.glsl>

void main()
{
    vec3 pos=(gl_TessCoord.x * position[0])+
                  (gl_TessCoord.y * position[1])+
                  (gl_TessCoord.z * position[2]);

    eyePos = pos;
    gl_Position = fisheyeProjectNoMV(pos, clipping_fov);
}
