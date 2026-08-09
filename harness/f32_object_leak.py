#!/usr/bin/env python3
"""F32 / INTENT §5.34 — `Object::operator=` never released the previous rep.

    cd claude/harness && export XAUTHORITY=$(ls /run/user/$(id -u)/.mutter-Xwaylandauth.*) \
        && DISPLAY=:2 ./f32_object_leak.py <absOutdir> --mode discover --bin <binary>
    ... ASAN_OPTIONS=... ./f32_object_leak.py <absOutdir> --mode leak \
        --tag asan_pre --bin <asan binary> --expect pre

WHAT §5.34 RECORDS. Both `Object::operator=` overloads (`tools/object.cpp:105-126`)
overwrote `rep` after retaining the new one, so every reassignment of an `Object`
LEAKED one reference. Two live instances of the class:
  - OLD path: a STAR selection change orphans its refcounted `StarWrapperBase`
    (`hip_star_wrapper.hpp:67-73`, delete-at-zero — a count that never reaches 0).
  - NEW path: a composed-body selection change orphans one `ModularObject`
    (`ModularObject.hpp:24-30`) AND leaves its `ModularBodyPtr` permanently in the
    static `ModularBodyPtr::ref` vector, which every body destruction scans
    (`ModularBodyPtr.cpp:32-46`).
Bodies/nebulae/constellations are unaffected: their `retain`/`release` are the
`ObjectBase` no-ops, which is why the defect stayed invisible.

THE OBSERVABLE. One fresh launch per run under ASan/LSan; the drive changes the
selection N times on each path; at `shutdown action now` LeakSanitizer reports
what is unreachable. A wrapper the fix releases correctly is FREED, so it is not
in the report; a wrapper whose count never reaches zero is. Blocks are attributed
by their own allocation stack — `Star1::createStelObject` (hip_star_wrapper.cpp)
for the old path, `SSystemFactory::searchObjectByEnglishName`/`new ModularObject`
for the new one — so the count is per-defect, not a heap total.

WHY TWO MODES. `--mode discover` is a separate fresh launch whose only job is to
ask the app WHICH HIP ids its own catalogue answers for (§11.51(d): the ladder is
read from the installed data through the app, never recalled). Its output file is
the input of `--mode leak`, so the leak run's predicted count is a hard number
written before that run starts.

THE COMPOSED BODIES come from the b24_select fixture (imported, not copied — I2):
four `compose = explicit` bodies grounded on the Moon, appended to the app's own
machine-generated composed twin, in the farm's `modularSystem/` (the real
`~/.spacecrafter` is never written; the frozen md5 pair is asserted in == out by
`f27_reply.Session`).
"""

import argparse, json, os, re, sys, time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import f27_reply as f27
import b24_select as b24s

HERE = Path(__file__).resolve().parent
DEFAULT_BIN = str(HERE.parents[1] / "build-claude/src/spacecrafter")
REAL_HOME = Path.home()

FAILS, NOTES = [], []


def fail(m):
    FAILS.append(m)
    print(f"FAIL: {m}", flush=True)


def ok(m):
    print(f"ok:   {m}", flush=True)


def note(m):
    NOTES.append(m)
    print(f"      {m}", flush=True)


# --------------------------------------------------------------- the fixture
def make_prepare():
    """Author the composed system in the farm before the launch. The composed
    grammar and the body set are b24_select's (§11.106's own fixture)."""
    twin_path = REAL_HOME / ".spacecrafter/modularSystem/SolarSystem.ini.disabled"
    if not twin_path.exists():
        raise RuntimeError("composed twin absent - launch once on the shipped state first")
    twin = twin_path.read_bytes()
    sections, _mr = b24s.scene_sections({n: r for n, r, _a, _b in b24s.BODIES})

    def prepare(dst):
        (dst / "modularSystem/SolarSystem.ini").write_bytes(
            twin + sections.encode("latin-1"))
    return prepare


COMPOSED = [n for n, _r, _a, _b in b24s.BODIES]     # BigA SmallB FarC OnDisc


