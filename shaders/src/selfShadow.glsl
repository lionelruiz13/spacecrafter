layout(constant_id=0) const float depthTextureSize = 4096;
layout(constant_id=1) const float biaisFactor = 1.42f; // sqrt(2)

// using sampler2DShadow shadowMap;

float computeEnlightment(vec3 pos, float lightDotNormal) {
    vec2 subPos = fract(pos.xy * depthTextureSize);
    vec4 interpCoef = vec4(subPos, 1 - subPos);
    pos -= vec3((subPos - 0.5f)/depthTextureSize, biaisFactor/depthTextureSize);
    return texture(shadowMap, pos);
}
