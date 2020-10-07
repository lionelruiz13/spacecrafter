//////////////////// PROJECTION FISHEYE ////////////////////////////////

#define M_PI 3.14159265358979323846

//We need to use the projection fisheye, assume to give vec3 clipping_fov
layout (binding=0) uniform uModelViewMatrix {mat4 ModelViewMatrix;};


// note: win.w != 1 tell us that point is behind us
// note: win.w == 1 tell us that point is front us
vec4 fisheyeProject(vec3 invec, vec3 clipping_fov)
{
	float zNear=clipping_fov[0];
	float zFar=clipping_fov[1];
	float fov=clipping_fov[2];

	float fisheye_scale_factor = 1.0/fov*180.0/M_PI*2.0; // same for all vertex

    vec4 win = vec4(invec,1);
    win=ModelViewMatrix*win;
    win.w=0.0;
	
	float depth = length(win);

    float rq1 = win.x*win.x+win.y*win.y;

	if (rq1 <= 0.0 ) {
		win.x = 0.;
		win.y = 0.;
		win.z = (win.z < 0.) ? 0. : 1e30;
		win.w = 0.;
		return win;
	}

	float oneoverh = 1.0/sqrt(rq1);
	float a = M_PI/2.0 + atan(win.z*oneoverh);
	float f = a * fisheye_scale_factor;

	f *= oneoverh;

	win.x = win.x * f;
	win.y = win.y * f;

	win.z = 1. - 2. * (abs(depth) - zNear) / (zFar-zNear); // on pourrait calculer globalement 2/(zFar-zNear)

	win.w = (a<0.9*M_PI) && (win.z != 0.0) ? 1. : 0.;
	if (win.z == 0.0)
		win.z = 1e30;

    return win;
}
