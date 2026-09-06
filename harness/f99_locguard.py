#!/usr/bin/env python3
"""F99 -- the `location_orbit` null-parent guard, both ways (INTENT S5.141, S5.50's
class one provider over).

WHAT THIS INSTRUMENT IS FOR.  S5.141 says a `location_orbit` body whose parent
cannot be resolved takes the app down on the new path.  The mint's read says the
row names the WRONG ARM (an unknown parent name is refused by
`ModularSystem::loadBody` at :1067-1071 before any loader runs) and misses the
FIRST dereference (the OLD path's own `location_orbit` branch,
`protosystem.cpp:599`, called at `ssystem_factory.cpp:829` BEFORE the new path at
:836).  Both of those are claims about which code runs, so this instrument
measures them instead of asserting them:

  ARM A   parent = a name no body carries      -> predicted: no crash, two refusals
  ARM B   parent = none                        -> predicted: SIGSEGV, on the OLD path
  CTL C   parent = none, coord_func still_orbit-> predicted: both halves, always

THE SITE ATTRIBUTION IS THE POINT, and it comes from the app's own flushed log,
not from the reading.  `cLog` flushes every write (`log.cpp:151-155`), so
`<farm>/.spacecrafter/log/spacecrafter.log` survives a SIGSEGV even though the
C++ stdout capture (fully buffered to a file) does not.  Two lines partition the
two paths for one body:

  "Loading new Stellar System object... <name>"   protosystem.cpp:517   OLD entered
  "Loading body <name>"                           ModularSystem.cpp:1061 NEW entered

Old present + new absent  => the app died inside the old path's addBody.
Both present              => the old path completed and the crash is the new one's.
That is a discriminating pair: each side of it is a different site.

CONTROL C IS NOT DECORATION.  "the body is on neither dump half" is also what a
`parent none` push that never worked at all would produce.  C carries the SAME
`parent none` through the SAME push channel with a coord_func both paths know, so
if C keeps both halves the refusal is the guard's and not the channel's.

PRECONDITIONS (asserted here, again in the wrapper): a PRIVATE farm under
/home/claude/sc-f99 (b3_farm.sh copies config.ini + ssystem.ini and symlinks the
rest, so the real ~/.spacecrafter is never written); fresh launch per arm; no
concurrent instance (/proc/<pid>/comm -- `pgrep -f <path>` self-matches its own
wrapper, S11.134(b)).

  DISPLAY=:2 ./f99_locguard.py <absOutdir> --bin <binary> --tag pre|post

Exit 0 iff every gate of the tag PASSes.
"""
import argparse
import json
import os
import re
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import f91_parity as F91  # noqa: E402  (build_farm / App / no_instance)
import dumpread  # noqa: E402

FARM_ROOT = Path(os.environ.get("F99_FARM_ROOT", "/home/claude/sc-f99"))

RESULT = {"checks": [], "arms": {}}
NFAIL = 0


def check(name, got, want, note=""):
    global NFAIL
    good = (got == want)
    if not good:
        NFAIL += 1
    RESULT["checks"].append({"name": name, "got": got, "want": want,
                             "pass": good, "note": note})
    print("  %s %-46s got=%r want=%r%s"
          % ("PASS" if good else "FAIL", name, got, want,
             ("   -- " + note) if note else ""))
    return good


def record(name, value, note=""):
    RESULT["checks"].append({"name": name, "got": value, "want": None,
                             "pass": None, "note": note})
    print("  ---- %-46s %r%s" % (name, value, ("   -- " + note) if note else ""))


# --------------------------------------------------------------- the pushes
def push(name, func, parent, extra):
    return ("body action load name %s parent %s type Artificial radius 50 "
            "coord_func %s %s tex_map bodies/generic.png"
            % (name, parent, func, extra))


LOC_KEYS = "orbit_lon 5.3667 orbit_lat 43.3 orbit_alt 400000"
STILL_KEYS = "orbit_x 0.001 orbit_y 0 orbit_z 0"

