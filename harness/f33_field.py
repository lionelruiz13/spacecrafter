#!/usr/bin/env python3
"""F33 field witness — the SHIPPED flow, played as a show, in real time.

`internal/fly_to_selected.sts` is the corpus's own use of all three travel
members in one sequence: transition_to point, two move_to body legs with an
altitude, transition_to body - through the `selected` name, which CoreLink
resolves for BOTH paths at one seam. The campaign (`f33_transitions.py`) steps
the clock so the trajectory is exactly comparable; this one does the opposite on
purpose - it lets the show RUN, at the shipped time rate, with the script's own
`wait`s - because that is how the field meets these commands, and because a
divergence that only exists at frame cadence would be invisible to a stepped
test.

Witnesses: no refusal in the log from EITHER path, the two paths' observer
positions agree at the end, the camera ends on the body, and the composed
screen shows a body that is close (the arrival) and did not before (the
control is the same frame at the start).
"""

import argparse, json, sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import numpy as np
from PIL import Image
from f27_reply import Session
import b24_equivalence as b24

HERE = Path(__file__).resolve().parent
SCRIPT = "internal/fly_to_selected.sts"
TARGET = "Mars"
CHECKS = []


def chk(ok, label, detail=""):
    CHECKS.append({"ok": bool(ok), "label": label, "detail": detail})
    print(("OK   " if ok else "FAIL ") + label + (("  -- " + detail) if detail else ""))


def eye_pos(m16):
    m = np.array(m16, float).reshape(4, 4).T
    return -m[:3, :3].T @ m[:3, 3]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--bin", default=str(HERE.parents[1] / "build-claude/src/spacecrafter"))
    a = ap.parse_args()
    out = Path(a.outdir); out.mkdir(parents=True, exist_ok=True)
    sess = Session(out, "f33field", a.bin)
    drv = sess.client("drv")
    n = [0]

    def dump(tag):
        n[0] += 1
        p = f"/tmp/f33field_{n[0]:02d}_{tag}.json"
        drv.send(f"body action dual_dump filename {p}", 1.2)
        with open(p) as f:
            head = json.loads(b24.sanitize_nonfinite(f.readline()))
            bodies = {}
            for line in f:
                line = line.strip()
                if line:
                    o = json.loads(b24.sanitize_nonfinite(line))
                    if o.get("type") == "body":
                        bodies[o["name"]] = o
        head["_old"] = eye_pos(head["helioToEye"]).tolist()
        head["_new"] = list(head["camera"]["rootPos"])
        head["_bodies"] = bodies
        return head

    def shot(tag):
        p = out / f"f33field_{tag}.png"
        drv.send(f"body action screenshot filename {p}", 2.0)
        return p

    drv.send("flag atmosphere off", 0.5)
    drv.send("flag landscape off", 0.5)
    drv.send(f"select planet {TARGET} pointer off", 1.0)
    drv.send(f"select planet {TARGET} pointer off", 1.0)
    before = dump("before")
    p_before = shot("before")

    mark = sess.logmark()
    drv.send(f"script action play filename {SCRIPT}", 2.0)
    # The show's own clock: two 3 s travels plus its waits. Read (not sleep) so
    # nothing the app says is dropped.
    drv.read(30.0)
    log = sess.lognew(mark)
    after = dump("after")
    p_after = shot("after")
    sess.stop(drv)

    old_ref = [l for l in log.splitlines() if "AnchorManager" in l and "error" in l]
    new_ref = [l for l in log.splitlines() if "camera action" in l
               and ("does nothing" in l or "IGNORED" in l or "NOT changed" in l)]
    # Both shapes: since F73 a refusal with a named origin reads `Error
    # executing <origin>: <line> #! <message>` and one without keeps the old
    # pair (INTENT 11.194). This leg plays a FILE, so it is the first shape it
    # would see - matching only the old needle would make it blind.
    could_not = [l for l in log.splitlines()
                 if "Could not execute" in l or "Error executing " in l]
    chk(not old_ref and not new_ref and not could_not,
        "the shipped flow runs with NO refusal from either path",
        f"old={old_ref} new={new_ref} cmd={could_not}")
    chk(after["camera"]["reference"] == TARGET,
        "the show ends with the camera referencing the body",
        f"reference={after['camera']['reference']}")
    d = float(np.linalg.norm(np.array(after["_old"]) - np.array(after["_new"])))
    chk(d < 3e-07, "the two paths agree on where the observer ended up",
        f"|old - new| = {d:.3e} AU")
    mars = np.array(after["_bodies"][TARGET]["new"]["ecl"], float)
    dist_after = float(np.linalg.norm(np.array(after["_new"]) - mars))
    dist_before = float(np.linalg.norm(np.array(before["_new"]) - mars))
    chk(dist_after < dist_before / 100,
        "the observer actually travelled: two orders of magnitude closer to the body",
        f"{dist_before:.6e} -> {dist_after:.6e} AU")
    A = np.asarray(Image.open(p_before).convert("L"), int)
    B = np.asarray(Image.open(p_after).convert("L"), int)
    chk((np.abs(A - B) > 32).sum() > 1000,
        "the COMPOSED SCREEN changed - the arrival is on the frame, not only in a dump",
        f"{int((np.abs(A-B) > 32).sum())} px>32; lit {int((A>16).sum())} -> {int((B>16).sum())}")

    res = {"checks": CHECKS,
           "distBefore": dist_before, "distAfter": dist_after,
           "endRef": after["camera"]["reference"],
           "endDistance": after["camera"]["distance"],
           "oldHeading": after["oldView"]["nav"]["heading"],
           "newHeading": after["camera"]["heading"],
           "failures": sum(1 for c in CHECKS if not c["ok"])}
    (out / "f33_field.json").write_text(json.dumps(res, indent=1, default=float))
    print(f"\n{res['failures']} failure(s); report {out/'f33_field.json'}")
    return 1 if res["failures"] else 0


if __name__ == "__main__":
    sys.exit(main())
