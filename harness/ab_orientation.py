#!/usr/bin/env python3
"""A/B terminal-observable verification of the orientation consolidation
(INTENT 11.34/6.8 implementation; verification-height rule: pieces-layer
parity must compose to pixels).

Scene M: Earth observer, tracked Moon, moon_scale 30 (the mirrored seam,
11.16 - planet_scale is now mirrored too, 11.45 closed the 11.35 gap),
planets_axis on.
Scene P: observer on Pluto, tracked Charon (real geometry, ~3.5 deg disc).

Channel: `body action screenshot` (app-side readback, 11.19a); the 1000 ms
auto-toggle alternates paths, shots every ~0.45 s, clustered into two phases
by pairwise distance; report within/between-phase disc diffs.

PRECONDITION SINCE 2026-07-21 (INTENT 11.50(c), verified 11.53): the toggle
is NO LONGER the default - the new path is pinned unless you opt in with
`~/.spacecrafter/beta_features.ini`:

    [dual_path]
    render_path = alternate

Without it every shot lands in ONE phase and the clustering below reports a
meaningless "between" distance on a 24/0 split.  Delete the file afterwards.
"""
import socket, time, sys, glob
import numpy as np
from PIL import Image

def send(sock, cmd, pause=0.5):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f">> {cmd}", flush=True)

def shots(sock, tag, n=12, spacing=0.45):
    for i in range(n):
        sock.sendall(f"body action screenshot filename /tmp/ab_{tag}_{i:02d}.png\n".encode())
        time.sleep(spacing)
    time.sleep(2)  # last readback lands async

def analyze(tag):
    files = sorted(glob.glob(f"/tmp/ab_{tag}_*.png"))
    imgs = [np.asarray(Image.open(f).convert("RGB"), dtype=np.int16) for f in files]
    if len(imgs) < 4:
        print(f"  [{tag}] only {len(imgs)} shots - channel problem"); return
    # cluster into two phases: seed = the two most-different shots
    n = len(imgs)
    d = np.zeros((n, n))
    for i in range(n):
        for j in range(i + 1, n):
            d[i, j] = d[j, i] = np.abs(imgs[i] - imgs[j]).mean()
    a, b = np.unravel_index(np.argmax(d), d.shape)
    ca = [i for i in range(n) if d[i, a] <= d[i, b]]
    cb = [i for i in range(n) if i not in ca]
    def stats(idx):
        vals = [d[i, j] for i in idx for j in idx if i < j]
        return max(vals) if vals else 0.0
    between = min(d[i, j] for i in ca for j in cb) if ca and cb else 0.0
    px = [(np.abs(imgs[i] - imgs[j]) > 8).sum() // 3 for i in ca for j in cb]
    print(f"  [{tag}] phases {len(ca)}/{len(cb)}  within<=({stats(ca):.3f},{stats(cb):.3f})"
          f"  between>={between:.3f} mean-|d|  px>8: min={min(px)} max={max(px)}")

if __name__ == "__main__":
    s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
    send(s, "date jday 2461233.5", 1)
    send(s, "timerate rate 0", 1)
    send(s, "flag landscape off"); send(s, "flag atmosphere off")
    send(s, "flag star_twinkle off"); send(s, "flag show_fps off")
    send(s, "flag planet_names off")
    # Scene M
    send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 1)
    send(s, "select planet Moon", 1)
    send(s, "flag track_object on", 15)
    send(s, "flag moon_scaled on", 1)
    send(s, "set moon_scale 30", 3)
    send(s, "flag planets_axis on", 2)
    shots(s, "m")
    send(s, "flag planets_axis off", 1)
    send(s, "set moon_scale 1", 1)
    send(s, "flag moon_scaled off", 1)
    # Scene P
    send(s, "set home_planet Pluto", 5)
    send(s, "moveto lat 0 lon 0 alt 100 duration 0", 2)
    send(s, "select planet Charon", 1)
    send(s, "flag track_object on", 15)
    shots(s, "p")
    s.close()
    print("captures done")
    analyze("m")
    analyze("p")
