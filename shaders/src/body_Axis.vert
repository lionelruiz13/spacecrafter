//
// body_Axis
//

#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (location=0) in vec4 position;

// layout (push_constant) uniform uMVP {mat4 MVP;};

void main()
{
	gl_Position = position;
}
