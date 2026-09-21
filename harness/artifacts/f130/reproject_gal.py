#!/usr/bin/env python3
# F130, OFFLINE, no launch: is the new-pin milky-way layer of the GAL run THE SAME SKY as the old-pin layer,
# turned by the rotation the two dumps give (Camera against Navigator), and nothing else?
# For every pixel of the NEW layer: eye direction -> frame direction (Camera's rotation, transposed) -> the OLD
# observer's eye direction (Navigator's helioToEye 3x3) -> pixel of the OLD layer. Compared where both hemispheres overlap.
# Unknown conventions (matrix storage order, image y sense, forward sign) are NOT assumed: all 8 combinations are printed;
# the claim holds only if ONE stands out. Fisheye law assumed equidistant (projector dump: FISHEYE, fov 180, radius 1024).
import json, sys, os
import numpy as np
HERE = '/home/claude/spacecrafter/claude/harness'
sys.path.insert(0, HERE)
from f56_starfield import load

def lum_layer(cap, tag='base'):
    _, on = load('%s/%s_on.png' % (cap, tag)); _, off = load('%s/%s_off.png' % (cap, tag))
    return (on - off).astype(np.float64)

def head(cap, tag='base'):
    return json.loads(open('%s/%s.json' % (cap, tag), 'rb').readline().decode('utf-8', 'replace'))

def m3(m):
    return np.array([[m[0], m[1], m[2]], [m[4], m[5], m[6]], [m[8], m[9], m[10]]], dtype=np.float64)

old_cap, new_cap = sys.argv[1], sys.argv[2]
A, B = lum_layer(old_cap), lum_layer(new_cap)
hA, hB = head(old_cap), head(new_cap)
Rold_arr = m3(hA['helioToEye'])          # the observer that DREW A (old pin: Navigator)
Rcam_arr = m3(hB['camera']['mat'])       # the observer that DREW B (new pin after the change: Camera)
prj = hB['oldView']['projector']
H, W = B.shape
cx, cy, R = prj['viewportCenter'][0], prj['viewportCenter'][1], prj['viewportRadius']
print('image %dx%d  viewport centre %.1f %.1f radius %.1f  fov %.1f' % (W, H, cx, cy, R, prj['fov']))
half = np.radians(prj['fov']) / 2
ys, xs = np.mgrid[0:H, 0:W].astype(np.float64)

def run(transpose, yflip, fwd):
    Rold = Rold_arr.T if transpose else Rold_arr
    Rcam = Rcam_arr.T if transpose else Rcam_arr
    dx = xs - cx; dy = (ys - cy) * (-1 if yflip else 1)
    r = np.hypot(dx, dy); inside = r < R
    th = r / R * half
    with np.errstate(invalid='ignore', divide='ignore'):
        ux, uy = np.where(r > 0, dx / r, 0), np.where(r > 0, dy / r, 0)
    d = np.stack([np.sin(th) * ux, np.sin(th) * uy, fwd * np.cos(th)], axis=-1)      # eye direction of each NEW pixel
    v = d @ Rcam                       # Rcam^T d  (frame direction)
    e = v @ Rold.T                     # Rold v    (eye direction for the OLD observer)
    thA = np.arccos(np.clip(fwd * e[..., 2], -1, 1))
    rA = thA / half * R
    n = np.hypot(e[..., 0], e[..., 1]); n[n == 0] = 1
    pxA = cx + rA * e[..., 0] / n
    pyA = cy + rA * e[..., 1] / n * (-1 if yflip else 1)
    ok = inside & (rA < R - 2)
    xi, yi = np.clip(pxA, 0, W - 2), np.clip(pyA, 0, H - 2)
    x0, y0 = np.floor(xi).astype(int), np.floor(yi).astype(int); fx, fy = xi - x0, yi - y0
    P = A[y0, x0] * (1 - fx) * (1 - fy) + A[y0, x0 + 1] * fx * (1 - fy) + A[y0 + 1, x0] * (1 - fx) * fy + A[y0 + 1, x0 + 1] * fx * fy
    b, p = B[ok], P[ok]
    corr = float(np.corrcoef(b, p)[0, 1]) if ok.sum() > 100 and b.std() > 0 and p.std() > 0 else float('nan')
    rel = float(np.abs(b - p).sum() / (0.5 * (np.abs(b).sum() + np.abs(p).sum()))) if ok.sum() else float('nan')
    ratio = float(np.abs(b).sum() / np.abs(p).sum()) if np.abs(p).sum() else float('nan')
    return int(ok.sum()), corr, rel, ratio

print('control, no reprojection (whole disc): corr %.6f' % np.corrcoef(A.ravel(), B.ravel())[0, 1])
print('%-9s %-5s %-4s %10s %10s %10s %10s' % ('transpose', 'yflip', 'fwd', 'overlap_px', 'corr', 'rel(lum)', 'ratio B/A'))
for t in (False, True):
    for yf in (False, True):
        for fwd in (-1, 1):
            n, c, rel, ratio = run(t, yf, fwd)
            print('%-9s %-5s %-4d %10d %10.6f %10.6f %10.6f' % (t, yf, fwd, n, c, rel, ratio))
