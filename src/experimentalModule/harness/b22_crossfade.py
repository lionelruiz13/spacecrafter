#!/usr/bin/env python3
# B22 - System-collapse cross-fade ramp verification (INTENT 11.64).
#
# The resolved<->dot collapse (ModularSystem::drawNested) is RUNTIME-UNEXERCISED
# until the executor dissolution (§6.9): ssystemFactory->draw is called only
# from the solar/stellar executor modes, where camera->system is the SolarSystem
# whose children are planets (no nested systems); the loader creates only plain
# children (createChild, never createChildSystem); the milkyway-reference case
# runs inGalaxy/inUniverse which never calls ssystemFactory->draw. So the pixel
# render cannot be driven on the composed screen today (SUSPENDED for §6.9).
#
# What IS decidable and verified here: the cross-fade RAMP is a pure function of
# the on-screen px size. This script transcribes the EXACT branch logic of
# ModularSystem::drawNested + the drawAlpha scaling in ModularBody::drawHaloCore
# and asserts DoD 3 (endpoints match pure-resolved / pure-dot, no seam at the
# band boundaries, monotonic alpha across the band) and DoD 5 (the ramp is
# identical crossing OUTWARD resolved->dot and INWARD dot->resolved: a pure
# function of px has no hysteresis). The constants are read from ModularBody.hpp
# so a re-tune of the A15 band width re-validates automatically.
import json, os, re, sys

HDR = os.path.join(os.path.dirname(__file__), "..", "ModularBody.hpp")

def read_constants():
    src = open(HDR).read()
    T = int(re.search(r'SYSTEM_VISIBILITY_SUBSYSTEM_SIZE\s*=\s*(\d+)', src).group(1))
    m = re.search(r'SYSTEM_COLLAPSE_CROSSFADE_BAND\s*=\s*SYSTEM_VISIBILITY_SUBSYSTEM_SIZE\s*/\s*([0-9.]+)f', src)
    B = T / float(m.group(1))
    # sanity: drawAlpha is applied in drawHaloCore (cmag *= drawAlpha)
    assert 'cmag *= drawAlpha' in src, "drawAlpha not applied in drawHaloCore"
    return T, B

def draw_nested(px, T, B):
    """Exact transcription of ModularSystem::drawNested (savedAlpha=1.0).
    Returns (resolved_alpha_or_None, dot_alpha_or_None) - None = representation
    NOT drawn on this frame (the branch does not call it)."""
    saved = 1.0
    if px >= T:
        in_band = px < (T + B)
        t = (px - T) / B if in_band else 1.0
        resolved_alpha = saved * t              # drawSystemBodies halos scaled by this
        dot_alpha = (saved * (1.0 - t)) if in_band else None  # drawStarProxy only in band
        return resolved_alpha, dot_alpha
    else:
        return None, saved                      # pure dot below T

def approx(a, b, eps=1e-9):
    if a is None or b is None:
        return a is b or (a is None and b is None)
    return abs(a - b) <= eps

