#define M_PI 3.14159265358979323846

vec2 fisheye2DNoMV(vec3 pos, in float fov)
{
	float rq = pos.x*pos.x+pos.y*pos.y;
    float depth = sqrt(rq + pos.z*pos.z);
	rq = sqrt(rq);

	float f = asin(rq/depth);
	if (pos.z > 0)
		f = M_PI - f;
	f /= rq * fov;
    return vec2(pos) * f;
}
