#!/usr/bin/env python3
"""F56 — the PHOTOMETRIC ARM of the environment canary: does the reference scene
still render the banked luminance?

It reads an `f51_dwell.json` (the output of `f51_run.sh`, which is the reference
scene §11.174(e) names) and asserts every photometric member against a band whose
numbers are passed IN by the caller — this file holds NO banked values on purpose.
`f56_canary.sh` owns the one VALUES block (I2: one authority; the band and the
stack fingerprint must not be able to drift apart from each other), and this file
is the pure function that applies it.  That also makes the both-ways demonstration
trivial: the same command, pointed at a committed dim-era `f51_dwell.json`, must
REFUSE it.

Members checked (all of them, never a subset — an absent member is a FAILURE, not
a pass: a green that cannot discriminate is not evidence):
  * every dwell sample        -> new path, disc_mean + hf_mean
  * legs `newpath_after`, `newpath_after_plus10s`   -> new path
  * legs `oldpath_fov10`, `oldpath_fov10_plus10s`   -> old path
The `oldpath_fov60` leg is a different framing (fov 60, ~389 k px of mask) and is
recorded as INFO, never gated: it is not in the banked pair.

Diagnostics follow §11.169's error schema — WHAT / CONSEQUENCES / PREVENTION —
because the canary is itself a user-facing diagnostic surface.

usage: f56_band.py <f51_dwell.json> --new-disc F --new-hf F --old-disc F --old-hf F
                   --tol-disc F --tol-hf F [--exact-spread F] [--frame-md5 HEX]
                   [--out <json>] [--label TEXT]
exit: 0 all members in band | 1 a member out of band | 3 unusable input
      | 4 nothing to check (no members found)
"""
import json
import sys
from pathlib import Path


def getarg(argv, name, cast=str, default=None, required=False):
    if name in argv:
        return cast(argv[argv.index(name) + 1])
    if required:
        print(f"MISSING ARGUMENT {name}")
        sys.exit(3)
    return default


