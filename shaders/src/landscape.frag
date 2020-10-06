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
	vec4 tex_color = vec4(texture(texunit0,TexCoord)).rgba;
	tex_color.rgb = sky_brightness * tex_color.rgb;

	if (tex_color.a == 0.0)
		discard;

	tex_color.a = tex_color.a * fader;
	FragColor = tex_color;
}