ARM_A = push("ProbeLocA", "location_orbit", "Nonexistent", LOC_KEYS)
ARM_B = push("ProbeLocB", "location_orbit", "none", LOC_KEYS)
CTL_C = push("ProbeCtlC", "still_orbit", "none", STILL_KEYS)


def applog(farm):
    """The FLUSHED log, which is the only channel that survives the signal."""
    p = Path(farm) / ".spacecrafter" / "log" / "spacecrafter.log"
    if not p.is_file():
        return ""
    return p.read_text(encoding="latin-1", errors="replace")


def count(txt, needle):
    return txt.count(needle)


def dump_halves(path, names):
    """{name: (old_present, new_present)} plus the total record count.

    Reads every record itself rather than through load_dump's body view, because
    the question here is precisely whether a record EXISTS at all.
    """
    out = {n: (False, False) for n in names}
    total = 0
    for line in Path(path).read_text(encoding="latin-1", errors="replace").splitlines():
        l = line.strip().rstrip(",")
        if not l.startswith("{"):
            continue
        try:
            o = json.loads(dumpread.sanitize_nonfinite(l))
        except Exception:
            continue
        if o.get("type") != "body":
            continue
        total += 1
        n = o.get("name")
        if n in out:
            out[n] = (o.get("old") is not None, o.get("new") is not None)
    return out, total


# --------------------------------------------------------------------- a leg
def lock_state():
    """The single-instance lock, RECORDED, and cleared only when it is a corpse.

    A crashing leg leaves /tmp/spacecrafter.lock holding its own dead pid.
    `is_lock_file` shells out `kill -0 <pid>` (main.cpp:149-164) and a dead pid
    lets the next launch through -- but a REUSED pid does not: the app then
    prints one warning and returns 0 at main.cpp:241-244, i.e. the next leg dies
    before opening the port and the instrument reads an environment accident as a
    result.  So the state is recorded either way, and a lock whose pid is dead is
    removed as this task's own residue.  (F90 deliberately keeps stale locks
    because it COUNTS startup silences; nothing here counts them.)
    """
    p = Path("/tmp/spacecrafter.lock")
    if not p.is_file():
        return {"present": False}
    pid = p.read_text().split()[0] if p.read_text().strip() else ""
    live = Path("/proc/%s" % pid).exists() if pid.isdigit() else False
    st = {"present": True, "pid": pid, "pid_live": live}
    if not live:
        p.unlink()
        st["removed"] = True
    return st


def run_arm(binary, out, tag, arm, cmds, expect_death):
    """One fresh launch, one arm.  Returns the leg dict."""
    lk = lock_state()
    print("    /tmp/spacecrafter.lock: %s" % json.dumps(lk))
    farm = FARM_ROOT / ("farm_%s_%s" % (tag, arm))
    shape = F91.build_farm(farm, "fr")
    legdir = Path(out) / ("%s_%s" % (tag, arm))
    legdir.mkdir(parents=True, exist_ok=True)
    print("\n=== ARM %s (%s) farm=%s" % (arm, tag, farm))
    print("    farm shape: %s" % json.dumps(shape))
    app = F91.App(binary, farm, legdir)
    tcp = app.start()
    print("    tcp up in %.1f s, pid %d" % (tcp, app.proc.pid))
    leg = {"arm": arm, "tag": tag, "farm": str(farm), "tcp_s": round(tcp, 1),
           "lock_before": lk,
           "sent": [], "alive_after": {}, "dump": None, "rc": None}

    def alive():
        return app.proc.poll() is None

    def send(c, pause=0.8):
        leg["sent"].append(c)
        try:
            app.send(c, pause)
            return True
        except Exception as e:                      # the socket dies with the app
            print("    send raised %s: %s" % (type(e).__name__, e))
            return False

    send("timerate rate 0", 0.6)
    send("meteors zhr 0", 0.4)
    print("    alive before any push: %s" % alive())
    leg["alive_after"]["before_push"] = alive()

    for label, c in cmds:
        print("    -> %s" % label)
        send(c, 2.5)
        time.sleep(2.0)
        a = alive()
        leg["alive_after"][label] = a
        print("       alive after %s: %s" % (label, a))
        if not a:
            break

    if alive():
        try:
            leg["dump"] = str(app.dump("dump"))
            print("    dump: %s" % leg["dump"])
        except Exception as e:
            print("    dump FAILED: %s" % e)
        leg["rc"] = app.stop()
    else:
        for _ in range(30):
            if app.proc.poll() is not None:
                break
            time.sleep(0.5)
        leg["rc"] = app.proc.poll()
    print("    rc = %r  (expect_death=%s)" % (leg["rc"], expect_death))

    log = applog(farm)
    (legdir / "spacecrafter.log").write_text(log, encoding="latin-1")
    leg["log_path"] = str(legdir / "spacecrafter.log")
    leg["log_bytes"] = len(log)
    leg["log"] = log
    return leg


