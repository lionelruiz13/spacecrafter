layout(constant_id=0) const float depthTextureSize = 4096;
layout(constant_id=1) const float biaisFactor = 1.42f; // sqrt(2)

// using sampler2D shadowMap;

float computeEnlightment(vec3 pos, float lightDotNormal)
{
    // xyz coordinates must be moved from [-1, 1] to [0, 1]
    pos = pos * 0.5 + 0.5;

    // Compute interpolation factor
    vec2 subPos = fract(pos.xy * depthTextureSize);
    vec4 interpCoef = vec4(subPos, 1 - subPos);

    // Compute biais and fixed sample position
    pos -= vec3((subPos - 0.5f)/depthTextureSize, (-biaisFactor/depthTextureSize)/lightDotNormal);

    // Fetch 4 texels to perform a compuation before the interpolations
    vec4 fetch = textureGather(shadowMap, pos.xy);
    fetch = step(fetch, vec4(pos.z));

    // Compute custom interpolation
    interpCoef = interpCoef.zyxw * interpCoef.yxwz;
    interpCoef = pow(interpCoef, vec4(1.4f));
    interpCoef /= interpCoef.x + interpCoef.y + interpCoef.z + interpCoef.w;

    // Apply the interpolation
    return min(dot(fetch, interpCoef)*1.5f, 1.0f);
}
