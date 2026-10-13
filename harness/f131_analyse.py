#!/usr/bin/env python3
# Judge the records of f131_obs.py against the banked readings: one PASS / FAIL / NOTE line per criterion.
#   f131_analyse.py judge <obs> <nat|ref|key|refkey> <new|old> <record dir>   the criteria of one launch (key, refkey: obs 4)
#   f131_analyse.py cross <obs> <nat|ref|key|refkey> <new dir> <old dir>      the criteria across the two pins
#   f131_analyse.py state <dir> <tag>                                         one dump as the criteria read it
#   f131_analyse.py ring <dir> <tag> [a] [b]                                  the seam ring of a dump, frames a..b
#   f131_analyse.py table <record dir>                                        write the record's per-frame fov table (obs 5)
# A line: PASS|FAIL|NOTE <obs> <pin> <criterion> observed=<v> expected=<v> tol=<t>. Exit 1 on a gate FAIL, 2 on usage.
import ast, functools, json, math, os, re, sys
import numpy as np
from PIL import Image
from scipy.ndimage import gaussian_filter, map_coordinates, maximum_filter, median_filter

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from f56_starfield import load as load_png

AU = 149597870.7
FIELD = os.path.expanduser("~/.spacecrafter/ssystem.ini")

_src = os.path.join(HERE, "f124_legs.py")
_node = next(n for n in ast.parse(open(_src).read()).body if isinstance(n, ast.FunctionDef) and n.name == "reuse")
exec(compile(ast.Module([_node], []), _src, "exec"), globals())
reuse(os.path.join(HERE, "b21_keypath.py"), ["_clean"])

