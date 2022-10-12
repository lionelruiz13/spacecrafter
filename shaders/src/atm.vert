#version 450

layout (location=0) in vec3 position;
layout (location=0) out vec3 pos;
layout (location=1) out vec2 eyePos;

#include <fisheye2DNoMV.glsl>

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

void main()
{
    vec3 Position = position * atmRadius;
    Position.z *= planetOneMinusOblateness;
    Position = vec3(ModelViewMatrix * vec4(Position, 1));
    pos = Position;
    eyePos = fisheye2DNoMV(Position, clipping_fov[2]);
}
