#!/usr/bin/env python3
"""F50 / INTENT §5.110 — the LIVE reading of the six `selected_*` script
variables on a real composed-body selection.

    cd claude/harness && export XAUTHORITY=$(ls /tmp/rt-claude/.mutter-Xwaylandauth.*) \
        && DISPLAY=:2 ./f50_selvars.py <absOutdir> [--bin <binary>]

WHAT §5.110 CLAIMS, AND WHAT THIS MEASURES
------------------------------------------
§5.110 was minted by READING (§11.158(j1)): `SolarSystemSelected::setSelected`
stores the selection only when `obj.getType() == OBJECT_BODY`
[observed: solarsystem_selected.cpp:48-54], a composed (B24) body arrives as a
`ModularObject` whose type is `OBJECT_MODULAR` [ModularObject.cpp:98-101], so
the slot the six variables read stays EMPTY and they fall to the
`ObjectUninitialized` singleton [object.cpp:30-84] — `getObsJ2000Pos` returns
`Vec3d(1,0,0)` [object.cpp:72] so `selected_distance` answers 1, `getMag` returns -10 so
`selected_magnitude` answers -10, and `getRaDeValue`/`getAltAz` are EMPTY
bodies so ra/de/az/alt keep the CALLER's own initialiser, 0
[ssystem_factory.hpp:580/586/603/609 — the four `double x=0` declarations].
The row's own owed clause is *"the LIVE reading, which this task attributed but
did not measure"*.  That is this instrument.

THE NAMING TRAP, recorded because it is a real hazard for anyone writing the
reproducing script: the ledger writes these as `#selected_ra` etc., but `#` is
the COMMENT character of a `.sts` script [doc/superscript.sts]. The tokens the
evaluator registers are BARE: `selected_ra`, `selected_de`, `selected_az`,
`selected_alt`, `selected_distance`, `selected_magnitude`
[base_command_interface.hpp:654-659 -> app_command_eval.cpp:45-50].  A script
line beginning `#selected_ra` is a comment and measures nothing.

THE OBSERVABLE CHANNEL, named so it is challengeable
----------------------------------------------------
`print <key> <token>` -> `AppCommandInterface::commandPrint`
[app_command_interface.cpp:1969-1983] -> `AppCommandEval::evalString(token)`
[app_command_eval.cpp:75-114] -> reserved-variable hit ->
`evalReservedVariable` [app_command_eval.cpp:304-315] -> `CoreLink::getSelected*`
-> `SSystemFactory::getSelected*` -> `ssystemSelected->getSelected()`.
`commandPrint` writes `[<key>] <value>` into the SCRIPT log
(`<HOME>/.spacecrafter/log/script-<date>.log`) — so the terminal observable is
the app's own log file, written by the shipped command, through the SAME
evaluator entry point a `.sts` script's variable reference uses.  Nothing is
read out of the process by any private route.

THE PROBE MUST BE ABLE TO FAIL, and the control is in the SAME launch
---------------------------------------------------------------------
A probe that silently no-ops answers the uninitialized constants for every
reason, including "the print never ran".  Five legs, one launch, in this order:

  O1  `select planet Mars`      an OLD-TREE body -> the six MUST answer real
                                values.  This is the positive control: it maps
                                the whole chain (command -> evaluator -> log)
                                as live, on the same socket, in the same run.
  D1  `deselect`                genuinely nothing selected -> the six MUST fall
                                to the singleton constants.  This is the
                                NEGATIVE reference the composed leg is compared
                                against, and it also proves the O1 reading is
                                not a latched string.
  C   `select planet BigA`      the composed body.  §5.110's subject.
  D2  `deselect`                the singleton constants again, after C.
  O2  `select planet Mars`      the same body as O1, same frozen epoch, same
                                observer -> the six MUST return to O1's values.
                                A channel that had latched on the constants
                                fails here.

Two INDEPENDENT witnesses that the composed selection really took (without
them, leg C is indistinguishable from `select planet <typo>`):
  W1  `get status object` -> `Core::getSelectedObjectInfo` ->
      `selected_object.getInfoString()` — R1 of §11.158(c).  Non-empty and
      naming the body on C; the literal "EOL" on D1/D2.
  W2  the dual_dump header's `camera.selected` — `ModularBody::getSelected()`,
      i.e. `SSystemFactory::newSelectedBody`, written by the same
      `SSystemFactory::setSelected` call that the type filter rejects
      [ssystem_factory.cpp:893-906].  It must read the composed body's name on
      C while the six read the singleton: ONE call, two aggregates, one of them
      populated.

A SEVENTH VARIABLE IS READ AT EVERY LEG, `body_selected`, because it is in the
same family but reads a DIFFERENT slot (`Core::selected_body_name`, written by
`setSelectedBodyName` at core.cpp:2388 for every type before the switch, from a
hard-coded english-name table core.cpp:2226-2276 whose `else` is 999).  Two
predictions follow from source and are committed below: it answers 999 for a
composed body (the name is not in the table), and — because `Core::unSelect`
[core.cpp:2614-2618] does NOT call `setSelectedBodyName` — it answers STALE 400
(Mars) on D1, i.e. after a `deselect`, against the shipped doc's own contract
*"body_selected which value is 999 except if is selected"* [doc/superscript.sts:1529].  That prediction is what makes D1 worth a leg of its own.

FIXTURE AND PRECONDITIONS
  - the composed scene is b24_select's own (imported, not copied — I2): four
    `compose = explicit` bodies grounded on the Moon, appended to the app's
    machine-generated composed twin, authored in the farm's `modularSystem/`.
  - temp-HOME symlink farm, concurrent-instance assert, frozen config/ssystem
    md5 in == out, exit-code check: all `f27_reply.Session`'s (I2).
  - NO `moveto` is issued and no display scale is touched, so the §5.109
    settle hazard is not exercised; the observer stays wherever config.ini puts
    it and the epoch is frozen (`timerate rate 0`).  The observer state is
    STAMPED from the dump rather than commanded.
  - record-only: no product code, no data writes, the real ~/.spacecrafter is
    never written.
"""

