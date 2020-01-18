//
// landscape2T
//

#version 420
#pragma debug(on)
#pragma optimize(off)

#define M_PI 3.14159265358979323846

//layout
//~ layout (location=0)in vec3 position;
//~ layout (location=1)in vec2 texcoord;
//~ layout (location=2)in vec3 normal; // useless unitl now

//layout
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

//externe
uniform sampler2D texunit0;
uniform sampler2D texunit1;
uniform float sky_brightness;
uniform float fader;
uniform mat4 ModelViewProjectionMatrix;

//out
//~ smooth out vec2 TexCoord;


in ValueTex
{
	vec2 TexCoord;
} valueTex[];

out ValueTexFrag
{
	vec2 TexCoord;
} valueTexFrag;



//////////////////// PROJECTION FISHEYE ////////////////////////////////

uniform mat4 inverseModelViewProjectionMatrix;
uniform mat4 ModelViewMatrix;
uniform ivec4 viewport; 
uniform vec3 viewport_center;
uniform vec3 clipping_fov;

//~ uniform float zNear;
//~ uniform float zFar;
//~ uniform float fov;

vec4 custom_project(vec4 invec)
{
	float zNear=clipping_fov[0];
	float zFar=clipping_fov[1];
	float fov=clipping_fov[2];

	float fisheye_scale_factor = 1.0/fov*180.0/M_PI*2.0;
	float viewport_center_x=viewport_center[0];
	float viewport_center_y=viewport_center[1];
	float viewport_radius=viewport_center[2];

    vec4 win = invec;
    win=ModelViewMatrix*win;
    win[3]=0.0;
	
	float depth = length(win);

    float rq1 = win.x*win.x+win.y*win.y;

	if (rq1 <= 0.0 ) {
		if (win.z < 0.0) {
			win.x = viewport_center_x;
			win.y = viewport_center_y;
			win.z = 1.0;
			win.w=0.0;
			return win;
		}
		win.x = viewport_center_x;
		win.y = viewport_center_y;
		win.z = -1e30;
		win.w = 0.0;
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
			win.w=1.0;
		else
			win.w=0.0;
        return win;
	}
}

vec4 custom_unproject(vec4 invec, vec4 viewport)
{  
	// gluUnproject    
	vec4 pos = invec;

	vec4 unproj_vec=vec4( (pos.x-viewport.x)/viewport.z*2.0-1.0,
						  (pos.y-viewport.y)/viewport.w*2.0-1.0,
						  2.0*pos.z-1.0,
						  1.0);

	unproj_vec=inverseModelViewProjectionMatrix *unproj_vec;
	if(unproj_vec.z==0.0)
		return vec4(0.0);
		
	unproj_vec.w=1.0/unproj_vec.w;
	unproj_vec.x=unproj_vec.x*unproj_vec.w;
	unproj_vec.y=unproj_vec.y*unproj_vec.w;
	unproj_vec.z=unproj_vec.z*unproj_vec.w;

	return unproj_vec;
}

//~ vec4 posToFisheye(vec3 pos)
//~ {
	//~ return custom_unproject(custom_project(pos), viewport);
//~ }
//////////////////// PROJECTION FISHEYE ////////////////////////////////


void main(void)
{
	vec4 pos1, pos2, pos3;

	pos1 = custom_project(gl_in[0].gl_Position);
	pos2 = custom_project(gl_in[1].gl_Position);
	pos3 = custom_project(gl_in[2].gl_Position);

	if ( pos1.w==1.0 && pos2.w==1.0 && pos3.w==1.0) {

		gl_Position = ModelViewProjectionMatrix * custom_unproject(pos1, viewport);
		valueTexFrag.TexCoord = valueTex[0].TexCoord;
		//~ faderColor.indice = valueFader[0].indice;
		EmitVertex();

		gl_Position = ModelViewProjectionMatrix * custom_unproject(pos2, viewport);
		valueTexFrag.TexCoord = valueTex[1].TexCoord;
		//~ faderColor.indice = valueFader[1].indice;
		EmitVertex();

		gl_Position = ModelViewProjectionMatrix * custom_unproject(pos3, viewport);
		valueTexFrag.TexCoord = valueTex[2].TexCoord;
		//~ faderColor.indice = valueFader[1].indice;
		EmitVertex();

		EndPrimitive();	
	}
}
