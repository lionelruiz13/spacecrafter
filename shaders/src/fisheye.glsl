//////////////////// PROJECTION FISHEYE ////////////////////////////////
#extension GL_KHR_shader_subgroup_vote : enable

#define M_PI 3.14159265358979323846

double my_atan(double x)
{
    double a, z, p, r, s, q, o;
    /* argument reduction:
       arctan (-x) = -arctan(x);
       arctan (1/x) = 1/2 * pi - arctan (x), when x > 0
    */
    z = abs (x);
    a = (z > 1.0) ? 1.0 / z : z;
    /* evaluate minimax polynomial approximation */
    s = a * a; // a**2
    q = s * s; // a**4
    o = q * q; // a**8
    /* use Estrin's scheme for low-order terms */
    p = fma (fma (fma (0.0000202585530444381072727802473032454599888296797871589660644531250, s,0.000223022403457582792975222307774174623773433268070220947265625), q,
                  fma (-0.00116407177799304783517853056906687925220467150211334228515625, s, 0.0038559749383629666162620619473955230205319821834564208984375)), o,
             fma (fma (-0.00918455921871650336762993305228519602678716182708740234375, s, 0.016978035834597275666180138387062470428645610809326171875), q,
                  fma (-0.02582679681449594200071118166306405328214168548583984375, s,0.034067811082715081238969645482939085923135280609130859375 )));
    /* use Horner's scheme for high-order terms */
    p = fma (fma (fma (fma (fma (fma (fma (fma (fma (fma (fma (fma (p, s,
        -0.040926382420509950510467689355209586210548877716064453125), s,
        0.0467394961991579871440904980772756971418857574462890625), s,
        -0.052392330054601317368412338737471145577728748321533203125), s,
        0.058773077721790849270444567764570820145308971405029296875), s,
        -0.06665860363351257256159243524962221272289752960205078125), s,
        0.07692212930586783681263796097482554614543914794921875), s,
        -0.090909012354005225287068014949909411370754241943359375), s,
        0.11111110678749423763544967869165702722966670989990234375), s,
        -0.14285714271334815084202318757888861000537872314453125), s,
        0.199999999997550192976092375829466618597507476806640625), s,
        -0.3333333333333186043745399729232303798198699951171875) * s, a, a);
    /* back substitution based on argument reduction */
    r = (z > 1.0) ? (1.5707963267948965579989817342720925807952880859375- p) : p;
	//~ return copysign (r, x);
    return (x>=0) ? r : -r;
}

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

	if (subgroupAny((clipping_fov[2] < 0.03f) || (win.z > 5.f))) {  // Don't use both atan and my_atan in a subgroup, it would reduce performances
		a = float(my_atan(win.z));
	} else {
		a = atan(win.z);
	}
	float f = fma(a, 1.f/M_PI, 0.5f) * fisheye_scale_factor;
    depth = (fisheye_scale_factor > 1.3 && a > M_PI*0.4f) ? -1e30 : (depth - zNear) / (zFar-zNear);
    return vec4(win.x * f, win.y * f, depth, 1.);
}
