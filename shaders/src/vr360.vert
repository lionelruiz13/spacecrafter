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

layout (binding=4, set=1) uniform udata {
	mat4 ModelViewMatrix;
	float fader;
	bool hasAlphaChannel;
};
// for main_clipping_fov
#include <cam_block_only.glsl>
//custom projection inclusion
#include <custom_project.glsl>

//out
layout (location=0) out vec2 TexCoord;

void main()
{
	gl_Position = custom_project(position, ModelViewMatrix, vec3(main_clipping_fov));
    TexCoord = texcoord;
}
