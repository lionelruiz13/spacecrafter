#!/usr/bin/env python3
"""B40 cross-binary tree parity on the REAL install (INTENT §11.115; ledger B40).

WHAT IT IS FOR. B40 lands two repairs in one commit — the `.galactic.ini`
path-concat regression (§5.37) and the hardened, whitespace- and comment-
tolerant `.ini` line grammar shared by every reader of the family (§5.38/§5.39,
D29). Both act at LOAD time on the shipped, frozen corpus (§2.0 D9), so the
question this gate answers is the D9 one: *what did the shipped data mean
before, and what does it mean now* — per body, per field, measured, never
argued from the diff.

    cd claude/harness && DISPLAY=:2 ./b40_parity.py <outdir> tag=/abs/binary ...

Each `tag=binary` pair is launched FRESH on the real `~/.spacecrafter` (the
same protocol as `b24_equivalence.py` phase A, whose comparison authority this
file IMPORTS rather than copies — I2), driven to the same frozen scene, and
dumped. Every pair of tags is then compared:

  - **body set**: names present under one binary and not the other are listed
    explicitly. This is the leg that carries the AUTHORISED change: repairing
    the path adds the galactic system NODES (17 on the shipped corpus, one per
    `galactic.ini` entry that names a system file), and nothing else may appear.
  - **per-body fields** on the COMMON set: `parent/relation/modules/routing/
    lastJD/bodyType/surfaceModel/trailLength` exact, `ecl`/`boundingRadius`
    under `b24_equivalence`'s own float32 jitter tolerance. Zero divergence on
    the 103 pre-existing bodies is the D9 claim; it is re-measured here rather
    than inherited from §11.113(h)'s derivation.
  - **galactic coordinates**: the `ecl` of every `<X>System` node is checked
    against `galactic.ini`'s own x/y/z as read by a CORRECT parse. This is
    §5.38's discriminator: five lost minus signs and two lost leading digits
    put six of seventeen systems at wrong coordinates, so a parse regression
    shows up as a sign flip here, not as a subtle drift.
  - **evidence from the app's own stdout**: `Params :` blocks (one per
    galactic section actually read — the §11.109(a) instrument) and the new
    malformed-line warnings.
  - **D9 hygiene**: `config.ini` / `ssystem.ini` / `galactic.ini` /
    `anchor.ini` md5 asserted in == out around every launch. The machine-owned
    `modularSystem/` twins are EXPECTED to change (they are regenerated at
    every legacy load and they record what the loader read) and are reported,
    not asserted.
"""

import hashlib, json, os, re, socket, subprocess, sys, time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import b24_equivalence as b24   # the per-body comparison authority (I2)

HERE = Path(__file__).resolve().parent
USERDIR = Path.home() / ".spacecrafter"
JD = "2461233.5"
FROZEN = ("config.ini", "ssystem.ini", "galactic.ini", "anchor.ini")

FAILS = []


def fail(msg):
    FAILS.append(msg)
    print(f"FAIL: {msg}", flush=True)


def ok(msg):
    print(f"ok:   {msg}", flush=True)


def md5(p):
    h = hashlib.md5()
    with open(p, "rb") as f:
        for b in iter(lambda: f.read(1 << 20), b""):
            h.update(b)
    return h.hexdigest()


def frozen_md5():
    return {n: md5(USERDIR / n) for n in FROZEN if (USERDIR / n).exists()}


def twin_md5():
    d = USERDIR / "modularSystem"
    return {p.name: md5(p) for p in sorted(d.iterdir())} if d.is_dir() else {}


def galactic_truth():
    """`galactic.ini` read by a CORRECT parse: {name: (x, y, z)} for every
    section that names a system file (those are the ones that become nodes)."""
    out, params = {}, {}
    raw = (USERDIR / "galactic.ini").read_bytes().decode("latin-1")

    def flush():
        if params.get("name") and params.get("system"):
            out[params["name"]] = tuple(float(params[k]) for k in "xyz")
        params.clear()

    for line in raw.split("\n"):
        h = line.find("#")
        if h != -1:
            line = line[:h]
        line = line.strip()
        if not line:
            continue
        if line.startswith("["):
            flush()
            continue
        eq = line.find("=")
        if eq != -1:
            params[line[:eq].strip()] = line[eq + 1:].strip()
    flush()
    return out


def run(tag, binary, out):
    dump = out / f"b40_{tag}.json"
    dump.unlink(missing_ok=True)
    applog = out / f"b40_{tag}.applog"
    before = frozen_md5()
    proc = subprocess.Popen([binary], cwd=str(USERDIR),
                            stdout=open(applog, "w"), stderr=subprocess.STDOUT,
                            env={**os.environ, "DISPLAY": os.environ.get("DISPLAY", ":2")})
    sock = b24.wait_port()
    time.sleep(10)
    b24.send(sock, "timerate rate 0")
    b24.send(sock, f"date jday {JD}")
    time.sleep(3)
    b24.send(sock, f"body action dual_dump filename {dump}")
    time.sleep(2)
    b24.send(sock, "shutdown action now")
    sock.close()
    try:
        rc = proc.wait(timeout=40)
    except subprocess.TimeoutExpired:
        proc.kill()
        fail(f"{tag}: app did not exit within 40 s after shutdown")
        rc = -1
    if not dump.exists():
        raise RuntimeError(f"{tag}: dump {dump} was not written")
    after = frozen_md5()
    if before != after:
        fail(f"D9 VIOLATION ({tag}): frozen file(s) rewritten: "
             f"{ {k: (before.get(k), after.get(k)) for k in before if before[k] != after.get(k)} }")
    else:
        ok(f"{tag}: frozen data md5 in == out ({', '.join(sorted(before))})")
    log = applog.read_text(errors="replace")
    return {
        "tag": tag, "binary": binary, "rc": rc, "dump": dump,
        "params_blocks": log.count("Params :"),
        "malformed": len(re.findall(r"Ignoring line without '='", log)),
        "twins": twin_md5(), "log": log,
    }