# The banked readings, one row per criterion; re-bank a row only with its argument written in the ledger.
#   pin: new / old / both / x (across the two pins); class: gate (sets the exit) or note (printed, never gates);
#   check: eq | near (|o - e| <= tol) | rel (tol in % of e) | earth_view, earth_chord, earth_angle (e carried to the run's jd).
BANK = """
# obs var pin  criterion                          class check       expected     tol
1   nat both scene_reached                        gate  eq          reached      -
1   nat both observers_view_apart_late_deg        gate  near        1.84e-5      4e-5
1   nat both observers_apart_late_km              gate  near        4.05         4
1   nat new  star_points_late                     gate  near        565          28
1   nat old  star_points_late                     gate  near        565          28
1   nat new  stars_in_drawn_buffer_late           gate  near        3320         166
1   nat old  stars_in_drawn_buffer_late           gate  near        3320         166
1   nat both late_after_sunset_plus_005_d         gate  near        0.00049      0.002
1   nat both separation_angle_deg                 note  near        2.52e-5      6e-5
1   nat both latitude_reported_deg                note  near        43.3         0.01
1   nat both longitude_reported_deg               note  near        5.36667      0.01
1   nat new  altitude_reported_m                  note  near        75.10        1
1   nat old  altitude_reported_m                  note  near        75.00        1
1   nat new  sky_brightness                       note  rel         0.01100      5%
1   nat old  sky_brightness                       note  rel         0.01100      5%
1   nat new  adaptation_luminance                 note  rel         4.728        5%
1   nat old  adaptation_luminance                 note  rel         4.729        5%
1   nat both camera_bound_pre                     note  eq          true         -
1   nat both camera_bound_late                    note  eq          true         -
1   nat both sunset_frame_minus_port_s            note  near        0.20         0.5
2   nat both scene_reached                        gate  eq          reached      -
2   nat both race_margin_ms                       gate  near        9            9
2   nat both race_forms_agree                     gate  eq          true         -
2   nat both camera_reference_final               gate  eq          Saturn       -
2   nat both old_home_final                       gate  eq          Saturn       -
2   nat both refusal_logged                       gate  eq          false        -
2   nat both mw_band_pre                          gate  rel         3.4005e6     1%
2   nat new  mw_band_end_over_pre                 gate  near        0.7033       0.01
2   nat old  mw_band_end_over_pre                 gate  near        0.7021       0.01
2   nat new  orbit_distance_km                    gate  near        260265.7     2
2   nat old  orbit_distance_km                    gate  near        260288.8     2
2   nat new  mw_total_end_over_pre                note  near        0.7856       0.02
2   nat old  mw_total_end_over_pre                note  near        0.7985       0.02
2   ref both scene_reached                        gate  eq          reached      -
2   ref both camera_reference_final               gate  eq          temp_point   -
2   ref both refusal_logged                       gate  eq          true         -
2   ref both old_home_final                       gate  eq          Saturn       -
2   ref both mw_band_pre                          gate  rel         3.4005e6     1%
2   ref new  mw_band_end_over_pre                 gate  near        0.00027      0.002
2   ref old  mw_band_end_over_pre                 gate  near        0.7018       0.01
2   ref old  orbit_distance_km                    gate  near        260312.0     20
2   ref new  camera_distance_to_saturn_km         note  near        260083.1     400
2   ref new  mw_total_end_over_pre                note  near        0.1208       0.02
2   ref old  mw_total_end_over_pre                note  near        0.7984       0.02
3   nat both scene_reached                        gate  eq          reached      -
3   nat both race_margin_ms                       gate  near        9            9
3   nat both race_forms_agree                     gate  eq          true         -
3   nat both camera_reference_final               gate  eq          Saturn       -
3   nat both old_home_final                       gate  eq          Saturn       -
3   nat both refusal_logged                       gate  eq          false        -
3   nat new  saturn_lit_px_z60                    gate  rel         915152       1%
3   nat old  saturn_lit_px_z60                    gate  rel         917926       1%
3   nat x    lit_px_new_over_old                  gate  near        0.99698      0.01
3   nat new  halo_px_z60                          note  near        25           10
3   nat new  halo_px_full                         note  near        37           10
3   nat old  halo_px_z60                          note  near        22           10
3   nat old  halo_px_full                         note  near        40           10
3   nat new  lit_mean_luminance                   note  rel         134.68       2%
3   nat old  lit_mean_luminance                   note  rel         134.18       2%
3   nat both camera_system                        note  eq          SolarSystem  -
3   nat both light_position_au                    note  rel         9.4577       1%
3   ref both scene_reached                        gate  eq          reached      -
3   ref both camera_reference_final               gate  eq          temp_point   -
3   ref both refusal_logged                       gate  eq          true         -
3   ref both old_home_final                       gate  eq          Saturn       -
3   ref new  saturn_lit_px_z60                    gate  rel         596064       1%
3   ref old  saturn_lit_px_z60                    gate  rel         917926       1%
3   ref new  halo_px_z60                          note  near        26           10
3   ref new  halo_px_full                         note  near        41           10
3   ref old  halo_px_z60                          note  near        22           10
3   ref old  halo_px_full                         note  near        40           10
3   ref new  lit_mean_luminance                   note  rel         154.75       2%
3   ref old  lit_mean_luminance                   note  rel         134.18       2%
3   ref both camera_system                        note  eq          Universe     -
3   ref both light_position_au                    note  rel         1.0166       1%
4   nat both scene_reached                        gate  eq          reached      -
4   nat both race_margin_ms                       gate  near        9            9
4   nat both race_forms_agree                     gate  eq          true         -
4   nat both camera_reference_final               gate  eq          Saturn       -
4   nat both old_home_final                       gate  eq          Saturn       -
4   nat both refusal_logged                       gate  eq          false        -
4   nat new  turn_lon_deg                         gate  near        9.873        0.1
4   nat new  turn_lat_deg                         gate  near        10.000       0.1
4   nat old  turn_lon_deg                         gate  near        10.068       0.1
4   nat old  turn_lat_deg                         gate  near        10.002       0.1
4   nat new  other_turn_lon_deg                   note  near        80.767       1
4   nat new  other_turn_lat_deg                   note  near        10.002       0.1
4   nat new  reported_altitude_before_km          note  near        199997.7     1
4   nat old  reported_altitude_before_km          note  near        200020.8     1
4   ref both scene_reached                        gate  eq          reached      -
4   ref both camera_reference_final               gate  eq          temp_point   -
4   ref both refusal_logged                       gate  eq          true         -
4   ref both old_home_final                       gate  eq          Saturn       -
4   ref new  turn_lon_deg                         gate  near        0.042        0.2
4   ref new  turn_lat_deg                         gate  near        0.042        0.2
4   ref old  turn_lon_deg                         gate  near        10.068       0.1
4   ref old  turn_lat_deg                         gate  near        10.002       0.1
4   ref new  other_turn_lon_deg                   note  near        103.342      1
4   ref new  other_turn_lat_deg                   note  near        10.001       0.1
4   ref new  other_distance_after_km              note  near        60268.0      1
4   ref old  other_turn_lon_deg                   note  near        29.590       1
4   ref old  other_turn_lat_deg                   note  near        5.250        1
4   ref new  reported_altitude_before_km          note  near        0.0          1
4   ref old  reported_altitude_before_km          note  near        200049.6     1
4   key both scene_reached                        gate  eq          reached      -
4   key both race_margin_ms                       gate  near        9            9
4   key both race_forms_agree                     gate  eq          true         -
4   key both camera_reference_final               gate  eq          Saturn       -
4   key both old_home_final                       gate  eq          Saturn       -
4   key both refusal_logged                       gate  eq          false        -
4   key new  lon_per_call_deg                     gate  near        0.4402817    1e-5
4   key new  lat_per_call_deg                     gate  near        0.4402817    1e-5
4   key new  other_lon_per_call_deg               gate  near        0.4402817    1e-6
4   key new  other_lat_per_call_deg               gate  near        0.4402817    1e-6
4   key old  lon_per_call_deg                     gate  near        0.4402935    1e-6
4   key old  lat_per_call_deg                     gate  near        0.4402935    1e-6
4   key old  other_lon_per_call_deg               gate  near        0.4402935    1e-5
4   key old  other_lat_per_call_deg               gate  near        0.4402935    1e-5
4   key new  turn_lon_deg                         gate  near        8.6940       0.1
4   key new  turn_lat_deg                         gate  near        8.8056       0.1
4   key old  turn_lon_deg                         gate  near        8.8147       0.1
4   key old  turn_lat_deg                         gate  near        8.8067       0.1
4   key both old_altitude_held_km                 gate  near        0            0.001
4   key new  other_turn_lon_deg                   note  near        8.8144       0.1
4   key new  other_turn_lat_deg                   note  near        8.8065       0.1
4   key old  other_turn_lon_deg                   note  near        8.6942       0.1
4   key old  other_turn_lat_deg                   note  near        8.8059       0.1
4   key new  other_distance_after_km              note  near        260288.8     2
4   key old  other_distance_after_km              note  near        260265.7     2
4   key both old_altitude_after_km                note  near        200020.8     1
4   key new  reported_altitude_before_km          note  near        199997.7     1
4   key old  reported_altitude_before_km          note  near        200020.8     1
4   key both log_lines_during_lon_drive           note  near        0            0
4   refkey both scene_reached                     gate  eq          reached      -
4   refkey both camera_reference_final            gate  eq          temp_point   -
4   refkey both refusal_logged                    gate  eq          true         -
4   refkey both old_home_final                    gate  eq          Saturn       -
4   refkey new  lon_per_call_deg                  gate  near        0            1e-6
4   refkey new  lat_per_call_deg                  gate  near        0            1e-6
4   refkey new  other_lon_per_call_deg            gate  near        0            1e-6
4   refkey new  other_lat_per_call_deg            gate  near        0            1e-6
4   refkey old  lon_per_call_deg                  gate  near        0.440307     1e-5
4   refkey old  lat_per_call_deg                  gate  near        0.440307     1e-5
4   refkey old  other_lon_per_call_deg            gate  near        0.440307     1e-5
4   refkey old  other_lat_per_call_deg            gate  near        0.440307     1e-5
4   refkey new  turn_lon_deg                      gate  near        0.03         0.2
4   refkey new  turn_lat_deg                      gate  near        0.03         0.2
4   refkey old  turn_lon_deg                      gate  near        8.8149       0.1
4   refkey old  turn_lat_deg                      gate  near        8.8070       0.1
4   refkey both old_altitude_held_km              gate  near        0            0.001
4   refkey new  other_turn_lon_deg                note  near        0.12         0.1
4   refkey new  other_turn_lat_deg                note  near        0.12         0.1
4   refkey old  other_turn_lon_deg                note  near        0.03         0.2
4   refkey old  other_turn_lat_deg                note  near        0.03         0.2
4   refkey new  other_distance_after_km           note  near        260313.4     10
4   refkey old  other_distance_after_km           note  near        260250       400
4   refkey both old_altitude_after_km             note  near        200049.6     10
4   refkey new  reported_altitude_before_km       note  near        0.0          1
4   refkey old  reported_altitude_before_km       note  near        200049.6     10
4   refkey both log_lines_during_lon_drive        note  near        0            0
5   nat both scene_reached                        gate  eq          reached      -
5   nat both wheel_windows                        gate  eq          9            -
5   nat new  max_abs_delta_w1_deg                 gate  rel         26.8089      5%
5   nat new  max_abs_delta_w2_deg                 gate  rel         25.0947      5%
5   nat new  max_abs_delta_w3_deg                 gate  rel         19.5333      5%
5   nat new  max_abs_delta_w4_deg                 gate  rel         9.7864       5%
5   nat new  max_abs_delta_w5_deg                 gate  rel         5.1681       5%
5   nat new  max_abs_delta_w6_deg                 gate  rel         4.6489       5%
5   nat new  max_abs_delta_w7_deg                 gate  rel         7.0579       5%
5   nat new  max_abs_delta_w8_deg                 gate  rel         10.5085      5%
5   nat new  max_abs_delta_w9_deg                 gate  rel         45.7145      5%
5   nat old  max_abs_delta_w1_deg                 gate  rel         25.3326      5%
5   nat old  max_abs_delta_w2_deg                 gate  rel         25.1531      5%
5   nat old  max_abs_delta_w3_deg                 gate  rel         19.5333      5%
5   nat old  max_abs_delta_w4_deg                 gate  rel         9.7667       5%
5   nat old  max_abs_delta_w5_deg                 gate  rel         5.1587       5%
5   nat old  max_abs_delta_w6_deg                 gate  rel         4.6517       5%
5   nat old  max_abs_delta_w7_deg                 gate  rel         6.9841       5%
5   nat old  max_abs_delta_w8_deg                 gate  rel         10.5085      5%
5   nat old  max_abs_delta_w9_deg                 gate  rel         45.7912      5%
5   nat new  lag_w1_ms                            gate  near        -34          20
5   nat new  lag_w2_ms                            gate  near        67           20
5   nat new  lag_w3_ms                            gate  near        -33          20
5   nat new  lag_w4_ms                            gate  near        -33          20
5   nat new  lag_w5_ms                            gate  near        -33          20
5   nat new  lag_w6_ms                            gate  near        66           20
5   nat new  lag_w7_ms                            gate  near        67           20
5   nat new  lag_w8_ms                            gate  near        66           20
5   nat new  lag_w9_ms                            gate  near        -650         20
5   nat old  lag_w1_ms                            gate  near        -33          20
5   nat old  lag_w2_ms                            gate  near        67           20
5   nat old  lag_w3_ms                            gate  near        -33          20
5   nat old  lag_w4_ms                            gate  near        -34          20
5   nat old  lag_w5_ms                            gate  near        -33          20
5   nat old  lag_w6_ms                            gate  near        67           20
5   nat old  lag_w7_ms                            gate  near        66           20
5   nat old  lag_w8_ms                            gate  near        66           20
5   nat old  lag_w9_ms                            gate  near        -650         20
5   nat both rest_delta_w1_deg                    gate  near        0            1e-4
5   nat both rest_delta_w2_deg                    gate  near        0            1e-4
5   nat both rest_delta_w5_deg                    gate  near        0            1e-4
5   nat both rest_delta_w8_deg                    gate  near        0            1e-4
5   nat both rest_delta_w9_deg                    gate  near        0            1e-4
5   nat both frame_dt_median_ms                   note  near        17           1
5   nat x    band_corr_cap_mid                    note  near        0.9971       0.005
5   nat x    stars_corr_cap_mid                   note  near        0.6946       0.1
5   nat x    band_corr_cap_rest180                note  near        0.9998       0.005
5   nat x    stars_corr_cap_rest180               note  near        0.9973       0.1
5   nat x    band_corr_cap_rest60                 note  near        0.9999       0.005
5   nat x    stars_corr_cap_rest60                note  near        0.9952       0.1
6   nat both scene_reached                        gate  eq          reached      -
6   nat both line_over_grid_cap_mid               gate  near        1.00         0.02
6   nat both line_over_grid_cap_rest60            note  near        1.00         0.02
6   nat both line_over_grid_cap_rest180           note  near        1.00         0.02
"""
# The bank's `late` frame for the earth rows: its jd and the signed separation (deg) of the two observers about Earth's axis.
EARTH_BANK = (2461310.2805862916, 16.5972)
REFUSAL = "a scripted travel is in flight"
VARIANTS = {"nat": "", "ref": "ref", "key": "key", "refkey": "refkey"}   # variant -> its suffix in a verdict line's <obs>
NATURAL = ("nat", "key")                                                 # the variants that play the tester's script (the race)


