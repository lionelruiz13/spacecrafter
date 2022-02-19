//
//	orbit 2D
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

//layout
layout (lines) in;
layout (line_strip, max_vertices = 16) out;

layout (location=0) in vec4 position[];


//externe
layout (push_constant) uniform uMat {
	layout (offset=16) mat4 ModelViewMatrix;
	float fov;
};

#include <fisheye2D.glsl>

void main(void)
{
	vec4 pos1 = gl_in[0].gl_Position;
	vec4 pos2 = gl_in[1].gl_Position;
	vec4 pos = pos1 - pos2;
	float spacing = sqrt(pos.x * pos.x + pos.y * pos.y);

	if (spacing > 1.2)
		return;

	gl_Position = pos1;
	EmitVertex();

	int nbLines = clamp(int(spacing * 32), 1, 15);
	pos = position[0];
	vec4 shift = (position[1] - pos) / nbLines;
	while (--nbLines > 0) {
		pos += shift;
		gl_Position = fisheye2D(pos, fov);
		EmitVertex();
	}

	gl_Position = pos2;
	EmitVertex();
	EndPrimitive();
}
