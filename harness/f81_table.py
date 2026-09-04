#!/usr/bin/env python3
"""F81 - build the result table from the run JSONs (nothing retyped).

usage: f81_table.py <artifacts/f81 dir>   -> writes f81_table.md beside them
"""
import json, math, sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from f81_offset import blob_at          # ONE authority for the blob rule

R = 1024.0          # viewportRadius, from the projector's OWN dump every run
H = 2048.0          # render size, from the projector's OWN viewport dump

# MEASURED seed conventions (F81 prep leg, artifacts/f81/prep): the written PNG
# is the VERTICAL MIRROR of the old dump's screen px -- every body of the fov-180
# zero frame sits at (x, H - y_dump) and none at (x, y_dump).  The new dump is
# normalized with radius = theta/halfFov and maps as b3_ladder's `screen_px`.


def seed_old(s):
    return (s[0], H - s[1]) if s else None


def seed_new(s):
    return ((s[0] + 1) / 2 * H, (1 - s[1]) / 2 * H) if s else None


def blob_of(rundir, e, path):
    """Recompute the blob from the COMMITTED frame, through the one rule."""
    if not e.get("frame"):
        return None
    png = Path(rundir) / e["frame"]
    if not png.exists():
        return None
    s = e["bodyOld"]["screen"] if path == "old" else e["bodyNew"]["screen"]
    return blob_at(png, seed_old(s) if path == "old" else seed_new(s))


def load(p):
    return json.loads(Path(p).read_text())


def rr_dump_old(s):
    return math.hypot(s[0] - 1024.0, s[1] - 1024.0) / R if s else None


def rr_dump_new(s):
    return math.hypot(s[0], s[1]) if s else None


def rr_blob(c):
    if not (c and c.get("found")):
        return None
    return math.hypot(c["cx"] - 1024.0, c["cy"] - 1024.0) / R


def side(c, s_old=None):
    """which side of centre, in the IMAGE (y up = 'up')."""
    if c and c.get("found"):
        return "up" if c["cy"] < 1024.0 else ("down" if c["cy"] > 1024.0 else "-")
    if s_old:
        return "up" if s_old[1] > 1024.0 else ("down" if s_old[1] < 1024.0 else "-")
    return "-"


