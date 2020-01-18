//
// skylineMVPDraw
//

#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (location=0)in vec2 position;

uniform mat4 MVP;

void main()
{
	gl_Position = MVP * vec4(position,0.0,1.0);
}
