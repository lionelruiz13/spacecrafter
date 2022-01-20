//
// cloud 3D
//
#version 420

layout (location=0) in vec3 direction;
layout (location=1) in vec3 texCoord;
layout (location=2) in vec4 color;
layout (location=3) in float lodFactor;

layout (location=0) out vec4 fragColor;

layout (binding=3) uniform sampler3D mapTexture;

layout (constant_id=0) const float maxlod = 8;
layout (constant_id=1) const float minlod = 0;

void main()
{
    vec3 tmp = (step(vec3(0.f), direction) - texCoord) / direction; // It would be better to replace step by OpSignBitSet of direction in SPIR-V assembly
    const float lod = max(maxlod + lodFactor, minlod);
    //float lod = textureQueryLod(mapTexture, texCoord).x;

    float coef = min(min(tmp.x, tmp.y), tmp.z);
    float t_max = floor(exp2(maxlod - lod));
    vec3 dir = direction * (coef / t_max);

    float opacity = 0;
    vec3 coord = texCoord + dir * 0.5f;
    float t;
    for (t = 0.5f; t < t_max; t += 1.f) {
        opacity += textureLod(mapTexture, coord, lod).x;
        coord += dir;
    }
    fragColor = vec4(vec3(color), opacity * color.a / t_max);
}