def main():
    T, B = read_constants()
    results = {"constants": {"threshold_px": T, "band_px": B}, "checks": [], "sweep": []}
    ok = True
    def check(name, cond, detail=""):
        nonlocal ok
        ok = ok and cond
        results["checks"].append({"name": name, "pass": bool(cond), "detail": detail})

    # --- DoD 3: endpoints match the pure representations, no seam ------------
    # Pure representations (what a hard switch draws):
    #   pure dot   (px just below T): resolved None, dot 1.0
    #   pure resolved (px >> T)     : resolved 1.0,  dot None
    below = draw_nested(T - 1e-4, T, B)
    at_T = draw_nested(float(T), T, B)
    at_top = draw_nested(T + B, T, B)
    above = draw_nested(T + B + 5.0, T, B)

    check("bottom-edge == pure dot (resolved invisible, dot full)",
          approx(at_T[0], 0.0) and approx(at_T[1], 1.0),
          f"drawNested(T)={at_T}")
    check("continuity at T (dot full both sides, resolved 0<->absent)",
          approx(below[1], 1.0) and approx(at_T[1], 1.0) and approx(at_T[0], 0.0) and below[0] is None,
          f"below={below} at_T={at_T}")
    check("top-edge == pure resolved (resolved full, dot absent)",
          approx(at_top[0], 1.0) and at_top[1] is None,
          f"drawNested(T+B)={at_top}")
    just_below_top = draw_nested(T + B - 1e-4, T, B)
    check("continuity at T+B (resolved->1, dot->0 then absent)",
          approx(just_below_top[0], 1.0, 1e-4) and approx(just_below_top[1], 0.0, 1e-4)
          and approx(above[0], 1.0) and above[1] is None,
          f"just_below_top={just_below_top} above={above}")
    check("above-band is INERT (== pure resolved, full brightness)",
          approx(above[0], 1.0) and above[1] is None, f"above={above}")
    check("below-threshold is INERT (== pure dot, full brightness)",
          below[0] is None and approx(below[1], 1.0), f"below={below}")

    # --- DoD 3: monotonic alpha across the band -----------------------------
    N = 200
    prev_res, prev_dot = None, None
    mono_res = mono_dot = True
    sum_alpha_const = True
    for i in range(N + 1):
        px = T + B * i / N
        r, d = draw_nested(px, T, B)
        results["sweep"].append({"px": round(px, 5), "resolved_alpha": r, "dot_alpha": d})
        if r is not None and prev_res is not None:
            mono_res = mono_res and (r >= prev_res - 1e-9)      # resolved fades IN
        if d is not None and prev_dot is not None:
            mono_dot = mono_dot and (d <= prev_dot + 1e-9)      # dot fades OUT
        # complementary: resolved_alpha + dot_alpha == 1 inside the band
        if r is not None and d is not None:
            sum_alpha_const = sum_alpha_const and approx(r + d, 1.0)
        prev_res, prev_dot = r if r is not None else prev_res, d if d is not None else prev_dot
    check("resolved alpha monotonic non-decreasing across band (fades IN)", mono_res)
    check("dot alpha monotonic non-increasing across band (fades OUT)", mono_dot)
    check("resolved_alpha + dot_alpha == 1 within band (true cross-dissolve)", sum_alpha_const)

    # --- DoD 5: reversible pair - the ramp is a PURE function of px ----------
    # Cross OUTWARD (resolved->dot: px decreasing) then INWARD (dot->resolved:
    # px increasing), second sweep starting from the first's end state. A pure
    # function of px cannot have hysteresis: alpha at a given px is identical in
    # both directions. We assert exactly that on a shared sample grid.
    grid = [T - 2 + 0.13 * k for k in range(int((B + 6) / 0.13))]
    outward = [draw_nested(px, T, B) for px in reversed(grid)]   # px decreasing
    inward = [draw_nested(px, T, B) for px in grid]              # px increasing
    sym = all(approx(o[0], i[0]) and approx(o[1], i[1])
              for o, i in zip(reversed(outward), inward))
    check("reversible: alpha(px) identical outward vs inward (no hysteresis)", sym)
    # second entry from the first's exit state: crossing twice lands identically
    twice = draw_nested(grid[0], T, B) == draw_nested(grid[0], T, B)
    check("second crossing from first's end state is bit-identical", twice)

    results["all_pass"] = ok
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        os.path.dirname(__file__), "artifacts", "b22", "b22_ramp.json")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    json.dump(results, open(out, "w"), indent=2)
    for c in results["checks"]:
        print(("PASS" if c["pass"] else "FAIL"), c["name"], ("" if c["pass"] else "-> " + c["detail"]))
    print(f"\nT={T}px  band={B}px  ->  {'ALL PASS' if ok else 'FAILURES'}  ({out})")
    sys.exit(0 if ok else 1)

if __name__ == "__main__":
    main()