import argparse, json, os, re, sys, time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import f27_reply as f27
import f32_object_leak as f32
import b24_select as b24s

HERE = Path(__file__).resolve().parent
DEFAULT_BIN = str(HERE.parents[1] / "build-claude/src/spacecrafter")
REAL_HOME = Path.home()

JD = "2461233.5"                 # the epoch §11.158's parity run used
OLD_BODY = "Mars"                # old-tree control body
COMPOSED_BODY = "BigA"           # b24_select's fixture, grounded on the Moon

# The six of §5.110, in the row's own order, plus the seventh contrast variable.
SIX = ["selected_ra", "selected_de", "selected_az", "selected_alt",
       "selected_distance", "selected_magnitude"]
SEVENTH = "body_selected"
VARS = SIX + [SEVENTH]

FAILS, NOTES = [], []


def fail(m):
    FAILS.append(m)
    print(f"FAIL: {m}", flush=True)


def ok(m):
    print(f"ok:   {m}", flush=True)


def note(m):
    NOTES.append(m)
    print(f"      {m}", flush=True)


# --------------------------------------------------------------- predictions
# Committed to disc (and to git) BEFORE the launch. Every number is derived from
# a read line of the shipped source, cited at the derivation.
UNINIT = {
    # ObjectUninitialized::getRaDeValue / ObjectBase::getAltAz are empty bodies
    # [object.cpp:40-42, object_base.hpp:79], so the caller's initialiser stands
    # [ssystem_factory.hpp:580 az=0, :586 alt=0, :603 ra=0, :609 de=0]; the
    # getters then multiply by 180/pi, which leaves 0.
    "selected_ra": "0", "selected_de": "0", "selected_az": "0",
    "selected_alt": "0",
    # getObsJ2000Pos -> Vec3d(1,0,0) [object.cpp:72]; getSelectedDistance
    # returns pos.length() [ssystem_factory.hpp:235-236] = 1.0; formatNumber emits
    # an exact integer as an integer [app_command_eval.cpp:17-27].
    "selected_distance": "1",
    # getMag -> -10 [object.cpp:76]
    "selected_magnitude": "-10",
}