def main():
    argv = sys.argv[1:]
    if not argv or argv[0].startswith("--"):
        print(__doc__)
        return 3
    src = Path(argv[0])
    new_disc = getarg(argv, "--new-disc", float, required=True)
    new_hf = getarg(argv, "--new-hf", float, required=True)
    old_disc = getarg(argv, "--old-disc", float, required=True)
    old_hf = getarg(argv, "--old-hf", float, required=True)
    tol_disc = getarg(argv, "--tol-disc", float, required=True)
    tol_hf = getarg(argv, "--tol-hf", float, required=True)
    exact = getarg(argv, "--exact-spread", float, 0.001)
    frame_md5 = getarg(argv, "--frame-md5", str, None)
    label = getarg(argv, "--label", str, str(src))

    try:
        d = json.loads(src.read_text())
    except Exception as e:                                   # noqa: BLE001
        print(f"CANARY FAIL [photometric.input]\n"
              f"  WHAT        : cannot read {src} as JSON: {e}\n"
              f"  CONSEQUENCES: the photometric arm did not run; the stack is "
              f"UNVERIFIED, so any measurement taken after this point is "
              f"uncontrolled for the §11.174 fault class.\n"
              f"  PREVENTION  : run the reference scene first "
              f"(`f56_canary.sh` with no --check-json), or pass the path of a "
              f"complete f51_dwell.json.")
        return 3

    members = []

    def add(kind, name, key, got, banked, tol):
        if got is None:
            members.append({"member": name, "metric": key, "observed": None,
                            "banked": banked, "tol": tol, "verdict": "MISSING"})
            return
        delta = round(got - banked, 4)
        verdict = "IN-BAND" if abs(delta) <= tol else "OUT-OF-BAND"
        members.append({"member": name, "metric": key, "kind": kind,
                        "observed": got, "banked": banked, "delta": delta,
                        "tol": tol, "verdict": verdict,
                        "exact_reproduction": abs(delta) <= exact})

    for s in d.get("samples", []):
        tag = f"dwell.s{s.get('i')}"
        add("new", tag, "disc_mean", s.get("disc_mean"), new_disc, tol_disc)
        add("new", tag, "hf_mean", s.get("hf_mean"), new_hf, tol_hf)
    legs = {l.get("tag"): l for l in d.get("legs", [])}
    for tag in ("newpath_after", "newpath_after_plus10s"):
        if tag in legs:
            add("new", f"leg.{tag}", "disc_mean", legs[tag].get("disc_mean"),
                new_disc, tol_disc)
            add("new", f"leg.{tag}", "hf_mean", legs[tag].get("hf_mean"),
                new_hf, tol_hf)
    for tag in ("oldpath_fov10", "oldpath_fov10_plus10s"):
        if tag in legs:
            add("old", f"leg.{tag}", "disc_mean", legs[tag].get("disc_mean"),
                old_disc, tol_disc)
            add("old", f"leg.{tag}", "hf_mean", legs[tag].get("hf_mean"),
                old_hf, tol_hf)

    info = {"oldpath_fov60": {k: legs.get("oldpath_fov60", {}).get(k)
                              for k in ("disc_mean", "hf_mean", "n")}
            if "oldpath_fov60" in legs else None,
            "binary_md5": d.get("binary_md5"),
            "n_samples": d.get("n_samples"),
            "fails_reported_by_driver": d.get("fails")}

    md5s = sorted({s.get("md5") for s in d.get("samples", []) if s.get("md5")})
    md5_note = None
    if frame_md5 and md5s:
        if md5s != [frame_md5]:
            md5_note = (f"dwell frame md5 {md5s} != banked {frame_md5} — the frames "
                        f"are not byte-identical to the banked run.  RECORDED, NOT "
                        f"GATED: a legitimate texture-cache or driver change moves "
                        f"the bytes without moving the luminance.")

    bad = [m for m in members if m["verdict"] in ("OUT-OF-BAND", "MISSING")]
    drift = [m for m in members
             if m["verdict"] == "IN-BAND" and not m["exact_reproduction"]]

    print(f"=== F56 PHOTOMETRIC ARM — {label}")
    print(f"  banked: new {new_disc}/{new_hf}  old {old_disc}/{old_hf}  "
          f"tol disc +-{tol_disc} hf +-{tol_hf}")
    for m in members:
        print(f"  {m['verdict']:<11} {m['member']:<28} {m['metric']:<9} "
              f"observed {m['observed']}  banked {m['banked']}  "
              f"delta {m.get('delta')}")
    if info["oldpath_fov60"]:
        print(f"  INFO        leg.oldpath_fov60          {info['oldpath_fov60']} "
              f"(not gated: different framing)")
    if md5_note:
        print(f"  NOTE        {md5_note}")
    if drift:
        print(f"  NOTE        {len(drift)} member(s) IN BAND but off the "
              f"exact-reproduction spread (+-{exact}): "
              f"{[ (m['member'], m['metric'], m['delta']) for m in drift ]} — the "
              f"banked runs reproduce to the printed digit, so a non-zero delta is "
              f"itself news even when it is harmless.")

    if bad:
        worst = max(bad, key=lambda m: abs(m.get("delta") or 9e9))
        print()
        print("CANARY FAIL [photometric.band]")
        print(f"  WHAT        : {len(bad)} of {len(members)} photometric members "
              f"are outside the banked band. Worst: {worst['member']} "
              f"{worst['metric']} = {worst['observed']} against banked "
              f"{worst['banked']} (delta {worst.get('delta')}, tolerance "
              f"+-{worst['tol']}).")
        print( "  CONSEQUENCES: the rendering stack is NOT the one every committed "
               "photometric baseline in this corpus was measured on (§11.174: two "
               "2026-08-29 sessions rendered ~2.7x dark from a faulty display "
               "dispatch and it was invisible to every instrument of the day). Any "
               "luminance, contrast or pixel-count number measured now is not "
               "comparable with the ledger's, and an A/B taken across this "
               "boundary is uninterpretable.")
        print( "  PREVENTION  : do not measure. Report the observed-vs-banked table "
               "above to the dispatcher/owner as an ENVIRONMENT FAULT — an "
               "environment-fault mitigation is an OWNER VETO ITEM (§0.5, "
               "§11.174(h)): your correction may differ from a mitigation, say the "
               "word. If the stack change is DELIBERATE and ratified, re-bank by "
               "re-measuring `f51_run.sh` and editing the single VALUES block at "
               "the top of `f56_canary.sh` (keys BANK_NEW_DISC / BANK_NEW_HF / "
               "BANK_OLD_DISC / BANK_OLD_HF / BANK_FRAME_MD5), never by widening "
               "the tolerance.")

    res = {"label": label, "source": str(src), "members": members,
           "n_members": len(members), "n_out_of_band": len(bad),
           "n_drift_in_band": len(drift), "info": info,
           "banked": {"new_disc": new_disc, "new_hf": new_hf,
                      "old_disc": old_disc, "old_hf": old_hf,
                      "tol_disc": tol_disc, "tol_hf": tol_hf,
                      "exact_spread": exact, "frame_md5": frame_md5},
           "frame_md5_observed": md5s, "md5_note": md5_note,
           "verdict": "FAIL" if bad else ("PASS" if members else "NO-MEMBERS")}
    if "--out" in argv:
        Path(argv[argv.index("--out") + 1]).write_text(json.dumps(res, indent=1))

    if not members:
        print("CANARY FAIL [photometric.empty]")
        print(f"  WHAT        : {src} carries no dwell sample and no leg — "
              f"nothing was checked.")
        print( "  CONSEQUENCES: a pass here would be a green that cannot fail; the "
               "stack would be reported verified on zero evidence.")
        print( "  PREVENTION  : check the driver's own exit code and log "
               "(`f51_run.sh` writes `f51_dwell.json` incrementally; an empty one "
               "means the launch or the TCP phase failed).")
        return 4
    print(f"  VERDICT     : {res['verdict']}  "
          f"({len(members) - len(bad)}/{len(members)} members in band)")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
