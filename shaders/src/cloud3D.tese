//
// cloud 3D
//
#version 420 core

layout (triangles, equal_spacing) in;

layout (location=0) in vec3 position[];
layout (location=1) in vec3 texCoord[];
layout (location=2) patch in vec4 color;
layout (location=3) patch in mat4 invModel; // Use of mat4 as mat3 due to the nvidia bug of mat3 in the vulkan implementation

layout (location=0) out vec3 direction;
layout (location=1) out vec3 texCoordOut;
layout (location=2) out flat vec4 colorOut;

layout (binding=1) uniform fov {
    vec3 clipping_fov;
};

layout (binding=2) uniform uproj {
    mat4 camRotToLocal; // Inverse rotation of ModelViewMatrix
};

#include <fisheyeNoMV.glsl>

void main(void)
{
    vec3 Position = position[0]*gl_TessCoord[0] + position[1]*gl_TessCoord[1] + position[2]*gl_TessCoord[2];
    texCoordOut = texCoord[0]*gl_TessCoord[0] + texCoord[1]*gl_TessCoord[1] + texCoord[2]*gl_TessCoord[2];
    colorOut = color;
    gl_Position = fisheyeProjectNoMV(Position, clipping_fov);
    direction = vec3(invModel * vec4(normalize(vec3(camRotToLocal * vec4(Position, 0.))), 0.)); // Use of mat4 as mat3 due to the nvidia bug of mat3 in the vulkan implementation
}