# ------------------------------------------------------------------- scoring
OLD_ENTER = "Loading new Stellar System object... %s"
NEW_ENTER = "Loading body %s"


def score_pre(legA, legB):
    print("\n=== P1 -- ARM A: no crash, both paths refuse by name")
    la = legA["log"]
    check("P1.1 armA app alive after push", legA["alive_after"].get("armA"), True)
    check("P1.1 armA rc", legA["rc"], 0)
    check("P1.2 old refusal line",
          count(la, "SolarSystem: can't find parent for ProbeLocA"), 1,
          "protosystem.cpp:534")
    check("P1.3 new refusal line",
          count(la, "Can't find parent Nonexistent for ProbeLocA"), 1,
          "ModularSystem.cpp:1069")
    check("P1.4 new path entry line present",
          count(la, NEW_ENTER % "ProbeLocA"), 1,
          "written at :1061 BEFORE the parent is resolved")
    check("P1.4b old path entry line present",
          count(la, OLD_ENTER % "ProbeLocA"), 1)
    if legA["dump"]:
        halves, total = dump_halves(legA["dump"], ["ProbeLocA", "ProbeCtlC"])
        record("armA dump body records", total)
        check("P1.5 ProbeLocA on neither half", halves["ProbeLocA"], (False, False))
        check("P3.5/pre control ProbeCtlC on BOTH halves",
              halves["ProbeCtlC"], (True, True),
              "the same `parent none` through the same push channel")
    else:
        check("P1.5 armA dump written", False, True)

    print("\n=== P2 -- ARM B: SIGSEGV pre-fix, attributed to the OLD path")
    lb = legB["log"]
    check("P2.1 armB app DEAD after push", legB["alive_after"].get("armB"), False)
    check("P2.1 armB rc == -11 (SIGSEGV)", legB["rc"], -11)
    check("P2.2 OLD path entered", count(lb, OLD_ENTER % "ProbeLocB"), 1,
          "protosystem.cpp:517")
    check("P2.2 NEW path NEVER entered", count(lb, NEW_ENTER % "ProbeLocB"), 0,
          "ModularSystem.cpp:1061 -- the site claim")
    check("P2.4 no parent diagnostic for ProbeLocB",
          len(re.findall(r"(?:can't|Can't) find parent.*ProbeLocB", lb)), 0,
          "both paths are SILENT on `none`")
    check("P2 control ProbeCtlC survived on both paths pre-crash",
          (count(lb, OLD_ENTER % "ProbeCtlC"), count(lb, NEW_ENTER % "ProbeCtlC")),
          (1, 1))
    record("armB last 6 log lines", lb.strip().splitlines()[-6:])


