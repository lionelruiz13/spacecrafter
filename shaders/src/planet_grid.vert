//
//	Planet Grid Vertex Shader
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (push_constant) uniform uMat {
	mat4 ModelViewMatrix;
	vec3 clipping_fov;
	float bodyRadius;
	float gridRadius;
	float axisRotation;
};

layout (location=0) in vec3 position;  // Static unit sphere
layout (location=1) in vec3 color;     // Vertex color (meridian/parallel)
layout (location=0) out vec3 pos;
layout (location=1) out vec3 vertColor; // Pass color to fragment shader

#define M_PI 3.14159265358979323846

vec4 fisheyeProjectDebug(vec3 eyePos, vec3 clipping_fov)
{
	float rq = eyePos.x*eyePos.x + eyePos.y*eyePos.y;
    float depth = sqrt(rq + eyePos.z*eyePos.z);
	rq = sqrt(rq);

    float f = asin(min(rq/depth, 1.0));
	if (eyePos.z > 0.0)
		f = M_PI - f;
	f /= rq * clipping_fov.z;
    depth = (depth - clipping_fov.x) / (clipping_fov.y - clipping_fov.x);
    return vec4(eyePos.x * f, eyePos.y * f, depth, 1.0);
}

void main()
{
    // 1. Axial rotation around Z
	float cosRot = cos(axisRotation);
	float sinRot = sin(axisRotation);
	vec3 rotatedPos;
	rotatedPos.x = position.x * cosRot - position.y * sinRot;
	rotatedPos.y = position.x * sinRot + position.y * cosRot;
	rotatedPos.z = position.z;

	// 2. Scaling
	vec3 scaledPos = rotatedPos * (bodyRadius * gridRadius);

    // 3. Matrix transformation
	vec4 eyePos = ModelViewMatrix * vec4(scaledPos, 1.0);

	// 4. Projection
	pos = eyePos.xyz;
	vertColor = color; // Pass color to fragment shader
	gl_Position = fisheyeProjectDebug(eyePos.xyz, clipping_fov);
}