//
// landscape2T
//

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D texunit0;
layout (binding=1) uniform sampler2D texunit1;
uniform float sky_brightness;
uniform float fader;
uniform bool haveNightTex;

vec4 tex_color;
//~ smooth in vec2 TexCoord;

in ValueTexFrag
{
	vec2 TexCoord;
} valueTexFrag;


out vec4 FragColor;
 
void main(void)
{
	vec4 text_color;
	vec4 tex_color1 = vec4(texture(texunit0,valueTexFrag.TexCoord)).rgba;

	if (!haveNightTex) { //il n'y a qu'une texture qu'on affichetout le temps
		tex_color.rgb = sky_brightness * tex_color1.rgb;
		tex_color.a = tex_color1.a;
	}
	else { //les 2 textures servent, on réalise un mix maison car 0<sky_brightness<0.25
		if (sky_brightness >0.25) {
			tex_color.rgb = sky_brightness * tex_color1.rgb;
			tex_color.a = tex_color1.a;
		}
		else {
			vec4 tex_color2 = vec4(texture(texunit1,valueTexFrag.TexCoord)).rgba;
			tex_color.rgb = sky_brightness * tex_color1.rgb + 4*(0.25-sky_brightness)* tex_color2.rgb;
			tex_color.a = max (tex_color1.a , tex_color2.a);
		}
	}

		if (tex_color.a == 0.0)
			discard;

		tex_color.a = tex_color.a * fader;
		FragColor = tex_color;
}