PRED = {
    "channel": "print -> AppCommandEval::evalString -> script log "
               "(<farm>/.spacecrafter/log/script-*.log), line form '[key] value'",
    "legs": ["O1", "D1", "C", "D2", "O2"],
    "O1": {"six": "REAL - every one of the six must differ from the UNINIT tuple; "
                  "specifically selected_distance != 1 and selected_magnitude != -10 "
                  "and (ra,de,az,alt) != (0,0,0,0)",
           "body_selected": "400",
           "objinfo": "non-empty, names Mars, != 'EOL'"},
    "D1": {"six": UNINIT,
           "body_selected": "400",
           "why": "STALE: Core::unSelect (core.cpp:2614-2618) does not call "
                  "setSelectedBodyName, so the last selected body's code survives "
                  "a deselect - against doc/superscript.sts:1529's own contract",
           "objinfo": "EOL"},
    "C":  {"six": UNINIT,
           "body_selected": "999",
           "why": "the composed name is absent from core.cpp:2226-2276's table",
           "objinfo": "non-empty, names " + COMPOSED_BODY + ", != 'EOL'",
           "dump_selected": COMPOSED_BODY},
    "D2": {"six": UNINIT, "body_selected": "999", "objinfo": "EOL"},
    "O2": {"six": "byte-identical to O1's printed strings (frozen epoch, "
                  "unmoved observer, same body)",
           "body_selected": "400",
           "objinfo": "non-empty, names Mars"},
    "C_vs_D": "leg C's seven printed values must equal leg D1's and D2's "
              "EXACTLY - that is §5.110's 'no diagnostic' claim made "
              "measurable: through this channel a composed selection is "
              "INDISTINGUISHABLE from nothing selected, while W1/W2 show the "
              "app itself knows the difference",
    "uninit_tuple": UNINIT,
}


# ------------------------------------------------------------- the log channel
LINE_RE = re.compile(r"\[(f50leg|f50v_[a-z_]+)\]\s*(\S.*?)\s*$")


def script_log_text(sess):
    """The SCRIPT log only. `commandPrint` writes the same string TWICE - once
    to LOG_FILE::SCRIPT and once to the default internal log
    [app_command_interface.cpp:1979-1980] - so reading the concatenation of all
    log files would double every reading and silently corrupt the leg
    partition."""
    logdir = sess.farm / ".spacecrafter" / "log"
    hits = sorted(logdir.glob("script*.log"))
    if len(hits) != 1:
        fail(f"expected exactly one script log in {logdir}, found {[h.name for h in hits]}")
    return "".join(h.read_text(errors="replace") for h in hits)


def parse_legs(text):
    """Walk the script log in order; a `[f50leg] <tag>` line opens a leg and
    every `[f50v_<name>] <value>` after it belongs to that leg."""
    legs, cur = {}, None
    for line in text.splitlines():
        m = LINE_RE.search(line)
        if not m:
            continue
        key, val = m.group(1), m.group(2)
        if key == "f50leg":
            cur = val
            legs.setdefault(cur, {})
        elif cur is not None:
            legs[cur][key[len("f50v_"):]] = val
    return legs


def read_vars(c, leg):
    """Issue the leg marker then the seven prints, one command each so the
    log carries them in a deterministic order (the arg hash is unordered)."""
    c.send(f"print f50leg {leg}", 0.35)
    for v in VARS:
        c.send(f"print f50v_{v} {v}", 0.35)
    time.sleep(0.6)


