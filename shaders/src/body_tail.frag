#version 450

// layout (binding=1) uniform sampler2D colorimetry;

layout (location=0) in vec3 normal;
layout (location=1) in vec3 viewDirection;
layout (location=2) in vec3 color;
layout (location=3) in float timeOffset;

layout (location=0) out vec4 fragColor;

void main()
{
    fragColor = vec4(color, max(-dot(normalize(normal), normalize(viewDirection)) - timeOffset, 0));
    // fragColor = texture(colorimetry, vec2(dot(normal, viewDirection), timeOffset));
}
