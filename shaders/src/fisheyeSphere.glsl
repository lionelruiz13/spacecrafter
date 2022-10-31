//////////////////// PROJECTION FISHEYE ////////////////////////////////
#extension GL_KHR_shader_subgroup_vote : enable

#define M_PI 3.14159265358979323846

#include <my_atan.glsl>

// note: win.w != 1 tell us that point is behind us
// note: win.w == 1 tell us that point is front us
vec4 fisheyeProject(vec3 invec, vec3 clipping_fov)
{
	float zNear=clipping_fov[0];
	float zFar=clipping_fov[1];
	float fisheye_scale_factor = 360./clipping_fov[2];

    vec4 win = ModelViewMatrix * vec4(invec,1.);

    float rq1 = win.x*win.x+win.y*win.y;

    float depth = sqrt(rq1 + win.z*win.z);

	win /= sqrt(rq1)+1e-30; // Don't divide by zero
	float a;

	if (usingDouble && subgroupAny((clipping_fov[2] < 0.03f) || (win.z > 5.f))) {  // Don't use both atan and my_atan in a subgroup, it would reduce performances
		a = float(my_atan(win.z));
	} else {
		a = atan(win.z);
	}
	float f = fma(a, 1.f/M_PI, 0.5f) * fisheye_scale_factor;
    if (allsphere) {
        	// Realtime ALLSPHERE distorsion
        	f = f *1200.f;
        	f = (((((((((-1.553958085e-26*f + 1.430207232e-22)*f -4.958391394e-19)*f + 8.938737084e-16)*f -9.39081162e-13)*f + 5.979121144e-10)*f -2.293161246e-7)*f + 4.995598119e-5)*f -5.508786926e-3)*f + 1.665135788)*f + 6.526610628e-2;
        	f = f/1200.f;
    }
    depth = (fisheye_scale_factor > 1.3 && a > M_PI*0.4f) ? -1e30 : (depth - zNear) / (zFar-zNear);
    return vec4(win.x * f, win.y * f, depth, 1.);
}