def compare(a, b):
    print(f"\n--- {a['tag']} vs {b['tag']} ---", flush=True)
    _, ba = b24.load_dump(a["dump"])
    _, bb = b24.load_dump(b["dump"])
    only_a, only_b = sorted(set(ba) - set(bb)), sorted(set(bb) - set(ba))
    print(f"    bodies: {a['tag']}={len(ba)}  {b['tag']}={len(bb)}", flush=True)
    if only_a:
        print(f"    only in {a['tag']}: {only_a}", flush=True)
    if only_b:
        print(f"    only in {b['tag']}: {only_b}", flush=True)
    common = sorted(set(ba) & set(bb))
    diffs = []
    for name in common:
        na, nb = ba[name], bb[name]
        for field in ("parent", "relation", "modules", "routing", "lastJD",
                      "bodyType", "surfaceModel", "trailLength"):
            if na.get(field) != nb.get(field):
                diffs.append(f"{name}.{field}: {na.get(field)!r} != {nb.get(field)!r}")
        if not b24.floats_close(",".join(map(str, na.get("ecl", []))),
                                ",".join(map(str, nb.get("ecl", [])))):
            diffs.append(f"{name}.ecl: {na.get('ecl')} != {nb.get('ecl')}")
        if not b24.scalar_close(na.get("boundingRadius"), nb.get("boundingRadius")):
            diffs.append(f"{name}.boundingRadius: {na.get('boundingRadius')} != {nb.get('boundingRadius')}")
    for d in diffs:
        print(f"    DIFF {d}", flush=True)
    print(f"    common bodies={len(common)}  divergent fields={len(diffs)}", flush=True)
    return only_a, only_b, common, diffs


def check_coordinates(res):
    """§5.38: every galactic system NODE sits at galactic.ini's own x/y/z."""
    truth = galactic_truth()
    _, bodies = b24.load_dump(res["dump"])
    checked = wrong = 0
    for name, xyz in sorted(truth.items()):
        node = bodies.get(name + "System")
        if node is None:
            continue
        checked += 1
        ecl = node.get("ecl")
        if ecl is None or len(ecl) != 3:
            fail(f"{res['tag']}: {name}System has no ecl")
            wrong += 1
            continue
        bad = [i for i in range(3) if not b24._close(float(ecl[i]), xyz[i])]
        if bad:
            wrong += 1
            print(f"    WRONG {name}System: dump {tuple(ecl)} != galactic.ini {xyz}", flush=True)
    print(f"    {res['tag']}: {checked} system nodes checked against galactic.ini, "
          f"{wrong} at wrong coordinates", flush=True)
    return checked, wrong


def main():
    out = Path(sys.argv[1]).resolve()
    out.mkdir(parents=True, exist_ok=True)
    runs = []
    for spec in sys.argv[2:]:
        tag, _, binary = spec.partition("=")
        print(f"\n=== launching {tag}: {binary}", flush=True)
        runs.append(run(tag, binary, out))
        r = runs[-1]
        print(f"    'Params :' blocks={r['params_blocks']}  malformed-line warnings={r['malformed']}  "
              f"exit={r['rc']}  twins={len(r['twins'])}", flush=True)

    summary = {"runs": [], "compares": []}
    for r in runs:
        checked, wrong = check_coordinates(r)
        summary["runs"].append({
            "tag": r["tag"], "binary": r["binary"], "rc": r["rc"],
            "params_blocks": r["params_blocks"], "malformed": r["malformed"],
            "twins": r["twins"], "nodes_checked": checked, "nodes_wrong": wrong,
        })
    for i in range(len(runs) - 1):
        a, b = runs[i], runs[i + 1]
        only_a, only_b, common, diffs = compare(a, b)
        tw = sorted(k for k in set(a["twins"]) | set(b["twins"])
                    if a["twins"].get(k) != b["twins"].get(k))
        print(f"    twins differing between the two launches: {tw}", flush=True)
        summary["compares"].append({
            "a": a["tag"], "b": b["tag"], "only_a": only_a, "only_b": only_b,
            "common": len(common), "diffs": diffs, "twins_differing": tw,
        })

    (out / "b40_parity.json").write_text(json.dumps(summary, indent=1))
    print(f"\n{'OK' if not FAILS else f'{len(FAILS)} FAILS'} -> {out}/b40_parity.json", flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
