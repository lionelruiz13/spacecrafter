#!/usr/bin/env python3
"""A/B terminal-observable verification of the orientation consolidation
(INTENT 11.34/6.8 implementation; verification-height rule: pieces-layer
parity must compose to pixels).

Scene M: Earth observer, tracked Moon, moon_scale 30 (the mirrored seam,
11.16 - planet_scale is now mirrored too, 11.45 closed the 11.35 gap),
planets_axis on.
Scene P: observer on Pluto, tracked Charon (real geometry, ~3.5 deg disc).

REWRITTEN 2026-07-25 (F0; INTENT §11.101(g) -> §11.103). WHAT WAS WRONG, and
why the rewrite is at the root rather than an added assert:

  The pre-F0 instrument shot a burst under the 1000 ms auto-toggle and INFERRED
  the two path phases by clustering (seed = the two most-different shots, then
  partition by proximity). That construction ALWAYS emits two clusters - on a
  single-path run it splits B30 pixel noise - and it printed `between` without
  ever comparing it to `within`. It could therefore not fail: a same-path run
  produced a small `between` that reads exactly like "the divergence class is
  absent". §11.53 corrected the sibling >=2.5 s criterion; this instrument was
  the surviving instance of the same vacuity.

  Inferring the partition from the data is what cannot carry path identity, so
  the partition is now COMMANDED: `flag experimental_path off|on` pins each
  path and each half is shot under its own pin (what 24 of the other 25
  screenshot harnesses already do, §11.101(g2)). That also yields the IN-SCENE
  same-path floor §11.80(a) requires, instead of borrowing another scene's.

CRITERION, and where every number comes from:
  statistic  = px>8 (pixels differing by more than 8/255 in any channel) - the
               statistic the recorded floors are expressed in.
  within_X   = MAX px>8 over pairs inside the X-pinned set = that path's own
               in-scene instability floor (B30 for new; easing residue for old).
  between    = MIN px>8 over cross-pinned pairs.
  PASS iff   between > K * max(within_old, within_new, FLOOR_MIN).

  K = 10, FLOOR_MIN = 5 px>8, derived from the ledger's measured floors:
    - §11.53(d): same-path px>8 = 0..5 vs cross-path 1153..1156 in the b26
      scene => the tightest genuine separation ever recorded is ~231x.
    - §11.35 scene M: 1968 px>8 against that same scene's 0..5 floor => ~400x.
    - §11.80(b): a SAME-phase floor of 278..279 px>32 in another scene => floors
      are scene-dependent and can be large in absolute terms, so the criterion
      must be a RATIO over the floor measured IN THIS scene, never an absolute
      px count (§11.80(a)'s own rule).
    K = 10 sits one order of magnitude above the in-scene floor and 23x below
    the tightest recorded true separation - it neither passes noise nor rejects
    a real signal. FLOOR_MIN = 5 is the recorded CEILING of the same-path floor,
    used as the denominator clamp because floors of exactly 0 are recorded; the
    criterion therefore never weakens below "between > 50 px>8".

REFUSAL (the second half of the fix). A PASS says "the two paths render this
scene differently, measurably above this scene's own noise". A FAIL does NOT
say "the paths agree": with no independent witness that a pin actually took
effect (the dumps expose no rendered-path field - b26 needed a gdb probe for
that), "paths agree" and "the pin was swallowed" produce the same pixels. So a
non-separating scene is reported INCONCLUSIVE and exits nonzero - the
instrument refuses to emit the negative it cannot support. That refusal is the
point: §11.35's Charon leg was read as "both classes measurably ABSENT" from
exactly such a number.

Tracking is released before every capture (§11.80(c) / §11.101(g3)): it eases
and never settles, and under it the floors above measure the tracking residual
instead of the path.

Usage:  ab_orientation.py [outdir]      (fresh launch, enable_tcp; no
        beta_features.ini needed any more - nothing is sampled from the toggle)
        exit 0 = every scene separated; 1 = at least one scene INCONCLUSIVE.
"""
import socket, time, sys, os, json
import numpy as np
from PIL import Image

K = 10
FLOOR_MIN = 5
THR = 8
N = 6                      # shots per pinned path

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "artifacts", "ab_orientation")
os.makedirs(OUT, exist_ok=True)


def send(sock, cmd, pause=0.5):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f">> {cmd}", flush=True)


