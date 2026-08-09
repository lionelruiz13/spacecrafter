# F36 / INTENT §5.77 — gdb-side reachability probe (sourced by f36_reach.gdb;
# NOT importable outside gdb: it uses the `gdb` module).
#
# WHAT IT MEASURES, AND WHY A BREAKPOINT AND NOT A LOG READ.
# §5.77 owes an enumeration of the *startup* paths that report only on the
# console. "Startup path" has a measurable meaning and a guessable one, and the
# difference matters: a failure-report line does NOT execute on a healthy
# startup, so reading the console output of a good launch enumerates the sites
# that FIRED, never the sites that COULD fire. The unit that answers the row is
# therefore the enclosing function: *was the function that contains this report
# entered before the app reached its main loop?* That is what this probe reads,
# one breakpoint per enclosing function of every console-output site
# `f36_enum.py` found.
#
# THE BOUNDARY IS MEASURED, NOT ASSUMED. One extra breakpoint sits on
# `App::startMainLoop` (main.cpp:348 calls it, after `app->firstInit()`), and it
# only flips a flag. Every hit is stamped STARTUP or POST against that flag, so
# "during startup" is a fact read off the same channel as the hit itself rather
# than a judgment applied afterwards.
#
# EVERY BREAKPOINT FIRES AT MOST ONCE. `stop()` records, sets `enabled = False`
# and returns False, so the inferior is never left stopped and a breakpoint on a
# hot function (`FilePath::FilePath`, `EventHandler::handle`) costs one stop for
# the whole run instead of one per call. The cost of the instrument is therefore
# bounded by the number of breakpoints, not by the app's behaviour.
#
# COVERAGE IS REPORTED, NOT ASSUMED. Before `run`, the probe emits one MANIFEST
# line per breakpoint carrying the spec and the number of resolved locations. A
# spec that resolved to 0 locations is a HOLE in the measurement and says so;
# without that line, an unresolved breakpoint and a function that is genuinely
# never entered produce the identical evidence (silence) — the silent-no-op
# probe class, INTENT §11.47.
#
# Output goes to $F36_PROBE with an open/flush/close per line, deliberately not
# through gdb's stdout, which is block-buffered when redirected (§11.139(b)).
import json
import os

import gdb

PROBE = os.environ.get("F36_PROBE", "/tmp/f36_probe.txt")
SITES = os.environ.get("F36_SITES", "")
REPO = "/home/claude/spacecrafter"

STATE = {"mainloop": False}


def emit(msg):
    with open(PROBE, "a") as f:
        f.write(msg + "\n")
        f.flush()


class Reach(gdb.Breakpoint):
    """Entered-or-not, once. `stop()` returning False resumes the inferior."""

    def __init__(self, spec, label):
        super(Reach, self).__init__(spec, gdb.BP_BREAKPOINT, internal=False)
        self.label = label
        self.spec = spec
        self.silent = True

    def stop(self):
        phase = "POST" if STATE["mainloop"] else "STARTUP"
        emit("REACH %-8s %s" % (phase, self.label))
        self.enabled = False
        return False


class Boundary(gdb.Breakpoint):
    """The startup/main-loop frontier. Flips the stamp every other hit carries."""

    def __init__(self, spec):
        super(Boundary, self).__init__(spec, gdb.BP_BREAKPOINT, internal=False)
        self.silent = True

    def stop(self):
        STATE["mainloop"] = True
        emit("BOUNDARY App::startMainLoop entered "
             "(every REACH above is startup, every REACH below is not)")
        self.enabled = False
        return False


def install():
    rows = json.load(open(SITES))
    # One breakpoint per distinct enclosing function of a project (non-EntityCore)
    # console-output site. EntityCore is read-only for this task: its sites are
    # part of the sizing, not of any fix, and are not probed.
    seen = {}
    for r in rows:
        if r["entitycore"] or r["func_line"] < 0:
            continue
        if r.get("skip_probe"):
            continue
        key = (r["file"], r["func_line"])
        seen.setdefault(key, r)

    bps = []
    for (f, fl), r in sorted(seen.items()):
        spec = "%s/%s:%d" % (REPO, f, fl)
        label = "%s:%d %s" % (f, r["line"], r["func"])
        try:
            bps.append(Reach(spec, label))
        except Exception as e:                       # noqa: BLE001
            emit("MANIFEST spec=%s locations=SPEC-ERROR(%s) label=%s"
                 % (spec, e, label))

    try:
        Boundary("App::startMainLoop")
        emit("MANIFEST spec=App::startMainLoop locations=boundary")
    except Exception as e:                           # noqa: BLE001
        emit("MANIFEST spec=App::startMainLoop locations=SPEC-ERROR(%s)" % e)

    resolved = 0
    for b in bps:
        n = 0
        try:
            n = len(b.locations)                     # gdb >= 13
        except Exception:                            # noqa: BLE001
            try:
                n = 0 if b.pending else 1
            except Exception:                        # noqa: BLE001
                n = -1
        resolved += 1 if n > 0 else 0
        emit("MANIFEST spec=%s locations=%s label=%s" % (b.spec, n, b.label))
    emit("MANIFEST-SUMMARY breakpoints=%d resolved=%d unresolved=%d"
         % (len(bps), resolved, len(bps) - resolved))


emit("PROBE LOADED (f36_probe.py) sites=%s" % SITES)
install()
