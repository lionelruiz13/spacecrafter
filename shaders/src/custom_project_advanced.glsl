#define M_PI 3.14159265358979323846

// Advanced custom projection for nebulaTex.vert
// Requires: Mat (mat4), main_clipping_fov (vec4), viewport_center (ivec4)
// These should be defined by including cam_block before this file
// Returns vec4(viewport_x, viewport_y, normalized_depth, visibility)
// visibility: 1.0 if visible, -1.0 if behind camera

// Current projectionType, using specialization constant for better optimization ("compiled" at pipeline creation)
// Fisheye = 0, Allsphere = 1, Ekisolid = 2, Aspheric = 3
layout(constant_id = 8) const int projectionType = 0;

// ================================ FISHEYE =================================
vec4 fisheyeProjectAdvanced(vec4 invec, vec3 clipping_fov)
{
	float zNear=clipping_fov[0];
	float zFar=clipping_fov[1];
	float fov=clipping_fov[2];

	vec4 win = Mat * invec;
    float rq1 = win.x*win.x+win.y*win.y;
	float depth = sqrt(rq1 + win.z*win.z);

	if (rq1 > 0) {
        rq1 = sqrt(rq1);
        float f = asin(min(rq1/depth, 1)); // min patch a driver bug were rq/depth > 1
        if (win.z > 0)
            f = M_PI - f;
        win.w = mix(-1.0, 1.0, f<0.9*M_PI);
        f /= fov * rq1;

        f *= viewport_center[2];

        win.x = win.x * f + viewport_center[0];
        win.y = win.y * f + viewport_center[1];
	} else {
		win.x = viewport_center[0];
		win.y = viewport_center[1];
		win.w = mix(-1.0, 1.0, win.z < 0);
	}
	win.z = (abs(depth) - zNear) / (zFar-zNear);
	if (win.z == 0.0)
		win.z = -1e30;
	return win;
}

// ================================ ALLSPHERE =================================
vec4 allsphereProjectAdvanced(vec4 invec, vec3 clipping_fov)
{
	float zNear=clipping_fov[0];
	float zFar=clipping_fov[1];
	float fov=clipping_fov[2];

	vec4 win = Mat * invec;
    float rq1 = win.x*win.x+win.y*win.y;
	float depth = sqrt(rq1 + win.z*win.z);

	if (rq1 > 0) {
        rq1 = sqrt(rq1);
        float f = asin(min(rq1/depth, 1));
        if (win.z > 0)
            f = M_PI - f;
        win.w = mix(-1.0, 1.0, f<0.9*M_PI);

        // Allsphere distortion - high precision polynomial
		// Normalize input by FOV BEFORE polynomial
		f = (f / fov) * 1200.0;
        f = (((((((((-1.553958085e-26*f + 1.430207232e-22)*f -4.958391394e-19)*f + 8.938737084e-16)*f -9.39081162e-13)*f + 5.979121144e-10)*f -2.293161246e-7)*f + 4.995598119e-5)*f -5.508786926e-3)*f + 1.665135788)*f + 6.526610628e-2;
        f = f / 1200.f;

        f /= rq1;
        f *= viewport_center[2];

        win.x = win.x * f + viewport_center[0];
        win.y = win.y * f + viewport_center[1];
	} else {
		win.x = viewport_center[0];
		win.y = viewport_center[1];
		win.w = mix(-1.0, 1.0, win.z < 0);
	}
	win.z = (abs(depth) - zNear) / (zFar-zNear);
	if (win.z == 0.0)
		win.z = -1e30;
	return win;
}

// ================================ EKISOLID =================================
vec4 ekisolidProjectAdvanced(vec4 invec, vec3 clipping_fov)
{
	// TODO: Implement EKISOLID projection
	// For now, use fisheye
	return fisheyeProjectAdvanced(invec, clipping_fov);
}

// ================================ ASPHERIC =================================
vec4 asphericProjectAdvanced(vec4 invec, vec3 clipping_fov)
{
	float zNear=clipping_fov[0];
	float zFar=clipping_fov[1];
	float fov=clipping_fov[2];

	vec4 win = Mat * invec;
    float rq1 = win.x*win.x+win.y*win.y;
	float depth = sqrt(rq1 + win.z*win.z);
	float tanHalfFovOver2 = tan(fov * 0.5); // fov already in radians / 2

	if (rq1 > 0) {
        rq1 = sqrt(rq1);
        float f = asin(min(rq1/depth, 1));
        if (win.z > 0)
            f = M_PI - f;
        win.w = mix(-1.0, 1.0, f<0.9*M_PI);

		// Stereographic projection: r = tan(alpha/2) / tan(alpha_max/2)
		f = tan(f * 0.5) / tanHalfFovOver2;

        f /= rq1;
        f *= viewport_center[2];

        win.x = win.x * f + viewport_center[0];
        win.y = win.y * f + viewport_center[1];
	} else {
		win.x = viewport_center[0];
		win.y = viewport_center[1];
		win.w = mix(-1.0, 1.0, win.z < 0);
	}
	win.z = (abs(depth) - zNear) / (zFar-zNear);
	if (win.z == 0.0)
		win.z = -1e30;
	return win;
}

// ================================ MAIN DISPATCHER =================================
vec4 custom_project(vec4 invec, vec3 clipping_fov)
{
	switch(projectionType) {
		case 1: return allsphereProjectAdvanced(invec, clipping_fov);
		case 2: return ekisolidProjectAdvanced(invec, clipping_fov);
		case 3: return asphericProjectAdvanced(invec, clipping_fov);
		default: return fisheyeProjectAdvanced(invec, clipping_fov);
	}
}

vec4 custom_project(vec4 invec)
{
	switch(projectionType) {
		case 1: return allsphereProjectAdvanced(invec, main_clipping_fov.xyz);
		case 2: return ekisolidProjectAdvanced(invec, main_clipping_fov.xyz);
		case 3: return asphericProjectAdvanced(invec, main_clipping_fov.xyz);
		default: return fisheyeProjectAdvanced(invec, main_clipping_fov.xyz);
	}
}
