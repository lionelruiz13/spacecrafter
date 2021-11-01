//
//	nebulaHint
//
#version 420
#pragma debug(on)
#pragma optimize(off)


layout (set=1, binding=0) uniform sampler2D mapTexture;
layout (push_constant) uniform ubo {float fader;};

layout (location=0) in vec3 Color;
layout (location=1) in vec2 TexCoord;

layout (location=0) out vec4 FragColor;

void main(void)
{
	FragColor = texture(mapTexture,TexCoord) * vec4(Color, fader);
}

