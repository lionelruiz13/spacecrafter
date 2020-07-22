//////////////////// PROJECTION FISHEYE ////////////////////////////////

#define M_PI 3.14159265358979323846

//uniform mat4 inverseModelViewProjectionMatrix; // unused now
uniform mat4 ModelViewMatrix;
//uniform ivec4 viewport; // unused now
//uniform vec3 viewport_center; // unused now
uniform vec3 clipping_fov;

//~ uniform float zNear;
//~ uniform float zFar;
//~ uniform float fov;

vec4 custom_project(vec4 invec)
{
	float zNear=clipping_fov[0];
	float zFar=clipping_fov[1];
	float fov=clipping_fov[2];

	float fisheye_scale_factor = 1.0/fov*180.0/M_PI*2.0; // same for all vertex

    vec4 win = invec;
    win=ModelViewMatrix*win;
    win[3]=0.0;
	
	float depth = length(win);

    float rq1 = win.x*win.x+win.y*win.y;

	if (rq1 <= 0.0 ) {
		win.x = 0.;
		win.y = 0.;
		win.z = (win.z < 0.) ? 0. : -1e30;
		win.w = 0.;
		return win;
	}

	float oneoverh = 1.0/sqrt(rq1);
	float a = M_PI/2.0 + atan(win.z*oneoverh);
	float f = a * fisheye_scale_factor;

	f *= oneoverh;

	win.x = win.x * f;
	win.y = win.y * f;

	win.z = 2. * (abs(depth) - zNear) / (zFar-zNear) - 1.; // on pourrait calculer globalement 2/(zFar-zNear)

	win.w = (a<0.9*M_PI) && (win.z != 0.0) ? 1. : 0.;

    return win;
}

vec4 posToFisheye(vec3 pos)
{
	return custom_project(vec4(pos, 1.));
}
