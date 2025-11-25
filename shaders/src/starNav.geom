//
// starNav
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

#define M_PI 3.14159265358979323846

layout (points) in;
layout (triangle_strip , max_vertices = 4) out;

layout (location=0) in float mag[1];
layout (location=1) in vec3 color[1];

layout (location=0) out vec2 TexCoord;
layout (location=1) out vec3 TexColor;

layout (binding=1, set=1) uniform uMat {
	mat4 Mat;
};

#include <cam_block_only.glsl>
#include <custom_project.glsl>

void main()
{
	float zNear=main_clipping_fov[0]; // 1.f
	float zFar=main_clipping_fov[1] * 8.f; // 16000.f
	float fov=main_clipping_fov[2];
	float viewport_center_x=viewport_center[0];
	float viewport_center_y=viewport_center[1];
	float viewport_radius=viewport_center[2];

	// Transform position
	vec4 win = Mat * gl_in[0].gl_Position;

	// Project using custom_project2D (handles all projection types)
	vec4 projected = custom_project2D(win, mat4(1.0), fov);

	// Calculate depth and visibility
	float rq1 = win.x*win.x+win.y*win.y;
	float depth = sqrt(rq1 + win.z*win.z);

	// Check visibility (w = 1.0 if visible, -1.0 if behind camera)
	rq1 = sqrt(rq1);
	float f = asin(min(rq1/depth, 1));
	if (win.z > 0)
		f = M_PI - f;
	float visible = (f<0.9*M_PI) ? 1.0 : -1.0;

	// Scale to viewport
	vec4 pos = vec4(
		viewport_center_x + projected.x * viewport_radius,
		viewport_center_y + projected.y * viewport_radius,
		(abs(depth) - zNear) / (zFar-zNear),
		visible
	);

	//test sur le centre afin d'écarter les stars invisibles ou hors caméra
	if (pos.w == 1.0) {
		// en Bas à droite
		gl_Position   = MVP2D * ( pos +vec4( mag[0], -mag[0], 0.0, 0.0) );
                gl_Position.z = max(pos.z * 8.f, 1.f);
		TexCoord= vec2(1.0f, .0f);
		TexColor= color[0];
		EmitVertex();

		// en haut à droite
		gl_Position   = MVP2D * ( pos +vec4( mag[0], mag[0], 0.0, 0.0) );
                gl_Position.z = max(pos.z * 8.f, 1.f);
		TexCoord= vec2(1.0f, 1.0f);
		TexColor= color[0];
		EmitVertex();

		// en Bas à gauche
		gl_Position   = MVP2D * ( pos +vec4( -mag[0], -mag[0], 0.0, 0.0) );
                gl_Position.z = max(pos.z * 8.f, 1.f);
		TexCoord= vec2(0.0f, 0.0f);
		TexColor= color[0];
		EmitVertex();

		// en haut à gauche
		gl_Position   = MVP2D * ( pos +vec4( -mag[0], mag[0], 0.0, 0.0) );
                gl_Position.z = max(pos.z * 8.f, 1.f);
		TexCoord= vec2(0.0f, 1.0f);
		TexColor= color[0];
		EmitVertex();
	}
}
