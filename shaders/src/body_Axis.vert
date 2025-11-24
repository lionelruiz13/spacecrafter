//
// body_Axis
//

#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (location=0) in vec3 position; // 3D position in object space

layout (push_constant) uniform pushConstants {
    mat4 ModelViewMatrix;
    vec3 clipping_fov;
};

#include <custom_project.glsl>

void main()
{
    // Transform to eye space first
	vec4 eyePos = ModelViewMatrix * vec4(position, 1.0);

	// Projection
	gl_Position = custom_projectNoMV(eyePos.xyz, clipping_fov);
}
