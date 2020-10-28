#version 420

layout (location=0) in vec4 Color;
layout (location=0) out vec4 FragColor;

void main(void)
{
	FragColor = Color;
}
