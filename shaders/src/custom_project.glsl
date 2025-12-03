#define M_PI 3.14159265358979323846

// Current projectionType, using specialization constant for better optimization ("compiled" at pipeline creation)
// Fisheye = 0, Allsphere = 1, Ekisolid = 2, Aspheric = 3
layout(constant_id = 8) const int projectionType = 0;

// ================================ STANDARD =================================
// FISHEYE PROJECTION
vec4 fisheyeProjectCustom(vec3 invec, mat4 ModelViewMatrix, vec3 clipping_fov)
{
	vec4 win = ModelViewMatrix * vec4(invec, 1);
	float rq = win.x*win.x+win.y*win.y;
    float depth = sqrt(rq + win.z*win.z);
	rq = sqrt(rq);

    float f = asin(min(rq/depth, 1)); // min patch a driver bug were rq/depth > 1
	if (win.z > 0)
		f = M_PI - f;
	f /= rq * clipping_fov.z;
    depth = (depth - clipping_fov.x) / (clipping_fov.y - clipping_fov.x);
    return vec4(win.x * f, win.y * f, depth, 1.);
}

// ALLSPHERE PROJECTION
vec4 allsphereProjectCustom(vec3 invec, mat4 ModelViewMatrix, vec3 clipping_fov)
{
	vec4 win = ModelViewMatrix * vec4(invec, 1);
	float rq = win.x*win.x+win.y*win.y;
    float depth = sqrt(rq + win.z*win.z);
	rq = sqrt(rq);

    float f = asin(min(rq/depth, 1)); // min patch a driver bug were rq/depth > 1
	if (win.z > 0)
		f = M_PI - f;

	// Allsphere distortion - high precision polynomial from advanced version
	// Normalize input by FOV BEFORE polynomial
	f = (f / clipping_fov.z) * 1200.0;
	f = (((((((((-1.553958085e-26*f + 1.430207232e-22)*f -4.958391394e-19)*f + 8.938737084e-16)*f -9.39081162e-13)*f + 5.979121144e-10)*f -2.293161246e-7)*f + 4.995598119e-5)*f -5.508786926e-3)*f + 1.665135788)*f + 6.526610628e-2;
	f = f / 1200.0;

	f /= rq;
    depth = (depth - clipping_fov.x) / (clipping_fov.y - clipping_fov.x);
    return vec4(win.x * f, win.y * f, depth, 1.);
}

// EKISOLID PROJECTION - TODO: Remplacer par la vraie formule
vec4 ekisolidProjectCustom(vec3 invec, mat4 ModelViewMatrix, vec3 clipping_fov)
{
	// Pour l'instant, identique à FISHEYE
	return fisheyeProjectCustom(invec, ModelViewMatrix, clipping_fov);
}

// ASPHERIC PROJECTION - TODO: Remplacer par la vraie formule
vec4 asphericProjectCustom(vec3 invec, mat4 ModelViewMatrix, vec3 clipping_fov)
{
	// Pour l'instant, identique à FISHEYE
	return fisheyeProjectCustom(invec, ModelViewMatrix, clipping_fov);
}

// PROJECTION DISPATCHING based on projectionType uniform from cam_block
vec4 custom_project(vec3 invec, mat4 ModelViewMatrix, vec3 clipping_fov)
{
	// Dispatch to the appropriate projection based on projectionType
	// 0 = FISHEYE, 1 = ALLSPHERE, 2 = EKISOLID, 3 = ASPHERIC
	// Using switch for better optimization (uniform branching)
	switch(projectionType) {
		case 1: return allsphereProjectCustom(invec, ModelViewMatrix, clipping_fov);
		case 2: return ekisolidProjectCustom(invec, ModelViewMatrix, clipping_fov);
		case 3: return asphericProjectCustom(invec, ModelViewMatrix, clipping_fov);
		default: return fisheyeProjectCustom(invec, ModelViewMatrix, clipping_fov); // case 0 or fallback
	}
}

