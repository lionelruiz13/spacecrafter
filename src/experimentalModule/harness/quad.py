#!/usr/bin/env python3
"""Quadruplet analysis (Earth, Moon, Sun, Mars) - INTENT.md 11.14a.

Minimal set separating the delta's properties: Earth = identity path,
Moon = one down-hop, Sun = up-hop(s), Mars = up-then-down (sibling branch,
accumulation test). For each path the analyzer DISCOVERS the effective
composition rule: it tests candidate compositions of the dumped hop pieces
against the dumped final matrices and reports which one reproduces each -
'the way it is constructed' is measured, not assumed. Then it prints the
relative matrix differential old vs new, per member and per hop.

Conventions measured, not assumed: matrices column-major (r[12..14] =
translation); candidate grammars cover both multiplication orders.
"""
import json, math, sys
import numpy as np

QUAD = ["Earth", "Moon", "Sun", "Mars"]

def M(a):  # 16-list column-major -> 4x4 numpy (row-major math view)
    return np.array(a, dtype=float).reshape(4, 4).T

def rot(m): return m[:3, :3]
def tr(m): return m[:3, 3]

def ang_axis(R):
    c = max(-1.0, min(1.0, (np.trace(R) - 1) / 2))
    a = math.degrees(math.acos(c))
    ax = np.array([R[2,1]-R[1,2], R[0,2]-R[2,0], R[1,0]-R[0,1]])
    n = np.linalg.norm(ax)
    return a, (ax / n if n > 1e-12 else ax)

def show(tag, m, prec=6):
    a, ax = ang_axis(rot(m))
    t = tr(m)
    print(f"  {tag:<26} rot {a:8.3f} deg about [{ax[0]:+.3f},{ax[1]:+.3f},{ax[2]:+.3f}]"
          f"  t [{t[0]:+.6e},{t[1]:+.6e},{t[2]:+.6e}]")

def close(a, b, tol):  # relative Frobenius closeness on 4x4
    return np.linalg.norm(a - b) <= tol * max(1.0, np.linalg.norm(b))

