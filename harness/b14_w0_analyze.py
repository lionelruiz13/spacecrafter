#!/usr/bin/env python3
"""B14-W0 discriminator (INTENT §11.79(a)/D4). Validates the rot_pole_w0 (IAU
prime-meridian) -> rot_rotation_offset conversion the loader now performs.

Reads a dual_dump and, for each test moon (dumped in the hops list):
  1. CONVERSION READOUT: the loader's re.offset (dumped `offset`) vs the offline
     closed-form conversion of the fetched IAU W0 (matches the vecmath.hpp
     operator* semantics; = ModularSystem.cpp resolveRotationFrame).
  2. MERIDIAN vs IAU at 2 dates: reconstruct the rendered prime meridian from
     the dumped (obliquity, ascendingNode, offset) + the file rotation period,
     and compare (about the pole) to the IAU formula W(t)=W0+Wdot*d evaluated on
     the fetched pole+PM (the eval_poles.py reproducible pattern). Retrograde
     Wdot sign is TAKEN FROM THE SOURCE (pck00011), not assumed.
The discriminator FAILS if raw (unconverted) W0 were written: reported as the
node-difference C = converted - raw.

pck00011.tpc constants (fetched, md5 3c0bdc01) for the 3 dumped test moons; the
file poles are the J2000-evaluated values actually loaded (what the app renders).

Usage: b14_w0_analyze.py <d1.json> <d2.json>
"""
import json, math, sys
import numpy as np
d2r = math.pi/180.0
def Ax(a): c,s=math.cos(a),math.sin(a); return np.array([[1,0,0],[0,c,-s],[0,s,c]])
def Az(a): c,s=math.cos(a),math.sin(a); return np.array([[c,-s,0],[s,c,0],[0,0,1]])
def Axz(x,z): return Az(z)@Ax(x)                       # xzrotation(x,z) operator* action
M = Ax(-23.4392803055555555556*d2r) @ Az(0.0000275*d2r)  # mat_j2000_to_vsop87
def s2r(l,b): return np.array([math.cos(l)*math.cos(b), math.sin(l)*math.cos(b), math.sin(b)])

# name: (pole_ra_file, pole_de_file, W0_pck, Wdot_pck, period_hours_file)
# period_hours_file updated to the B14-periode-corrected rot_periode (=8640/Wdot,
# signed; INTENT §11.88). The pre-periode garbage values that produced the §11.86(d)
# residuals (Proteus 1.122852570617394 d-in-h, Puck +0.7625674771168559 d-in-h wrong
# -sign) are recorded there; merid_d2 now collapses to ~0.
TEST = {
 "Iapetus": (318.16,   75.03,   355.2,   4.5379572,     1903.940390),
 "Proteus": (299.2134, 42.4321, 93.38,   320.7654228,   26.935571560613983),
 "Puck":    (257.0919, -15.4127,91.24,  -472.5450690,  -18.28397028517083),
}
J2000 = 2451545.0

def loader_pole(ra0,de0):
    p=M@s2r(ra0*d2r,de0*d2r); ra=math.atan2(p[1],p[0]); de=math.asin(p[2]/np.linalg.norm(p))
    return math.pi/2-de, ra+math.pi/2

def convert_w0(ra0,de0,W0):     # closed form == resolveRotationFrame
    obl,node=loader_pole(ra0,de0)
    n=s2r(ra0*d2r,de0*d2r); Q0=np.array([-math.sin(ra0*d2r),math.cos(ra0*d2r),0.]); P0=np.cross(n,Q0)
    W=W0*d2r; pm=M@(math.cos(W)*Q0+math.sin(W)*P0)
    ex=Axz(obl,node)@np.array([1,0,0.]); ey=Axz(obl,node)@np.array([0,1,0.])
    o=math.degrees(math.atan2(-(pm@ex),(pm@ey)))%360
    return o

def loader_meridian(obl,ascN,offset_deg,jd,period_days):
    axisRot=math.fmod((jd-J2000)/period_days*2*math.pi + offset_deg*d2r, 2*math.pi)
    return Axz(obl,ascN)@Az(axisRot+math.pi/2)@np.array([1,0,0.])

def iau_meridian(ra0,de0,W0,Wdot,jd):
    n=s2r(ra0*d2r,de0*d2r); Q0=np.array([-math.sin(ra0*d2r),math.cos(ra0*d2r),0.]); P0=np.cross(n,Q0)
    W=(W0+Wdot*(jd-J2000))*d2r
    return M@(math.cos(W)*Q0+math.sin(W)*P0)

def angabout(pole,a,b):     # signed angle a->b about pole (deg)
    pole=pole/np.linalg.norm(pole)
    return math.degrees(math.atan2(pole@np.cross(a,b), a@b))

def load_hops(path):
    hops={}; jd=None
    for line in open(path):
        r=json.loads(line)
        if r.get("type")=="header": jd=r["jd"]
        elif r.get("type")=="hops": hops[r["name"]]=r["new"][0]
    return jd,hops

def main(d1,d2):
    jd1,h1=load_hops(d1); jd2,h2=load_hops(d2)
    print(f"dates: d1={jd1} (d={jd1-J2000:.1f})  d2={jd2} (d={jd2-J2000:.1f})")
    print(f"{'moon':<9}{'W0_pck':>9}{'convert':>10}{'dumped':>10}{'d_off':>9} | "
          f"{'merid_d1':>10}{'merid_d2':>10}  {'rawErr(C)':>10}")
    allok=True
    for nm,(ra0,de0,W0,Wdot,perH) in TEST.items():
        if nm not in h1: print(f"{nm:<9} NOT IN HOPS"); allok=False; continue
        conv=convert_w0(ra0,de0,W0)
        dumped=h1[nm]["offset"]%360
        doff=((dumped-conv+180)%360)-180
        perD=perH/24.0
        pole=M@s2r(ra0*d2r,de0*d2r)
        res=[]
        for jd,h in ((jd1,h1),(jd2,h2)):
            hd=h[nm]
            lm=loader_meridian(hd["obliquity"],hd["ascendingNode"],hd["offset"],jd,perD)
            im=iau_meridian(ra0,de0,W0,Wdot,jd)
            res.append(angabout(pole,im,lm))
        # C = converted - raw(=W0): what an unconverted write would be off by
        C=((conv-W0+180)%360)-180
        ok = abs(doff)<1e-2 and abs(res[0])<0.05
        allok &= ok
        print(f"{nm:<9}{W0:>9.2f}{conv:>10.3f}{dumped:>10.3f}{doff:>9.2e} | "
              f"{res[0]:>10.4f}{res[1]:>10.4f}  {C:>10.3f}  {'OK' if ok else 'FAIL'}")
    print("\nmerid_d1 = rendered-vs-IAU meridian residual at J2000 (deg; the CONVERSION test)")
    print("merid_d2 = same at +3652 d (adds the file-period-vs-Wdot RATE residual)")
    print("rawErr(C)= node-difference the conversion applies; an UNCONVERTED W0 write")
    print("           would render the meridian off by exactly this (the discriminator)")
    print("RESULT:", "ALL OK" if allok else "FAIL")
    return 0 if allok else 1

if __name__=="__main__":
    sys.exit(main(sys.argv[1],sys.argv[2]))
