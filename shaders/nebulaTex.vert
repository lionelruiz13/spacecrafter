// nebulaTex

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;

out VInterpolators
{
	vec2 texCoord;
} dataVertex;

void main(void)
{
	gl_Position = vec4(Position,1.0);
	dataVertex.texCoord = TexCoord;
}
