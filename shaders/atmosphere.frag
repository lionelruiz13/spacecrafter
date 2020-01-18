//
//	atmosphere
//
#version 420
#pragma debug(on)
#pragma optimize(off)

smooth in vec3 Color;
 
out vec3 FragColor;
 
void main(void)
{
	//~ FragColor = vec3(0.0, 1.0, 0.0);
	FragColor = Color;
}
