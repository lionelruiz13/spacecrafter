//
//	sun_big_halo
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout(location=0) in vec2 Center;
layout(location=1) in vec2 TexCoord;
 
layout(binding=2) uniform u1 {float cmag;};
layout(binding=4) uniform u2 {vec3 color;};
layout(binding=3) uniform u3 {float radius;}; //rayon apparent du soleil en px

layout (binding=0) uniform sampler2D texunit0;

layout(location=0) out vec3 FragColor;

//constantes à adapter
float C_RADIUS=1.1; //coefficient liée au radius
float ACCENTUATION1 = 1.1; //pour jouer sur la transparance
float ACCENTUATION2 = 0.7; //je ne sais pas a quoi cela sert

void main(void)
{
	// calcul du halo de loin
	vec3 tex_color = vec3(texture(texunit0, TexCoord)).rgb;
	vec3 farHalo = vec3(tex_color.r * color.r, tex_color.g * color.g, tex_color.b * color.b);
	
	// calcul du halo au centre 
	vec3 nearHalo;
	vec2 pos = gl_FragCoord.xy-Center;
	float posPixel = sqrt(dot(pos,pos));
	if (posPixel < C_RADIUS * radius)
		nearHalo= vec3(color * min(1.0, max(0.0,ACCENTUATION2*(1.0-ACCENTUATION1 *posPixel/(C_RADIUS*radius)))));
	else
		nearHalo = vec3(0.0);

	// assemblage des deux halo
	FragColor = farHalo*max(1, cmag+0.1) + nearHalo; //0.1 afin que l'on voit encore farHalo
}
