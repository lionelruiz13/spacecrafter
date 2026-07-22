#!/usr/bin/env python3
"""B14 analysis (INTENT 11.68): copy-paste-cluster commutator spectrum +
moon-absolute-pole axis discriminator.

Two modes:

  spectrum <dump>
      Full planet-moon commutator spectrum (same construction as
      orientation_check.py) annotated with 15.5/213.7 copy-paste-cluster
      membership. Confirms DoD 2: the large (63-155 deg) commutator angles
      sit exactly on the cluster bodies, so correcting the DATA (not a render
      convention) is what removes the ambiguity holding the subclass open.

  axis <moon> <parent> <abs_d1> <abs_d2> [<rel_d1> <rel_d2>] [--ra R --de D]
      Moon-absolute-pole discriminator (DoD 3). Reads the moon's per-hop
      `tilt` + absoluteTiltFrame + obliquity/ascendingNode and its parent's
      `tilt` from the dumpHops chain, and:
        - absolute axis  = tilt_self^T . z          (walk skipped)
        - parent-rel axis = (tilt_parent . tilt_self)^T . z  (walk applied)
      Verifies the absolute axis == the DECLARED pole (mat_j2000_to_vsop87 .
      spheToRect(ra,de)), is date-STABLE across the two epochs, and that the
      parent_relative interpretation of the SAME numbers differs by the parent
      tilt. An assertion that can't tell absolute from parent-relative is
      decoration - the delta is the test.
"""
import json, math, sys
import numpy as np

# ---- shared matrix helpers (orientation_check.py convention) --------------
def M(a):  return np.array(a, dtype=float).reshape(4, 4).T   # app col-major -> np
def rot(m): return np.array(m)[:3, :3]
def ang(a, b):
    R = a @ b.T
    f = np.linalg.norm(R - np.eye(3))
    if f < 1e-3:
        return math.degrees(f / math.sqrt(2))
    c = max(-1.0, min(1.0, (np.trace(R) - 1) / 2))
    return math.degrees(math.acos(c))

# ---- loader-exact J2000-equatorial -> ecliptic(VSOP87) --------------------
# mat_j2000_to_vsop87 = xrotation(-eps) * zrotation(z0)  [ModularSystem.cpp:26]
EPS = 23.4392803055555555556 * math.pi / 180.0
Z0  = 0.0000275 * math.pi / 180.0
def Rx(t): c,s=math.cos(t),math.sin(t); return np.array([[1,0,0],[0,c,-s],[0,s,c]])
def Rz(t): c,s=math.cos(t),math.sin(t); return np.array([[c,-s,0],[s,c,0],[0,0,1]])
MAT_J2000_TO_VSOP87 = Rx(-EPS) @ Rz(Z0)
def sphe_to_rect(lng, lat):     # Utility::spheToRect
    cl = math.cos(lat)
    return np.array([math.cos(lng)*cl, math.sin(lng)*cl, math.sin(lat)])
def declared_pole(ra_deg, de_deg):
    v = MAT_J2000_TO_VSOP87 @ sphe_to_rect(math.radians(ra_deg), math.radians(de_deg))
    return v / np.linalg.norm(v)
def loader_obliq_ascnode(ra_deg, de_deg):
    v = declared_pole(ra_deg, de_deg)
    ra = math.atan2(v[1], v[0]); de = math.asin(v[2]/np.linalg.norm(v))
    return (math.pi/2 - de), (ra + math.pi/2)     # ModularSystem.cpp:747-748

# The 34 copy-paste-cluster bodies in the LOADED ~/.spacecrafter/ssystem.ini
# (rot_obliquity = 15.5; 33 also carry rot_equator_ascending_node = 213.7,
# Iapetus is the 34th with 15.5 only). English names as they appear in dumps.
CLUSTER = {"Metis","Adrastea","Amalthea","Thebe","Himalia","Elara","Carme",
    "Pasiphae","Lysithea","Ananke","Sinope","Leda","Iapetus","Juliet","Portia",
    "Rosalind","Belinda","Puck","Caliban","Sycorax","Prospero","Setebos",
    "Naiad","Thalassa","Despina","Galatea","Larissa","Proteus","Neried",
    "Halimede","Sao","Laomedeia","Psamathe","Neso"}

def load(path):
    header, bodies, hops = None, {}, {}
    for line in open(path):
        line = line.strip()
        if not line: continue
        r = json.loads(line)
        if r["type"]=="header": header = r
        elif r["type"]=="body": bodies[r["name"]] = r
        elif r["type"]=="hops": hops[r["name"]] = r["new"]
    return header, bodies, hops

