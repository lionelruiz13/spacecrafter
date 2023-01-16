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

layout (binding=2) uniform sampler3D colorTexture;

layout (constant_id=0) const float colorScale = 2;

void main()
{
    vec3 ray;
    {
        const vec3 tmp = (step(0.f, direction) - texCoord) / direction; // It would be better to replace step by OpSignBitSet of direction in SPIR-V assembly
        const float coefNormalize = min(min(tmp.x, tmp.y), tmp.z);      // direction * coefNormalize = ray
        ray = direction * coefNormalize;
    }

    const float t_max = floor(min(length(ray * rayCoef), 1.5) * rayPoints);
    if (t_max < 0.5) {
        fragColor = vec4(0);
        return;
    }

    ray = ray * texCoef / t_max;
    float opacity = 0;
    vec3 coord = texCoord * texCoef + ray * 0.5f;
    vec3 color = vec3(0);
    for (float t = 0.5f; t < t_max; t += 1.f) {
        vec4 tmp = texture(colorTexture, coord);
        float factor = tmp.a * (1.f - opacity) / colorScale;
        color += vec3(tmp) * factor;
        opacity += factor;
        if (opacity > 0.99f) { // Avoid processing for less than 1% of the color processing
            fragColor = vec4(color / opacity, 1);
            return;
        }
        coord += ray;
    }
    fragColor = vec4(color, opacity);
}
