//
//Starting splash
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D mapTexture;

layout(location=0) in vec2 TexCoord;
 
layout(location=0) out vec4 FragColor;

void main(void)
{
	vec3 tex_color = vec3(texture(mapTexture,TexCoord)).rgb;
	// Coloration en rouge, on indique au programme que l'on veut du rouge !
	// couleur R G B A ou A est la transparence
	//FragColor = vec4 (1.0, 0.0, 0.0, 1.0);
	FragColor = vec4 (tex_color,1.0);
}

