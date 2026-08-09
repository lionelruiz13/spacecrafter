#!/usr/bin/env python3
"""F30 / INTENT §5.64 - the measured half of the getTimeSpeed() consumer sweep.

The row owes an ENUMERATION ("which consumers read getTimeSpeed() and therefore
already behave as if paused"). That enumeration is complete at source and closed:
the gated readout has exactly two direct callers, `CoreLink::timeGetSpeed`
(coreLink.cpp:502) and `MeteorMgr::update` (meteor_mgr.cpp:90), and
`timeGetSpeed` in turn has exactly four consumers, all four inside
`AppCommandInterface::commandTimerate` (app_command_interface.cpp:3327 / 3345 /
3360 / 3376 - increment / sincrement / decrement / sdecrement).

This script MEASURES the consequence at those four sites, because "behaves as if
paused" understates what they do: each one reads the GATED value as if it were
"the rate the operator set", drops the pause, and then recomputes a new rate from
it. While a pause is held the gated value is 0, so the operator's rate is not
slowed - it is LOST, and the decrement ladder's zero branch lands on -JD_SECOND.

PREDICTIONS, written before the run (the ladder at app_command_interface.cpp:
3376-3384, with JD_SECOND = 1/86400 d/s):

  Leg A (a pause is held)
    A0  after `timerate rate 3`              rate  +3.0   paused false
    A1  after `timerate action pause`        rate  +3.0   paused TRUE
                                             ... and JDay ADVANCED A0->A1,
                                             which is §5.64's headline itself
    A2  after `timerate action decrement`    rate  -1.0   paused false
        (s = getTimeSpeed() = 0 while locked -> branch `s > -JD_SECOND &&
         s <= 0.` -> s = -JD_SECOND: time runs BACKWARD at real time)

  Leg B (no pause held - the positive control, same command, same set rate)
    B0  after `timerate rate 3`              rate  +3.0   paused false
    B1  after `timerate action decrement`    rate  +1.5   paused false
        (s = 3*JD_SECOND > JD_SECOND -> s /= 2)

The discriminator is the SIGN: A2 = -1.0 vs B1 = +1.5, one command, one
difference - whether a pause was held when it was read.

Both legs run in ONE launch on purpose: the observable is a commanded state
transition read back from the engine's own table, not a rendered quantity, and
`timerate rate` clears the pause itself (app_command_interface.cpp:3313), so
leg B starts from a state leg A cannot contaminate - and B0 is dumped to prove
it rather than to assume it.
"""
import json
import os
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from f27_reply import Session, note, ok, fail  # noqa: E402

JD_SECOND = 1.0 / 86400.0
OUT = HERE / "artifacts" / "f30"
OUT.mkdir(parents=True, exist_ok=True)

PREDICT = {"A0": (+3.0, False), "A1": (+3.0, True), "A2": (-1.0, False),
           "B0": (+3.0, False), "B1": (+1.5, False)}


def main():
    binary = os.environ.get("SC_BIN", str(HERE / ".." / ".." / "build-claude"
                                          / "src" / "spacecrafter"))
    sess = Session(OUT, "f30tr", binary)
    cli = sess.client("drive")
    samples = {}

    def dump(tag):
        path = OUT / f"f30tr_{tag}.json"
        cli.send(f"body action dual_dump filename {path}", pause=1.2)
        for _ in range(20):
            if path.exists() and path.stat().st_size > 0:
                break
            time.sleep(0.25)
        with open(path) as f:
            head = json.loads(f.readline())
        rate = head["timeSpeed"] / JD_SECOND
        samples[tag] = {"rate": rate, "timeSpeed": head["timeSpeed"],
                        "paused": head["timePaused"], "jd": head["jd"]}
        note(f"  {tag}: rate={rate:+.6f}  paused={head['timePaused']}  "
             f"jd={head['jd']:.9f}")
        return samples[tag]

    try:
        note("--- leg A: a pause is held when `decrement` reads the rate ---")
        cli.send("timerate rate 3", pause=1.0)
        dump("A0")
        cli.send("timerate action pause", pause=1.0)
        time.sleep(1.5)          # let the clock run WHILE paused, on purpose
        dump("A1")
        cli.send("timerate action decrement", pause=1.0)
        dump("A2")

        note("--- leg B (control): same command, same set rate, no pause ---")
        cli.send("timerate rate 3", pause=1.0)
        dump("B0")
        cli.send("timerate action decrement", pause=1.0)
        dump("B1")
    finally:
        # Session.stop asserts the frozen config/ssystem md5 in == out and
        # takes the app down through its own `shutdown action now`.
        rc = sess.stop(cli)
        note(f"app exit rc={rc}")

    note("")
    note("=== prediction vs measurement ===")
    allok = True
    for tag, (prate, ppaused) in PREDICT.items():
        s = samples.get(tag)
        if s is None:
            fail(f"{tag}: no sample")
            allok = False
            continue
        hit = abs(s["rate"] - prate) < 1e-6 and bool(s["paused"]) == ppaused
        (ok if hit else fail)(
            f"{tag}: predicted rate {prate:+.4f} paused {ppaused} | "
            f"measured rate {s['rate']:+.6f} paused {s['paused']}")
        allok = allok and hit

    # §5.64's headline, re-measured on this same drive: the clock advanced
    # while the pause was held.
    if "A0" in samples and "A1" in samples:
        djd = samples["A1"]["jd"] - samples["A0"]["jd"]
        note(f"JDay advance across the HELD pause (A0 -> A1): {djd:+.9f} d "
             f"= {djd * 86400:+.3f} simulated seconds")
        (ok if djd > 0 else fail)(
            "the simulation clock advanced while `timerate action pause` was in "
            "effect" if djd > 0 else "no clock advance observed across the pause")

    with open(OUT / "f30_timerate.json", "w") as f:
        json.dump({"samples": samples,
                   "predictions": {k: {"rate": v[0], "paused": v[1]}
                                   for k, v in PREDICT.items()}},
                  f, indent=2, sort_keys=True)
    note(f"written: {OUT}/f30_timerate.json")
    return 0 if allok else 1


if __name__ == "__main__":
    sys.exit(main())
