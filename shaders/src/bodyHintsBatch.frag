//
//	body_Hints - batched variant (new-path HINT service family)
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location=0) in vec4 vColor;

layout (location=0) out vec4 FragColor;

void main(void)
{
	FragColor = vColor;
}
