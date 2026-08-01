#!/usr/bin/env python3
"""F21 gate, part 2: the per-body override ledger (b31-design §2 group D, §6.2
T7/T8; INTENT §11.129).

The ledger is a DELTA, not a snapshot: a field is written only when it differs
from what the DATA gave the body at load. Everything below exists to show that
this is true and that it is what makes the two hard checks pass.

  T2-ledger   the ledger's own round trip: overrides set by command, saved, the
              app QUIT, a fresh launch, restored - and the ledger the restored
              app writes is the one the first app wrote.
  T2-CONTROL  the same fresh launch, dumped BEFORE the restore: the overrides
              must NOT already be there, or the round trip proves nothing.
  T4-ledger   with a POPULATED ledger, the second session file is still
              byte-identical to the first (the fixed point survives the new
              sections).
  T5b-ledger  ONE ledger key is edited by hand; exactly that body's field moves
              and nothing else does.
  T7          a body is RENAMED between save and restore. The override must be
              REPORTED (log + an inline annotation in the session file) and NOT
              applied to anything else - shown BOTH WAYS, because the same
              session restored against the ORIGINAL name must still apply.
  T8          an AUTHORED value changes between save and restore. The NEW
              authored value must be in effect AND the operator's override must
              still apply on top. A snapshot design fails this by construction,
              and the counterfactual is computed from the file itself.

The rename and the authored change use `body action load`, which is the runtime
declaration channel: no file in the frozen corpus is touched by this gate, and
the md5s are asserted in == out around it.

    cd claude/harness && DISPLAY=:2 ./f21_ledger.py [outdir]
"""

import json
import re
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import f21_s563 as H

_args = [a for a in sys.argv[1:] if not a.startswith("--")]
OUT = (Path(_args[0]) if _args else Path(__file__).resolve().parent / "artifacts/f21ledger").resolve()
OUT.mkdir(parents=True, exist_ok=True)
H.OUT = OUT
SESSIONS = H.SESSIONS
FAILS = []

COMMON = ("parent Sun type Planet oblateness 0.0 albedo 0.3 halo false "
          "tex_map bodies/moon.png coord_func still_orbit "
          "orbit_x 900 orbit_y 0 orbit_z 0")


def fail(m):
    FAILS.append(m)
    print("FAIL: " + m, flush=True)


def ok(m):
    print("ok:   " + m, flush=True)


def ledger(path):
    """{body: {key: value}} from the session file's [body:*] sections."""
    out, cur = {}, None
    for ln in Path(path).read_text().splitlines():
        s = ln.strip()
        if s.startswith("[") and "]" in s:
            h = s[1:s.index("]")]
            cur = h[5:] if h.startswith("body:") else None
            if cur:
                out[cur] = {}
            continue
        if not cur or not s or s.startswith("#") or "=" not in s:
            continue
        k, v = s.split("=", 1)
        out[cur][k.strip()] = v.strip()
    return out


def dump_body(app, tag, name):
    p = OUT / f"f21L_{tag}.json"
    p.unlink(missing_ok=True)
    app.cmd(f"body action dual_dump filename {p}", 2.0)
    for _ in range(20):
        if p.exists():
            break
        time.sleep(0.2)
    for ln in p.read_text().splitlines():
        ln = ln.strip().replace("-nan", "null").replace("nan", "null")
        if not ln:
            continue
        o = json.loads(ln)
        if o.get("type") == "body" and o.get("name") == name:
            return o.get("new") or {}
    return {}


def save(app, name):
    p = SESSIONS / f"{name}.ini"
    p.unlink(missing_ok=True)
    app.cmd(f"session action save filename {name}", 1.6)
    for _ in range(25):
        if p.exists():
            break
        time.sleep(0.2)
    return p


def overrides(app):
    """The scene: one override of every group-D row that has a command."""
    app.cmd("timerate rate 0", 1.0)
    app.cmd("date jday 2461233.5", 1.0)
    app.cmd("body name Venus hidden true", 0.6)                    # D1
    app.cmd("planet_scale name Mars scale 2", 0.6)                 # D2
    app.cmd("body name Mars color halo color_value 200,10,10", 0.6)  # D3
    app.cmd("body name Earth color label color_value 10,200,10", 0.6)  # D4
    app.cmd("body name Mars datum_radius 1000", 0.6)               # D6
    app.cmd("body name Jupiter orbit true", 0.6)                   # D8


