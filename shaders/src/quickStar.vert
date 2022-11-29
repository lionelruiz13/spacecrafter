#version 420

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 0) flat out vec4 colorOut;

layout (binding=0, set=0) uniform uMat {
	mat4 ModelViewMatrix;
	vec3 clipping_fov;
};

#include <fisheye.glsl>

void main()
{
	vec4 pos = fisheyeProject(position, clipping_fov);
	float alpha = 1 - pos.z;
	colorOut = vec4(color, alpha);
	gl_Position = pos;
	gl_PointSize = 1.5;
}
