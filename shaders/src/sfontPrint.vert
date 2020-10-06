//
// sfontPrint
//

#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (location=0)in vec2 position;
layout (location=1)in vec2 texCoord;

layout(push_constant) uniform pushConstants {
	layout (offset=16) mat4 MVP;
};

layout (location=0) out vec2 TexCoord;


void main()
{
	gl_Position = MVP * vec4(position,0.0,1.0);
	TexCoord = texCoord;
}
