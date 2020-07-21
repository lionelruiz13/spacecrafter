#version 420
#pragma debug(on)
#pragma optimize(off)

#define M_PI 3.14159265358979323846

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 MV;
uniform vec3 clipping_fov;

in vec3 Color[];
out vec3 Coloring;

vec4 custom_project(vec4 invec)
{
	float zNear=clipping_fov[0];
	float zFar=clipping_fov[1];
	float fov = clipping_fov[2];

	float fisheye_scale_factor = 1.0/fov*180.0/M_PI*2.0;

    vec4 win = invec;
    win=MV*win;
    win[3]=0.0;
	
	float depth = length(win);

    float rq1 = win.x*win.x+win.y*win.y;

	if (rq1 <= 0.0 ) {
		if (win.z < 0.0) {
			win.x = 0.;
			win.y = 0.;
			win.z = 1.0;
			win.w=0.0;
			return win;
		}
		win.x = 0.;
		win.y = 0.;
		win.z = -1e30;
		win.w = 0.0;
		return win;
	}
	else{
        float oneoverh = 1.0/sqrt(rq1);
        float a = M_PI/2.0 + atan(win.z*oneoverh);
        float f = a * fisheye_scale_factor;

        f *= oneoverh;

        win.x = win.x * f;
        win.y = win.y * f;

        win.z = (abs(depth) - zNear) / (zFar-zNear);

		win.w = (a<0.9*M_PI) ? 1. : 0.;
        return win;
	}
}

vec4 custom_unproject(vec4 pos)
{  
	vec4 unproj_vec=vec4(pos.x,
						pos.y,
						  2.0*pos.z-1.0,
						  1.0);

	//unproj_vec = invMVP * unproj_vec;
	//if (unproj_vec.z==0.0)
	//	return vec4(0.0);
		
	//unproj_vec.w=1.0/unproj_vec.w;
	//unproj_vec.x=unproj_vec.x*unproj_vec.w;
	//unproj_vec.y=unproj_vec.y*unproj_vec.w;
	//unproj_vec.z=unproj_vec.z*unproj_vec.w;

	return unproj_vec;
}

void main()
{
	vec4 pos1, pos2, pos3;

	pos1 = custom_project(gl_in[0].gl_Position);
	pos2 = custom_project(gl_in[1].gl_Position);
	pos3 = custom_project(gl_in[2].gl_Position);

	if ( pos1.w==1.0 && pos2.w==1.0 && pos3.w==1.0) {

        Coloring = Color[0];

        gl_Position = custom_unproject(pos1);
        EmitVertex();

        gl_Position = custom_unproject(pos2);
        EmitVertex();

        gl_Position = custom_unproject(pos3);
        EmitVertex();

        EndPrimitive();
    }
}
