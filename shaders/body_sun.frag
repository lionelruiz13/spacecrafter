//
// body sun
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D mapTexture;


vec3 tex_color;
smooth in vec2 TexCoord;
 
out vec4 FragColor;
 
void main(void)
{
	vec3 tex_color = vec3(texture(mapTexture,TexCoord)).rgb;
	FragColor = vec4 (tex_color,1.0);
}




//~ uniform sampler2D mapTexture; 
//~ varying vec2 TexCoord;

//~ void main(void)
//~ {
	//~ vec3 color = vec3(texture(mapTexture, TexCoord));
	//~ gl_FragColor = vec4(color, 1.0);
//~ }
