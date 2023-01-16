//
// Draw volumetric object when inside
//
#version 450

// Direction in the volumetric object, with max(abs(direction.x), abs(direction.y), abs(direction.z)) == 1
layout (location=0) in vec3 direction;

layout (location=0) out vec4 fragColor;

layout (binding=1) uniform ubo {
    vec3 camCoord; // Position of the camera in the volumetric object in [0, 1]
};

layout (binding=2) uniform sampler3D colorTexture;

layout(constant_id=0) const float colorScale = 2;
layout(constant_id=1) const bool z_reflection = false;

// Viewport cropping
layout(constant_id=2) const int radius2 = 512*512;
layout(constant_id=3) const int centerX = 512;
layout(constant_id=4) const int centerY = 512;

void main()
{
    { // Viewport cropping
        ivec2 tmp = ivec2(gl_FragCoord.x - centerX, gl_FragCoord.y - centerY);
        if (dot(tmp, tmp) > radius2) {
            fragColor = vec4(0);
            return; // Avoid processing hidden pixels
        }
    }
    int t;
    vec3 coord = camCoord;
    if (z_reflection) {
        // Clamp to edge
        vec3 dirCoef = (step(0.f, -direction) - camCoord) / direction;
        float rectification = max(max(dirCoef.x, dirCoef.y), dirCoef.z);
        if (rectification > 0)
            coord += direction * rectification;
        // Ray lenght computation
        dirCoef = (step(0.f, direction) - coord) / direction;
        t = int(min(min(dirCoef.x, dirCoef.y), dirCoef.z)); // Distance in samples to the border
    } else {
        // Ray lenght computation
        vec3 dirCoef = (step(0.f, direction) - camCoord) / direction;
        t = int(min(min(dirCoef.x, dirCoef.y), dirCoef.z)); // Distance in samples to the border
    }

    vec3 dir = direction;
    if (z_reflection) {
        coord.z *= 2;
        dir.z *= 2;
    }
    vec3 color = vec3(0);
    float opacity = 0;
    while (t-- >= 0) {
        vec4 tmp = texture(colorTexture, coord);
        float factor = tmp.a * (1.f - opacity) / colorScale;
        color += vec3(tmp) * factor;
        opacity += factor;
        if (opacity > 0.99f) { // Avoid processing for less than 1% of the color processing
            fragColor = vec4(color / opacity, 1);
            return;
        }
        coord += dir;
    }
    fragColor = vec4(color, opacity);
}