# ------------------------------------------------------------ reading the app
def obj_info(c, deadline=6.0):
    """`get status object` -> Core::getSelectedObjectInfo() ->
    selected_object.getInfoString(). "EOL" when nothing is selected. This is the
    per-step witness that a selection took AND that the rep behind it is live.

    The socket is DRAINED first and the answer is then waited for up to
    `deadline`: a fixed pause turns a slow frame into a fake "nothing selected",
    which is how the first sweep of this script reported 3 of 14 HIP ids
    answering when the catalogue in fact answered for 13 of them."""
    c.read(0.25)                       # drop anything still in flight
    c.send("get status object", 0)
    t0 = time.time()
    while time.time() - t0 < deadline:
        raw = c.read(0.25)
        msgs = [m.strip() for m in f27.messages(raw)]
        for m in reversed(msgs):
            if m and not m.startswith("$"):
                return m
    return None


def first_line(s):
    return "" if s is None else s.splitlines()[0].strip()


# ------------------------------------------------------------------ discovery
def mode_discover(args, outdir):
    """One fresh launch: which HIP ids does the installed catalogue answer for,
    and what does each one report? The ladder walked here is a fixed arithmetic
    sweep; what it KEEPS comes from the app."""
    sess = f27.Session(outdir, args.tag, args.bin, env_extra=args.env_extra,
                       port_wait=args.port_wait)
    c = sess.client("driver")
    found = []
    try:
        c.send("timerate rate 0", 1)
        c.send("date jday 2461233.5", 1)
        c.send("deselect", 0.5)
        base = first_line(obj_info(c))
        note(f"nothing selected -> {base!r}")
        for hp in args.sweep:
            c.send(f"select hp {hp} pointer off", 0.5)
            info = first_line(obj_info(c))
            if info and info != base and info != "EOL":
                found.append({"hp": hp, "info": info})
            c.send("deselect", 0.4)
        ok(f"{len(found)}/{len(args.sweep)} swept HIP ids answer")
        seen, uniq = set(), []
        for f in found:
            if f["info"] not in seen:
                seen.add(f["info"])
                uniq.append(f)
        if len(uniq) != len(found):
            note(f"{len(found) - len(uniq)} duplicate info strings dropped")
        (outdir / "f32_hip.json").write_text(json.dumps(uniq, indent=1))
        ok(f"wrote {len(uniq)} distinct-answer HIP ids -> {outdir}/f32_hip.json")
    finally:
        rc = sess.stop(c, exit_wait=args.exit_wait)
        note(f"app exit rc={rc}")
    return uniq


