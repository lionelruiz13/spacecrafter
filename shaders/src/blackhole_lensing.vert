// Full-screen gravitational lens composition
#version 420

layout(location=0) in vec2 Position;
layout(location=0) out vec2 TexCoord;

void main()
{
    gl_Position = vec4(Position, 0.0, 1.0);
    TexCoord = Position * 0.5 + 0.5;
}
