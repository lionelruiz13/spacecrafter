//
//	VIDEO PLAYER
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D texunit0;

smooth in vec2 TexCoord;
uniform bool transparency;
uniform vec4 noColor;
uniform float fader;
out vec4 FragColor;
 
void main(void)
{
	vec3 tex_color = vec3(texture(texunit0,TexCoord)).rgb;

	if (transparency) {
		float delta = noColor.a;
		if (noColor.r-delta < tex_color.r && tex_color.r <noColor.r+delta)
			if (noColor.b-delta < tex_color.b && tex_color.b <noColor.b+delta)
				if (noColor.g-delta < tex_color.g && tex_color.g <noColor.g+delta)
					discard;
		}

	FragColor = vec4(tex_color, fader);
}
