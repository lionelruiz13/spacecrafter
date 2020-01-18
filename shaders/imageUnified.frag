//
//	imageUnified
//
#version 420
#pragma debug(on)
#pragma optimize(off)


layout (binding=0) uniform sampler2D mapTexture;
uniform float fader;
uniform bool transparency;
uniform vec4 noColor;

smooth in vec2 TexCoord;
 
out vec4 FragColor;

void main(void)
{
	vec4 tex_color = texture(mapTexture,TexCoord);
	tex_color.a *= fader;

	if (transparency) {
		float delta = noColor.a;
		if (noColor.r-delta < tex_color.r && tex_color.r <noColor.r+delta)
			if (noColor.b-delta < tex_color.b && tex_color.b <noColor.b+delta)
				if (noColor.g-delta < tex_color.g && tex_color.g <noColor.g+delta)
					discard;
		}

	FragColor = tex_color;
}
