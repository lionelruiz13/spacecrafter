#!/usr/bin/env python3
"""B14-periode discriminator (INTENT §11.86(d)/§11.87(e)).

Verifies the rot_periode correction for the 20 pole-landed moons against the
IAU rotation rate Wdot (pck00011 BODY_PM[1], 3-source), using the new dumped
`period` hops field (re.period, DAYS, float32 as loaded).

Four checks, on a same-binary A/B (baseline = pre-edit loaded data, post = edited):
  1. LOAD   : post period*24 (hours) == 8640/Wdot to the float32 bound (each moon
              loaded the intended IAU-derived value; sign = spin direction).
  2. MOVERS : period_A vs period_B over ALL hopped bodies - exactly the 19 edited
              moons change; Iapetus (unedited) + Venus/planets ULP-0.
  3. POLE   : offset / obliquity / ascendingNode / absoluteTiltFrame ULP-0 post vs
              baseline for EVERY hopped body (rate-only edit; pole channel intact).
  4. MERID  : 2-date meridian-vs-IAU residual for the 5 dumped probes, using the
              DUMPED float32 period -> the engine's ACTUAL render residual (bounded
              by float32 re.period), plus the double-ideal residual (data value).

Usage: b14_periode_analyze.py <base_d1> <base_d2> <post_d1> <post_d2>
"""
import json, math, sys
import numpy as np
import b14_w0_analyze as W   # reuse the meridian math (single source)

d2r = math.pi/180.0
# Wdot deg/day (pck00011 BODY_PM 2nd coeff; pck00010 byte-identical; Archinal text verbatim)
WDOT = {
 "Amalthea":722.6314560,"Thebe":533.7004100,"Iapetus":4.5379572,"Telesto":190.6979332,
 "Pandora":572.7891000,"Janus":518.2359876,"Helene":131.6174056,"Epimetheus":518.4907239,
 "Prometheus":587.2890000,"Juliet":-730.1253660,"Portia":-701.4865870,"Rosalind":-644.6311260,
 "Belinda":-577.3628170,"Puck":-472.5450690,"Naiad":1222.8441209,"Thalassa":1155.7555612,
 "Despina":1075.7341562,"Galatea":839.6597686,"Larissa":649.0534470,"Proteus":320.7654228,
}
EDITED = [m for m in WDOT if m != "Iapetus"]           # 19 (Iapetus left bit-identical)
# probe pole+W0 for the meridian check (file J2000 pole, pck W0)
PROBE = {
 "Iapetus":   (318.16,   75.03,   355.2,   4.5379572),
 "Proteus":   (299.2134, 42.4321, 93.38,   320.7654228),
 "Puck":      (257.0919,-15.4127, 91.24,  -472.5450690),
 "Janus":     (39.8195,  83.3601, 58.83,   518.2359876),
 "Prometheus":(40.58,    83.53,   296.14,  587.2890000),
}

def f32(x):
    import struct
    return struct.unpack('f', struct.pack('f', x))[0]

def load(path):
    jd=None; hops={}
    for line in open(path):
        try: r=json.loads(line)
        except: continue
        if r.get("type")=="header": jd=r["jd"]
        elif r.get("type")=="hops": hops[r["name"]]=r["new"][0]
    return jd, hops