def burst(sock, scene, pin, n=N, spacing=0.8):
    """Pin one path, let it settle, shoot n frames. The pin IS the partition."""
    send(sock, f"flag experimental_path {pin}", 6)   # old re-entry eases the big
                                                     # halo for seconds (§11.80(b))
    files = []
    for i in range(n):
        p = f"{OUT}/ab_{scene}_{pin}_{i:02d}.png"
        sock.sendall(f"body action screenshot filename {p}\n".encode())
        files.append(p)
        time.sleep(spacing)
    time.sleep(2.5)   # last readback lands async
    return files


def load(files):
    return [np.asarray(Image.open(f).convert("RGB"), dtype=np.int16) for f in files]


def px(a, b):
    return int((np.abs(a - b) > THR).any(axis=2).sum())


def meand(a, b):
    return float(np.abs(a - b).mean())


def analyze(scene, files_off, files_on):
    off, on = load(files_off), load(files_on)
    within_off = max((px(off[i], off[j]) for i in range(len(off)) for j in range(i + 1, len(off))), default=0)
    within_on = max((px(on[i], on[j]) for i in range(len(on)) for j in range(i + 1, len(on))), default=0)
    between = min(px(a, b) for a in off for b in on)
    floor = max(within_off, within_on, FLOOR_MIN)
    ok = between > K * floor
    md_within = max(
        max((meand(off[i], off[j]) for i in range(len(off)) for j in range(i + 1, len(off))), default=0.0),
        max((meand(on[i], on[j]) for i in range(len(on)) for j in range(i + 1, len(on))), default=0.0))
    md_between = min(meand(a, b) for a in off for b in on)
    print(f"  [{scene}] px>{THR}: within(old)={within_off} within(new)={within_on} "
          f"between={between} | floor={floor} criterion=between>{K}*floor={K * floor} "
          f"| ratio={between / floor:.1f}x | mean-|d| within<={md_within:.3f} between>={md_between:.3f}",
          flush=True)
    print(f"  [{scene}] {'SEPARATED - the two paths render this scene differently' if ok else 'INCONCLUSIVE - no separation above this scene noise; this is NOT evidence the classes are absent (a swallowed pin looks identical)'}",
          flush=True)
    return {"scene": scene, "within_old": within_off, "within_new": within_on,
            "between": between, "floor": floor, "K": K, "ok": bool(ok),
            "meand_within": md_within, "meand_between": md_between}


def scene_m(s):
    send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 1)
    send(s, "select planet Moon", 1)
    send(s, "flag track_object on", 15)
    send(s, "flag moon_scaled on", 1)
    send(s, "set moon_scale 30", 8)        # ASmooth ease - settle
    send(s, "flag planets_axis on", 2)
    send(s, "flag track_object off", 3)    # aim done; shoot released (§11.80(c))


def scene_m_off(s):
    send(s, "flag planets_axis off", 1)
    send(s, "set moon_scale 1", 1)
    send(s, "flag moon_scaled off", 1)


def scene_p(s):
    send(s, "set home_planet Pluto", 5)
    send(s, "moveto lat 0 lon 0 alt 100 duration 0", 2)
    send(s, "select planet Charon", 1)
    send(s, "flag track_object on", 15)
    send(s, "flag track_object off", 3)


if __name__ == "__main__":
    s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
    send(s, "timerate rate 0", 1)          # FIRST, before the epoch (b19 lesson)
    send(s, "date jday 2461233.5", 1)
    send(s, "timerate rate 0", 1)
    send(s, "flag landscape off"); send(s, "flag atmosphere off")
    send(s, "flag star_twinkle off"); send(s, "flag show_fps off")
    send(s, "flag planet_names off")

    results = []
    scene_m(s)
    m_off = burst(s, "m", "off")
    m_on = burst(s, "m", "on")
    scene_m_off(s)
    scene_p(s)
    p_off = burst(s, "p", "off")
    p_on = burst(s, "p", "on")
    s.close()
    print("captures done", flush=True)

    results.append(analyze("m", m_off, m_on))
    results.append(analyze("p", p_off, p_on))
    json.dump(results, open(f"{OUT}/ab_orientation_result.json", "w"), indent=1)
    bad = [r for r in results if not r["ok"]]
    print(f"=== {len(results) - len(bad)}/{len(results)} scenes SEPARATED ===", flush=True)
    sys.exit(1 if bad else 0)