class Missing(str):
    """Mark an observation that could not be made, with its reason."""


@functools.lru_cache(maxsize=None)
def load(d, tag):
    """Return (header, bodies) of dump <d>/<tag>.json; raise on a line that does not parse."""
    hdr, bodies = None, {}
    for ln in open("%s/%s.json" % (d, tag)):
        if not ln.strip():
            continue
        o = json.loads(_clean(ln))
        if o.get("type") == "header":
            hdr = o
        elif o.get("type") == "body":
            bodies[o["name"]] = o
    if hdr is None:
        raise ValueError("%s/%s.json holds no header" % (d, tag))
    return hdr, bodies


def state(d, tag):
    h, b = load(d, tag)
    c, ctl, ov = h["camera"], h["control"], h["oldView"]
    s = {"tag": tag, "jd": h["jd"], "drawn": ctl["drawnPath"], "cam_ref": c["reference"], "cam_free": c["freeMode"],
         "cam_bound": c["boundToSurface"], "cam_tracked": c["tracked"], "cam_heading_deg": math.degrees(c["heading"]),
         "lat": ctl["latitude"], "lon": ctl["longitude"], "alt_m": ctl["altitude"], "heading": ctl["heading"],
         "old_home": ov["observer"]["homePlanet"], "old_onBody": ov["observer"]["onBody"],
         "fov_old": ov["projector"]["fov"], "fov_new": 2 * math.degrees(c["halfFov"]),
         "skyBrightness": ov["tone"]["skyBrightness"], "atmShown": ov["tone"]["atmosphereShown"],
         "atmFade": ov["tone"]["atmosphereFadeIntensity"], "worldAdapt": ov["tone"]["worldAdaptationLuminance"],
         "stars_nbToDraw": ov["stars"]["nbStarsToDraw"], "stars_fader": ov["stars"]["fader"],
         "anchors": h["anchors"], "cam_rootPos": c["rootPos"], "cam_pos_len_km": math.sqrt(sum(x * x for x in c["position"])) * AU,
         "cam_dist_member_km": c["distance"] * AU, "cam_selected": c["selected"], "absFwd": c["absFwd"]}
    return s, h, b


