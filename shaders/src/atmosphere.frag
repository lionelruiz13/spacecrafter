//
//	atmosphere
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location = 0) in vec3 Color;

layout (location = 0) out vec3 FragColor;
 
void main(void)
{
	//~ FragColor = vec3(0.0, 1.0, 0.0);
	FragColor = Color;
}
