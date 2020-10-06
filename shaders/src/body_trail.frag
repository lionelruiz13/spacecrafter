//
//	body_trail
//
#version 420
#pragma debug(on)
#pragma optimize(off)


layout (binding=1) uniform uColor {vec3 Color;};

layout (location=0) out vec4 FragColor;

layout (location=0) in FaderColor
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