def score_post(legA, legB):
    print("\n=== P1 -- ARM A unchanged post-fix")
    la = legA["log"]
    check("P1.1 armA app alive after push", legA["alive_after"].get("armA"), True)
    check("P1.1 armA rc", legA["rc"], 0)
    check("P1.2 old refusal line",
          count(la, "SolarSystem: can't find parent for ProbeLocA"), 1)
    check("P1.3 new refusal line",
          count(la, "Can't find parent Nonexistent for ProbeLocA"), 1)
    if legA["dump"]:
        halves, total = dump_halves(legA["dump"], ["ProbeLocA", "ProbeCtlC"])
        record("armA dump body records", total)
        check("P1.5 ProbeLocA on neither half", halves["ProbeLocA"], (False, False))
        check("armA control ProbeCtlC on BOTH halves", halves["ProbeCtlC"], (True, True))
    else:
        check("P1.5 armA dump written", False, True)

    print("\n=== P3 -- ARM B: a diagnosed refusal on both paths")
    lb = legB["log"]
    check("P3.1 armB app ALIVE after push", legB["alive_after"].get("armB"), True)
    check("P3.1 armB rc", legB["rc"], 0)
    check("P3.2 OLD path S2(f) line, once",
          count(lb, "Body 'ProbeLocB': coord_func = location_orbit needs a parent body"), 1,
          "protosystem.cpp, before the :599 dereference")
    check("P3.2 NEW path S2(f) line, once",
          count(lb, "location_orbit orbit of 'ProbeLocB': parent 'none' is not a loaded body"), 1,
          "LocationOrbitLoader.hpp -- the S11.193 anchor")
    check("P3.3 consumer refusal fires",
          count(lb, "Invalid orbit 'location_orbit' for body 'ProbeLocB', "
                    "skip loading this body."), 1,
          "ModularSystem.cpp:1246 -- 'no orbit, no body' already existed")
    check("P3.3b the loader did NOT throw into the default loader",
          count(lb, "OrbitCreatorComet::handle unknown type"), 0,
          "a throw is swallowed at ModuleLoaderMgr.cpp:106 into SpecialOrbitLoader, "
          "whose own miss line this is -- its absence + P3.3 present is the pair")
    check("P3.2b both paths were entered for ProbeLocB",
          (count(lb, OLD_ENTER % "ProbeLocB"), count(lb, NEW_ENTER % "ProbeLocB")),
          (1, 1),
          "old no longer dies, so the new path is reached at last")
    if legB["dump"]:
        halves, total = dump_halves(legB["dump"], ["ProbeLocB", "ProbeCtlC"])
        record("armB dump body records", total)
        check("P3.4 ProbeLocB on neither half", halves["ProbeLocB"], (False, False))
        check("P3.5 control ProbeCtlC on BOTH halves", halves["ProbeCtlC"], (True, True),
              "the guard refused the class, not the push channel")
    else:
        check("P3.4 armB dump written", False, True)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--bin", required=True)
    ap.add_argument("--tag", required=True, choices=["pre", "post"])
    a = ap.parse_args()
    out = Path(a.out)
    out.mkdir(parents=True, exist_ok=True)

    hits = F91.no_instance()
    if hits:
        print("ABORT: another spacecrafter process exists: %s" % hits)
        return 2
    print("F99 %s leg -- binary %s" % (a.tag, a.bin))
    print("  md5 %s" % subprocess.run(["md5sum", a.bin], capture_output=True,
                                      text=True).stdout.split()[0])

    legA = run_arm(a.bin, out, a.tag, "armA",
                   [("ctlC", CTL_C), ("armA", ARM_A)], expect_death=False)
    legB = run_arm(a.bin, out, a.tag, "armB",
                   [("ctlC", CTL_C), ("armB", ARM_B)],
                   expect_death=(a.tag == "pre"))

    if a.tag == "pre":
        score_pre(legA, legB)
    else:
        score_post(legA, legB)

    for leg in (legA, legB):
        leg.pop("log", None)
    RESULT["arms"] = {"armA": legA, "armB": legB}
    RESULT["nfail"] = NFAIL
    (out / ("f99_%s.json" % a.tag)).write_text(json.dumps(RESULT, indent=1))
    print("\n=== F99 %s: %d check(s), %d FAIL -> %s"
          % (a.tag, len([c for c in RESULT["checks"] if c["pass"] is not None]),
             NFAIL, out / ("f99_%s.json" % a.tag)))
    return 1 if NFAIL else 0


if __name__ == "__main__":
    sys.exit(main())
