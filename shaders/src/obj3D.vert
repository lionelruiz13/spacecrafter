//
// cloud 3D
//
#version 420
#extension GL_ARB_shader_storage_buffer_object : require

layout (binding=0) uniform proj {
    mat4 ModelViewMatrix;
};

layout (binding=1) uniform fov {
    vec3 clipping_fov;
};

#include <fisheyeNoMV.glsl>

layout (location=0) in vec3 position;
layout (location=1) in mat4 model;
layout (location=5) in mat4 invmodel;
layout (location=9) in vec3 data;

layout (location=0) out vec3 positionOut;
layout (location=1) out vec3 texCoord;
layout (location=2) out vec3 dataOut;
layout (location=3) out int visible;
layout (location=4) out mat4 invmodelOut; // To transmit to tese

void main()
{
    texCoord = position * 0.5 + 0.5;
    vec3 pos = vec3(ModelViewMatrix * model * vec4(position, 1.));
    positionOut = pos;
    dataOut = data;
    invmodelOut = invmodel;
    gl_Position = fisheyeProjectNoMV(pos, vec3(clipping_fov.x, clipping_fov.y, M_PI/2));
    visible = int(gl_Position.z >= 0.);
}
