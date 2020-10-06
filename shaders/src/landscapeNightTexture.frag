//
// landscape
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0, set=1) uniform sampler2D texunit0;
layout (binding=1, set=1) uniform sampler2D texunit1;

//entrée
layout (binding=3, set=1) uniform uFrag {
	float sky_brightness;
	float fader;
};
//uniform bool haveNightTex;

//sortie
layout (location=0) out vec4 FragColor;

// global
//vec4 tex_color;

// structure
layout (location=0) in vec2 TexCoord;

void main(void)
{
	vec4 tex_color1 = vec4(texture(texunit0,TexCoord)).rgba;
	vec4 tex_color2 = vec4(texture(texunit1,TexCoord)).rgba;
	vec4 tex_color;
	tex_color.rgb = sky_brightness * tex_color1.rgb + 4*(0.25-sky_brightness)* tex_color2.rgb;
	tex_color.a = max (tex_color1.a , tex_color2.a);

	if (tex_color.a == 0.0)
		discard;

	tex_color.a = tex_color.a * fader;
	FragColor = tex_color;
}

