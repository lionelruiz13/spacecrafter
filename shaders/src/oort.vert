// oort

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location = 0) in vec4 Position;

layout (binding=0, set=1) uniform uMat {
	mat4 ModelViewMatrix;
};

#include <fisheye2D.glsl>
#include <cam_block_only.glsl>

void main(void)
{
	gl_Position = fisheye2D(Position, main_clipping_fov[2]);
	gl_PointSize = 1.5;
}
