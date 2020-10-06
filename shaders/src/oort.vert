// oort

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location = 0) in vec3 Position;

layout (binding=0, set=1) uniform uMat {
	mat4 Mat;
};

#include <cam_block_only.glsl>
#include <custom_project.glsl>

void main(void)
{
	vec4 pos = custom_project( vec4(Position,1.0) );
	pos.z = 0.0;
	pos.w = 1.0;
	gl_Position = MVP2D * pos;
}
