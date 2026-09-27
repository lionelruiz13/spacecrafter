#!/usr/bin/env python3
# Print the numbers F134's block P9-P13, P20, P21 name, from one canary run's scene/ directory.
#   python3 dwellparse.py <canary dir>      (the dir holding scene/)
import json, math, os, sys
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
from dumpread import load_dump

AU_KM = 149597870.691
MOON_R_KM = 1737.4


def moon(path):
    h, pairs, mn, mo = load_dump(path, require_old=False)
    new = old = None
    for r in pairs:
        if r['name'] == 'Moon':
            new, old = r['new'], r['old']
    return h, new, old


def km(au):
    return au * AU_KM


def show(tag, path):
    if not os.path.exists(path):
        print(f'{tag}: MISSING {path}')
        return
    h, n, o = moon(path)
    c = h['camera']
    ctl = h.get('control', {})
    alt = ctl.get('altitude', {})
    pos = c['position']
    pnorm = km(math.sqrt(sum(x * x for x in pos)))
    print(f'{tag}: drawnPath={ctl.get("drawnPath")} reference={c["reference"]} freeMode={c["freeMode"]} '
          f'refCached={c["refCached"]} refAoI_km={km(c["refAoI"]):.2f} camera.distance_km={km(c["distance"]):.2f} '
          f'|position|_km={pnorm:.2f}')
    print(f'  control.altitude: new={alt.get("new")} old={alt.get("old")} reported={alt.get("reported")}')
    if n:
        print(f'  Moon.new: dist_km={km(n["dist"]):.2f} scaledDatumRadius_km={km(n["scaledDatumRadius"]):.2f} '
              f'scaledGroundRadius_km={km(n["scaledGroundRadius"]):.2f} boundingRadius_km={km(n["boundingRadius"]):.2f} '
              f'scaling={n["scaling"]} target={n["scalingTarget"]} visible={n["visible"]} screen={n["screen"]} '
              f'screenSize={n["screenSize"]}')
    if o:
        print(f'  Moon.old: dist_km={km(o["dist"]):.2f} screenSz={o["screenSz"]} visible={o["visible"]}')


def main(d):
    s = os.path.join(d, 'scene')
    for tag in ('dwell_settle', 'dwell_dump_start', 'dwell_dump_end', 'dwell_dump_old'):
        show(tag, os.path.join(s, tag + '.json'))
    tp = os.path.join(s, 'dwell_settle_trace.json')
    if os.path.exists(tp):
        t = json.load(open(tp))
        worst = 0.0
        for smp in t:
            dev = abs(smp['scaledDatumRadius_km'] / MOON_R_KM - smp['scaling'])
            worst = max(worst, dev)
            print(f'  trace: scaledDatumRadius_km={smp["scaledDatumRadius_km"]:.6f} scaling={smp["scaling"]} '
                  f'target={smp["scalingTarget"]} boundingRadius_km={smp["boundingRadius_km"]:.2f} '
                  f'datum/1737.4-scaling={smp["scaledDatumRadius_km"] / MOON_R_KM - smp["scaling"]:.6f}')
        print(f'  trace: samples={len(t)} max|datum_km/1737.4 - scaling|={worst:.6f} (P12 criterion < 1e-3)')
    fp = os.path.join(s, 'f51_dwell.json')
    if os.path.exists(fp):
        f = json.load(open(fp))
        print(f'  f51_dwell.json fails={f.get("fails")} n_samples={f.get("n_samples")}')


if __name__ == '__main__':
    main(sys.argv[1])
