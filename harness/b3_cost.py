#!/usr/bin/env python3
"""D11 frame-cost probe for the 5.29 ray-march depth write (task F1-P1).

Writing gl_FragDepth disables early-z for the MESH_RAYMARCH colour pass, so the
cost claim is mandatory (INTENT 2.0 D11: denominator 1 ms/frame).

METHOD - throughput, not CPU stage timing.  The `query_statistics` capture
(CaptureMetrics, app.cpp:117) appends a fixed number of TimePoints per frame to
$HOME/.spacecrafter/log/statistics.dat in ring-buffer halves, so the file's
GROWTH RATE during a static dwell is proportional to the frame rate.  The
CPU-side stage timings (b22_parse_stats) cannot see this cost: it is GPU
shading, not draw recording.  `maximum_fps` is raised in the TEMP-HOME config
only (D12: acting default logged) so the cap cannot mask the difference.

Scene: the b3_ladder scene - nadir over the Moon at 8000 km altitude, fov 20,
FISHEYE, moon_scaled off.  The Moon's ray-marched proxy shell covers the whole
2048x2048 render target, i.e. the maximum per-frame load this shader can carry
in a real scene.  Same scene, two deployed bodyRayMarch.frag.spv, fresh
launches (the runner swaps the deployed shader; the BINARY is the same in both
legs, so the measurement isolates the shader change).

usage: b3_cost.py <outdir> <tag> [scene: base|sph]
"""
import json, os, socket, subprocess, sys, time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import b3_ladder as L

WINDOWS = int(os.environ.get("B3_COST_WINDOWS", "3"))
DWELL = float(os.environ.get("B3_COST_DWELL", "20"))


def main():
    out = Path(sys.argv[1]); out.mkdir(parents=True, exist_ok=True)
    tag = sys.argv[2]
    scene = sys.argv[3] if len(sys.argv) > 3 else "base"
    farm = L.FARM / ".spacecrafter"
    stats = farm / "log/statistics.dat"
    # acting defaults for this measurement, temp HOME only (D12)
    cfg = (farm / "config.ini").read_text(encoding="latin-1").splitlines(keepends=True)
    cfg = ["query_statistics               = true\n" if l.startswith("query_statistics")
           else ("maximum_fps                    = 10000\n" if l.startswith("maximum_fps") else l)
           for l in cfg]
    (farm / "config.ini").write_text("".join(cfg), encoding="latin-1")

    twin = (L.REAL_HOME / ".spacecrafter/modularSystem/SolarSystem.ini.disabled").read_bytes()
    smod, _, _ = L.model_radius(L.SPHERE_MODEL)
    sections = ("" if scene == "base" else
                "".join(L.section("S" + t, L.SPHERE_MODEL, L.site_lon(o), 0.0, alt, r / smod)
                        for t, r, alt, o in L.LADDER))
    (farm / "modularSystem/SolarSystem.ini").write_bytes(twin + sections.encode("latin-1"))
    stats.unlink(missing_ok=True)

    env = {**os.environ, "HOME": str(L.FARM), "DISPLAY": os.environ.get("DISPLAY", ":2")}
    proc = subprocess.Popen([L.SC_BIN], cwd=str(farm),
                            stdout=open(out / f"{tag}.applog", "w"), stderr=subprocess.STDOUT, env=env)
    rates = []
    try:
        s = L.wait_port(); time.sleep(10)
        L.send(s, "flag experimental_path on"); L.send(s, "timerate rate 0")
        L.send(s, "meteors zhr 0"); L.send(s, f"date jday {L.JD}")
        L.send(s, "set home_planet Moon", 2)
        L.send(s, "camera action free_mode state on")
        L.send(s, "flag atmosphere off"); L.send(s, "flag landscape off")
        L.send(s, "flag moon_scaled off", 2)
        L.send(s, "select planet Moon")
        L.send(s, f"moveto lat {L.OBS_LAT} lon {L.OBS_LON} alt {L.OBS_ALT_M} duration 0", 5)
        L.send(s, "flag track_object on", 2)
        L.send(s, f"zoom fov {L.FOV_WIDE} duration 0", 2)
        L.send(s, "flag track_object off", 2)
        L.send(s, "flag experimental_shadows off", 2)
        time.sleep(8)   # settle: no command traffic during the windows

        def window(label):
            a = stats.stat().st_size if stats.exists() else 0
            t0 = time.time(); time.sleep(DWELL); dt = time.time() - t0
            b = stats.stat().st_size if stats.exists() else 0
            rates.append({"label": label, "bytes": b - a, "seconds": dt,
                          "bytes_per_s": (b - a) / dt})
            print(f"window {label}: {b-a} bytes in {dt:.2f}s = {(b-a)/dt:.1f} B/s", flush=True)

        if os.environ.get("B3_COST_RATIO"):
            # NORMALIZED instrument: the GPU clock state varies by up to 25%
            # between launches (measured), which swamps a few-% shader delta.
            # Alternate WITHIN one launch between the ray regime (close, the
            # shader under test covers the whole 2048^2 target) and a view
            # outside it (far: distance > 64*scaledRadius, no ray-march at all,
            # every other cost identical).  A clock factor k multiplies both
            # frame times, so the RATIO t_close/t_far is k-free, and the
            # marginal ray-march cost is (ratio - 1) * t_far.
            for i in range(WINDOWS):
                L.send(s, f"moveto lat {L.OBS_LAT} lon {L.OBS_LON} alt {L.OBS_ALT_M} duration 0", 4)
                window(f"close{i}")
                L.send(s, f"moveto lat {L.OBS_LAT} lon {L.OBS_LON} alt 150000000 duration 0", 4)
                window(f"far{i}")
        else:
            for i in range(WINDOWS):
                window(str(i))
        L.send(s, "shutdown action now", 1); s.close()
        try:
            proc.wait(timeout=40)
        except subprocess.TimeoutExpired:
            proc.kill()
    finally:
        if proc.poll() is None:
            proc.kill()
    if stats.exists():
        (out / f"{tag}.dat").write_bytes(stats.read_bytes())
    res = {"tag": tag, "scene": scene, "windows": rates,
           "median_bytes_per_s": sorted(r["bytes_per_s"] for r in rates)[len(rates) // 2]}
    close = [r["bytes_per_s"] for r in rates if r["label"].startswith("close")]
    far = [r["bytes_per_s"] for r in rates if r["label"].startswith("far")]
    if close and far:
        # frame time ratio = fps_far / fps_close = rate_far / rate_close
        res["ratio_tclose_over_tfar"] = (sum(far) / len(far)) / (sum(close) / len(close))
        print(f"ratio t_close/t_far = {res['ratio_tclose_over_tfar']:.4f}", flush=True)
    (out / f"cost_{tag}.json").write_text(json.dumps(res, indent=1))
    print(json.dumps(res, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
