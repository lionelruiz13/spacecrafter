//
//	orbit 3D
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (push_constant) uniform ubo {
	layout (offset=16) mat4 ModelViewMatrix;
	vec3 clipping_fov;
};

#include <fisheye.glsl>

//layout
layout (lines) in;
layout (line_strip, max_vertices = 16) out;

layout (location=0) in vec3 position[];

#define TOLERANCE 1.2

void main(void)
{
	vec4 pos1 = gl_in[0].gl_Position;
	vec4 pos2 = gl_in[1].gl_Position;
	float spacing = distance(vec2(pos1), vec2(pos2));

	if (clipping_fov.z > 0.3 && spacing > TOLERANCE)
		return;

	gl_Position = pos1;
	EmitVertex();

	int nbLines = clamp(int(spacing * 32), 1, 15);
	vec3 pos = position[0];
	vec3 shift = (position[1] - pos) / nbLines;
	while (--nbLines > 0) {
		pos += shift;
		gl_Position = fisheyeProjectClamped(pos, clipping_fov);
		EmitVertex();
	}
	gl_Position = pos2;
	EmitVertex();
	EndPrimitive();
}
