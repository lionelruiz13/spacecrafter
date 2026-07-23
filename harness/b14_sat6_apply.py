#!/usr/bin/env python3
"""B14-sat6 data edit (INTENT §11.75(b), widening via §2.0 D8): land cited IAU
poles for the 6 pole-bearing non-cluster garbage-tilt Saturn moons - Telesto,
Pandora, Janus, Helene, Epimetheus, Prometheus (the 63-121 deg commutator
errors). Replicates the §11.69 + §11.86 method exactly:

  rot_rotation_offset = <garbage>  ->  rot_pole_w0 = <IAU W0>      (loader converts)
  rot_obliquity       = <garbage>  ->  rot_pole_ra = <alpha0>
                                       rot_pole_de = <delta0>
                                       rot_frame   = absolute_pole
  rot_periode                        LEFT UNCHANGED (B14-periode's scope)

Values (deg), all from the 3 agreeing sources (pck00011.tpc md5 3c0bdc01 ==
pck00010.tpc == Archinal et al. 2015 report text; §11.69(a) discipline). Pole
ra/de = J2000-evaluated (base secular + Saturn nutation S1/S2 at T=0, d=0;
Telesto/Pandora/Helene/Prometheus are pure secular so = the verbatim constant;
Janus/Epimetheus carry periodic terms, eval_sat6.py reproducible). W0 = the
BODY_PM base constant (periodic PM omitted, the §11.86 convention). NEVER from
recall (§11.51(d) red line).

Hyperion STAYS UNWRITTEN (chaotic tumbler, no pole in any of the 3 sources,
§11.75(b)) but gains an inline data comment next to its garbage tilt datum
(§11.66(b) annotation discipline).

Byte-exact: reads/writes latin-1 to preserve ISO-8859 (loaded file); only the
target sections' lines change. Idempotent-guarded (refuses re-run).

Usage: b14_sat6_apply.py <ssystem.ini path>   (edits in place)
"""
import sys, re

# section -> (rot_pole_ra, rot_pole_de, rot_pole_w0)  [strings, source-verbatim]
POLE = {
    "telesto":    ("50.51",   "84.06",   "56.88"),
    "pandora":    ("40.58",   "83.53",   "162.92"),
    "janus":      ("39.8195", "83.3601", "58.83"),
    "helene":     ("40.85",   "83.34",   "245.12"),
    "epimetheus": ("40.9269", "83.1713", "293.87"),
    "prometheus": ("40.58",   "83.53",   "296.14"),
}
HYP_COMMENT = ("# Hyperion tumbles chaotically - no defined rotation pole in "
               "IAU/WGCCRE (absent from pck00011.tpc / pck00010.tpc / Archinal "
               "et al. 2015 report); values below are placeholders, NOT a "
               "measured orientation (INTENT 11.75(b)).")
HYP_MARK = "Hyperion tumbles chaotically"

def apply(path):
    with open(path, "r", encoding="latin-1") as f:
        text = f.read()
    lines = text.split("\n")
    # idempotency guard
    cur = None
    for ln in lines:
        m = re.match(r"\s*\[([^\]]+)\]\s*$", ln)
        if m: cur = m.group(1).strip().lower()
        if cur in POLE and re.match(r"\s*rot_pole_ra\s*=", ln):
            print(f"ERROR: {cur} already has rot_pole_ra - refusing (not idempotent-safe)")
            sys.exit(2)
    have_hyp_comment = HYP_MARK in text

    out = []
    cur = None
    did = {k: {"w0": False, "pole": False} for k in POLE}
    hyp_done = have_hyp_comment
    for ln in lines:
        m = re.match(r"\s*\[([^\]]+)\]\s*$", ln)
        if m:
            cur = m.group(1).strip().lower()
        if cur in POLE:
            ra, de, w0 = POLE[cur]
            if re.match(r"\s*rot_rotation_offset\s*=", ln):
                ind = ln[:len(ln)-len(ln.lstrip())]
                out.append(f"{ind}rot_pole_w0 = {w0}")
                did[cur]["w0"] = True
                continue
            if re.match(r"\s*rot_obliquity\s*=", ln):
                ind = ln[:len(ln)-len(ln.lstrip())]
                out.append(f"{ind}rot_pole_ra = {ra}")
                out.append(f"{ind}rot_pole_de = {de}")
                out.append(f"{ind}rot_frame = absolute_pole")
                did[cur]["pole"] = True
                continue
        if cur == "hyperion" and not hyp_done and re.match(r"\s*rot_obliquity\s*=", ln):
            ind = ln[:len(ln)-len(ln.lstrip())]
            out.append(f"{ind}{HYP_COMMENT}")
            hyp_done = True
            out.append(ln)
            continue
        out.append(ln)

    problems = []
    for k, st in did.items():
        if not st["w0"]:   problems.append(f"{k}: no rot_rotation_offset line")
        if not st["pole"]: problems.append(f"{k}: no rot_obliquity line")
    if not hyp_done: problems.append("hyperion: no rot_obliquity line to annotate")
    if problems:
        print("ERROR:", "; ".join(problems)); sys.exit(2)

    with open(path, "w", encoding="latin-1", newline="") as f:
        f.write("\n".join(out))
    print(f"OK: 6 moons pole-written + Hyperion annotated in {path}")

if __name__ == "__main__":
    apply(sys.argv[1])
