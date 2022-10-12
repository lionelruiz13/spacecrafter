#define M_PI 3.14159265358979323846

vec2 fisheye2DNoMV(vec3 pos, in float fov)
{
	pos /= sqrt(pos.x*pos.x + pos.y*pos.y) + 1e-30; // Don't divide by zero
	float f = (atan(pos.z) / M_PI + 0.5f) * 360./fov;
	return vec2(pos) * f;
}