def ring(h):
    return h["seam"]["steps"]


def star_points(png):
    """Count star-like points: pixels 40 grey above the 11x11 median, in the upper sky half of the disc."""
    img = np.asarray(Image.open(png).convert("L"), dtype=np.float32)
    med = median_filter(img, size=11)
    peak = (img - med > 40) & (img == maximum_filter(img, size=5))
    n = img.shape[0]
    yy, xx = np.mgrid[0:n, 0:n]
    r = np.hypot(xx - (n - 1) / 2, yy - (n - 1) / 2)
    return int((peak & (r < 0.40 * n)).sum())


def sunset_jd(jd, lon, lat):
    """Compute TimeMgr::dateSunSet (src/coreModule/time_mgr.cpp), ported line for line."""
    d2r, r2d = math.pi / 180.0, 180.0 / math.pi
    f32 = lambda x: float(np.float32(x))
    n = round(jd - 2451545.0009 - lon / 360.0)
    temp = f32(lon / 360.0)
    J = temp + n + f32(2451545.0009)
    M = float(np.fmod(np.float32(357.5291 + 0.98560028 * (J - 2451545.0)), np.float32(360.0)))
    C = (1.9148 * math.sin(d2r * M)) + (0.0200 * math.sin(d2r * 2.0 * M)) + (0.0003 * math.sin(d2r * 3.0 * M))
    el = float(np.fmod(np.float32(M + 102.9372 + C + 180.0), np.float32(360.0)))
    s = r2d * math.asin(math.sin(d2r * el) * math.sin(d2r * 23.45))
    Ho = (math.sin(-0.83 * d2r) - (math.sin(d2r * lat) * math.sin(d2r * s))) / (math.cos(d2r * lat) * math.cos(d2r * s))
    if abs(Ho) > 1:
        return J + 0.5
    w = r2d * math.acos(Ho)
    jset = 2451545.0009 + (w + lon) / 360.0 + n + (0.0053 * math.sin(d2r * M)) - (0.0069 * math.sin(d2r * 2.0 * el))
    nova = jset - 2.0 * ((1.0 / 24.0) * (lon / 15.0))
    if nova - jd >= 0.99:
        nova -= 1
    if jd - nova >= 0.99:
        nova += 1
    return nova


@functools.lru_cache(maxsize=None)
def race(d, tag="final"):
    """Replay the script's 3000 ms wait against the new travel's arrival: accepted iff jd(frame before line 38) >= arrival."""
    h, b = load(d, tag)
    st, tr = h["seam"]["steps"], h["seam"]["travels"]
    new = [t for t in tr if t["engine"] == 1 and t["endTime"] - t["startTime"] > 1e-6]
    if not new:
        return {"install": None, "note": "no timed travel on the new registry"}
    t = new[-1]
    byf = {s["frame"]: s for s in st}
    f0 = t["frame"]
    acc, k = 0, f0
    while acc < 3000:
        k += 1
        acc += byf[k]["dt"]
    dt0 = byf[f0]["dt"]
    s_km1 = acc - byf[k]["dt"]
    jd_before = byf[k - 1]["jd"]
    log = open(d + "/log_spacecrafter.txt", errors="replace").read()
    refused = REFUSAL in log
    return {"install_frame": f0, "line38_frame": k, "dt0_ms": dt0, "S_ms": s_km1, "dt0_plus_S": dt0 + s_km1,
            "predicted_accept_ms_form": dt0 + s_km1 >= 3000,
            "jd_before_line38": jd_before, "travel_endTime": t["endTime"], "jd_margin_s": (jd_before - t["endTime"]) * 86400,
            "predicted_accept_jd_form": jd_before >= t["endTime"], "log_says_refused": refused,
            "camera_reference_final": h["camera"]["reference"], "old_home_final": h["oldView"]["observer"]["homePlanet"]}


