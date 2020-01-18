//
//	ART
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D mapTexture;
uniform float Intensity;
uniform vec3 Color;

//~ smooth in vec2 TexCoord;

in Interpolators
{
	//~ float intensity;
	vec2 TexCoord;
} interData;

out vec4 FragColor;

void main(void)
{
	vec3 textureColor = vec3(texture(mapTexture,interData.TexCoord)).rgb;

	textureColor.r *= Color.r;
	textureColor.g *= Color.g;
	textureColor.b *= Color.b;
	
	textureColor *= Intensity;
	FragColor = vec4(textureColor, 1.0);
	//~ FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}

//~ in Interpolators
//~ {
	//~ float intensity;
	//~ vec2 TexCoord;
//~ } interData;


//~ uniform sampler2D mapTexture;

//~ out vec4 FragColor;

//~ void main(void)
//~ {
	//~ vec4 tex_color = vec4(texture(mapTexture,interData.TexCoord)).rgba;
	//~ tex_color *= interData.intensity;
	//~ FragColor = tex_color;
//~ }
