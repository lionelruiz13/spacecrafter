//
// my earth tessellation
//

#version 450

layout(vertices=3) out;

// All positions are relative to the camera (camPos is at origin) - without projection
layout (binding=0) uniform ubo {
    mat4 ModelViewMatrix;
    vec3 sunPos;
    float planetRadius; // 1.0  ;
    vec3 bodyPos;
    float planetOneMinusOblateness;
    vec3 clipping_fov;
    float atmRadius;    // 1.05 ;
    ivec2 TesParam;     // [min_tes_lvl, max_tes_lvl]
    float atmAlpha;     // 1.0  ; // this value is a scale for atmosphere transparency
};

layout(location=0) in vec3 PositionIn[];
layout(location=1) in vec2 glPositionIn[];

layout(location=0) out vec3 PositionOut[];

void main(void)
{
    PositionOut[gl_InvocationID] = PositionIn[gl_InvocationID];
    if (gl_InvocationID == 0) {
        vec2 center = (glPositionIn[0] + glPositionIn[1] + glPositionIn[2]) / 3;
        float centerDistance = length(center);
        vec2 v1 = glPositionIn[1].xy;
        vec2 v2 = glPositionIn[2].xy;
        {
            vec2 v0 = glPositionIn[0].xy;
            v1 -= v0;
            v2 -= v0;
        }
        vec3 tes = vec3(distance(v1, v2), length(v2), length(v1));
        if (v1.x * v2.y < v1.y * v2.x || centerDistance > (1 + (tes.x + tes.y + tes.z)/2)) {
            gl_TessLevelInner[0]=0;
            gl_TessLevelOuter[0]=0;
            gl_TessLevelOuter[1]=0;
            gl_TessLevelOuter[2]=0;
            return;
        }
        tes = clamp(tes * 128, TesParam[0], TesParam[1]);

        gl_TessLevelOuter[0] = tes[0];
        gl_TessLevelOuter[1] = tes[1];
        gl_TessLevelOuter[2] = tes[2];

        gl_TessLevelInner[0]=(tes[0] + tes[1] + tes[2])/3;
    }
}
