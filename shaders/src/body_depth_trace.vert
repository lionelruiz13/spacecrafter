#version 420

layout (push_constant) uniform depthTraceInfo {
	mat4 ModelViewMatrix;
	vec3 clipping_fov;
	float planetScaledRadius;
	float planetOneMinusOblateness;
};

layout (location=0) in vec3 position;

#include <custom_project.glsl>

void main()
{
	vec3 pos = position * planetScaledRadius;
	pos.z *= planetOneMinusOblateness;
	gl_Position = custom_project(pos, ModelViewMatrix, clipping_fov);
}
