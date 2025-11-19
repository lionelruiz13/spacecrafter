//
//	Planet Grid Vertex Shader
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (push_constant) uniform uMat {
	mat4 ModelViewMatrix; // Now includes rotation and scaling
	vec3 clipping_fov;
};

layout (location=0) in vec3 position;  // Static unit sphere
layout (location=1) in vec3 color;     // Vertex color (meridian/parallel)
layout (location=0) out vec3 pos;
layout (location=1) out vec3 vertColor; // Pass color to fragment shader

#include <cam_block.glsl>
#include <custom_project.glsl>

void main()
{
    // All transformations (rotation, scaling, model-view) are now pre-computed in ModelViewMatrix
	vec4 eyePos = ModelViewMatrix * vec4(position, 1.0);

	// Projection
	pos = eyePos.xyz;
	vertColor = color; // Pass color to fragment shader
	gl_Position = custom_projectNoMV(eyePos.xyz, clipping_fov);
}