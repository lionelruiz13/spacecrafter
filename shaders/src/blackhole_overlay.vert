//
// black hole screen overlay
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location=0) in vec2 pos;

layout(location=0) out vec2 posOut;

void main()
{
    posOut = pos;
}
