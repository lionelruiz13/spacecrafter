#!/usr/bin/env python3
"""F41 — display-scaling ownership is FORMAT-SCOPED (§5.104 / §11.154(b)(c)).

THE RULE [vixy 2026-08-26]: *"for the legacy star system format, the scaling is
owned by config.ini - for the new star system format (modular one), the scaling
is owned by the new file, deprecating the config.ini one whenever the new format
is used for the solar system."*

ONE instrument, five legs, run identically on a PRE and a POST binary; every gate
states what a PRE leg must show, so none of them is a tautology.

  L1 `legacy`     legacy-served (no composed SolarSystem.ini), shipped config
                  (moon 5/on, sun 5/OFF).
                  · the generated twin carries `display_scale = 5` for the Moon
                    and NO such key for the Sun.  PRE: neither.
                  · NO deprecation line (config.ini owns it here).  Both.
                  · `body action reload` KEEPS scaling 5.  PRE: drops to 1 —
                    §5.104 itself, reproduced on the pre binary.
  L2 `twin`       the POST-generated twin activated (§11.51(a)), shipped config.
                  · Moon 5 from the FILE, ONE deprecation line naming it.
                  · reload keeps 5 (D31: the file was re-read).  PRE: 5 too, but
                    from config.ini — L3 is what separates the two.
  L3 `twin_cfg2`  same file, `moon_scale = 2`.  POST: 5 (the file wins, and the
                  line says 2 was ignored).  PRE: 2 (config wins).  ← the
                  ownership discriminator, both ways.
  L4 `twin_sunon` same file, `flag_sun_scaled = true`, `sun_scale = 3`.  POST: 1
                  (the file authored no Sun scale, and absent means 1 — ownership
                  is the FORMAT's, not the key's presence) + a Sun deprecation
                  line.  PRE: 3.
  L5 `twin_noflag` same file, `flag_moon_scaled = false`.  POST: 5 (the file) and
                  NO deprecation line — a config value that was not live was not
                  overridden, and saying so would be false.  PRE: 1.

Every leg is a FRESH launch on a temp-HOME farm (symlink mirror of
~/.spacecrafter with config.ini, log/ and modularSystem/ as REAL entries), so the
field pair 03fbee59/545a51ef is untouched and asserted in==out.

Usage:
    DISPLAY=:2 ./f41_ownership.py <absOutdir> --bin /abs/binary --tag pre|post \\
                                  [--twin /abs/composed.ini] [--legs L1,L2,...]

`--twin` is the composed file to ACTIVATE for L2-L5; L1 needs none and writes the
one it generated to <outdir>/<tag>_twin_generated.ini.
"""
import argparse
import hashlib
import json
import os
import re
import shutil
import socket
import subprocess
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import dumpread

HOME = Path.home()
FIELD = HOME / ".spacecrafter"
FARM = Path("/tmp/f41home")
JD = 2461234.0

FAILS = []
def fail(m): FAILS.append(m); print(f"FAIL: {m}", flush=True)
def ok(m):   print(f"ok:   {m}", flush=True)
def note(m): print(f"      {m}", flush=True)


def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()[:8]


def concurrent():
    n = 0
    for p in Path("/proc").iterdir():
        try:
            if (p / "comm").read_text().strip() == "spacecrafter":
                n += 1
        except Exception:
            pass
    return n


def build_farm(seds, twin):
    """Symlink mirror of the field ~/.spacecrafter; config.ini, log/ and
    modularSystem/ are REAL (the twin must be writable and must NOT reach the
    field). `twin`, when given, is installed as the ENABLED composed file, which
    is exactly §11.51(a)'s activation route: the same bytes, without .disabled."""
    if FARM.exists():
        shutil.rmtree(FARM)
    (FARM / ".spacecrafter").mkdir(parents=True)
    for f in FIELD.iterdir():
        if f.name in ("config.ini", "log", "modularSystem"):
            continue
        os.symlink(f, FARM / ".spacecrafter" / f.name)
    (FARM / ".spacecrafter" / "log").mkdir()
    cfg = FARM / ".spacecrafter" / "config.ini"
    shutil.copy2(FIELD / "config.ini", cfg)
    text = cfg.read_text(encoding="latin-1")
    for pat, repl in seds:
        text, n = re.subn(pat, repl, text, flags=re.M)
        assert n == 1, f"config edit {pat!r} matched {n} lines, expected 1"
    cfg.write_text(text, encoding="latin-1")
    md = FARM / ".spacecrafter" / "modularSystem"
    md.mkdir()
    for f in (FIELD / "modularSystem").iterdir():
        # The .disabled twins come along (they are read by nothing); an ENABLED
        # composed file of the field would silently serve this run, so it does
        # not - what serves is only what `twin` installs.
        if f.name.endswith(".ini.disabled"):
            shutil.copy2(f, md / f.name)
    if twin:
        shutil.copy2(twin, md / "SolarSystem.ini")
    return FARM


