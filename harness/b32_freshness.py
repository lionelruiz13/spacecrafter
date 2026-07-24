#!/usr/bin/env python3
"""B32 spin-phase freshness gate (INTENT §5.24 / D20 §11.79(n); ledger B32).

Recompute-at-use under the D8 §11.76 barrier: the per-frame tick may genuinely
FREEZE spin for non-visible bodies (no new per-frame cost, D11), and every USE
channel recomputes the spin phase from the ROOT-fresh lastJD through the ONE
authority ModularBody::computeAxisRotation (I2). The dump `axisRot`/`attitude`
and dumpHops `spin` are the user-reachable read channels; they are now a
deterministic function of the (bit-identical) lastJD.

This gate = the cross-launch determinism discriminator (§5.24, discriminator 1).
Two IDENTICAL fresh launches, same frozen scene, dual_dump; every body's
axisRot MUST agree EXACT-string. On a PRE-B32 binary the launch-wall-clock spin
leak scatters the invisible / frozen-under-invisible-parent bodies (measured up
to 4.49 rad on Moon/Deimos/Phobos/Mars/Mercury) - that is the discrimination
proving the field is now load-bearing.

Optional freeze/use demonstration mode (--freeze): one launch, dump at JD1, jump
+DT days, dump; per body dAxisRot must == dAttitude (the dump USE recomputes),
and a hidden body dumped mid-freeze reads its fresh spin.

    DISPLAY=:2 ./b32_freshness.py [outdir]            # cross-launch gate (exit 0/1)
    DISPLAY=:2 ./b32_freshness.py [outdir] --freeze   # freeze/use demonstration

The gate owns its app lifecycle (launches BOTH runs itself; do NOT pre-launch).
"""
import json, os, re, socket, subprocess, sys, time
from pathlib import Path

HOME = Path.home()
SC_BIN = os.environ.get("SC_BIN", str(Path(__file__).resolve().parents[2] / "build-claude/src/spacecrafter"))
USERDIR = HOME / ".spacecrafter"
ARGS = [a for a in sys.argv[1:] if not a.startswith("--")]
# ABSOLUTE (§11.91(f)): the app writes dual_dump paths relative to ITS cwd
# (USERDIR), so a relative OUT lands in an uncreated USERDIR path and reads fail.
OUT = (Path(ARGS[0]) if ARGS else Path(__file__).parent / "artifacts/b32").resolve()
FREEZE = "--freeze" in sys.argv
JD = "2461233.5"
DT = 40.0
# The pole-bearing moons §5.24 named + the invisible/frozen bodies that scatter
# in the b24 default scene (Moon/Deimos/Phobos/Mars/Mercury).
WATCH = ["Moon", "Mars", "Deimos", "Phobos", "Mercury", "Venus", "Jupiter", "Io",
         "Belinda", "Puck", "Caliban", "Iapetus", "Amalthea", "Thebe", "Proteus"]


def wait_port(timeout=90):
    t0 = time.time()
    while time.time() - t0 < timeout:
        try:
            return socket.create_connection(("127.0.0.1", 7805), timeout=1)
        except OSError:
            time.sleep(1)
    raise RuntimeError("port 7805 never opened")


def send(sock, cmd, pause=0.7):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)


def launch(tag):
    proc = subprocess.Popen([SC_BIN], cwd=str(USERDIR),
                            stdout=open(OUT / f"b32_{tag}.applog", "w"),
                            stderr=subprocess.STDOUT,
                            env={**os.environ, "DISPLAY": os.environ.get("DISPLAY", ":2")})
    return proc, wait_port()


def dump(sock, path):
    send(sock, f"body action dual_dump filename {path}"); time.sleep(2)


def shutdown(proc, sock):
    send(sock, "shutdown action now"); sock.close()
    try:
        return proc.wait(timeout=30)
    except subprocess.TimeoutExpired:
        proc.kill(); return -1


def fields(path):
    out = {}
    for line in open(path):
        m = re.search(r'"name":"([^"]+)".*?"new":(\{.*\})[,\s]*$', line.strip().rstrip(","))
        if m:
            d = {}
            for k in ("axisRot", "attitude", "lastJD", "visible", "relation"):
                e = re.search(r'"%s":([^,\}]+)' % k, m.group(2))
                if e:
                    d[k] = e.group(1)
            out[m.group(1)] = d
    return out