def spectrum(path):
    header, bodies, hops = load(path)
    rows = []
    for n in bodies:
        o = bodies[n]["old"]; p = o["parent"]
        if p not in bodies or p in ("Sun","") or bodies[p]["old"]["parent"] != "Sun":
            continue
        if n in ("Moon","Charon","Sun","Mars","Earth","Pluto"): continue
        Rm = rot(M(o["rotLocalToParent"]))
        po = bodies[p]["old"]
        Rvar = rot(M(po["rotLocalToParentUnprecessed"] if not o.get("useParentPrecession")
                     else po["rotLocalToParent"]))
        Rp = rot(M(po["rotLocalToParent"]))
        rows.append((ang(Rp@Rm, Rm@Rvar), ang(Rm, Rm@Rvar), n, p))
    rows.sort(reverse=True)
    print(f"jd={header['jd']}  {len(rows)} planet-moons")
    print(f"{'moon':<12}{'parent':<9}{'commutator':>11}{'cluster?':>10}")
    big_cluster = big_noncluster = small = restored = 0
    for fix, cur, n, p in rows:
        inc = n in CLUSTER
        tag = "CLUSTER" if inc else "-"
        if fix < 1e-3: restored += 1
        elif fix >= 60:
            if inc: big_cluster += 1
            else:   big_noncluster += 1
        else: small += 1
        # print all cluster rows + any big non-cluster row (the exceptions)
        if inc or fix >= 60 or fix < 1e-3 and False:
            print(f"{n:<12}{p:<9}{fix:11.4f}{tag:>10}")
    print(f"\nSUMMARY: {restored} parity-restored (fix~0, zero own tilt); "
          f"large(>=60deg): {big_cluster} CLUSTER + {big_noncluster} non-cluster; "
          f"moderate(0-60): {small}")
    incluster_fix = [f for f,c,n,p in rows if n in CLUSTER]
    print(f"cluster commutator range: {min(incluster_fix):.2f}..{max(incluster_fix):.2f} deg "
          f"(n={len(incluster_fix)} of 34 enumerated; rest not planet-moons in this dump)")

def axis(moon, parent, dumps, ra=None, de=None):
    zc = np.array([0,0,1.0])
    print(f"== moon-absolute-pole axis: {moon} (parent {parent})")
    if ra is not None:
        dv = declared_pole(ra, de)
        O, N = loader_obliq_ascnode(ra, de)
        print(f"DECLARED test pole RA={ra} DE={de} (SYNTHETIC - not a real pole)")
        print(f"  declared dir (VSOP87 root)   = [{dv[0]:+.6f},{dv[1]:+.6f},{dv[2]:+.6f}]")
        print(f"  loader would set obliquity={math.degrees(O):.6f} deg  ascNode={math.degrees(N):.6f} deg")
    # Pole extraction convention (verified to 2.3e-6 deg vs the independently
    # computed declared pole): with T = rot(M(tilt)), the body's north pole in
    # the ROOT frame is T @ z (third column). The accumulation composes
    # ancestor-first on the LEFT (orientation_check.py's established form:
    # fix = R_parent @ R_self), so the parent-RELATIVE pole = R_parent @ pole_self.
    axes = {}
    for tag, path in dumps:
        _, _, hops = load(path)
        chain = hops.get(moon)
        if not chain:
            print(f"  [{tag}] {moon} not in hops"); continue
        self_n = chain[0]; par_n = next((c for c in chain if c["name"]==parent), None)
        Tself = rot(M(self_n["tilt"]))
        absF = self_n["absoluteTiltFrame"]
        ax_self = Tself @ zc                       # absolute pole (walk skipped)
        line = (f"  [{tag}] absFrame={str(absF):<5} obliq={math.degrees(self_n['obliquity']):9.5f} "
                f"ascN={math.degrees(self_n['ascendingNode']):9.5f}\n"
                f"         axis(self,abs)   =[{ax_self[0]:+.6f},{ax_self[1]:+.6f},{ax_self[2]:+.6f}]")
        if par_n:
            Tpar = rot(M(par_n["tilt"]))
            ax_acc = Tpar @ ax_self                # parent-relative pole
            d_selfacc = ang_vec(ax_self, ax_acc)
            line += (f"\n         axis(parent_rel) =[{ax_acc[0]:+.6f},{ax_acc[1]:+.6f},{ax_acc[2]:+.6f}]"
                     f"  DELTA abs^parentrel = {d_selfacc:.4f} deg ({parent} frame applied to the same pole)")
            axes[tag] = (ax_self, ax_acc, absF)
        if ra is not None:
            line += f"\n         angle(axis(abs), DECLARED pole) = {ang_vec(ax_self, dv):.6f} deg"
        print(line)
    # cross-date stability of the absolute axis (must be ~0: absolute => no
    # dependence on the parent's orbital position between the two epochs)
    tags = list(axes)
    if len(tags) >= 2:
        print("\n  cross-date absolute-axis stability (proves independence of parent orbital pos):")
        base = axes[tags[0]][0]
        for t in tags[1:]:
            print(f"    |abs({tags[0]}) ^ abs({t})| = {ang_vec(base, axes[t][0]):.6e} deg")
    return axes

def ang_vec(a, b):
    a = a/np.linalg.norm(a); b = b/np.linalg.norm(b)
    return math.degrees(math.acos(max(-1,min(1,float(np.dot(a,b))))))

if __name__ == "__main__":
    if sys.argv[1] == "spectrum":
        spectrum(sys.argv[2])
    elif sys.argv[1] == "axis":
        moon, parent = sys.argv[2], sys.argv[3]
        # remaining args: dump files (tag:path) and optional --ra/--de
        ra = de = None
        args = sys.argv[4:]
        dumps = []
        i = 0
        while i < len(args):
            if args[i] == "--ra": ra = float(args[i+1]); i += 2
            elif args[i] == "--de": de = float(args[i+1]); i += 2
            else:
                tag, path = args[i].split(":", 1); dumps.append((tag, path)); i += 1
        axis(moon, parent, dumps, ra, de)