def main():
    if not H.assert_no_other_instance():
        return 1
    SESSIONS.mkdir(exist_ok=True)
    for f in SESSIONS.glob("f21L*.ini"):
        f.unlink()
    frozen_in = {n: H.md5(H.USERDIR / n) for n in H.FROZEN if (H.USERDIR / n).exists()}

    # ---------------- launch 1: the overrides, and the save ----------------
    app = H.App("save")
    overrides(app)
    # T8's subject: a body DECLARED at runtime with an authored value, then
    # overridden on a DIFFERENT field. `datum_radius` is the authored one and
    # the halo colour is the operator's.
    app.cmd(f"body action load name LedgerA radius 6000 datum_radius 5000 "
            f"color 0.9,0.9,0.9 {COMMON}", 2.0)
    app.cmd("body name LedgerA color halo color_value 10,10,200", 0.8)
    # T7's own subject, kept separate from T8's: the body whose NAME the data
    # will lose. It must be a different body, or T8's need for LedgerA to exist
    # would make the rename resolve and the miss could never fire.
    app.cmd(f"body action load name LedgerT7 radius 6000 datum_radius 5000 "
            f"color 0.9,0.9,0.9 {COMMON.replace('orbit_x 900', 'orbit_x 1200')}", 2.0)
    app.cmd("body name LedgerT7 color halo color_value 10,200,10", 0.8)
    a_saved = dump_body(app, "a_saved", "LedgerA")
    f1 = save(app, "f21L")
    first = f1.read_bytes()
    f1b = save(app, "f21Lb")
    led1 = ledger(f1)
    print(f"   the ledger carries {len(led1)} body/bodies: {sorted(led1)}", flush=True)
    app.quit()

    for want, key in (("Venus", "hidden"), ("Mars", "scale"), ("Mars", "datum_radius"),
                      ("Mars", "halo_color"), ("Earth", "label_color"),
                      ("Jupiter", "orbit"), ("LedgerA", "halo_color"),
                      ("LedgerT7", "halo_color")):
        if key not in led1.get(want, {}):
            fail(f"the ledger did not record {want}.{key}")
    if all(k in led1.get("Mars", {}) for k in ("scale", "datum_radius", "halo_color")):
        ok("the ledger records one entry per CHANGED field (D30's delta), "
           f"e.g. Mars: {sorted(led1['Mars'])}")
    # The delta property itself: a body nobody touched has no section.
    if "Saturn" in led1:
        fail("Saturn is in the ledger and nothing overrode it - this is a snapshot, "
             "not a delta")
    else:
        ok("a body nobody overrode has NO section (so the file is a delta)")
    if any("path" not in v for v in led1.values()):
        fail("a ledger entry carries no qualified path for the miss report")
    else:
        ok("every entry carries the system-qualified path, for the miss report only "
           f"(e.g. {led1['Venus']['path']})")

    # ---------------- launch 2: control, restore, T4, T2 ----------------
    app = H.App("restore")
    app.cmd("timerate rate 0", 1.0)
    ctrl = save(app, "f21Lctrl")
    ledc = ledger(ctrl)
    already = [b for b in ("Venus", "Mars", "Earth", "Jupiter") if b in ledc]
    if already:
        fail(f"T2-CONTROL: {already} already carry overrides in a fresh launch — "
             f"the round trip below would prove nothing")
    else:
        ok(f"T2-CONTROL: a fresh launch's ledger has none of the overrides "
           f"({len(ledc)} unrelated entries)")
    # LedgerA must exist for its own override to apply: it is runtime-declared
    # content, and D31 says a declaration is a LOAD. Same declaration, same name.
    app.cmd(f"body action load name LedgerA radius 6000 datum_radius 5000 "
            f"color 0.9,0.9,0.9 {COMMON}", 2.0)
    app.cmd(f"body action load name LedgerT7 radius 6000 datum_radius 5000 "
            f"color 0.9,0.9,0.9 {COMMON.replace('orbit_x 900', 'orbit_x 1200')}", 2.0)
    app.cmd("session action load filename f21L", 4.0)
    f2 = save(app, "f21L2")
    led2 = ledger(f2)
    moved = []
    for b, kv in led1.items():
        for k, v in kv.items():
            if led2.get(b, {}).get(k) != v:
                moved.append(f"{b}.{k}: {v} -> {led2.get(b, {}).get(k)}")
    if moved:
        fail(f"T2-ledger: {len(moved)} ledger field(s) did not come back: {moved[:6]}")
    else:
        ok(f"T2-ledger: every field of all {len(led1)} entries came back through "
           f"quit + fresh launch + restore")
    if f2.read_bytes() != first:
        fail(f"T4-ledger: the session written from the restored state is not "
             f"byte-identical ({len(f2.read_bytes())} vs {len(first)} B)")
    else:
        ok(f"T4-ledger: the session written from the restored state is BYTE-IDENTICAL "
           f"with a populated ledger ({len(first)} B)")
    app.quit()

    # ---------------- launch 3: T5b on a ledger key ----------------
    txt = f1.read_text()
    assert "scale = 2" in txt
    Path(f1).write_text(txt.replace("scale = 2", "scale = 3", 1))
    app = H.App("t5b")
    app.cmd("timerate rate 0", 1.0)
    app.cmd(f"body action load name LedgerA radius 6000 datum_radius 5000 "
            f"color 0.9,0.9,0.9 {COMMON}", 2.0)
    app.cmd(f"body action load name LedgerT7 radius 6000 datum_radius 5000 "
            f"color 0.9,0.9,0.9 {COMMON.replace('orbit_x 900', 'orbit_x 1200')}", 2.0)
    app.cmd("session action load filename f21L", 4.0)
    f3 = save(app, "f21L3")
    led3 = ledger(f3)
    diffs = []
    for b in set(led1) | set(led3):
        for k in set(led1.get(b, {})) | set(led3.get(b, {})):
            if led1.get(b, {}).get(k) != led3.get(b, {}).get(k):
                diffs.append(f"{b}.{k}")
    if diffs != ["Mars.scale"]:
        fail(f"T5b-ledger: editing Mars.scale moved {diffs}, expected exactly ['Mars.scale']")
    else:
        ok("T5b-ledger: one hand-edited override moves exactly its own field "
           "(Mars.scale) and nothing else")
    app.quit()
    Path(f1).write_text(txt)   # put the file back

    # ---------------- launch 4: T7 (the rename) + T8 (the authored change) ----
    app = H.App("t7t8")
    app.cmd("timerate rate 0", 1.0)
    app.cmd("date jday 2461233.5", 1.0)
    # T7: the body the session's key names is GONE from the data - it is
    # declared under a new name instead. T8 rides the same launch: LedgerA is
    # re-declared with a DIFFERENT authored datum_radius.
    app.cmd(f"body action load name LedgerRenamed radius 6000 datum_radius 5000 "
            f"color 0.9,0.9,0.9 {COMMON.replace('orbit_x 900', 'orbit_x 1200')}", 2.0)
    app.cmd(f"body action load name LedgerA radius 6000 datum_radius 7000 "
            f"color 0.9,0.9,0.9 {COMMON}", 2.0)
    app.cmd("body name Venus hidden false", 0.6)
    ren_before = dump_body(app, "ren_before", "LedgerRenamed")
    app.cmd("session action load filename f21L", 4.0)
    a_after = dump_body(app, "a_after", "LedgerA")
    ren_after = dump_body(app, "ren_after", "LedgerRenamed")
    log = (OUT / "f21s_t7t8.applog").read_text(errors="replace")

    # --- T7, first way: the miss is REPORTED
    missed = re.findall(r"no body of that name is in the tree now", log)
    if not missed:
        fail("T7: no miss was reported for an override whose body the data renamed")
    else:
        ok(f"T7: the unresolved override is reported ({len(missed)} entry/entries), "
           f"with the path the save recorded")
    # --- T7, second way: it was applied to NOTHING ELSE
    if ren_after.get("haloColor") != ren_before.get("haloColor"):
        fail(f"T7: the renamed body's halo colour MOVED "
             f"({ren_before.get('haloColor')} -> {ren_after.get('haloColor')}) — the "
             f"override bound to a body it does not name")
    else:
        ok(f"T7: the renamed body is untouched ({ren_after.get('haloColor')}) — the "
           f"override was not applied to a near match")
    # --- T7, the annotation is IN the file
    ann = f1.read_text()
    if "#!sc:" not in ann or "override-key-unresolved" not in ann and "no body of that name" not in ann:
        fail("T7: the session file carries no inline annotation for the unresolved key")
    else:
        ok("T7: the session file itself carries the `#!sc:` annotation above the entry "
           "(report-and-KEEP: the entry is still there)")
    kept = ledger(f1)
    if "Venus" not in kept or "Mars" not in kept:
        fail("T7: entries were dropped from the file instead of being kept")
    else:
        ok(f"T7: all {len(kept)} entries are still in the file after the miss")

    # --- T8: the NEW authored value, with the override on top
    dr = a_after.get("scaledDatumRadius")
    halo = a_after.get("haloColor")
    AU_KM = 149597870.7
    dr_km = dr * AU_KM if isinstance(dr, (int, float)) else None
    saved_halo = a_saved.get("haloColor")
    if dr_km is None or abs(dr_km - 7000) > 1.0:
        fail(f"T8: the NEW authored datum_radius (7000 km) is not in effect after the "
             f"restore: {dr_km} km")
    else:
        ok(f"T8: the NEW authored value is in effect ({dr_km:.1f} km, was 5000 km when "
           f"the session was saved) — a snapshot design would have restored 5000")
    if halo != saved_halo:
        fail(f"T8: the operator's override did NOT survive on top: {halo} vs {saved_halo}")
    else:
        ok(f"T8: the operator's override still applies on top of the new authored value "
           f"({halo})")
    if "datum_radius" in led1.get("LedgerA", {}):
        fail("T8 counterfactual is void: the ledger recorded LedgerA.datum_radius, so the "
             "restore would have written 5000 back and the check tests nothing")
    else:
        ok("T8 counterfactual: the ledger carries NO datum_radius for LedgerA (nobody "
           "overrode it), which is exactly why the new authored value survives — a "
           "snapshot would carry it and fail")
    app.quit()

    frozen_out = {n: H.md5(H.USERDIR / n) for n in H.FROZEN if (H.USERDIR / n).exists()}
    if frozen_in != frozen_out:
        fail("frozen md5 in != out")
    else:
        ok("frozen config/ssystem/galactic/anchor md5 in == out")

    print(f"\n{'FAILURES: ' + str(len(FAILS)) if FAILS else 'ALL GREEN'}", flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