def wheel(d, tag="final", out=None):
    """Tabulate each wheel window per frame and time each engine's half path."""
    h, b = load(d, tag)
    st = h["seam"]["steps"]
    rows, wins, cur = [], [], None
    for i, s in enumerate(st):
        new = 2 * math.degrees(s["halfFovNew"])
        active = s["zoomTNew"] > 0 or abs(s["aimFovOld"] - s["fovOld"]) > 1e-9
        rows.append((s["frame"], s["dt"], s["fovOld"], new, s["fovOld"] - new, s["aimFovOld"], active))
        if active and cur is None:
            cur = [i, i]
        elif active:
            cur[1] = i
        elif cur is not None and i - cur[1] > 3:
            wins.append(tuple(cur)); cur = None
    if cur:
        wins.append(tuple(cur))
    res = []
    for a, bnd in wins:
        start_old, start_new = rows[a - 1][2], rows[a - 1][3]
        end_old, end_new = rows[min(bnd + 3, len(rows) - 1)][2], rows[min(bnd + 3, len(rows) - 1)][3]
        t, t50o, t50n, mx = 0.0, None, None, 0.0
        for r in rows[a:bnd + 4]:
            t += r[1] / 1000.0
            if t50o is None and abs(r[2] - start_old) >= 0.5 * abs(end_old - start_old):
                t50o = t
            if t50n is None and abs(r[3] - start_new) >= 0.5 * abs(end_new - start_new):
                t50n = t
            mx = max(mx, abs(r[4]))
        rest = max(abs(r[4]) for r in rows[bnd + 1:bnd + 31]) if bnd + 1 < len(rows) else None
        res.append({"frames": (rows[a][0], rows[bnd][0]), "start": (round(start_old, 4), round(start_new, 4)),
                    "end": (round(end_old, 4), round(end_new, 4)), "max_abs_delta_deg": round(mx, 4),
                    "t50_stars_s": t50o, "t50_bodies_s": t50n,
                    "lag_stars_minus_bodies_ms": None if t50o is None or t50n is None else round((t50o - t50n) * 1000, 1),
                    "rest_30f_max_delta": rest})
    if out:
        with open(out, "w") as f:
            f.write("frame dt_ms fov_old_deg fov_new_deg delta_deg aim_old_deg active\n")
            for a, bnd in wins:
                for r in rows[a - 2:bnd + 6]:
                    f.write("%d %d %.6f %.6f %.6f %.6f %d\n" % (r[0], r[1], r[2], r[3], r[4], r[5], r[6]))
                f.write("\n")
    return res


@functools.lru_cache(maxsize=None)
def earth_period_h():
    """Read Earth's rotation period (hours) from the loaded field."""
    sec = None
    for ln in open(FIELD, encoding="latin-1"):
        s = ln.split("#")[0].strip()
        if s.startswith("["):
            sec = s.lower()
        elif sec == "[earth]" and s.split("=")[0].strip() == "rot_periode":
            return float(s.split("=")[1])
    raise ValueError("no rot_periode under [earth] in " + FIELD)


def carried(check, value, jd):
    """Carry a banked separation reading to jd: the unbound Camera keeps its place in Earth's non-rotating frame."""
    jd0, a0 = EARTH_BANK
    a = a0 + 360.0 * 24.0 / earth_period_h() * (jd - jd0)
    s0, s = abs(math.sin(math.radians(a0) / 2)), abs(math.sin(math.radians(a) / 2))
    if check == "earth_view":
        return 2 * math.degrees(math.asin(math.sin(math.radians(value) / 2) / s0 * s))
    if check == "earth_chord":
        return value / s0 * s
    return 2 * math.degrees(math.asin(s))


def scene(d):
    """Return the driver's closing verdict on its scene."""
    ends = [ln for ln in open(d + "/drive.log", errors="replace") if ln.startswith("=== obs")]
    return "reached" if ends and "SCENE REACHED" in ends[-1] else ("stopped" if ends else "no-verdict")


def refused(d):
    """Tell whether the engine logged the new registry's refusal of a switch during a travel."""
    return REFUSAL in open(d + "/log_spacecrafter.txt", errors="replace").read()


@functools.lru_cache(maxsize=None)
def mw(d, tag):
    """Split the milky way layer (luminance ON - OFF) into its band (15 px median) and points: (total rgb, band, points) energies."""
    a, la = load_png("%s/%s_on.png" % (d, tag))
    b, lb = load_png("%s/%s_off.png" % (d, tag))
    layer = la - lb
    band = median_filter(layer, size=15)
    return float(np.abs((a - b).astype(np.float64)).sum()), float(np.abs(band).sum()), float(np.abs(layer - band).sum())


@functools.lru_cache(maxsize=None)
def places(d, tag):
    """Return the old observer and the Camera relative to Saturn's centre (AU), and the dump's header."""
    h, b = load(d, tag)
    m = np.array(h["helioToEye"]).reshape(4, 4).T
    old = -m[:3, :3].T @ m[:3, 3] - np.array(b["Saturn"]["old"]["ecl"])
    new = np.array(h["camera"]["rootPos"]) - np.array(b["Saturn"]["new"]["ecl"])
    return old, new, h


def turn(u, v):
    """Return the angle (deg) between two places seen from Saturn's centre."""
    return math.degrees(math.acos(max(-1.0, min(1.0, float(np.dot(u, v) / np.linalg.norm(u) / np.linalg.norm(v))))))


@functools.lru_cache(maxsize=None)
def lit(png):
    """Return the lit mask (mean of R, G, B above 30), that mean, and the frame's rgb."""
    rgb, _ = load_png(png)
    m = rgb.astype(np.float64).mean(axis=2)
    return m > 30, m, rgb


def halo(png):
    """Count high-chroma pixels (max - min channel > 80 and max > 80) over the frame."""
    rgb = lit(png)[2]
    mx, mn = rgb.max(axis=2).astype(int), rgb.min(axis=2).astype(int)
    return int((((mx - mn) > 80) & (mx > 80)).sum())


def gdb_light(d):
    """Return |ModularBody::lightPosition| (AU) and the Camera's system name printed between the driver's light markers."""
    g = open(d + "/gdb.log", errors="replace").read()
    i = g.index("F131BEGIN light")
    blk = g[i:g.index("F131END_light", i)]
    v = [float(x) for x in re.search(r"v = \{([^}]*)\}", blk).group(1).split(",")]
    return math.sqrt(sum(x * x for x in v)), re.search(r'"([^"]*)"', blk).group(1)


def capture_corr(dn, do, tag):
    """Correlate two pins' captures: band (15 px median of luminance) and stars (sigma-3 blur of what rises above it)."""
    def split(d):
        _, lum = load_png("%s/%s.png" % (d, tag))
        band = median_filter(lum, size=15)
        return band, gaussian_filter(np.clip(lum - band, 0, None), 3)
    def corr(a, b):
        a, b = a - a.mean(), b - b.mean()
        return float((a * b).mean() / (a.std() * b.std()))
    (ba, sa), (bb, sb) = split(dn), split(do)
    return corr(ba, bb), corr(sa, sb)


def measure(got, name, fn):
    """Store fn()'s observation under name, or the reason it could not be made."""
    try:
        got[name] = fn()
    except FileNotFoundError as e:
        got[name] = Missing("no-record:" + os.path.basename(e.filename or "?"))
    except Exception as e:
        got[name] = Missing("error:%s:%s" % (type(e).__name__, re.sub(r"\s+", "_", str(e))[:60]))


