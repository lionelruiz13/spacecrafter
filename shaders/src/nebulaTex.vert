// nebulaTex

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location = 0) in vec4 Position;
layout (location = 1) in vec2 TexCoord;

layout (binding=1) uniform ubo {
	mat4 Mat;
};

#include <cam_block_only.glsl>
#include <custom_project.glsl>

layout (location=0) out vec2 texCoord;

void main(void)
{
	vec4 pos = custom_project(Position);
	pos.z = 0;
	gl_Position = pos;
	texCoord = TexCoord;
}
