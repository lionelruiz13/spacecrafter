//
// sky_draw
//
#version 420
#pragma debug(on)
#pragma optimize(off)


layout (location=0) out vec4 FragColor;	//fragment output colour



smooth in vec3 Color;

//~ out vec4 FragColor;
 
void main(void)
{
	FragColor = vec4 (Color,1.0);
}

