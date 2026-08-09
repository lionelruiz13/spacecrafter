#include "tools/vecmath.hpp"
#include <cstdio>
#include <cmath>
#include <algorithm>
// Is observedToBodyLocalPos (Camera.hpp:264-274) the inverse of viewMat's
// anchored branch?  Round trip: take a point q in the reference's BODY frame,
// push it through viewMat (the map the renderer consumes), then run the
// expression observedToBodyLocalPos evaluates, and see whether q comes back.
int main(){
    const float lon=1.221730476f, lat=0.523598776f, dist=4.929e-05f, theta=0.7f;
    const Mat4f R = Mat4f::zrotation(0.31f).multiplyFast(Mat4f::xrotation(-0.77f));
    const Mat4f S = Mat4f::zrotation(-theta);
    Mat4f mat{R};
    mat.multiplyTranslation(Vec3f(0,0,-dist));
    mat = mat.multiplyFast(Mat4f::xrotation(lat-M_PI_2)).multiplyFast(Mat4f::zrotation(-lon));
    mat = mat.multiplyFast(S);                       // bound to surface
    const Vec3f q(1.3e-04f, -7.0e-05f, 4.1e-05f);    // a body-frame point
    // observedPos = mat . q   (Vec4 multiply, w=1)
    Vec4f q4(q[0],q[1],q[2],1.f); Vec4f o4 = mat*q4;
    const Vec3f o(o4[0],o4[1],o4[2]);
    // ---- the expression in Camera.hpp:264-274, verbatim -------------------
    Vec3f ret = R.transpose().multiplyWithoutTranslation(o);   // observedToLocalPos
    ret.v[2] -= dist;
    ret = Mat4f::yrotation(lat-M_PI_2).multiplyWithoutTranslation(ret);
    ret = Mat4f::zrotation(-lon).multiplyWithoutTranslation(ret);
    printf("q                 %+.9f %+.9f %+.9f\n", q[0],q[1],q[2]);
    printf("shipped expr      %+.9f %+.9f %+.9f   |err| %.3e\n",
           ret[0],ret[1],ret[2],(ret-q).length());
    // ---- the algebraic inverse of the same composition --------------------
    Vec3f inv = R.transpose().multiplyWithoutTranslation(o);
    inv.v[2] += dist;
    inv = Mat4f::zrotation(lon).multiplyFast(Mat4f::xrotation(M_PI_2-lat)).multiplyWithoutTranslation(inv);
    inv = S.transpose().multiplyWithoutTranslation(inv);
    printf("algebraic inverse %+.9f %+.9f %+.9f   |err| %.3e\n",
           inv[0],inv[1],inv[2],(inv-q).length());
    // and the DIRECTION error, which is what observedPosToRaDe consumes
    printf("angle(shipped,q) %.4f deg ; angle(inverse,q) %.4f deg\n",
        acos(ret.dot(q)/(ret.length()*q.length()))*180/M_PI,
        acos(inv.dot(q)/(inv.length()*q.length()))*180/M_PI);
    // ---- and the FREE branch of the same method ---------------------------
    Vec3f position = Vec3f(cosf(-lon)*cosf(lat), sinf(-lon)*cosf(lat), sinf(lat))*dist;
    for (int bound = 1; bound >= 0; --bound) {
        Mat4f fm{R};
        fm.multiplyTranslation(position);
        if (bound) fm = fm.multiplyFast(S);
        Vec4f fo4 = fm*Vec4f(q[0],q[1],q[2],1.f);
        const Vec3f fo(fo4[0],fo4[1],fo4[2]);
        Vec3f fr = R.transpose().multiplyWithoutTranslation(fo);
        fr -= position;                                   // Camera.hpp:267
        const Vec3f truth = bound ? S.transpose().multiplyWithoutTranslation(fr) : fr;
        printf("free branch bound=%d : |err| %.3e  (truth recovers %.3e)  angle %.4f deg\n",
               bound, (fr-q).length(), (truth-q).length(),
               acos(std::min(1.f,fr.dot(q)/(fr.length()*q.length())))*180/M_PI);
    }
    return 0;
}
