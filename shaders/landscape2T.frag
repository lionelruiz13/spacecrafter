//
// landscape2T
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D texunit0;
layout (binding=1) uniform sampler2D texunit1;

//entrée
uniform float sky_brightness;
uniform float fader;
//uniform bool haveNightTex;

//sortie
out vec4 FragColor;

// global
vec4 tex_color;

// structure
in G2F
{
	vec2 TexCoord;
} g2f;

// utilisation de fonctions spécialisées
subroutine void parametricColor();
subroutine uniform parametricColor defineColor;

subroutine(parametricColor)
void withoutNightTex()
{
	vec4 tex_color1 = vec4(texture(texunit0,g2f.TexCoord)).rgba;
	tex_color.rgb = sky_brightness * tex_color1.rgb;
	tex_color.a = tex_color1.a;
}

subroutine(parametricColor)
void withNightTex()
{
	vec4 tex_color1 = vec4(texture(texunit0,g2f.TexCoord)).rgba;
	vec4 tex_color2 = vec4(texture(texunit1,g2f.TexCoord)).rgba;
	tex_color.rgb = sky_brightness * tex_color1.rgb + 4*(0.25-sky_brightness)* tex_color2.rgb;
	tex_color.a = max (tex_color1.a , tex_color2.a);
}

void main(void)
{
	defineColor();

	if (tex_color.a == 0.0)
		discard;

	tex_color.a = tex_color.a * fader;
	FragColor = tex_color;
}