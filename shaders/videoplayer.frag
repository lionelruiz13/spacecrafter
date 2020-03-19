//
//	VIDEO PLAYER
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D s_texture_y;
layout (binding=1) uniform sampler2D s_texture_u;
layout (binding=2) uniform sampler2D s_texture_v;

smooth in vec2 TexCoord;
uniform bool transparency;
uniform vec4 noColor;
uniform float fader;
out vec4 FragColor;
 
void main(void)
{
	highp float y = texture2D(s_texture_y, TexCoord).r;  
    highp float u = texture2D(s_texture_u, TexCoord).r - 0.5;  
    highp float v = texture2D(s_texture_v, TexCoord).r - 0.5;  
    highp float r = y +             1.402 * v;  
    highp float g = y - 0.344 * u - 0.714 * v;  
    highp float b = y + 1.772 * u;  
    vec3 tex_color = vec3(r,g,b);  
	//vec3 tex_color = vec3(texture(texunit0,TexCoord)).rgb;

	if (transparency) {
		float delta = noColor.a;
		if (noColor.r-delta < tex_color.r && tex_color.r <noColor.r+delta)
			if (noColor.b-delta < tex_color.b && tex_color.b <noColor.b+delta)
				if (noColor.g-delta < tex_color.g && tex_color.g <noColor.g+delta)
					discard;
		}

	FragColor = vec4(tex_color, fader);
}
