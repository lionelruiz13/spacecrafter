//
//	skylineDraw
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0, set=1) uniform ubo {
	vec4 Color;
};

layout (location=0) out vec4 FragColor;

void main(void)
{
	FragColor = Color;
}
