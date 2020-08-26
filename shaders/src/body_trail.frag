//
//	body_trail
//
#version 420
#pragma debug(on)
#pragma optimize(off)


uniform vec3 Color;

 
out vec4 FragColor;

in FaderColor
{
	smooth float indice;
} faderColor;

void main(void)
{
	FragColor = vec4(Color, faderColor.indice);

	//Coloration en rouge, on indique au programme que l'on veut du rouge !
	// couleur R G B A ou A est la transparence
	//~ FragColor = vec4 (1.0, 0.0, 0.0, fader);
}