def obs1(d, var, pin):
    """Measure observation 1 (initial_night.sts): the two observers' separation, the stars, the date."""
    got = {}
    late = lambda: load(d, "late")[0]
    last = lambda: late()["seam"]["steps"][-1]
    ctl = lambda k: late()["control"][k]["reported"]
    sunset = lambda: sunset_jd(late()["seam"]["steps"][0]["jd"], ctl("longitude"), ctl("latitude"))
    measure(got, "_carry_jd", lambda: last()["jd"])
    measure(got, "observers_view_apart_late_deg", lambda: last()["viewAngleAbs"])
    measure(got, "observers_apart_late_km", lambda: last()["posDelta"] * AU)
    measure(got, "star_points_late", lambda: star_points(d + "/late.png"))
    measure(got, "stars_in_drawn_buffer_late", lambda: late()["oldView"]["stars"]["nbStarsToDraw"][late()["oldView"]["stars"]["drawIdx"]])
    measure(got, "late_after_sunset_plus_005_d", lambda: late()["jd"] - sunset() - 0.05)
    measure(got, "separation_angle_deg", lambda: 2 * math.degrees(math.asin(
        math.sin(math.radians(last()["viewAngleAbs"]) / 2) / math.cos(math.radians(ctl("latitude"))))))
    measure(got, "latitude_reported_deg", lambda: ctl("latitude"))
    measure(got, "longitude_reported_deg", lambda: ctl("longitude"))
    measure(got, "altitude_reported_m", lambda: ctl("altitude"))
    measure(got, "sky_brightness", lambda: late()["oldView"]["tone"]["skyBrightness"])
    measure(got, "adaptation_luminance", lambda: late()["oldView"]["tone"]["worldAdaptationLuminance"])
    measure(got, "camera_bound_pre", lambda: load(d, "pre")[0]["camera"]["boundToSurface"])
    measure(got, "camera_bound_late", lambda: late()["camera"]["boundToSurface"])
    measure(got, "sunset_frame_minus_port_s", lambda: (sunset_frame(late())["jd"] - sunset()) * 86400)
    return got


def sunset_frame(h):
    """Return the ring frame that ran `date sun set`: the one before the +0.05 d jump of `date relative 0.05`."""
    st = h["seam"]["steps"]
    k = next(i for i in range(len(st) - 1, 0, -1) if abs(st[i]["jd"] - st[i - 1]["jd"] - 0.05) < 1e-3)
    return st[k - 1]


def fly(d, var):
    """Measure what every fly launch shares: the final reference, the refusal, and on a natural launch the race."""
    got = {}
    fin = lambda: load(d, "final")[0]
    measure(got, "camera_reference_final", lambda: fin()["camera"]["reference"])
    measure(got, "old_home_final", lambda: fin()["oldView"]["observer"]["homePlanet"])
    measure(got, "refusal_logged", lambda: refused(d))
    if var in NATURAL:
        measure(got, "race_margin_ms", lambda: race(d)["dt0_plus_S"] - 3000)
        measure(got, "race_forms_agree", lambda: bool(
            race(d)["predicted_accept_ms_form"] == race(d)["predicted_accept_jd_form"]
            == (fin()["camera"]["reference"] == "Saturn") == (not race(d)["log_says_refused"])))
    return got


def obs2(d, var, pin):
    """Measure observation 2: the milky way's band before and after the flight, the drawn observer's orbit."""
    got = fly(d, var)
    drawn = 1 if pin == "new" else 0
    measure(got, "mw_band_pre", lambda: mw(d, "pre_mw")[1])
    measure(got, "mw_band_end_over_pre", lambda: mw(d, "end_mw")[1] / mw(d, "pre_mw")[1])
    measure(got, "mw_total_end_over_pre", lambda: mw(d, "end_mw")[0] / mw(d, "pre_mw")[0])
    measure(got, "orbit_distance_km", lambda: float(np.linalg.norm(places(d, "end")[drawn])) * AU)
    measure(got, "camera_distance_to_saturn_km", lambda: load(d, "final")[1]["Saturn"]["new"]["dist"] * AU)
    return got


def obs3(d, var, pin):
    """Measure observation 3: Saturn's lit area at fov 60 (the rings), the halos, the light the Camera's system left."""
    got = fly(d, var)
    z60, full = d + "/end_z60.png", d + "/end_full.png"
    measure(got, "saturn_lit_px_z60", lambda: int(lit(z60)[0].sum()))
    measure(got, "halo_px_z60", lambda: halo(z60))
    measure(got, "halo_px_full", lambda: halo(full))
    measure(got, "lit_mean_luminance", lambda: float(lit(z60)[1][lit(z60)[0]].mean()))
    measure(got, "camera_system", lambda: gdb_light(d)[1])
    measure(got, "light_position_au", lambda: gdb_light(d)[0])
    return got


@functools.lru_cache(maxsize=None)
def key_frames(d, key):
    """Count the frames the key drive held <key> for, as its gdb log printed them."""
    n = len(re.findall(r"F131KEY %s call \d+ old=" % key, open(d + "/gdb.log", errors="replace").read()))
    if not n:
        raise ValueError("no F131KEY %s frame in gdb.log" % key)
    return n


def step_per_frame(d, axis, key, a, b, engine):
    """Return one engine's own longitude or latitude change (deg) between dumps a and b, per frame <key> was held."""
    own = lambda tag: load(d, tag)[0]["control"]["longitude" if axis == "lon" else "latitude"][engine]
    return ((own(b) - own(a) + 180.0) % 360.0 - 180.0) / key_frames(d, key)


def old_altitude_km(d, tag):
    """Return the old observer's altitude (km) in dump <tag>."""
    return load(d, tag)[0]["control"]["altitude"]["old"] / 1000.0


def log_lines_between(d, first, last):
    """Count the engine log lines written strictly between the first line naming <first> and the next naming <last>."""
    ln = open(d + "/log_spacecrafter.txt", errors="replace").read().splitlines()
    i = next(k for k, s in enumerate(ln) if first in s)
    return next(k for k in range(i + 1, len(ln)) if last in ln[k]) - i - 1


