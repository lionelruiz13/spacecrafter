//
//	orbit 3D
//
#version 420
#pragma debug(on)
#pragma optimize(off)


uniform vec4 Color;
 
out vec4 FragColor;

void main(void)
{
	FragColor = Color;
	//Coloration en rouge, on indique au programme que l'on veut du rouge !
	// couleur R G B A ou A est la transparence
	//~ FragColor = vec4 (1.0, 0.0, 0.0, 1.0);
}

