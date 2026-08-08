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
#   P3  the subject's orbit-line mask moves pre->post, and post-fix passes
#       within ~1 px of the subject's own screen position (the control body
#       does so on BOTH binaries - the positive map of the measurement).
#   P4  the two no-parented-up-chain-ancestor control scenes are pixel
#       identical pre vs post, against the in-run A/A floor.

import json, os, sys
import numpy as np
from PIL import Image

PRE, POST = os.path.abspath(sys.argv[1]), os.path.abspath(sys.argv[2])
THR = 12


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
ok = True

print("=== P1  eclRoot == mat[12:15] invariant, per scene ===")
p1 = {}
for s in ("E", "M", "S"):
    va = sorted(A[f"scene_{s}"]["invariant_violations"])
    vb = sorted(B[f"scene_{s}"]["invariant_violations"])
    p1[s] = {"pre": va, "post": vb}
    print(f"  scene {s}: pre violates {va or '[]'}   post violates {vb or '[]'}")
    if vb:
        ok = False
out["P1"] = p1

print("\n=== P2  subject frame frozen across the reference switch ===")
print(f"  pre  eclRoot(E)={A['P2_subject_eclRoot_E']}")
print(f"  pre  eclRoot(M)={A['P2_subject_eclRoot_M']}  frozen={A['P2_frozen']}")
print(f"  post eclRoot(E)={B['P2_subject_eclRoot_E']}")
print(f"  post eclRoot(M)={B['P2_subject_eclRoot_M']}  frozen={B['P2_frozen']}")
out["P2"] = {"pre_frozen": A["P2_frozen"], "post_frozen": B["P2_frozen"]}
if not A["P2_frozen"] or B["P2_frozen"]:
    ok = False

print("\n=== P3  the render: where the up-chain ancestor's orbit draws ===")
p3 = {}
for s in ("E", "M", "S"):
    ma, mb = A[f"scene_{s}"]["mask_subject"], B[f"scene_{s}"]["mask_subject"]
    ca, cb = A[f"scene_{s}"]["mask_control"], B[f"scene_{s}"]["mask_control"]
    p3[s] = {"subject_mask_pre": ma, "subject_mask_post": mb,
             "subject_mask_moved": ma != mb,
             "control_mask_moved": ca != cb,
             "subject_notch_pre": A[f"scene_{s}"][SUB]["notch_px_flipY"],
             "subject_notch_post": B[f"scene_{s}"][SUB]["notch_px_flipY"],
             "control_notch_pre": A[f"scene_{s}"][CTL]["notch_px_flipY"],
             "control_notch_post": B[f"scene_{s}"][CTL]["notch_px_flipY"]}
    print(f"  scene {s}: subject mask moved={p3[s]['subject_mask_moved']} "
          f"control mask moved={p3[s]['control_mask_moved']}")
    print(f"    subject notch->own screen px: pre {p3[s]['subject_notch_pre']} "
          f"post {p3[s]['subject_notch_post']}")
    print(f"    control notch->own screen px: pre {p3[s]['control_notch_pre']} "
          f"post {p3[s]['control_notch_post']}")
out["P3"] = p3
out["P3_mask_E_eq_M"] = {"pre": A["P3_mask_E_eq_M"], "post": B["P3_mask_E_eq_M"]}
print(f"  subject mask identical E vs M (the freeze, on screen): "
      f"pre={A['P3_mask_E_eq_M']} post={B['P3_mask_E_eq_M']}")
if not A["P3_mask_E_eq_M"] or B["P3_mask_E_eq_M"]:
    ok = False

print("\n=== P4  the as-if controls, pixel level (pre vs post) ===")
p4 = {}
for tag in ("E_off", "E_on_a", "E_on_b", "S_off", "S_on_a", "S_on_b",
            "M_off", "M_on_a", "M_on_b"):
    n, mx = dpx(tag)
    p4[tag] = {"px": n, "max": mx}
    floor = A["scene_" + tag.split("_")[0]]["aa_floor_px"]
    print(f"  {tag:9s} pre-vs-post {n} px (max {mx});  in-run A/A floor {floor} px")
out["P4"] = p4
for tag in ("E_off", "E_on_a", "S_off", "S_on_a"):
    if p4[tag]["px"] != 0:
        ok = False
if p4["M_on_a"]["px"] == 0:
    ok = False   # the subject scene MUST change - a null there is a dead instrument

out["VERDICT"] = "PASS" if ok else "FAIL"
print(f"\nVERDICT: {out['VERDICT']}")
with open(os.path.join(POST, "f29_compare.json"), "w") as fh:
    json.dump(out, fh, indent=1, sort_keys=True)
print(f"-> {POST}/f29_compare.json")
sys.exit(0 if ok else 1)
