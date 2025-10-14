//
//	Planet Grid Fragment Shader - Vertices with color
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout(location=0) in vec3 pos;
layout(location=1) in vec3 vertColor; // Color interpolated from vertex shader

layout(location=0) out vec4 FragColor;

void main(void)
{
	FragColor = vec4(vertColor, 1.0); // Use vertex color with full opacity
}