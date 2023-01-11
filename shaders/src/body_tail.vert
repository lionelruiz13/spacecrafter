#version 450

layout (push_constant) uniform ubo {
    float fov;
};

// Vertex input
layout (location=0) in vec3 normal;
layout (location=1) in float timeOffset;

// Instance input
layout (location=2) in vec3 offset; // ModelViewMatrix already applied
layout (location=3) in vec3 expandDirection; // ModelViewMatrix already applied
layout (location=4) in vec3 expandCorrection; // ModelViewMatrix already applied
layout (location=5) in vec3 coefRadius;
layout (location=6) in vec3 color;
layout (location=7) in mat3 ModelViewMatrix;

layout (location=0) out vec3 outNormal;
layout (location=1) out vec3 outViewDirection;
layout (location=2) out vec3 outColor;
layout (location=3) out float outTimeOffset;

#include <fisheye2DNoMV.glsl>

void main()
{
    outColor = color;
    outTimeOffset = timeOffset;
    // Transform normals
    vec3 pos = ModelViewMatrix * normal;
    outNormal = pos;
    // Scale normal by radius
    pos *= (coefRadius.x * timeOffset + coefRadius.y) * timeOffset + coefRadius.z;
    // Apply expansion offset
    pos += (expandCorrection * timeOffset + expandDirection) * timeOffset;
    // Apply base offset
    pos += offset;
    // vec3 tmp = normalize(expandDirection);
    // vec3 viewDirection = normalize(pos);
    // outViewDirection = normalize((-dot(tmp, tmp) / dot(viewDirection, tmp)) * viewDirection + tmp);
    outViewDirection = pos;
    // Project final position
    gl_Position = vec4(fisheye2DNoMV(pos, fov), 0, 1);
}
