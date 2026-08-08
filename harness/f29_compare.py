#!/usr/bin/env python3
# F29 / INTENT §5.46 - the CROSS-BINARY half of the prediction (see
# f29_upchain.py's header for the derivation; this file evaluates it, it does
# not restate it).
#
#   ./f29_compare.py <pre_dir> <post_dir>
#
# Verdicts, each named by the prediction it discharges:
#   P1  the eclRoot == mat-translation invariant: violated pre-fix exactly on
#       the up-chain ancestors, clean post-fix, in EVERY scene.
#   P2  the subject's frame frozen across the reference switch pre-fix, fresh
#       post-fix.
#   P3  the subject's TRAIL head is drawn at project(eclRoot) on BOTH binaries
#       (that IS the mechanism), and project(eclRoot) == the subject's own
#       screen position only post-fix; the control body's head is at its own
#       position on both.
#   P4  the two control scenes are pixel identical pre vs post, against the
#       in-run A/A floor; the subject scene is NOT (a null there would mean a
#       dead instrument, not a clean fix).

import json, os, sys
import numpy as np
from PIL import Image

PRE, POST = os.path.abspath(sys.argv[1]), os.path.abspath(sys.argv[2])
THR = 12
TOL = 3.0    # px: the head is a rasterised line end, not a point sample


def rep(d):
    return json.load(open(os.path.join(d, "f29_report.json")))


def img(d, tag):
    return np.asarray(Image.open(os.path.join(d, f"t_{tag}.png"))
                      .convert("RGB")).astype(np.int32)


def dpx(tag):
    a, b = img(PRE, tag), img(POST, tag)
    if a.shape != b.shape:
        return None, None
    d = np.abs(a - b).max(axis=2)
    return int((d > THR).sum()), int(d.max())


A, B = rep(PRE), rep(POST)
SUB, CTL = A["subject"], A["control"]
out = {"pre_dir": PRE, "post_dir": POST}
ok, notes = True, []

print("=== P1  eclRoot == mat[12:15] invariant, per scene ===")
p1 = {}
for s in ("E", "S", "X", "M"):
    va = sorted(A[f"scene_{s}"]["invariant_violations"])
    vb = sorted(B[f"scene_{s}"]["invariant_violations"])
    p1[s] = {"pre": va, "post": vb}
    print(f"  scene {s}: pre violates {va or '[]'}   post violates {vb or '[]'}")
    if vb:
        ok = False; notes.append(f"P1 post-fix violation in scene {s}: {vb}")
if not p1["M"]["pre"]:
    ok = False; notes.append("P1 pre-fix showed no violation in the subject scene")
out["P1"] = p1

print("\n=== P2  subject frame frozen across the reference switch ===")
print(f"  pre  X={A['P2_subject_eclRoot_X']}  M={A['P2_subject_eclRoot_M']}  frozen={A['P2_frozen']}")
print(f"  post X={B['P2_subject_eclRoot_X']}  M={B['P2_subject_eclRoot_M']}  frozen={B['P2_frozen']}")
print(f"  freeze source centred at ndc pre={A['X_subject_centred_ndc']} post={B['X_subject_centred_ndc']}")
out["P2"] = {"pre_frozen": A["P2_frozen"], "post_frozen": B["P2_frozen"]}
if not A["P2_frozen"]:
    ok = False; notes.append("P2 pre-fix frame was not frozen")
if B["P2_frozen"]:
    ok = False; notes.append("P2 post-fix frame is still frozen")

print("\n=== P3  the render: where the up-chain ancestor's trail draws ===")
p3 = {}
for s in ("E", "S", "X", "M"):
    e = {}
    for tagname, R in (("pre", A), ("post", B)):
        for who in (SUB, CTL):
            b = R[f"scene_{s}"][who]
            e[f"{who}_{tagname}"] = {
                "n_px": b["n_px"], "head_px": b["head_px"],
                "head_to_own_position_px": b.get("head_to_matT_flipY"),
                "head_to_cached_frame_px": b.get("head_to_eclRoot_flipY")}
    p3[s] = e
    print(f"  scene {s}:")
    for k, v in e.items():
        print(f"    {k:14s} trail {v['n_px']:5d} px  head {v['head_px']}  "
              f"-> own {v['head_to_own_position_px']}  -> cached-frame "
              f"{v['head_to_cached_frame_px']}")
