#version 420

layout (binding=0) uniform ubo {
	mat4 ModelViewMatrix;
	vec3 clipping_fov;
	float RingScale;
};

#include <fisheye_noMV.glsl>

layout (location=0) in vec3 Position;
layout (location=3) in vec4 Color; // missing component 3 is set to 1 (see Vulkan spec)

layout (location=6) in vec3 Shift;

layout (location=0) out vec4 ColorOut;

void main(void)
{
	gl_Position = fisheyeProject(RingScale * (Position + Shift), clipping_fov);
	ColorOut = Color;
}