# ---------------------------------------------------------------- the leak run
def mode_leak(args, outdir):
    hip = json.loads((outdir / "f32_hip.json").read_text())
    if not hip:
        raise RuntimeError("no HIP id discovered - run --mode discover first")
    ids = [h["hp"] for h in hip]
    # cycle the discovered ids: `searchHP` mints a FRESH StarWrapper on every
    # call, so re-selecting a star already selected two steps ago is a real
    # selection change with a real new wrapper (and never the
    # `isSameLogicalObject` early return, which only fires against the CURRENT
    # selection).
    ladder = [ids[i % len(ids)] for i in range(args.stars)]

    # ---- prediction, written BEFORE the launch --------------------------
    # star wrappers minted = one per `select hp` that resolves
    n_star = len(ladder) + 2                  # ladder + the reversible pair x2
    # modular bridges minted = one per `select planet <new-only name>`
    n_comp = 1 + len(COMPOSED) + 2 + 1        # probe + churn + pair x2 + post-reload
    pred = {
        "hip_ladder": ladder,
        "composed": COMPOSED,
        "star_wrappers_minted": n_star,
        "modular_bridges_minted": n_comp,
        "pre": {
            # every selection change retains the new rep and drops the old one on
            # the floor. At `shutdown action now` the star side has ONE wrapper
            # still held (`SSystemFactory::selected_object` is written for stars
            # and never cleared, ssystem_factory.hpp:164 / core.cpp:2318), so it
            # is unreachable only once the app object graph is destroyed; the
            # modular side ends on `deselect`, which holds nothing.
            "star_wrappers_unreleased": [n_star - 1, n_star],
            "modular_bridges_unreleased": [n_comp],
        },
        "post": {"star_wrappers_unreleased": [0], "modular_bridges_unreleased": [0]},
        "caveat": "a leaked ModularObject stays REGISTERED in the static "
                  "ModularBodyPtr::ref vector; if LSan's check runs before that "
                  "global is destroyed the bridges read as reachable and the "
                  "modular count is 0 on BOTH binaries - in which case the "
                  "modular leg is not this instrument's to answer (gdb leg).",
    }
    (outdir / f"f32_predict_{args.tag}.json").write_text(json.dumps(pred, indent=1))
    print(json.dumps(pred, indent=1), flush=True)

    sess = f27.Session(outdir, args.tag, args.bin, prepare=make_prepare(),
                       env_extra=args.env_extra, port_wait=args.port_wait)
    c = sess.client("driver")
    r = {"tag": args.tag, "bin": str(args.bin), "expect": args.expect,
         "pred": pred, "steps": []}
    try:
        c.send("timerate rate 0", 1)
        c.send("date jday 2461233.5", 1)
        c.send("deselect", 0.5)
        empty = first_line(obj_info(c))
        r["empty_readout"] = empty

        # ---- the composed bodies must exist at all (the fixture's own control)
        c.send(f"select planet {COMPOSED[0]} pointer off", 0.8)
        got = first_line(obj_info(c))
        r["composed_probe"] = got
        if got and COMPOSED[0] in got:
            ok(f"composed body {COMPOSED[0]} is selectable: {got!r}")
        else:
            fail(f"composed fixture did not load: 'select planet {COMPOSED[0]}' -> {got!r}")
        c.send("deselect", 0.5)

        # ---- STAR CHURN (old path) -------------------------------------
        for hp in ladder:
            c.send(f"select hp {hp} pointer off", 0.6)
            info = first_line(obj_info(c))
            r["steps"].append({"cmd": f"select hp {hp}", "info": info})
            if not info or info == "EOL":
                fail(f"select hp {hp}: nothing selected ({info!r})")
        # the reversible pair, entered TWICE, the second entry from the first
        # exit's state
        for entry in (1, 2):
            c.send("deselect", 0.5)
            offv = first_line(obj_info(c))
            c.send(f"select hp {ladder[0]} pointer off", 0.5)
            onv = first_line(obj_info(c))
            r["steps"].append({"cmd": f"star pair entry {entry}",
                               "off": offv, "on": onv})
            if offv != empty:
                fail(f"star pair entry {entry}: deselect readout {offv!r} != {empty!r}")
            if not onv or onv == "EOL":
                fail(f"star pair entry {entry}: reselect gave {onv!r}")
        c.send("deselect", 0.5)

        # ---- COMPOSED CHURN (new path) ---------------------------------
        for name in COMPOSED:
            c.send(f"select planet {name} pointer off", 0.6)
            info = first_line(obj_info(c))
            r["steps"].append({"cmd": f"select planet {name}", "info": info})
            if not info or info == "EOL":
                fail(f"select planet {name}: nothing selected ({info!r})")
        for entry in (1, 2):
            c.send("deselect", 0.5)
            offv = first_line(obj_info(c))
            c.send(f"select planet {COMPOSED[0]} pointer off", 0.6)
            onv = first_line(obj_info(c))
            r["steps"].append({"cmd": f"composed pair entry {entry}",
                               "off": offv, "on": onv})
            if offv != empty:
                fail(f"composed pair entry {entry}: deselect readout {offv!r} != {empty!r}")
            if not onv or onv == "EOL":
                fail(f"composed pair entry {entry}: reselect gave {onv!r}")
        c.send("deselect", 0.5)

        # ---- a body REMOVAL with the selection churn behind it: the new
        # path's `ref` vector is scanned on every body destruction, so a
        # reload is where a stale entry would be felt.
        c.send("body action reload", 4.0)
        r["after_reload"] = first_line(obj_info(c))
        c.send(f"select planet {COMPOSED[0]} pointer off", 0.8)
        r["after_reload_select"] = first_line(obj_info(c))
    finally:
        rc = sess.stop(c, exit_wait=args.exit_wait)
        r["exit_rc"] = rc
        note(f"app exit rc={rc}")

    r["sanitizer"] = parse_sanitizer(sess.applog)
    judge(r)
    (outdir / f"f32_result_{args.tag}.json").write_text(json.dumps(r, indent=1))
    print(f"\n-> {outdir}/f32_result_{args.tag}.json", flush=True)
    return r


# ------------------------------------------------------- sanitizer accounting
LEAK_HDR = re.compile(r"^(Direct|Indirect) leak of (\d+) byte\(s\) in (\d+) object\(s\)")
ERR_HDR = re.compile(r"ERROR: AddressSanitizer: ([a-z\-]+)")