def obs4(d, var, pin):
    """Measure observation 4: how far each observer turns about Saturn under the lat / lon keys, and each engine's own step per held frame."""
    got = fly(d, var)
    drawn = 1 if pin == "new" else 0
    other = 1 - drawn
    p = lambda tag, k: places(d, tag)[k]
    measure(got, "turn_lon_deg", lambda: turn(p("before", drawn), p("after_lon", drawn)))
    measure(got, "turn_lat_deg", lambda: turn(p("after_lon", drawn), p("after_lat", drawn)))
    measure(got, "other_turn_lon_deg", lambda: turn(p("before", other), p("after_lon", other)))
    measure(got, "other_turn_lat_deg", lambda: turn(p("after_lon", other), p("after_lat", other)))
    measure(got, "other_distance_after_km", lambda: float(np.linalg.norm(p("after_lat", other))) * AU)
    measure(got, "reported_altitude_before_km", lambda: places(d, "before")[2]["control"]["altitude"]["reported"] / 1000.0)
    engine = ("old", "new")
    for axis, key, a, b in (("lon", "KP_6", "before", "after_lon"), ("lat", "KP_8", "after_lon", "after_lat")):
        for who, k in (("", drawn), ("other_", other)):
            measure(got, "%s%s_per_call_deg" % (who, axis),
                    lambda axis=axis, key=key, a=a, b=b, k=k: step_per_frame(d, axis, key, a, b, engine[k]))
    measure(got, "old_altitude_held_km", lambda: max(abs(old_altitude_km(d, t) - old_altitude_km(d, "before")) for t in ("after_lon", "after_lat")))
    measure(got, "old_altitude_after_km", lambda: old_altitude_km(d, "after_lat"))
    measure(got, "log_lines_during_lon_drive", lambda: log_lines_between(d, "before.json", "after_lon.png"))
    return got


def obs5(d, var, pin):
    """Measure observation 5: per wheel window the two fovs' separation, their half-path lag, the rest."""
    got = {}
    win = functools.lru_cache(maxsize=None)(lambda: wheel(d, "final"))
    measure(got, "wheel_windows", lambda: len(win()))
    for i in range(1, 10):
        measure(got, "max_abs_delta_w%d_deg" % i, lambda i=i: win()[i - 1]["max_abs_delta_deg"])
        measure(got, "lag_w%d_ms" % i, lambda i=i: win()[i - 1]["lag_stars_minus_bodies_ms"])
        measure(got, "rest_delta_w%d_deg" % i, lambda i=i: win()[i - 1]["rest_30f_max_delta"])
    measure(got, "frame_dt_median_ms", lambda: float(np.median([s["dt"] for s in load(d, "final")[0]["seam"]["steps"]])))
    return got


def runs(profile, thr=60.0, step=0.25, maxlen=8.0):
    """Return (centre px, weight) of each run of profile above thr no longer than maxlen px: a ray's crossings with thin curves."""
    above, out, i = profile > thr, [], 0
    while i < len(profile):
        if not above[i]:
            i += 1
            continue
        j = i
        while j < len(profile) and above[j]:
            j += 1
        w = profile[i:j] - thr
        if (j - i) * step <= maxlen:
            out.append((float((np.arange(i, j) * w).sum() / w.sum()) * step, float(w.sum() * step)))
        i = j
    return out


def progression(ts):
    """Fit t = a + d k to crossings that may skip members: the largest spacing keeping the most crossings within 2 px."""
    ts, best = np.array(sorted(ts)), None
    for i in range(len(ts)):
        for j in range(i + 1, len(ts)):
            if ts[j] - ts[i] < 10:
                continue
            k = np.round((ts - ts[i]) / (ts[j] - ts[i]))
            (a, d), *_ = np.linalg.lstsq(np.vstack([np.ones_like(k), k]).T, ts, rcond=None)
            key = (int((np.abs(ts - a - d * k) < 2.0).sum()), d)
            if best is None or key > best[0]:
                best = (key, float(a), float(d), [int(x) for x in k])
    return best


def view_axis_decs(d):
    """Return the declinations (deg) of the two engines' view axes in the scene's first dump: {navigator, camera}."""
    h = load(d, "pre")[0]
    ev = np.array(h["oldView"]["nav"]["equVision"])
    m = np.array(h["camera"]["mat"]).reshape(4, 4).T
    f = m[:3, :3].T @ np.array([0.0, 0.0, -1.0])
    return {"navigator": math.degrees(math.asin(ev[2] / np.linalg.norm(ev))),
            "camera": math.degrees(math.asin(f[2] / np.linalg.norm(f)))}


@functools.lru_cache(maxsize=None)
def line_over_grid(d, tag):
    """Return the equator line's distance from the screen centre over the grid's equator ring's, read along the hour circle
    through the line's culmination (the fisheye's radius is linear in the angle there, so the grid's parallels, 10 deg apart,
    cross it as an arithmetic progression). The ring is the member at the drawn frame's equator distance |dec of the view
    axis|; which engine's frame drew the grid is read from the progression's phase. The ring is placed whether drawn or
    hidden under a coinciding line (the line draws after the grid)."""
    rgb, _ = load_png("%s/%s.png" % (d, tag))
    rgb = rgb.astype(np.float64)
    red = rgb[..., 0] - np.maximum(rgb[..., 1], rgb[..., 2])
    blue = rgb[..., 2] - np.maximum(rgb[..., 0], rgb[..., 1])
    n = red.shape[0]
    c = (n - 1) / 2.0
    ys, xs = np.nonzero(red > 60)
    near = np.argsort(np.hypot(xs - c, ys - c))[:20]
    u = np.array([xs[near].mean() - c, ys[near].mean() - c])
    u /= np.linalg.norm(u)
    t = np.arange(0, n / 2.0, 0.25)
    def profile(img):
        best = np.full(t.shape, -1e9)
        for off in (-2, -1, 0, 1, 2):
            x, y = c + t * u[0] - off * u[1], c + t * u[1] + off * u[0]
            best = np.maximum(best, map_coordinates(img, [y, x], order=1, mode="constant", cval=0))
        return best
    reds, blues = runs(profile(red)), runs(profile(blue))
    line = max(reds, key=lambda r: r[1])[0]
    (inliers, _), a, dd, ks = progression([b[0] for b in blues])
    phase = (10.0 * a / dd) % 10.0
    match = {}
    for engine, dec in view_axis_decs(d).items():
        miss = (abs(dec) - phase + 5.0) % 10.0 - 5.0
        match[engine] = (abs(miss), abs(dec), dec)
    engine = min(match, key=lambda e: match[e][0])
    k_ring = int(round((match[engine][1] * dd / 10.0 - a) / dd))
    ring = a + dd * k_ring
    rec = {"ray": u.tolist(), "red_runs": reds, "blue_runs": blues, "spacing_px": dd, "offset_px": a, "members": ks,
           "inliers": inliers, "phase_deg": phase, "frames": {e: {"axis_dec_deg": v[2], "phase_miss_deg": v[0]}
           for e, v in match.items()}, "grid_frame": engine, "ring_member": k_ring, "ring_drawn": k_ring in ks,
           "line_px": line, "ring_px": ring, "ratio": line / ring}
    json.dump(rec, open("%s/o6_%s.json" % (d, tag), "w"), indent=1)
    if match[engine][0] > 1.0 or inliers < 2:
        raise ValueError("no ring placed: phase %.2f deg matches no engine's axis (%s), %d members" % (
            phase, ", ".join("%s %.2f" % (e, v[1]) for e, v in match.items()), inliers))
    return line / ring


