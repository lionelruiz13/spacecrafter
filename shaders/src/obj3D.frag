//
// cloud 3D
//
#version 420

layout (location=0) in vec3 direction;
layout (location=1) in vec3 texCoord;
layout (location=2) in vec3 data; // {texOffset, coefScale, lod}
//layout (location=3) in float lod;

layout (location=0) out vec4 fragColor;

layout (binding=3) uniform sampler3D mapTexture;
layout (binding=4) uniform sampler2D colorTexture;

layout (constant_id=0) const float maxlod = 8;
layout (constant_id=1) const float texScale = 1;
layout (constant_id=2) const float colorScale = 2;

void main()
{
    const float texOffset = data.x;
    const float lod = max(maxlod + data.z, 0);
    const vec3 tmp = (step(vec3(0.f), direction) - texCoord) / direction; // It would be better to replace step by OpSignBitSet of direction in SPIR-V assembly

    const float coefNormalize = min(min(tmp.x, tmp.y), tmp.z);      // direction * coefNormalize = ray
    const float base_t_max = exp2(maxlod - lod);                    // Value of t_max for a ray of length 1.f
    const float t_max = floor(base_t_max * coefNormalize * data.y); // Value of t_max scaled by the ray length
    vec3 dir = direction * coefNormalize / t_max;             // Ray length / t_max

    float opacity = 0;
    vec3 coord = texCoord + dir * 0.5f;
    coord.x = coord.x * texScale + texOffset;
    dir.x *= texScale;
    float t;
    vec4 color = vec4(0);
    for (t = 0.5f; t < t_max; t += 1.f) {
        float localOpacity = textureLod(mapTexture, coord, lod).x / colorScale * (1.f - opacity);
        color += localOpacity * textureLod(colorTexture, vec2(coord), lod);
        opacity += localOpacity;
        if (opacity > 0.99f) {
            color /= opacity;
            opacity = 1.f;
            break;
        }
        coord += dir;
    }
    color /= opacity;
    color.a = opacity;
    fragColor = color;
}

// When transiting from one mipmap to the next one :
// value = (2. - value) * value