// Overload for vec4 input
vec4 custom_project(vec4 invec, mat4 ModelViewMatrix, vec3 clipping_fov)
{
	return custom_project(invec.xyz, ModelViewMatrix, clipping_fov);
}

// ================================ STANDARD CLAMPED =================================
// Clamped version (clamps depth to [0,1])
vec4 custom_projectClamped(vec3 invec, mat4 ModelViewMatrix, vec3 clipping_fov)
{
	vec4 result = custom_project(invec, ModelViewMatrix, clipping_fov);
	result.z = clamp(result.z, 0.0, 1.0);
	return result;
}

vec4 custom_projectClamped(vec4 invec, mat4 ModelViewMatrix, vec3 clipping_fov)
{
	return custom_projectClamped(invec.xyz, ModelViewMatrix, clipping_fov);
}

// ================================ NO MODELVIEW VERSION =================================
// NoMV version (without ModelView transformation - position already transformed)
vec4 fisheyeProjectCustomNoMV(vec3 win, vec3 clipping_fov)
{
	float rq = win.x*win.x+win.y*win.y;
    float depth = sqrt(rq + win.z*win.z);
	rq = sqrt(rq);

	float f = asin(min(rq/depth, 1)); // min patch a driver bug were rq/depth > 1
	if (win.z > 0)
		f = M_PI - f;
	f /= rq * clipping_fov.z;
    depth = (depth - clipping_fov.x) / (clipping_fov.y - clipping_fov.x);
    return vec4(win.x * f, win.y * f, depth, 1.);
}

vec4 allsphereProjectCustomNoMV(vec3 win, vec3 clipping_fov)
{
	float rq = win.x*win.x+win.y*win.y;
    float depth = sqrt(rq + win.z*win.z);
	rq = sqrt(rq);

	float f = asin(min(rq/depth, 1));
	if (win.z > 0)
		f = M_PI - f;

	// Allsphere distortion - high precision polynomial
	// Normalize input by FOV BEFORE polynomial
	f = (f / clipping_fov.z) * 1200.0;
	f = (((((((((-1.553958085e-26*f + 1.430207232e-22)*f -4.958391394e-19)*f + 8.938737084e-16)*f -9.39081162e-13)*f + 5.979121144e-10)*f -2.293161246e-7)*f + 4.995598119e-5)*f -5.508786926e-3)*f + 1.665135788)*f + 6.526610628e-2;
	f = f / 1200.0;

	f /= rq;
    depth = (depth - clipping_fov.x) / (clipping_fov.y - clipping_fov.x);
    return vec4(win.x * f, win.y * f, depth, 1.);
}

vec4 ekisolidProjectCustomNoMV(vec3 win, vec3 clipping_fov)
{
	return fisheyeProjectCustomNoMV(win, clipping_fov);
}

vec4 asphericProjectCustomNoMV(vec3 win, vec3 clipping_fov)
{
	return fisheyeProjectCustomNoMV(win, clipping_fov);
}

vec4 custom_projectNoMV(vec3 win, vec3 clipping_fov)
{
	switch(projectionType) {
		case 1: return allsphereProjectCustomNoMV(win, clipping_fov);
		case 2: return ekisolidProjectCustomNoMV(win, clipping_fov);
		case 3: return asphericProjectCustomNoMV(win, clipping_fov);
		default: return fisheyeProjectCustomNoMV(win, clipping_fov);
	}
}

// =============================== 2D VERSION =================================
// 2D versions (return vec4 with z=0, w=1)
vec4 fisheye2DCustom(vec4 win, mat4 ModelViewMatrix, float fov)
{
	win = ModelViewMatrix * win;
	float rq = win.x*win.x+win.y*win.y;
    float depth = sqrt(rq + win.z*win.z);
	rq = sqrt(rq);

	float f = asin(min(rq/depth, 1)); // min patch a driver bug were rq/depth > 1
	if (win.z > 0)
		f = M_PI - f;
	f /= rq * fov;
    return vec4(win.x * f, win.y * f, 0, 1.);
}

