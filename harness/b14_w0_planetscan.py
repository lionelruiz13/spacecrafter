#!/usr/bin/env python3
# W0(IAU, ICRF-equator-node) -> file offset(loader, ecliptic-node) conversion.
# IAU body-fixed frame built by DIRECT axis construction (unambiguous), loader
# frame replicated from vecmath.hpp operator* action matrices.
import numpy as np
d2r=np.pi/180
def Ax(a): c,s=np.cos(a),np.sin(a); return np.array([[1,0,0],[0,c,-s],[0,s,c]])
def Az(a): c,s=np.cos(a),np.sin(a); return np.array([[c,-s,0],[s,c,0],[0,0,1]])
def Axz(x,z): return Az(z)@Ax(x)          # xzrotation(x,z) operator* action
M = Ax(-23.4392803055555555556*d2r) @ Az(0.0000275*d2r)   # mat_j2000_to_vsop87
def s2r(l,b): return np.array([np.cos(l)*np.cos(b),np.sin(l)*np.cos(b),np.sin(b)])

def loader_pole(ra0,de0):
    p=M@s2r(ra0*d2r,de0*d2r)
    ra=np.arctan2(p[1],p[0]); de=np.arcsin(p[2]/np.linalg.norm(p))
    return np.pi/2-de, ra+np.pi/2          # obliquity, ascendingNode (rad)

def loader_surf2ecl(ra0,de0,offset_deg):
    # Empirical: Axz(obl,node) operator* action maps body-eq -> ecl (its 3rd col = pole_ecl).
    # draw: matrix = mat.multiplyFast(computeBodyToSurface) = Axz @ zrotation(getAxisRotation)
    #   => surface -> ecl = Axz(obl,node) @ Az(W_ax)
    obl,node=loader_pole(ra0,de0)
    W_ax=offset_deg*d2r + np.pi/2           # getAxisRotation at epoch (axisRotation+pi/2)
    return Axz(obl,node) @ Az(W_ax)         # surface->ecl (columns = surface axes in ecl)

def iau_bf2ecl(ra0,de0,W0_deg):
    n = s2r(ra0*d2r,de0*d2r)                 # pole in ICRF
    Q0 = np.array([-np.sin(ra0*d2r), np.cos(ra0*d2r), 0.0])  # ascending node of body-eq on ICRF-eq
    b = np.cross(n,Q0)                        # completes RH: x=Q0,y=b,z=n
    W=W0_deg*d2r
    x = np.cos(W)*Q0 + np.sin(W)*b            # prime meridian at W (rotate about pole)
    y = -np.sin(W)*Q0 + np.cos(W)*b
    bf2icrf = np.column_stack([x,y,n])
    return M @ bf2icrf                        # bf->ecl

def solve_offset(ra0,de0,W0):
    # find offset so loader surface x-axis matches IAU prime meridian x-axis (about pole)
    pole = M@s2r(ra0*d2r,de0*d2r); pole/=np.linalg.norm(pole)
    xi = iau_bf2ecl(ra0,de0,W0)[:,0]
    def ang(offset):
        xl = loader_surf2ecl(ra0,de0,offset)[:,0]
        # signed angle from xi to xl about pole
        return np.degrees(np.arctan2(np.dot(pole,np.cross(xi,xl)), np.dot(xi,xl)))
    # linear in offset with slope +/-1; two samples -> solve for ang=0
    a0=ang(0.0); a1=ang(10.0); slope=(a1-a0)/10.0
    off = (-a0/slope) % 360.0
    # pole agreement (both frames' 3rd col)
    dp = np.degrees(np.arccos(np.clip(np.dot(loader_surf2ecl(ra0,de0,0)[:,2],
                                             iau_bf2ecl(ra0,de0,W0)[:,2]),-1,1)))
    return off, slope, dp

val = [
  ("Venus",    272.76,    67.16,     137.45,      160.20,   "EXACT"),
  ("Uranus",   257.311,  -15.175,    331.18,      203.81,   "EXACT(retro)"),
  ("Saturn",   40.5908,   83.537,    358.922,     38.90,    "~0.002"),
  ("Jupiter",  268.05,    64.49,     107.0,       284.95,   "~0.006"),
  ("Mercury",  281.001,   61.45,     291.20,      329.5988, "~0.03"),
  ("Neptune",  299.33,    42.95,     228.65,      249.978,  "~0.5 old"),
  ("Mars",     317.6725,  52.88212,  136.005,     176.049863,"~1.5 old"),
]
print(f"{'body':<9}{'W0':>10}{'->offset':>10}{'file':>9}{'diff':>8}{'slope':>7}{'poleErr':>8}  match")
for name,ra0,de0,fo,w0,pm in val:
    off,sl,dp=solve_offset(ra0,de0,w0)
    diff=((off-fo+180)%360)-180
    print(f"{name:<9}{w0:>10.4f}{off:>10.3f}{fo:>9.3f}{diff:>8.3f}{sl:>7.2f}{dp:>8.4f}  {pm}")