def wait_port(proc, timeout=150):
    t0 = time.time()
    while time.time() - t0 < timeout:
        if proc.poll() is not None:
            raise RuntimeError("app died before the port opened")
        try:
            return socket.create_connection(("127.0.0.1", 7805), timeout=1)
        except OSError:
            time.sleep(1)
    raise RuntimeError("port 7805 never opened")


def send(sock, cmd, pause=0.7):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.25); sock.recv(8192)
    except socket.timeout:
        pass
    sock.settimeout(None)


def load_new(path):
    """name -> the NEW path's record. dumpread holds the grammar (I2, §5.103)."""
    bodies = {}
    for line in open(path, encoding="utf-8", errors="replace"):
        if not line.strip():
            continue
        try:
            o = dumpread.loads(line)
        except json.JSONDecodeError:
            continue
        if o.get("type") == "body" and o.get("new") is not None:
            bodies[o["name"]] = o["new"]
    return bodies


DEPRECATION = re.compile(r"config\.ini \[viewing\] (\w+) = ([^ ]+) is IGNORED for '(\w+)'")


def run_leg(name, binary, tag, out, seds, twin):
    out.mkdir(parents=True, exist_ok=True)
    cfg_in, ssy_in = md5(FIELD / "config.ini"), md5(FIELD / "ssystem.ini")
    assert concurrent() == 0, "another spacecrafter is running"
    farm = build_farm(seds, twin)
    env = {**os.environ, "HOME": str(farm), "DISPLAY": os.environ.get("DISPLAY", ":2")}
    proc = subprocess.Popen([binary], cwd=str(farm / ".spacecrafter"),
                            stdout=open(out / f"{tag}_{name}.stdout", "w"),
                            stderr=subprocess.STDOUT, env=env)
    res = {"leg": name, "tag": tag, "binary_md5": md5(binary)}
    try:
        s = wait_port(proc)
        time.sleep(10)
        send(s, "flag experimental_path on")
        send(s, "timerate rate 0")
        send(s, "meteors zhr 0")
        send(s, f"date jday {JD}")
        # The 5 s ASmooth ramp the config value starts at init must have landed
        # before anything is read (§11.152(o): it settles at 1.00000012, so every
        # equality below is a TOLERANCE).
        time.sleep(8)

        def grab(label):
            p = out / f"{tag}_{name}_{label}.json"
            send(s, f"body action dual_dump filename {p}", 2.0)
            b = load_new(p)
            return {k: {f: b[k].get(f) for f in
                        ("scaling", "scalingTarget", "inheritedScaling",
                         "scaledDatumRadius", "composedDecl")}
                    for k in ("Moon", "Sun") if k in b}

        res["settled"] = grab("settled")
        # `body action reload` - the §5.104 subject. The observer is left where
        # the config put it; the reload is the whole event.
        send(s, "body action reload", 4)
        time.sleep(8)                      # a ramp, if one were started, lands
        res["reloaded"] = grab("reloaded")
        # THE COMMAND CONTROL, and it is what keeps the deprecation honest: the
        # ruling deprecates config.ini's value, NOT the operator's command, and
        # §11.152(c) says the file value is the authored DEFAULT under the
        # runtime `scaling`. So a command must still act on a body whose file
        # owns the default - on BOTH binaries, and identically.
        send(s, "set moon_scale 7", 1)
        send(s, "flag sun_scaled on", 1)
        time.sleep(8)
        res["commanded"] = grab("commanded")
        # ...and a reload is a LOAD (D31, §11.113(j)): it re-reads the file and
        # does NOT replay the operator's override. On a modular-served system
        # the Moon therefore returns to the FILE's value; on a legacy-served one
        # it keeps 7, because the command wrote the very holder config.ini owns.
        # This is also the SECOND traverse of the reload pair, entered from the
        # state the first exit produced.
        send(s, "body action reload", 4)
        time.sleep(8)
        res["reloaded2"] = grab("reloaded2")
        # The scaling toggle, traversed TWICE, each second entry starting from
        # the state the first exit produced (the standing rare-path rule).
        for i in (1, 2):
            send(s, "flag moon_scaled off", 1); time.sleep(8)
            res[f"moon_off{i}"] = grab(f"moon_off{i}")
            send(s, "flag moon_scaled on", 1); time.sleep(8)
            res[f"moon_on{i}"] = grab(f"moon_on{i}")
    finally:
        if proc.poll() is None:
            proc.terminate()
            try:
                proc.wait(8)
            except subprocess.TimeoutExpired:
                proc.kill()
        time.sleep(1)
    logs = sorted((farm / ".spacecrafter" / "log").glob("spacecrafter*.log"))
    applog = out / f"{tag}_{name}.applog"
    if logs:
        shutil.copy2(logs[-1], applog)
    res["deprecations"] = [m.groups() for m in DEPRECATION.finditer(
        applog.read_text(encoding="latin-1", errors="replace") if applog.exists() else "")]
    # Only a LEGACY-served leg generates a twin. A composed-served leg has the
    # field's copy sitting there untouched, and reporting it as "the twin this
    # leg produced" would be fiction.
    twin_out = farm / ".spacecrafter" / "modularSystem" / "SolarSystem.ini.disabled"
    if not twin and twin_out.exists():
        kept = out / f"{tag}_{name}_twin.ini.disabled"
        shutil.copy2(twin_out, kept)
        res["twin"] = str(kept)
        res["twin_display_scale"] = dict(re.findall(
            r"^\[(\w+)\][^\[]*?^display_scale = ([\d.eE+-]+)",
            twin_out.read_text(encoding="latin-1", errors="replace"), re.M | re.S))
    res["md5_in"] = [cfg_in, ssy_in]
    res["md5_out"] = [md5(FIELD / "config.ini"), md5(FIELD / "ssystem.ini")]
    if res["md5_in"] != res["md5_out"]:
        fail(f"{tag}/{name}: FIELD CONFIG TOUCHED {res['md5_in']} -> {res['md5_out']}")
    return res


