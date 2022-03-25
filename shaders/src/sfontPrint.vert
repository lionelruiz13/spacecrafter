//
// sfontPrint
//

#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (location=0)in vec4 position;
layout (location=1)in vec2 texCoord;

layout(push_constant) uniform pushConstants {
	layout (offset=16) mat4 MVP;
};

layout (location=0) out vec2 TexCoord;


void main()
{
	gl_Position = MVP * position;
	TexCoord = texCoord;
}