def main(path):
    header, bodies, hops = None, {}, {}
    for line in open(path):
        line = line.strip()
        if not line: continue
        r = json.loads(line)
        if r["type"] == "header": header = r
        elif r["type"] == "body": bodies[r["name"]] = r
        elif r["type"] == "hops": hops[r["name"]] = r["new"]

    print(f"jd={header['jd']}")
    H = M(header["helioToEye"])              # old: helio -> eye (double)
    C = M(header["camera"]["mat"])           # new: camera mat as dispatched
    show("old helioToEye", H)
    show("new camera.mat", C)

    # ---- OLD path: verify the known composition against dumped mats -------
    # satellite: mat = H . T(p.ecl) . MLP(B) . p.rot[_unprec by flag]
    # top-level: mat = H . MLP(B)   (parent = system root at origin or Sun)
    print("\n== OLD composition check (residual of known formula vs dumped mat)")
    for n in QUAD:
        if n not in bodies: print(f"  {n}: missing"); continue
        o = bodies[n]["old"]
        dumped = M(o["mat"])
        pname = o["parent"]
        MLP = M(o["matLocalToParent"])
        cand = {}
        if pname in bodies:
            p = bodies[pname]["old"]
            T = np.eye(4); T[:3, 3] = np.array(p["ecl"])
            prot = M(p["rotLocalToParent"]); prot_u = M(p["rotLocalToParentUnprecessed"])
            cand["H.T(p).MLP.pRot"] = H @ T @ MLP @ prot
            cand["H.T(p).MLP.pRotUnprec"] = H @ T @ MLP @ prot_u
            cand["H.T(p).MLP"] = H @ T @ MLP
        cand["H.MLP"] = H @ MLP
        best = min(cand, key=lambda k: np.linalg.norm(cand[k] - dumped))
        res = np.linalg.norm(cand[best] - dumped) / max(1.0, np.linalg.norm(dumped))
        print(f"  {n:<6} parent={pname:<12} best='{best}'  rel-residual={res:.2e}"
              f"  (flag useParentPrecession={o.get('useParentPrecession')})")

    # ---- NEW path: discover the composition from hop pieces ---------------
    # Chain root->body known from hops (body..root). Candidates per down-hop:
    # pieces down (tilt+translate), tilt, spin in several orders.
    print("\n== NEW composition discovery (which hop grammar reproduces dumped mat)")
    for n in QUAD:
        if n not in hops or n not in bodies or bodies[n]["new"] is None:
            print(f"  {n}: missing"); continue
        dumped = M(bodies[n]["new"]["mat"])
        chain = hops[n]           # body .. root
        ref = header["camera"]["reference"]
        # path from reference to n: expressed with up-hops from ref + down-hops to n.
        # Build eye->node for the reference chain first: E = C (reference is where camera mat lands)
        if n == ref:
            grammars = {"C": C,
                        "C.tilt": C @ M(chain[0]["tilt"]),
                        "C.spin": C @ M(chain[0]["spin"]),
                        "C.tilt.spin": C @ M(chain[0]["tilt"]) @ M(chain[0]["spin"])}
        else:
            # need reference hops to go up from it
            refchain = hops.get(ref)
            if refchain is None: print(f"  {n}: no ref hops"); continue
            refnames = [h["name"] for h in refchain]
            names = [h["name"] for h in chain]
            # common ancestor = first name of ref chain present in n's chain
            common = next(nm for nm in refnames if nm in names)
            up = refchain[:refnames.index(common)]          # ref..(below common)
            down = chain[:names.index(common)]              # n..(below common), applied reversed
            base = C.copy()
            for h in up:                                     # up-hops from reference
                base = base @ M(h["up"])
            def compose(kind):
                m = base.copy()
                for h in reversed(down):                     # descend to n
                    if kind == "down":       m = m @ M(h["down"])
                    elif kind == "downOnly": T = np.eye(4); T[:3,3] = -np.array(h["ecl"]); m = m @ T
                    elif kind == "downSpin": m = m @ M(h["down"]) @ M(h["spin"])
                    elif kind == "posTilt":  T = np.eye(4); T[:3,3] = -np.array(h["ecl"]); m = m @ T @ M(h["tilt"])
                return m
            grammars = {f"C.up*.{k}*": compose(k) for k in ["down", "downOnly", "downSpin", "posTilt"]}
        best = min(grammars, key=lambda k: np.linalg.norm(grammars[k] - dumped))
        res = np.linalg.norm(grammars[best] - dumped) / max(1.0, np.linalg.norm(dumped))
        print(f"  {n:<6} best='{best}'  rel-residual={res:.2e}")

    # ---- Differential old vs new per member --------------------------------
    print("\n== Differential D = new_mat . old_mat^-1 (per member; float-converted old)")
    for n in QUAD:
        if n not in bodies or bodies[n]["new"] is None: continue
        Do = M(bodies[n]["old"]["mat"]); Dn = M(bodies[n]["new"]["mat"])
        D = Dn @ np.linalg.inv(Do)
        show(f"{n} D", D)
    print("\n== Same-member direction/translation deltas")
    for n in QUAD:
        if n not in bodies or bodies[n]["new"] is None: continue
        o, w = bodies[n]["old"], bodies[n]["new"]
        to, tn = np.array(o["mat"][12:15]), np.array(w["mat"][12:15])
        dno, dnn = np.linalg.norm(to), np.linalg.norm(tn)
        cosang = float(to @ tn / (dno * dnn)) if dno > 1e-15 and dnn > 1e-15 else float("nan")
        print(f"  {n:<6} |t_old|={dno:.6e} |t_new|={dnn:.6e} ddist={dnn-dno:+.3e}"
              f"  angle(t_old,t_new)={math.degrees(math.acos(max(-1,min(1,cosang)))):7.3f} deg")
    # ---- Raw hop matrices for eye inspection --------------------------------
    print("\n== NEW hop pieces (rot angle/axis + translation), per quadruplet chain")
    for n in QUAD:
        if n not in hops: continue
        print(f" {n}:")
        for h in hops[n]:
            print(f"  node {h['name']} (ecl {['%.3e' % v for v in h['ecl']]}, jd {h['lastJD']})")
            for k in ["up", "down", "tilt", "spin"]:
                show(f"   {k}", M(h[k]))

if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "/tmp/dual_trace.json")