vec4 allsphere2DCustom(vec4 win, mat4 ModelViewMatrix, float fov)
{
	win = ModelViewMatrix * win;
	float rq = win.x*win.x+win.y*win.y;
    float depth = sqrt(rq + win.z*win.z);
	rq = sqrt(rq);

	float f = asin(min(rq/depth, 1));
	if (win.z > 0)
		f = M_PI - f;

	// Allsphere distortion - high precision polynomial
	// Normalize input by FOV BEFORE polynomial
	f = (f / fov) * 1200.0;
	f = (((((((((-1.553958085e-26*f + 1.430207232e-22)*f -4.958391394e-19)*f + 8.938737084e-16)*f -9.39081162e-13)*f + 5.979121144e-10)*f -2.293161246e-7)*f + 4.995598119e-5)*f -5.508786926e-3)*f + 1.665135788)*f + 6.526610628e-2;
	f = f / 1200.0;

	f /= rq;
    return vec4(win.x * f, win.y * f, 0, 1.);
}

vec4 ekisolid2DCustom(vec4 win, mat4 ModelViewMatrix, float fov)
{
	return fisheye2DCustom(win, ModelViewMatrix, fov);
}

vec4 aspheric2DCustom(vec4 win, mat4 ModelViewMatrix, float fov)
{
	return fisheye2DCustom(win, ModelViewMatrix, fov);
}

vec4 custom_project2D(vec4 win, mat4 ModelViewMatrix, float fov)
{
	switch(projectionType) {
		case 1: return allsphere2DCustom(win, ModelViewMatrix, fov);
		case 2: return ekisolid2DCustom(win, ModelViewMatrix, fov);
		case 3: return aspheric2DCustom(win, ModelViewMatrix, fov);
		default: return fisheye2DCustom(win, ModelViewMatrix, fov);
	}
}

// ============================== 2D NO MODELVIEW VERSION =================================
// 2D NoMV version (no ModelView transformation)
vec4 fisheye2DCustomNoMV(vec3 win, float fov)
{
	float rq = win.x*win.x+win.y*win.y;
    float depth = sqrt(rq + win.z*win.z);
	rq = sqrt(rq);

	float f = asin(min(rq/depth, 1));
	if (win.z > 0)
		f = M_PI - f;
	f /= rq * fov;
    return vec4(win.x * f, win.y * f, 0, 1.);
}

vec4 allsphere2DCustomNoMV(vec3 win, float fov)
{
	float rq = win.x*win.x+win.y*win.y;
    float depth = sqrt(rq + win.z*win.z);
	rq = sqrt(rq);

	float f = asin(min(rq/depth, 1));
	if (win.z > 0)
		f = M_PI - f;

	// Allsphere distortion - high precision polynomial
	// Normalize input by FOV BEFORE polynomial
	f = (f / fov) * 1200.0;
	f = (((((((((-1.553958085e-26*f + 1.430207232e-22)*f -4.958391394e-19)*f + 8.938737084e-16)*f -9.39081162e-13)*f + 5.979121144e-10)*f -2.293161246e-7)*f + 4.995598119e-5)*f -5.508786926e-3)*f + 1.665135788)*f + 6.526610628e-2;
	f = f / 1200.0;

	f /= rq;
    return vec4(win.x * f, win.y * f, 0, 1.);
}

vec4 ekisolid2DCustomNoMV(vec3 win, float fov)
{
	return fisheye2DCustomNoMV(win, fov);
}

vec4 aspheric2DCustomNoMV(vec3 win, float fov)
{
	return fisheye2DCustomNoMV(win, fov);
}

vec4 custom_project2DNoMV(vec3 win, float fov)
{
	switch(projectionType) {
		case 1: return allsphere2DCustomNoMV(win, fov);
		case 2: return ekisolid2DCustomNoMV(win, fov);
		case 3: return aspheric2DCustomNoMV(win, fov);
		default: return fisheye2DCustomNoMV(win, fov);
	}
}
