#version 420

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 0) flat out vec4 colorOut;

layout (binding=0, set=1) uniform uMat {
	mat4 ModelViewMatrix;
	vec3 clipping_fov;
	float fading;
};

#include <cam_block_only.glsl>
#include <custom_project.glsl>

void main()
{
	vec4 pos = custom_projectAberrated(position, ModelViewMatrix, clipping_fov, light_aberration_beta.xyz);
	pos.z = clamp(pos.z, 0.0, 1.0);
	colorOut = vec4(color, fading);
	gl_Position = pos;
	gl_PointSize = 1.5;
}
