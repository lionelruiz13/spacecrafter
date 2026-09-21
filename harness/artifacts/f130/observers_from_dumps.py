#!/usr/bin/env python3
# F130 pre-edit read of F128's STORED dumps (no launch): per capture, the rotation the old entry
# hands MilkyWay::drawEnv (Navigator, helioToEye 3x3) against the rotation the member chain would
# hand MilkyWayEnv (the `mat` of the node that owns the member, as the dump prints it).
import json, sys, math, glob, os

def rot3(m):  # 16 values, r[0..15]; keep the 3x3 as MilkyWayEnv does: rows (r0 r1 r2)(r4 r5 r6)(r8 r9 r10)
    return [[m[0], m[1], m[2]], [m[4], m[5], m[6]], [m[8], m[9], m[10]]]

def mul(a, b):
    return [[sum(a[i][k] * b[k][j] for k in range(3)) for j in range(3)] for i in range(3)]

def tr(a):
    return [[a[j][i] for j in range(3)] for i in range(3)]

def angle(a, b):  # rotation angle of a * b^T, degrees
    r = mul(a, tr(b))
    c = (r[0][0] + r[1][1] + r[2][2] - 1) / 2
    return math.degrees(math.acos(max(-1.0, min(1.0, c))))

def load(path):
    head = None; bodies = {}
    with open(path, 'rb') as f:
        for raw in f:
            try:
                d = json.loads(raw.decode('utf-8', 'replace'))
            except Exception:
                continue
            if d.get('type') == 'header':
                head = d
            elif d.get('type') == 'body' and isinstance(d.get('new'), dict) and 'mat' in d['new']:
                bodies[d['name']] = d['new']
    return head, bodies

for path in sys.argv[1:]:
    head, bodies = load(path)
    if not head:
        print(path, 'NO HEADER'); continue
    cam = head['camera']
    old = rot3(head['helioToEye'])
    out = [os.path.relpath(path), 'drawn=' + str(head.get('control', {}).get('drawnPath')), 'ref=' + cam['reference'],
           'free=' + str(cam['freeMode'])]
    for owner in ('MilkyWay', 'SolarSystem'):
        b = bodies.get(owner)
        if b:
            out.append('%s.mat|old %.4f deg' % (owner, angle(rot3(b['mat']), old)))
    out.append('camera.mat|old %.4f deg' % angle(rot3(cam['mat']), old))
    o = head['oldView']['observer']
    out.append('old lon/lat %.3f/%.3f' % (o['longitude'], o['latitude']))
    p = cam.get('rootPos')
    if p:
        n = math.sqrt(sum(x * x for x in p))
        out.append('camera place lon/lat %.3f/%.3f' % (math.degrees(math.atan2(p[1], p[0])), math.degrees(math.asin(p[2] / n))))
    print('  '.join(out))
