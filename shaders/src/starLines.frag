//
// starLines
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=1, set=1) uniform ubo {
	vec4 ColorAndFader;
};
layout (location=0) out vec4 FragColor;

void main(void)
{
	FragColor = ColorAndFader;
}

