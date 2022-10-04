//
// Draw volumetric object when inside
//
#version 450

// Direction in the volumetric object, with max(abs(direction.x), abs(direction.y), abs(direction.z)) == 1
layout (location=0) in vec3 direction;

layout (location=0) out vec4 fragColor;

layout (binding=1) uniform ubo {
    vec3 camCoord; // Position of the camera in the volumetric object in [0, 1]
    int zScale; // used to double the depth
};

layout (binding=2) uniform sampler3D mapTexture;
layout (binding=3) uniform sampler3D colorTexture;

layout (constant_id=0) const float colorScale = 2;
layout(constant_id=1) const int radius2 = 512*512;
layout(constant_id=2) const int centerX = 512;
layout(constant_id=3) const int centerY = 512;

float absMax(in vec3 v)
{
    return max(max(abs(v.x), abs(v.y)), abs(v.z));
}

void main()
{
    {
        ivec2 tmp = ivec2(gl_FragCoord.x - centerX, gl_FragCoord.y - centerY);
        if (dot(tmp, tmp) > radius2)
            discard; // Avoid processing hidden pixels
    }
    int t;
    {
        vec3 dirCoef = (step(vec3(0.f), direction) - camCoord) / direction;
        t = int(min(min(dirCoef.x, dirCoef.y), dirCoef.z*zScale)); // Distance in samples to the border
        if (t < 1)
            discard;
    }

    vec4 color = vec4(0);
    for (vec3 coord = vec3(camCoord.x, camCoord.y, camCoord.z*zScale); t-- >= 0; coord += direction) {
        color += texture(colorTexture, coord) * (texture(mapTexture, coord).x * ((1.f - color.a) / colorScale));
        if (color.a > 0.99f) { // Avoid processing for less than 1% of the color processing
            fragColor = color / color.a;
            return;
        }
    }
    fragColor = vec4(vec3(color)/color.a, color.a);
}
