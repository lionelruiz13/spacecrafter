//
// Volumetric 3D object
//
#version 420

layout (location=0) in vec3 direction;
layout (location=1) in vec3 texCoord;

layout (location=0) out vec4 fragColor;

layout (binding=1) uniform ubo {
    vec3 texCoef; // Coefficient for the texture
    float rayPoints; // Number of points sampled for a ray of length 1
    vec3 rayCoef; // Coefficient of ray length per axis
};

layout (binding=2) uniform sampler3D mapTexture;
layout (binding=3) uniform sampler3D colorTexture;

layout (constant_id=0) const float colorScale = 2;

void main()
{
    const vec3 tmp = (step(vec3(0.f), direction) - texCoord) / direction; // It would be better to replace step by OpSignBitSet of direction in SPIR-V assembly

    const float coefNormalize = min(min(tmp.x, tmp.y), tmp.z);      // direction * coefNormalize = ray
    vec3 ray = direction * coefNormalize;
    const float t_max = max(floor(length(ray * rayCoef) * rayPoints), 1);
    ray = ray * texCoef / t_max;

    float opacity = 0;
    vec3 coord = texCoord * texCoef + ray * 0.5f;
    vec4 color = vec4(0);
    for (float t = 0.5f; t < t_max; t += 1.f) {
        float localOpacity = texture(mapTexture, coord).x / colorScale * (1.f - opacity);
        color += localOpacity * texture(colorTexture, coord);
        opacity += localOpacity;
        if (opacity > 0.99f) { // Check if it increase or reduce preformances
            color /= opacity;
            color.a = 1.f;
            fragColor = color;
            return;
        }
        coord += ray;
    }
    color /= opacity;
    color.a = opacity;
    fragColor = color;
}