LEGS = {
    # name         config edits                                        needs twin
    "L1_legacy":     ([], False),
    "L2_twin":       ([], True),
    "L3_twin_cfg2":  ([(r"^moon_scale .*$", "moon_scale                     = 2")], True),
    "L4_twin_sunon": ([(r"^flag_sun_scaled .*$", "flag_sun_scaled                = true"),
                       (r"^sun_scale .*$", "sun_scale                      = 3")], True),
    "L5_twin_noflag":([(r"^flag_moon_scaled .*$", "flag_moon_scaled               = false")], True),
}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--bin", required=True)
    ap.add_argument("--tag", required=True, choices=("pre", "post"))
    ap.add_argument("--twin", default="")
    ap.add_argument("--legs", default=",".join(LEGS))
    a = ap.parse_args()
    out = Path(a.outdir)
    out.mkdir(parents=True, exist_ok=True)
    results = {}
    for name in a.legs.split(","):
        seds, needs_twin = LEGS[name]
        if needs_twin and not a.twin:
            print(f"skip {name}: no --twin"); continue
        print(f"--- {a.tag} / {name} ---", flush=True)
        results[name] = run_leg(name, a.bin, a.tag, out, seds,
                                a.twin if needs_twin else "")
        r = results[name]
        for phase in ("settled", "reloaded", "commanded", "reloaded2",
                      "moon_off1", "moon_on1", "moon_off2", "moon_on2"):
            if phase in r:
                note(f"{phase:10s} " + "  ".join(
                    f"{b}={r[phase][b]['scalingTarget']}/{r[phase][b]['scaling']}"
                    for b in ("Moon", "Sun") if b in r[phase]))
        note(f"deprec   {r['deprecations']}")
        note(f"twin ds  {r.get('twin_display_scale')}")
    (out / f"{a.tag}_results.json").write_text(json.dumps(results, indent=1))
    print(f"\nwrote {out}/{a.tag}_results.json")
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