STAR_SITES = ("createStelObject", "StarWrapper", "hip_star_wrapper")
MODULAR_SITES = ("ModularObject", "searchObjectByEnglishName", "searchNewOnlyObjectAt")


def parse_sanitizer(applog):
    """Every leak block with its own stack, plus every AddressSanitizer ERROR.
    Blocks are attributed by the frames of the allocation that leaked."""
    txt = Path(applog).read_text(errors="replace")
    blocks, cur = [], None
    for line in txt.splitlines():
        m = LEAK_HDR.match(line.strip())
        if m:
            if cur:
                blocks.append(cur)
            cur = {"kind": m.group(1), "bytes": int(m.group(2)),
                   "objects": int(m.group(3)), "frames": []}
            continue
        if cur is not None:
            s = line.strip()
            if s.startswith("#"):
                cur["frames"].append(s)
            elif not s:
                blocks.append(cur)
                cur = None
    if cur:
        blocks.append(cur)

    def attribute(b, sites):
        return any(any(k in f for k in sites) for f in b["frames"])

    stars = [b for b in blocks if attribute(b, STAR_SITES)]
    mods = [b for b in blocks if attribute(b, MODULAR_SITES)]
    summary = re.search(r"SUMMARY: AddressSanitizer: (\d+) byte\(s\) leaked in "
                        r"(\d+) allocation\(s\)", txt)
    return {
        "lsan_ran": "LeakSanitizer: detected memory leaks" in txt
                    or bool(summary),
        "errors": ERR_HDR.findall(txt),
        "total_blocks": len(blocks),
        "total_bytes": int(summary.group(1)) if summary else None,
        "total_allocs": int(summary.group(2)) if summary else None,
        "star_blocks": stars,
        "star_objects": sum(b["objects"] for b in stars),
        "modular_blocks": mods,
        "modular_objects": sum(b["objects"] for b in mods),
    }


def judge(r):
    s, p = r["sanitizer"], r["pred"]
    if not s["lsan_ran"]:
        fail("LeakSanitizer produced no report - the instrument did not run")
        return
    ok(f"LeakSanitizer ran: {s['total_allocs']} leaked allocations, "
       f"{s['total_bytes']} bytes, {s['total_blocks']} blocks parsed")
    uaf = [e for e in s["errors"] if e != "detected"]
    if uaf:
        fail(f"AddressSanitizer errors during the run: {uaf}")
    else:
        ok("no AddressSanitizer memory error during the churn "
           "(use-after-free / double-free / heap-buffer-overflow)")
    for what, key, predkey in (("star wrappers", "star_objects", "star_wrappers_unreleased"),
                               ("modular bridges", "modular_objects", "modular_bridges_unreleased")):
        got = s[key]
        exp = p[r["expect"]][predkey]
        exp = exp if isinstance(exp, list) else [exp]
        if got in exp:
            ok(f"{what}: {got} unreleased, predicted {exp} for '{r['expect']}'")
        else:
            fail(f"{what}: {got} unreleased, predicted {exp} for '{r['expect']}'")


# ------------------------------------------------------------------------ main
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--mode", choices=("discover", "leak"), required=True)
    ap.add_argument("--tag", default=None)
    ap.add_argument("--bin", default=DEFAULT_BIN)
    ap.add_argument("--expect", choices=("pre", "post"), default="pre")
    ap.add_argument("--stars", type=int, default=6)
    ap.add_argument("--port-wait", dest="port_wait", type=int, default=90)
    ap.add_argument("--exit-wait", dest="exit_wait", type=int, default=40)
    ap.add_argument("--sweep", default="1,101,1001,5001,10001,20001,30001,"
                                       "40001,50001,60001,70001,80001,90001,100001")
    a = ap.parse_args()
    a.outdir = Path(a.outdir).resolve()
    a.outdir.mkdir(parents=True, exist_ok=True)
    a.tag = a.tag or a.mode
    a.sweep = [int(x) for x in a.sweep.split(",")]
    a.env_extra = {k: os.environ[k] for k in ("ASAN_OPTIONS", "LSAN_OPTIONS")
                   if k in os.environ}
    if a.mode == "discover":
        mode_discover(a, a.outdir)
    else:
        mode_leak(a, a.outdir)
    print(f"\n{'F32 LEAK LEG GREEN' if not FAILS else f'{len(FAILS)} FAILURES'}",
          flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