def run_once(tag):
    OUT.mkdir(parents=True, exist_ok=True)
    d = OUT / f"{tag}.json"; d.unlink(missing_ok=True)
    proc, sock = launch(tag)
    time.sleep(10)
    send(sock, "timerate rate 0"); send(sock, f"date jday {JD}"); time.sleep(3)
    dump(sock, d)
    rc = shutdown(proc, sock)
    return d, rc


def cross_launch_gate():
    d1, rc1 = run_once("run1")
    d2, rc2 = run_once("run2")
    a, b = fields(d1), fields(d2)
    common = sorted(set(a) & set(b))
    disagree = [n for n in common
                if "axisRot" in a[n] and "axisRot" in b[n]
                and a[n]["axisRot"] != b[n]["axisRot"]]
    maxd = 0.0; maxbody = None
    for n in common:
        try:
            dd = abs(float(a[n]["axisRot"]) - float(b[n]["axisRot"]))
        except (KeyError, ValueError):
            continue
        if dd > maxd:
            maxd, maxbody = dd, n
    print(f"launches rc={rc1},{rc2}; {len(common)} common bodies")
    print(f"max |dAxisRot| = {maxd:.6e} on {maxbody}")
    print(f"exact-string axisRot disagreements: {len(disagree)} bodies")
    for n in disagree:
        print(f"  FAIL {n}: {a[n]['axisRot']} vs {b[n]['axisRot']} "
              f"(vis {a[n].get('visible')}/{b[n].get('visible')})")
    (OUT / "b32_result.json").write_text(json.dumps(
        {"mode": "cross_launch", "n_common": len(common), "max_dAxisRot": maxd,
         "max_dAxisRot_body": maxbody, "disagree": disagree, "rc": [rc1, rc2]}, indent=1))
    ok = not disagree
    print(f"\n{'CROSS-LAUNCH DETERMINISM HOLDS' if ok else f'{len(disagree)} SPIN DIVERGENCES'} "
          f"({len(common)} bodies) -> {OUT}/b32_result.json")
    return 0 if ok else 1


def freeze_demo():
    OUT.mkdir(parents=True, exist_ok=True)
    proc, sock = launch("freeze")
    time.sleep(10)
    send(sock, "timerate rate 0"); send(sock, f"date jday {JD}"); time.sleep(3)
    d1 = OUT / "freeze_jd1.json"; dump(sock, d1)
    send(sock, f"date jday {float(JD)+DT}"); time.sleep(3)
    d2 = OUT / "freeze_jd2.json"; dump(sock, d2)
    send(sock, "body name Moon hidden true"); time.sleep(1)
    send(sock, f"date jday {float(JD)+2*DT}"); time.sleep(3)
    dh = OUT / "freeze_hidden.json"; dump(sock, dh)
    rc = shutdown(proc, sock)
    f1, f2, fh = fields(d1), fields(d2), fields(dh)
    print(f"rc={rc} DT={DT}d\nbody       vis@1  | dAxisRot(use)  dAttitude(fresh)  match")
    bad = 0
    for n in WATCH:
        if n not in f1 or n not in f2:
            continue
        try:
            dax = abs(float(f2[n]["axisRot"]) - float(f1[n]["axisRot"]))
            dat = abs(float(f2[n]["attitude"]) - float(f1[n]["attitude"]))
        except (KeyError, ValueError):
            continue
        match = abs(dax - dat) < 1e-4
        bad += not match
        print(f"{n:10s} {f1[n].get('visible','?'):6s} | {dax:.4e}     {dat:.4e}      {'OK' if match else 'MISMATCH'}")
    if "Moon" in fh:
        m = fh["Moon"]
        fresh = abs(float(m["axisRot"]) - float(m["attitude"])) < 1e-4
        print(f"hidden Moon@JD3: axisRot={m['axisRot']} attitude={m['attitude']} "
              f"rel={m.get('relation')} -> dump USE fresh: {'OK' if fresh else 'STALE'}")
        bad += not fresh
    print(f"\n{'FREEZE/USE DEMO OK' if not bad else f'{bad} MISMATCHES'} (dump USE recompute == fresh spin)")
    return 0 if not bad else 1


if __name__ == "__main__":
    sys.exit(freeze_demo() if FREEZE else cross_launch_gate())
