//
// tully
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

#define M_PI 3.14159265358979323846

layout (points) in;
layout (triangle_strip , max_vertices = 4) out;

in vertexData
{
	float texture;
	float radius;
} vertexIn[];

out Interpolators
{
	vec2 TexCoord;
	float intensity;
} interData;

uniform mat4 Mat;
uniform vec3 camPos;
uniform int nbTextures;

layout (std140) uniform cam_block
{
	ivec4 viewport;
	ivec4 viewport_center;
	vec4 main_clipping_fov;
	mat4 MVP2D;
	float ambient;
	float time;
};


vec4 custom_project(vec4 invec)
{
	float zNear=main_clipping_fov[0];
	float zFar=main_clipping_fov[1];
	float fov=main_clipping_fov[2];

	float fisheye_scale_factor = 1.0/fov*180.0/M_PI*2.0;
	float viewport_center_x=viewport_center[0];
	float viewport_center_y=viewport_center[1];
	float viewport_radius=viewport_center[2];

	vec4 win = invec;
    win = Mat * win;
    win.w = 0.0;

	float depth = length(win);

    float rq1 = win.x*win.x+win.y*win.y;

	if (rq1 <= 0.0 ) {
		if (win.z < 0.0) {
			win.x = viewport_center_x;
			win.y = viewport_center_y;
			win.z = 1.0;
			win.w =-1.0;
			return win;
		}
		win.x = viewport_center_x;
		win.y = viewport_center_y;
		win.z = -1.0;
		win.w = -1.0;
		return win;
	}
	else{
        float oneoverh = 1.0/sqrt(rq1);
        float a = M_PI/2.0 + atan(win.z*oneoverh);
        float f = a * fisheye_scale_factor;

        f *= viewport_radius * oneoverh;

        win.x = viewport_center_x + win.x * f;
        win.y = viewport_center_y + win.y * f;

        win.z = (abs(depth) - zNear) / (zFar-zNear);
        if (a<0.9*M_PI) 
			win.w = 1.0;
        else
			win.w = -1.0;
        return win;
	}
}

void main()
{
	// position de la galaxie
	vec4 pos = custom_project( gl_in[0].gl_Position );
	// distance de la galaxie à la caméra correspond anciennement à d=sqrt((x-a)*(x-a)+(y-b)*(y-b)+(z-c)*(z-c));
	//~ float distance = length(gl_in[0].gl_Position-vec4(camPos, 1.0)); 
	// taille apparente de la galaxie correspond à radiusTully.push_back(.3/(d*scaleTully[i]));
	//~ float radius = 0.3 / (vertexIn[0].scale * distance);
	float radius = vertexIn[0].radius;

	if ((pos.w == 1.0) && (radius>=2)) {
		// TODO : ici intensity fixé à 0.8 car radius >1.0
		//~ float intensity = max(min(radius,0.8), 0.2);
		float intensity = 1.0;
		//~ // en bas à droite
		gl_Position   = MVP2D * ( pos +vec4( radius, -radius, 0.0, 0.0) );
		interData.TexCoord= vec2((vertexIn[0].texture+1)/nbTextures, .0f);
		interData.intensity = intensity;
		EmitVertex();

		//~ // en haut à droite
		gl_Position   = MVP2D * ( pos +vec4( radius, radius, 0.0, 0.0) );
		interData.TexCoord= vec2((vertexIn[0].texture+1)/nbTextures, 1.0f);
		interData.intensity = intensity;
		EmitVertex();    

		// en Bas à gauche
		gl_Position   = MVP2D * ( pos +vec4( -radius, -radius, 0.0, 0.0) );
		interData.TexCoord= vec2(vertexIn[0].texture/nbTextures, 0.0f);
		interData.intensity = intensity;
		EmitVertex();

		// en haut à gauche
		gl_Position   = MVP2D * ( pos +vec4( -radius, radius, 0.0, 0.0) );
		interData.TexCoord= vec2(vertexIn[0].texture/nbTextures, 1.0f);
		interData.intensity = intensity;
		EmitVertex();
		EndPrimitive();
	}
}