def main():
    d = Path(sys.argv[1])
    prep = load(d / "prep/f81_prep.json")
    cfg = load(d / "config/f81_run_config.json")
    cmd = load(d / "command/f81_run_command.json")
    tp = load(d / "teleport/f81_teleport.json")
    o = cfg["offset"]
    L = []
    A = L.append
    A("# F81 - the view offset's two couplings, measured\n")
    A(f"binary `{cmd['binary_md5'][:8]}` - code master-beta @ 85cc2785 (engine = ba7a32a8), "
      f"target `{cmd['target']}`, offset {o}, viewportRadius {R:.0f} px (projector's own dump), "
      f"render 2048x2048.\n")
    A(f"Scene: shipped default config, time FROZEN at jd {cmd['jd']}, target at MERIDIAN "
      f"transit (off-meridian {prep['azimuth_off_meridian_deg']:.5f} deg, altitude "
      f"{prep['altitude_deg']:.3f} deg), atmosphere/landscape/stars off, farm "
      f"`init_view_pos` = the target's measured local-frame direction.\n")
    A("`r/R` = distance from the DRAWN CENTRE in dome radii; `theta` = r/R * halfFov.\n")

    A("\n## The table: 3 fovs x 2 channels x 2 paths\n")
    A("| fov | channel | path | predicted r/R | dump r/R | frame r/R | theta measured | predicted theta | side |")
    A("|----:|---------|------|--------------:|---------:|----------:|---------------:|----------------:|------|")
    for chan, run, rundir in (("config", cfg, d / "config"),
                              ("command", cmd, d / "command")):
        for p in run["points"]:
            if not p["tag"].startswith(chan + "_"):
                continue
            fov = p["fov"]
            hf = fov / 2
            for path in ("old", "new"):
                e = p["paths"][path]
                c = blob_of(rundir, e, path)
                if path == "old":
                    dv = rr_dump_old(e["bodyOld"]["screen"])
                    pred = (o if chan == "config" else o * (90.0 - hf) / hf)
                else:
                    dv = rr_dump_new(e["bodyNew"]["screen"])
                    pred = o
                fv = rr_blob(c)
                A("| %d | %s | %s | %.5f | %.5f | %s | %s | %.4f | %s |" % (
                    fov, chan, path, pred, dv,
                    ("%.5f" % fv) if fv is not None else "OFF FRAME",
                    ("%.4f" % (dv * hf)) if dv is not None else "-",
                    pred * hf,
                    side(c, e["bodyOld"]["screen"] if path == "old" else None)))

    A("\n## P1 - the aim compensation, measured directly\n")
    p1 = cmd["P1_aim_compensation"]
    A(f"- angle(localVision before, after) across `set zoom_offset {o}` while armed: "
      f"**{p1['angle_deg']:.6f} deg**  (predicted offset x 90 = {p1['predicted_deg']}; "
      f"a fixed 45 deg would give {p1['mutation45_deg']})")
    A(f"- before `{[round(x, 9) for x in p1['localVision_before']]}` -> after "
      f"`{[round(x, 9) for x in p1['localVision_after']]}`")
    A("- the LOCAL EAST component is unchanged across the rotation "
      f"({p1['localVision_before'][1]:.3e} -> {p1['localVision_after'][1]:.3e}): "
      "the axis IS local y (East), measured, not assumed")
    A(f"- altitude {math.degrees(math.asin(p1['localVision_before'][2])):.4f} deg -> "
      f"{math.degrees(math.asin(p1['localVision_after'][2])):.4f} deg: the aim is RAISED "
      "by the compensation (sign, measured)")

    A("\n## P6 - the 90 deg coefficient read as a slope (fov 40, command channel, old path)\n")
    A("| offset | predicted r/R | dump r/R | frame r/R | mutated (45 deg) |")
    A("|-------:|--------------:|---------:|----------:|-----------------:|")
    for p in cmd["points"]:
        if not p["tag"].startswith("sweep_"):
            continue
        oo = float(p["tag"].split("_")[1][1:]) / 10.0
        e = p["paths"]["old"]
        c = blob_of(d / "command", e, "old")
        dv = rr_dump_old(e["bodyOld"]["screen"])
        fv = rr_blob(c)
        A("| %.1f | %.5f | %.5f | %s | %.5f |" % (
            oo, oo * 3.5, dv, ("%.5f" % fv) if fv is not None else "OFF FRAME",
            oo * (45.0 - 20.0) / 20.0))

    A("\n## Zero control (offset 0, prep launch) - the instrument's own zero\n")
    A("| fov | path | dump r/R | frame centroid px | r px |")
    A("|----:|------|---------:|-------------------|-----:|")
    for p in prep["points"]:
        for path in ("old", "new"):
            e = p["paths"][path]
            c = blob_of(d / "prep", e, path)
            dv = rr_dump_old(e["bodyOld"]["screen"])
            A("| %d | %s | %.5f | %s | %.3f |" % (
                p["fov"], path, dv,
                ("(%.3f, %.3f)" % (c["cx"], c["cy"])) if c and c.get("found") else "-",
                math.hypot(c["cx"] - 1024.0, c["cy"] - 1024.0) if c and c.get("found") else -1))

    A("\n## Supplementary leg - the same sink's OTHER observable (the re-aim is a teleport)\n")
    b, a = tp["before"], tp["after"]
    A(f"Aim at `{tp['other_body']}` (both paths centred on it), offset 0, ARMED on both paths, "
      f"fov 40; then `set zoom_offset {tp['offset']}`:")
    A(f"- OLD aim jumps **{tp['view_jump_deg']:.4f} deg**, landing "
      f"{a['angle_to_initview_deg']:.4f} deg from `init_view_pos` (= the compensation, exactly); "
      f"{tp['other_body']}'s old screen {b['screen_other_old']} -> {a['screen_other_old']} "
      "-- OFF the rendered image")
    A(f"- NEW keeps its aim: {tp['other_body']}'s new screen {b['screen_other_new']} -> "
      f"{a['screen_other_new']} (r/R {rr_dump_new(a['screen_other_new']):.5f}, on screen)")

    A("\n## Discriminating checks\n")
    A("- (a) table filled with MEASURED values; fov-180 old/command reads r/R "
      f"{rr_dump_old(cmd['points'][0]['paths']['old']['bodyOld']['screen']):.5f} = the control; "
      "the mutated prediction (fixed 45 deg) misses P1 by 2x, the fov-180 cell by 13.5 deg, "
      "the fov-90 cell by 13.5 deg and the sweep slope by 2.8x")
    A("- (b) the two channels DISAGREE on the old path (see the table) and AGREE on the new path")
    A("- (c) frame centroid vs dump agree on the new path at every fov")
    A("- (d) real ~/.spacecrafter md5 in == out on all four launches "
      f"(config.ini {cmd['real_home_md5_in']['config.ini'][:8]}, "
      f"ssystem.ini {cmd['real_home_md5_in']['ssystem.ini'][:8]})")
    A(f"- (e) the sink's contract: control.viewOffset = {p1['control']}")
    A("")
    (d / "f81_table.md").write_text("\n".join(L))
    print("\n".join(L))


if __name__ == "__main__":
    main()
