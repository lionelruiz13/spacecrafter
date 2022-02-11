//
// Volumetric 3D object
//
#version 420

layout (triangles, equal_spacing) in;

layout (binding=0) uniform ubo {
    mat4 ModelViewMatrix;
    mat4 NormalMatrix; // Inverse rotation and scaling of ModelViewMatrix
    vec3 clipping_fov;
};

#include <fisheyeNoMV.glsl>

layout (location=0) in vec3 pos[];
layout (location=1) in vec3 tex[];

layout (location=0) out vec3 direction;
layout (location=1) out vec3 texOut;

void main()
{
    vec3 position = normalize(pos[0] * gl_TessCoord[0] + pos[1] * gl_TessCoord[1] + pos[2] * gl_TessCoord[2]);
    texOut = tex[0] * gl_TessCoord[0] + tex[1] * gl_TessCoord[1] + tex[2] * gl_TessCoord[2];
    direction = vec3(NormalMatrix * vec4(position, 0.));
    gl_Position = fisheyeProjectNoMV(position, clipping_fov);
}