def obs6(d, var, pin):
    """Measure observation 6: the equator line against the grid's equator, mid-zoom and at rest."""
    got = {}
    for tag in ("cap_mid", "cap_rest60", "cap_rest180"):
        measure(got, "line_over_grid_" + tag, lambda tag=tag: line_over_grid(d, tag))
    return got


def across(obs, dn, do):
    """Measure the criteria that compare the two pins' records of one leg."""
    got = {}
    if obs == "3":
        measure(got, "lit_px_new_over_old", lambda: float(lit(dn + "/end_z60.png")[0].sum()) / float(lit(do + "/end_z60.png")[0].sum()))
    if obs == "5":
        for tag in ("cap_mid", "cap_rest180", "cap_rest60"):
            measure(got, "band_corr_" + tag, lambda tag=tag: capture_corr(dn, do, tag)[0])
            measure(got, "stars_corr_" + tag, lambda tag=tag: capture_corr(dn, do, tag)[1])
    return got


def bank():
    """Return the banked rows keyed by (obs, var, pin), in table order."""
    rows = {}
    for ln in BANK.splitlines():
        f = ln.split()
        if not f or f[0].startswith("#"):
            continue
        obs, var, pin, name, cls, check, exp, tol = f
        for p in (("new", "old") if pin == "both" else (pin,)):
            rows.setdefault((obs, var, p), []).append((name, cls, check, exp, tol))
    return rows


def fmt(v):
    """Render an observation for a verdict line."""
    if isinstance(v, (bool, np.bool_)):
        return "true" if v else "false"
    if isinstance(v, (float, np.floating)):
        return "%.7g" % v
    return str(v)


def verdicts(obs, var, pin, got):
    """Print one line per banked row of (obs, var, pin) and return the number of gate FAILs."""
    label = obs + VARIANTS[var]
    jd, count = got.get("_carry_jd"), {"PASS": 0, "FAIL": 0, "NOTE": 0}
    for name, cls, check, exp, tol in bank().get((obs, var, pin), []):
        o, shown = got.get(name, Missing("unmeasured")), exp
        if isinstance(o, Missing) or o is None:
            ok = False
        elif check == "eq":
            ok = fmt(o) == exp
        elif check == "rel":
            ok = abs(float(o) - float(exp)) <= float(tol.rstrip("%")) / 100.0 * abs(float(exp))
        elif check.startswith("earth") and isinstance(jd, Missing):
            o, ok = Missing("no-jd:" + jd), False
        else:
            e = carried(check, float(exp), jd) if check.startswith("earth") else float(exp)
            shown, ok = fmt(e), abs(float(o) - e) <= float(tol)
        word = "NOTE" if cls == "note" else ("PASS" if ok else "FAIL")
        count[word] += 1
        print("%s %s %s %s observed=%s expected=%s tol=%s%s" % (word, label, pin, name, fmt(o), shown, tol,
              (" in_tol=" + ("yes" if ok else "no")) if cls == "note" else ""), flush=True)
    print("=== %s %s: %d PASS, %d FAIL, %d NOTE ===" % (label, pin, count["PASS"], count["FAIL"], count["NOTE"]), flush=True)
    return count["FAIL"]


def main(argv):
    obs_ok = lambda a: a[1] in "123456" and len(a[1]) == 1 and a[2] in VARIANTS and (a[2] in ("nat", "ref") or a[1] == "4")
    if len(argv) == 5 and argv[0] == "judge" and obs_ok(argv) and argv[3] in ("new", "old"):
        got = {"1": obs1, "2": obs2, "3": obs3, "4": obs4, "5": obs5, "6": obs6}[argv[1]](argv[4], argv[2], argv[3])
        measure(got, "scene_reached", lambda: scene(argv[4]))
        return 1 if verdicts(argv[1], argv[2], argv[3], got) else 0
    if len(argv) == 5 and argv[0] == "cross" and obs_ok(argv):
        return 1 if verdicts(argv[1], argv[2], "x", across(argv[1], argv[3], argv[4])) else 0
    if len(argv) == 2 and argv[0] == "table":
        wheel(argv[1], "final", argv[1] + "/fov_table.txt")
        return 0
    if len(argv) == 3 and argv[0] == "state":
        print(json.dumps(state(argv[1], argv[2])[0], indent=1))
        return 0
    if len(argv) in (3, 4, 5) and argv[0] == "ring":
        st = ring(load(argv[1], argv[2])[0])
        a = int(argv[3]) if len(argv) > 3 else 0
        b = int(argv[4]) if len(argv) > 4 else st[-1]["frame"]
        for x in st:
            if a <= x["frame"] <= b:
                print(json.dumps(x))
        return 0
    sys.stdout.write("".join(open(__file__).readlines()[1:8]))
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
