// oort

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location = 0) in vec4 Position;

layout (binding=0, set=1) uniform uMat {
	mat4 ModelViewMatrix;
};

#include <cam_block_only.glsl>
#include <custom_project.glsl>

void main(void)
{
	gl_Position = custom_project2D(Position, ModelViewMatrix, main_clipping_fov[2]);
	gl_PointSize = 1.5;
}
