#version 450

// layout (binding=1) uniform sampler2D colorimetry;

layout (location=0) flat in vec3 color;
layout (location=1) smooth in float alpha;

layout (location=0) out vec4 fragColor;

void main()
{
    fragColor = vec4(color, alpha);
    // fragColor = texture(colorimetry, vec2(dot(normal, viewDirection), timeOffset));
}
