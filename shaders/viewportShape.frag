//
//	VIEWPORT SHAPE
//
#version 420
#pragma debug(on)
#pragma optimize(off)

uniform sampler2D texunit0;
uniform int radius;
uniform int decalage_x;
uniform int decalage_y;
//~ smooth in vec2 TexCoord;
 
out vec4 FragColor;
 
void main(void)
{
	vec2 pos = gl_FragCoord.xy-vec2(radius+decalage_x,radius+decalage_y);
	float dist_squared = dot(pos, pos);

	if (dist_squared > radius*radius)
		FragColor = vec4(0.0,0.0,0.0,1.0);
	else
		discard;
}
