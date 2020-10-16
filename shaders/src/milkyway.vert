//
// Milkyway
//
#version 420
#pragma debug(on)
#pragma optimize(off)

//layout
layout (location=0)in vec3 position;
layout (location=1)in vec2 texcoord;
//~ layout (location=2)in vec3 normal; // useless unitl now

//fisheye projection inclusion
layout (binding=2, set=1) uniform uModelViewMatrix {mat4 ModelViewMatrix;};
#include <fisheye_noMV.glsl>

// for main_clipping_fov
#include <cam_block_only.glsl>

//out
layout (location=0) out vec2 TexCoord;
layout (location=1) out vec4 Position;

void main()
{
	Position = fisheyeProject(position, vec3(main_clipping_fov));
    TexCoord = texcoord;
}