def main(bd1, bd2, pd1, pd2):
    jb1,B1=load(bd1); jb2,B2=load(bd2); jp1,P1=load(pd1); jp2,P2=load(pd2)
    ok=True

    print("="*78)
    print("CHECK 1  LOAD: post period (hours) vs IAU 8640/Wdot  [float32 bound]")
    print(f"{'moon':<11}{'Wdot':>13}{'derived_h':>16}{'loaded_h':>16}{'d(h)':>11} sign")
    for m,w in WDOT.items():
        der = 8640.0/w
        loaded_h = P1[m]["period"]*24.0
        d = loaded_h - der
        # float32 bound: the file value truncated to float32 (days) then *24
        bound = abs(f32(der/24.0)*24.0 - der) + 5e-7*abs(der)
        good = abs(d) <= max(bound, 1e-4)
        ok &= good
        sgn = "RETRO" if w<0 else "prog"
        print(f"{m:<11}{w:>13.6f}{der:>16.6f}{loaded_h:>16.6f}{d:>11.2e} {sgn:<5} {'ok' if good else 'FAIL'}")

    print("="*78)
    print("CHECK 2  MOVERS: period_A(base) vs period_B(post), ALL hopped bodies")
    allnames = sorted(set(B1)&set(P1))
    movers=[]; still=[]
    for nm in allnames:
        a=B1[nm].get("period"); b=P1[nm].get("period")
        if a is None or b is None: continue
        if a==b: still.append(nm)
        else: movers.append((nm, a, b))
    print(f"  MOVED ({len(movers)}):")
    for nm,a,b in sorted(movers):
        tag = "  <-- EDITED" if nm in EDITED else "  <== UNEXPECTED"
        if nm not in EDITED: ok=False
        print(f"    {nm:<11} {a:>14.7g} -> {b:>14.7g}{tag}")
    unmoved_edited=[m for m in EDITED if m in still]
    print(f"  ULP-0 ({len(still)}): {', '.join(still)}")
    if unmoved_edited:
        ok=False; print(f"  !! EDITED-BUT-UNMOVED: {unmoved_edited}")
    if "Iapetus" in still: print("  Iapetus ULP-0 (unedited control): PASS")
    else: ok=False; print("  Iapetus MOVED (should be bit-identical): FAIL")

    print("="*78)
    print("CHECK 3  POLE: offset/obliquity/ascendingNode/absFrame ULP-0 post vs base")
    worst=0.0; badpole=[]
    for nm in allnames:
        for k in ("offset","obliquity","ascendingNode"):
            da=abs(B1[nm].get(k,0)-P1[nm].get(k,0)); worst=max(worst,da)
            if da>0: badpole.append((nm,k,da))
        if B1[nm].get("absoluteTiltFrame")!=P1[nm].get("absoluteTiltFrame"):
            badpole.append((nm,"absFrame",1)); ok=False
    if badpole:
        ok=False
        for nm,k,da in badpole: print(f"    NONZERO {nm} {k} d={da:.3e}")
    print(f"  worst |d(offset/obliquity/ascNode)| over {len(allnames)} bodies = {worst:.3e}  {'ULP-0 PASS' if worst==0 else 'FAIL'}")

    print("="*78)
    print("CHECK 4  MERID: 2-date meridian-vs-IAU (post), DUMPED float32 period")
    print(f"{'moon':<11}{'merid_d1':>11}{'merid_d2(f32)':>15}{'merid_d2(ideal)':>17}")
    J2000=W.J2000
    for m,(ra0,de0,W0,Wd) in PROBE.items():
        if m not in P1 or m not in P2: print(f"  {m}: not dumped"); continue
        pole=W.M@W.s2r(ra0*d2r,de0*d2r)
        res=[]; resi=[]
        perd_f32 = P1[m]["period"]            # engine float32 (days)
        perd_ideal = 8640.0/Wd/24.0           # exact double
        for jd,H in ((jp1,P1),(jp2,P2)):
            h=H[m]
            for perd,acc in ((perd_f32,res),(perd_ideal,resi)):
                lm=W.loader_meridian(h["obliquity"],h["ascendingNode"],h["offset"],jd,perd)
                im=W.iau_meridian(ra0,de0,W0,Wd,jd)
                acc.append(W.angabout(pole,im,lm))
        # float32 residual prediction
        pred = 3652*360*(1.0/perd_f32 - 1.0/perd_ideal); pred=((pred+180)%360)-180
        good = abs(res[0])<0.05 and abs(resi[1])<1e-2
        ok &= good
        print(f"{m:<11}{res[0]:>11.4f}{res[1]:>15.4f}{resi[1]:>17.4f}   (f32 pred {pred:+.4f}) {'ok' if good else 'FAIL'}")
    print("  merid_d2(f32)  = engine's ACTUAL render residual (float32 re.period) - attributed")
    print("  merid_d2(ideal)= double-precision period -> the DATA VALUE residual (~0)")

    print("="*78)
    print("RESULT:", "ALL OK" if ok else "FAIL")
    return 0 if ok else 1

if __name__=="__main__":
    sys.exit(main(*sys.argv[1:5]))
