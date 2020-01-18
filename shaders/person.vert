// person

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location = 0) in vec3 Position;

void main(void)
{
	gl_Position = vec4(Position,1.0);
}
