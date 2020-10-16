//
//	sun_big_halo
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout(location=0) in vec2 Center;
layout(location=1) in vec2 TexCoord;
layout(location=2) in float viewport_y;

layout(binding=2) uniform u1 {float cmag;};
layout(binding=4) uniform u2 {vec3 color;};
layout(binding=3) uniform u3 {float radius;}; //rayon apparent du soleil en px

layout (binding=0) uniform sampler2D texunit0;

layout(location=0) out vec3 FragColor;

//constantes à adapter
const float C_RADIUS=1.1; //coefficient liée au radius
const float ACCENTUATION1 = 1.1; //pour jouer sur la transparance
const float ACCENTUATION2 = 0.7; //je ne sais pas a quoi cela sert

void main(void)
{
	// calcul du halo de loin
	vec3 farHalo = vec3(texture(texunit0, TexCoord)).rgb;
	
	// calcul du halo au centre 
	vec3 nearHalo;
	vec2 pos = vec2(gl_FragCoord.x-Center.x, viewport_y-gl_FragCoord.y-Center.y); // because y axis is inverted
	float posPixel = length(pos);
	nearHalo= vec3(clamp(ACCENTUATION2*(1.0-ACCENTUATION1 *posPixel/(C_RADIUS*radius)), 0.0, 1.0));

	// assemblage des deux halo
	FragColor = color * (farHalo*max(1, cmag+0.1) + nearHalo); //0.1 afin que l'on voit encore farHalo
}
