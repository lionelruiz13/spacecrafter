//
// landscape
//

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=2, set=1) uniform uModelViewMatrix {mat4 ModelViewMatrix;};

// for main_clipping_fov and projectionType
#include <cam_block_only.glsl>
//custom projection inclusion
#include <custom_project.glsl>


//layout
layout (location=0)in vec3 position;
layout (location=1)in vec2 texcoord;

layout (location=0) out vec2 TexCoord;
layout (location=1) out vec4 Position;

void main()
{
    Position = custom_project(position, ModelViewMatrix, vec3(main_clipping_fov));
    TexCoord = texcoord;
}
