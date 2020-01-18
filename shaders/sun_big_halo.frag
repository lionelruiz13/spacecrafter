//
//	sun_big_halo
//
#version 420
#pragma debug(on)
#pragma optimize(off)


in Interpolators
{
	vec2 Center;
	vec2 TexCoord;
} interData;
 
uniform float cmag;
uniform vec3 color;
uniform float radius; //rayon apparent du soleil en px
layout (binding=0) uniform sampler2D texunit0;

out vec3 FragColor;

//constantes à adapter
float C_RADIUS=1.1; //coefficient liée au radius
float ACCENTUATION1 = 1.1; //pour jouer sur la transparance
float ACCENTUATION2 = 0.7; //je ne sais pas a quoi cela sert

void main(void)
{
	// calcul du halo de loin
	vec3 tex_color = vec3(texture(texunit0, interData.TexCoord)).rgb;
	vec3 farHalo = vec3(tex_color.r * color.r, tex_color.g * color.g, tex_color.b * color.b);
	
	// calcul du halo au centre 
	vec3 nearHalo;
	vec2 pos = gl_FragCoord.xy-interData.Center;
	float posPixel = sqrt(dot(pos,pos));
	if (posPixel < C_RADIUS * radius)
		nearHalo= vec3(color * min(1.0, max(0.0,ACCENTUATION2*(1.0-ACCENTUATION1 *posPixel/(C_RADIUS*radius)))));
	else
		nearHalo = vec3(0.0);

	// assemblage des deux halo
	FragColor = farHalo*max(1, cmag+0.1) + nearHalo; //0.1 afin que l'on voit encore farHalo
}



//~ //
//~ //	sun_big_halo
//~ //
//~ #version 420
//~ #pragma debug(on)
//~ #pragma optimize(off)


//~ uniform float cmag;
//~ uniform vec3 color;
//~ uniform vec2 center;
//~ uniform float radius; //rayon apparent du soleil en px
//~ smooth in vec2 TexCoord;

//~ uniform sampler2D texunit0;

//~ out vec3 FragColor;

//~ //constantes à adapter
//~ float C_RADIUS=1.1; //coefficient liée au radius
//~ float ACCENTUATION1 = 1.1; //pour jouer sur la transparance
//~ float ACCENTUATION2 = 0.7; //je ne sais pas a quoi cela sert


//~ void main(void)
//~ {
	//~ // calcul du halo de loin
	//~ vec3 tex_color = vec3(texture(texunit0,TexCoord)).rgb;
	//~ vec3 farHalo = vec3(tex_color.r * color.r, tex_color.g * color.g, tex_color.b * color.b);
	
	//~ // calcul du halo au centre 
	//~ vec3 nearHalo;
	//~ vec2 pos = gl_FragCoord.xy-center;
	//~ float posPixel = sqrt(dot(pos,pos));
	//~ if (posPixel < C_RADIUS * radius)
		//~ nearHalo= vec3(color * min(1.0, max(0.0,ACCENTUATION2*(1.0-ACCENTUATION1 *posPixel/(C_RADIUS*radius)))));
	//~ else
		//~ nearHalo = vec3(0.0);

	//~ // assemblage des deux halo
	//~ FragColor = farHalo*max(1, cmag+0.1) + nearHalo; //0.1 afin que l'on voit encore farHalo
//~ }