out["P3"] = p3

sub_pre = p3["M"][f"{SUB}_pre"]
sub_post = p3["M"][f"{SUB}_post"]
ctl_pre = p3["M"][f"{CTL}_pre"]
ctl_post = p3["M"][f"{CTL}_post"]


def near(v):
    return v is not None and v <= TOL


# The control maps the measurement positively on BOTH binaries.
if not (near(ctl_pre["head_to_own_position_px"]) and near(ctl_post["head_to_own_position_px"])):
    ok = False; notes.append("P3 control body's trail head is NOT at its own position - "
                             "the measurement is not positively mapped")
# The mechanism: the head sits at the CACHED frame, before and after.
if not (near(sub_pre["head_to_cached_frame_px"]) and near(sub_post["head_to_cached_frame_px"])):
    ok = False; notes.append("P3 subject's head is not at project(eclRoot) - "
                             "the stated mechanism is refuted")
# The defect and its repair.
if near(sub_pre["head_to_own_position_px"]):
    ok = False; notes.append("P3 pre-fix subject's trail already ended at the body")
if not near(sub_post["head_to_own_position_px"]):
    ok = False; notes.append("P3 post-fix subject's trail does NOT end at the body")
# The pre-fix head must land at the FRAME CENTRE (P3''): the freeze state
# centred the subject, so the frozen frame's image of it is the view axis.
h, w = A["scene_M"]["shape"][1], A["scene_M"]["shape"][0]
centre = (w / 2.0, h / 2.0)
pre_head = sub_pre["head_px"]
head_to_centre = (None if not pre_head
                  else float(np.hypot(pre_head[0] - centre[0], pre_head[1] - centre[1])))
print(f"  pre-fix subject head {pre_head} vs frame centre {centre}: {head_to_centre} px")
out["P3_pre_head_to_frame_centre_px"] = head_to_centre

out["P3_verdicts"] = {
    "pre_head_at_frame_centre": head_to_centre is not None and head_to_centre <= 12.0,
    "control_positively_mapped": near(ctl_pre["head_to_own_position_px"]) and near(ctl_post["head_to_own_position_px"]),
    "mechanism_holds_both_ways": near(sub_pre["head_to_cached_frame_px"]) and near(sub_post["head_to_cached_frame_px"]),
    "defect_present_pre": not near(sub_pre["head_to_own_position_px"]),
    "repaired_post": near(sub_post["head_to_own_position_px"]),
    "moved_px": (None if not (sub_pre["head_px"] and sub_post["head_px"])
                 else float(np.hypot(sub_pre["head_px"][0] - sub_post["head_px"][0],
                                     sub_pre["head_px"][1] - sub_post["head_px"][1])))}
print(f"  verdicts: {out['P3_verdicts']}")

print("\n=== P4  the as-if controls, pixel level (pre vs post) ===")
p4 = {}
for tag in ("E_off", "E_a", "E_b", "S_off", "S_a", "S_b", "X_a", "X_b", "M_a", "M_b"):
    n, mx = dpx(tag)
    floor = A["scene_" + tag.split("_")[0]]["aa_floor_px"]
    p4[tag] = {"px": n, "max": mx, "in_run_AA_floor": floor}
    print(f"  {tag:5s} pre-vs-post {n} px (max {mx});  in-run A/A floor {floor} px")
out["P4"] = p4
for tag in ("E_a", "S_a"):
    if p4[tag]["px"] != 0:
        ok = False; notes.append(f"P4 control scene {tag} changed pre->post ({p4[tag]['px']} px)")
if p4["M_a"]["px"] == 0:
    ok = False; notes.append("P4 subject scene did not change - dead instrument")

out["VERDICT"] = "PASS" if ok else "FAIL"
out["notes"] = notes
print(f"\nVERDICT: {out['VERDICT']}")
for n in notes:
    print(f"  ! {n}")
with open(os.path.join(POST, "f29_compare.json"), "w") as fh:
    json.dump(out, fh, indent=1, sort_keys=True)
print(f"-> {POST}/f29_compare.json")
sys.exit(0 if ok else 1)