def dump_selected(sess, c, tag):
    """W2: the dual_dump header's camera.selected = ModularBody::getSelected()."""
    p = sess.outdir / f"f50_{tag}.json"
    p.unlink(missing_ok=True)
    c.send(f"body action dual_dump filename {p}", 2.2)
    for _ in range(20):
        if p.exists() and p.stat().st_size > 0:
            break
        time.sleep(0.3)
    if not p.exists():
        fail(f"{tag}: dual_dump wrote nothing - W2 unavailable")
        return None, None
    cam = None
    for line in open(p, errors="replace"):
        line = line.strip().rstrip(",")
        if not line:
            continue
        try:
            o = json.loads(line)
        except json.JSONDecodeError:
            continue
        if o.get("type") == "header":
            cam = o.get("camera")
            break
    if cam is None:
        fail(f"{tag}: dump carries no header/camera - W2 unavailable")
        return None, None
    return cam.get("selected"), cam


# ---------------------------------------------------------------------- main
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--bin", default=DEFAULT_BIN)
    ap.add_argument("--port-wait", type=int, default=90)
    a = ap.parse_args()
    out = Path(a.outdir).resolve()
    out.mkdir(parents=True, exist_ok=True)

    (out / "f50_predict.json").write_text(json.dumps(PRED, indent=1))
    print(json.dumps(PRED, indent=1), flush=True)

    prepare = f32.make_prepare()          # b24_select's composed fixture (I2)
    sess = f27.Session(out, "f50", a.bin, prepare=prepare, port_wait=a.port_wait)
    c = sess.client("driver")
    res = {"bin": str(a.bin), "pred": PRED, "legs": [],
           "composed_body": COMPOSED_BODY, "old_body": OLD_BODY}
    try:
        c.send("timerate rate 0", 1)
        c.send("flag experimental_path on", 1)
        c.send(f"date jday {JD}", 1)
        c.send("deselect", 0.6)

        plan = [("O1", f"select planet {OLD_BODY}"),
                ("D1", "deselect"),
                ("C",  f"select planet {COMPOSED_BODY}"),
                ("D2", "deselect"),
                ("O2", f"select planet {OLD_BODY}")]
        for leg, cmd in plan:
            c.send(cmd, 1.2)
            info = f32.first_line(f32.obj_info(c))
            sel, cam = dump_selected(sess, c, leg)
            read_vars(c, leg)
            res["legs"].append({"leg": leg, "cmd": cmd, "objinfo": info,
                                "dump_selected": sel,
                                "camera": {k: cam.get(k) for k in
                                           ("reference", "freeMode", "boundToSurface",
                                            "longitude", "latitude", "distance",
                                            "tracked", "selected")} if cam else None})
            print(f"  {leg}: {cmd!r} -> objinfo {info!r} dump.selected {sel!r}", flush=True)

        text = script_log_text(sess)
        (out / "f50_script.log").write_text(text)
        legs = parse_legs(text)
        res["readings"] = legs
        for row in res["legs"]:
            row["vars"] = legs.get(row["leg"], {})
        res["refused"] = sess.refused()
    finally:
        rc = sess.stop(c)
        res["exit_rc"] = rc
        note(f"app exit rc={rc}")

    # ------------------------------------------------------------- the gates
    L = res["readings"]
    for leg in PRED["legs"]:
        got = L.get(leg, {})
        missing = [v for v in VARS if v not in got]
        if missing:
            fail(f"{leg}: the channel produced no reading for {missing} - "
                 f"the probe did not run, so nothing this leg says is evidence")

    # G1 positive control - the probe CAN answer real values
    o1 = L.get("O1", {})
    if o1:
        real = (o1.get("selected_distance") != UNINIT["selected_distance"]
                and o1.get("selected_magnitude") != UNINIT["selected_magnitude"]
                and any(o1.get(v) not in (None, "0") for v in SIX[:4]))
        if real:
            ok(f"G1 positive control: {OLD_BODY} answers real values through the "
               f"SAME channel: " + ", ".join(f"{v}={o1.get(v)}" for v in SIX))
        else:
            fail(f"G1 positive control FAILED: {OLD_BODY} answers "
                 + ", ".join(f"{v}={o1.get(v)}" for v in SIX)
                 + " - the channel cannot discriminate and no other leg is evidence")

    # G2 the composed selection really took (W1 + W2)
    cleg = next((r for r in res["legs"] if r["leg"] == "C"), {})
    if cleg.get("dump_selected") == COMPOSED_BODY:
        ok(f"G2/W2: dump camera.selected == {COMPOSED_BODY!r} - the composed "
           f"selection reached SSystemFactory::newSelectedBody")
    else:
        fail(f"G2/W2: dump camera.selected == {cleg.get('dump_selected')!r}, "
             f"expected {COMPOSED_BODY!r} - the selection did NOT take, so leg C "
             f"measures 'nothing selected', not §5.110")
    info = cleg.get("objinfo") or ""
    if info and info != "EOL" and COMPOSED_BODY.lower() in info.lower():
        ok(f"G2/W1: get status object -> {info!r} - Core::selected_object holds it")
    else:
        fail(f"G2/W1: get status object -> {info!r} on leg C")

    # G3 the reading, per variable, against the committed prediction
    cvars = L.get("C", {})
    per = {}
    for v in SIX:
        got, exp = cvars.get(v), UNINIT[v]
        per[v] = {"predicted": exp, "measured": got, "match": got == exp}
        (ok if got == exp else fail)(
            f"G3 {v}: predicted {exp!r}, measured {got!r}"
            + ("" if got == exp else "  <-- MISMATCH, trace it through the row's chain"))
    res["per_variable"] = per

    # G4 indistinguishability from 'nothing selected'
    for d in ("D1", "D2"):
        dv = L.get(d, {})
        same = all(cvars.get(v) == dv.get(v) for v in SIX)
        if same:
            ok(f"G4 {d}: leg C's six are IDENTICAL to '{d} deselect' - through this "
               f"channel a composed selection and nothing-selected are the same answer")
        else:
            fail(f"G4 {d}: leg C differs from deselect on "
                 + str([v for v in SIX if cvars.get(v) != dv.get(v)])
                 + " - §5.110's 'no diagnostic' claim needs re-reading")
    dinfo = next((r["objinfo"] for r in res["legs"] if r["leg"] == "D2"), None)
    if dinfo == "EOL":
        ok("G4: and the app itself CAN tell them apart - get status object is "
           "'EOL' on deselect and the body's info string on C")
    else:
        fail(f"G4: get status object on D2 is {dinfo!r}, expected 'EOL' - the "
             f"contrast that makes the indistinguishability claim meaningful is absent")

    # G5 A/B/A - the channel is not latched
    o2 = L.get("O2", {})
    diff = [v for v in SIX if o1.get(v) != o2.get(v)]
    if o1 and o2 and not diff:
        ok("G5 A/B/A: O2 reproduces O1 on all six - the channel is live "
           "throughout and the constants at C are not a latched string")
    else:
        fail(f"G5 A/B/A: O2 differs from O1 on {diff} "
             f"(O1 {[o1.get(v) for v in SIX]}, O2 {[o2.get(v) for v in SIX]})")

    # G6 the seventh variable, both predictions
    b = {leg: L.get(leg, {}).get(SEVENTH) for leg in PRED["legs"]}
    res["body_selected"] = b
    if b.get("C") == "999":
        ok(f"G6a body_selected on the composed body = 999 (name absent from the table)")
    else:
        fail(f"G6a body_selected on the composed body = {b.get('C')!r}, predicted '999'")
    if b.get("D1") == "400":
        ok("G6b body_selected is STALE after `deselect`: 400 (Mars) with nothing "
           "selected - Core::unSelect never clears it, against superscript.sts:1530")
    else:
        fail(f"G6b body_selected on D1 = {b.get('D1')!r}, predicted stale '400' "
             f"(Core::unSelect does not call setSelectedBodyName)")

    if res.get("refused"):
        note(f"app refused {len(res['refused'])} command(s): {res['refused'][:5]}")
    res["fails"], res["notes"] = FAILS, NOTES
    (out / "f50_result.json").write_text(json.dumps(res, indent=1, default=str))
    print(f"\n{len(FAILS)} FAIL, {len(NOTES)} note -> {out}/f50_result.json", flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
