//
// VR360
//

#version 420
#pragma debug(on)
#pragma optimize(off)

//layout
layout (location=0)in vec3 position;
layout (location=1)in vec2 texcoord;
layout (location=2)in vec3 normal;

//fisheye projection inclusion
layout (binding=3, set=1) uniform uModelViewMatrix {mat4 ModelViewMatrix;};
#include <fisheye.glsl>

// for main_clipping_fov
#include <cam_block_only.glsl>

//out
layout (location=0) out vec2 TexCoord;

void main()
{
	gl_Position = fisheyeProject(position, vec3(main_clipping_fov));
    TexCoord = texcoord;
}
