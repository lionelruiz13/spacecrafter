//
//	body_halo
//
#version 420
#pragma debug(on)
#pragma optimize(off)

//externe
layout (binding=0) uniform sampler2D mapTexture;
uniform vec3 Color;
uniform float cmag;

//in
smooth in vec2 TexCoord;

//out
out vec3 FragColor;

void main(void)
{
	vec3 tex_color = cmag * vec3(texture(mapTexture,TexCoord)).rgb;
	FragColor = vec3(tex_color.r * Color.r, tex_color.g * Color.g, tex_color.b * Color.b);
	//~ FragColor = vec3(1.0, 0.0, 0.0);
	
	//Coloration en rouge, on indique au programme que l'on veut du rouge !
	// couleur R G B A ou A est la transparence
	//~ FragColor = vec4 (1.0, 0.0, 0.0, 1.0);
}

