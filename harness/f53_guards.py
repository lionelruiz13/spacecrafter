#!/usr/bin/env python3
"""F53 / INTENT §5.113 — the three shipped reaches of the unguarded
`ObjectUninitialized` singleton, MEASURED in one launch.

    cd claude/harness && export XAUTHORITY=$(ls /tmp/rt-claude/.mutter-Xwaylandauth.*) \
        && DISPLAY=:2 ./f53_guards.py <absOutdir> [--bin <binary>]

WHY A NEW SCRIPT AND NOT AN EXTENSION OF `f50_selvars.py`
---------------------------------------------------------
§5.113's owed clause names `f50_selvars.py`'s five-leg driver as the vehicle.
That driver is hard-wired to §5.110's question: its legs (O1 D1 C D2 O2), its
fixture (b24_select's composed body), its seven variables and all six of its
gates are statements about a COMPOSED-BODY selection.  §5.113 fires with
NOTHING selected and needs no composed body at all — sharing the leg plan would
add a variable the defect does not involve, and rewriting the leg plan in place
would break the "`f50_selvars.py` stays runnable as delivered" requirement.
What IS shared is shared by IMPORT, not by copy (I2): `f27_reply.Session` owns
the temp-HOME farm, the /proc/<pid>/comm concurrent-instance assert, the frozen
config/ssystem md5 pair and the exit gate; `f27_reply.Client` owns the socket;
`f32_object_leak.obj_info` owns the `get status object` witness;
`dumpread.sanitize_nonfinite` owns the dump's NaN grammar.  The composed
fixture (`f32.make_prepare`) is deliberately NOT used.

THE THREE REACHES, AND THE CHANNEL EACH IS MEASURED THROUGH
------------------------------------------------------------
A. `set home_planet selected` with nothing selected
   [app_command_interface.cpp:2164-2165] -> `Core::setHomePlanet`, whose
   `if (planet=="selected") result = ssystemFactory->switchToAnchor(selected_object);`
   has no truthiness test [core.cpp:2160-2164] -> `SSystemFactory::switchToAnchor
   (const Object&)` [ssystem_factory.hpp:999-1003] -> `AnchorManager::switchToAnchor
   (const Object&)` [anchor_manager.cpp:377-385]: the by-name branch is asked
   `getEnglishName()`, the singleton answers `{}`, `anchors.find("")` misses, and
   :381 builds an `AnchorPointObservatory` at
   `navigator->earthPosEquToHelio(selection.getEarthEquPos(navigator))` — i.e. at
   `earthPosEquToHelio((1,0,0))`, a point ONE AU from the observer along the
   equatorial x axis — then :382 registers it under the EMPTY NAME.

   CHANNEL: the dual dump's `oldView.observer` block, which is
   `Observer::dumpTrace` [observer.cpp:481-493] and carries `homePlanet`
   (= `getHomeBody() ? name : ""` [observer.cpp:323-326]), `onBody`
   (= `anchor->isOnBody()` [observer.cpp:121-123]) and `distanceFromCenter`
   (= `anchor->getBody()->getRadius() + altitude/(1000*AU)` on a body, and
   `altitude/(1000*AU)` ALONE off one [observer.cpp:81-91]).  Second channel:
   `print <k> selected_distance` after `select planet <X>` — F50's channel,
   already shown able to answer real values.  Third channel: `get status
   position`, which the row's owed clause names.

   THE LATCH IS PART OF THE PREDICTION.  A second `set home_planet selected`
   enters `switchToAnchor("")` first [anchor_manager.cpp:357-375]; if :382 really
   registered under `""`, `anchors.find("")` now HITS, the cached anchor is
   re-installed and the observer does NOT move again.  A rebuild would displace
   it a further AU.  So: the printed distances after the second invocation must
   be BYTE-IDENTICAL to those after the first.  That is the discriminating check.

B. `flag object_coordinates on` with nothing selected.  `SkyDisplayMgr::draw` is
   handed `core->selected_object.getEarthEquPos(nav)` unconditionally, every
   frame [executorModule/solarSystemModule.cpp:210, stellarSystemModule.cpp:205]
   — NB both files live under `executorModule/`, not `experimentalModule/`, and
   the call is NOT behind `experimental_path`.  `SkyCoords::draw` gates on its
   fader alone [skyDisplay.cpp:419-422] and prints four labels: `alt:`, `az :`,
   `ra :`, `dec:` [skyDisplay.cpp:463-579].  `rectToSphe` of `(1,0,0)` is
   (RA 0, DEC 0) exactly, so `ra` and `dec` are CONSTANTS while `alt`/`az` come
   from `nav->earthEquToLocal((1,0,0))` [skyDisplay.cpp:437] and therefore move
   with the clock.

   CHANNEL: the screen.  The scene is stripped to black (every sky flag off) so
   the labels are the only lit pixels, and a labels-OFF control frame is taken at
   every step so the isolation is measured rather than assumed.  Projection is
   the shipped FISHEYE at `init_fov = 180` [~/.spacecrafter/config.ini:39,257],
   so the whole visible hemisphere is on screen and the vernal point needs no
   aiming; the sim clock is stepped through a full day so the point rises and
   sets inside the run.  POSITIVE CONTROL, same channel, same geometry: a real
   star selected, whose labels must read the star's own RA/DEC — which the app
   reports independently through `selected_star_ra`/`selected_star_de`
   [app_command_eval.cpp:317-320].  That control also validates the READING of
   the crops, which is otherwise an unchecked human channel.

C. `illuminate hp 0 display on size 5` [app_command_interface.cpp:1929-1937 ->
   coreLink.cpp:399-401 -> illuminate_mgr.cpp:74-83 -> :126].  `IlluminateMgr::
   load`'s guard is `num > NR_OF_HIP` [:128] while `HipStarMgr::searchHP`
   requires `0 < _HP` [hip_star_mgr.cpp:1054-1055] and otherwise returns a null
   `ObjectBaseP` [:1064].  `Object(nullptr)` installs the singleton
   [object.cpp:97-101]; `double ra, de;` at :143 is NOT initialised;
   `getRaDeValue` is an empty body [object.cpp:40-42], so the two doubles handed
   to `loadIlluminate` [:166] and thence into the spatial grid [:170-176] are
   whatever the stack held.

   NO VALUE IS PREDICTED, and that is not modesty: indeterminate memory has no
   value a source reading can name, so any predicted number would be
   unfalsifiable dressing.  What IS predicted is the mechanism's observable
   shell — accepted, silent, an Illuminate inserted — plus a NAME-SCOPED
   identification that only an inserted hp-0 illuminate can pass:
   `illuminate hp <V> display off` removes the illuminate named V and only that
   one [illuminate_mgr.cpp:180-183], and `illuminate hp 0 display off` then
   removes the one named 0.  If a blob survives the first removal and vanishes
   on the second, an illuminate named 0 was there — positively identified, not
   inferred.

   GEOMETRY, and it is why the field of view is narrowed for this check: at
   `size 5` (arc-minutes) the quad is ~0.08 deg across, which at the fisheye's
   180 deg over 1332 px is SUB-PIXEL.  Leg C therefore runs at fov 2 with the
   star centred (~55 px), where a blob is unambiguous.  The grid's own view
   filter [illuminate_mgr.cpp:201] then also restricts what can be seen to a
   ~3.7 deg cone around V, which is exactly the discrimination wanted: a blob in
   frame means hp 0 landed AT V's coordinates.  A wide-sky leg at fov 180 with
   `size 600` follows, so the "it went somewhere else" branch has a channel too.

   POSITIVE CONTROL: `illuminate hp <V> display on size 5` for a V the installed
   catalogue answers for, through the SAME channel (the screen), with the star
   centred by `flag track_object on` so no external astronomy is needed.  The
   candidate ids are not recalled: they are F32's own measured answers
   [artifacts/f32/f32_hip.json], and the run RE-VERIFIES each against the app
   before using it (§11.51(d): a cached conclusion is re-checked, never trusted).

PRECONDITIONS
  - temp-HOME farm, /proc/<pid>/comm assert, frozen md5 in == out, exit gate:
    all `f27_reply.Session`'s.
  - NO `moveto` and no display-scale command is issued, so the §5.109 settle
    hazard is not exercised.  What IS exercised is the anchor switch, which arms
    a navigator transition [navigator.cpp:541-545], so every switch is followed
    by a settle asserted BY MEASUREMENT on the dump's own `nav.plans`, never by
    a bare sleep.
  - record-only: no product code, no data writes, the real ~/.spacecrafter is
    never written (the farm symlinks it; config.ini and ssystem.ini are copies).
"""

import argparse, json, os, re, sys, time
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
import f27_reply as f27
import f32_object_leak as f32
import dumpread

HERE = Path(__file__).resolve().parent
DEFAULT_BIN = str(HERE.parents[1] / "build-claude/src/spacecrafter")

JD = 2461233.5                     # the epoch §11.158 / F50 used
BODIES = ["Earth", "Mars", "Sun"]  # old-tree bodies, the distance probes
# F32's MEASURED answers, not recall [artifacts/f32/f32_hip.json]; re-verified
# against the app before use.
HIP_CANDIDATES = [3, 40001, 40010]

SIX = ["selected_ra", "selected_de", "selected_az", "selected_alt",
       "selected_distance", "selected_magnitude"]
VARS = SIX + ["selected_star_ra", "selected_star_de", "body_selected"]

# Everything that can put a lit pixel on the screen.  Every name here is in
# `base_command_interface.hpp`'s ACP_FN_ list, checked before the run - an
# unknown flag name would be REFUSED and trip the refusal gate.  The labels-off
# control frame at every step means the list does not have to be COMPLETE for
# the measurement to hold.
DARK_FLAGS = ["stars", "star_names", "star_twinkle", "star_lines",
              "star_lines_selected", "stars_trace", "planets", "planet_names",
              "planet_orbits", "orbits", "planets_orbits", "satellites_orbits",
              "planets_axis", "satellites", "nebulae", "nebula_names",
              "nebula_hints", "bright_nebulae", "dso_pictograms", "milky_way",
              "zodiacal_light", "oort", "tully", "landscape", "atmosphere",
              "fog", "clouds", "constellation_drawing", "constellation_art",
              "constellation_boundaries", "constellation_names",
              "azimuthal_grid", "equatorial_grid", "ecliptic_grid",
              "galactic_grid", "equator_line", "ecliptic_line", "galactic_line",
              "meridian_line", "zenith_line", "vertical_line", "greenwich_line",
              "tropic_lines", "circumpolar_circle", "precession_circle",
              "polar_circle", "polar_point", "ecliptic_center", "galactic_pole",
              "galactic_center", "vernal_points", "aries_line", "analemma",
              "analemma_line", "zodiac", "cardinal_points", "object_trails",
              "body_trace", "personal", "personeq", "nautical_alt",
              "nautical_ra", "mouse_coordinates", "angular_distance",
              "loxodromy", "orthodromy", "show_latlon", "show_tui_datetime",
              "show_tui_short_obj_info", "subtitle"]

B_STEPS = 12                       # sim-time steps across one day
FAILS, NOTES = [], []


def fail(m):
    FAILS.append(m)
    print(f"FAIL: {m}", flush=True)


def ok(m):
    print(f"ok:   {m}", flush=True)


def note(m):
    NOTES.append(m)
    print(f"      {m}", flush=True)


# ---------------------------------------------------------------- predictions
# Committed to git BEFORE the launch.  Every entry names the line it is derived
# from, so a mismatch is traceable rather than absorbable.
PRED = {
    "code": "master-beta @ d6aec251",
    "A": {
        "accepted": "`set home_planet selected` with nothing selected is "
                    "ACCEPTED - zero new 'Could not execute' lines; there is no "
                    "test anywhere on the path [core.cpp:2160-2164]",
        "diagnostic": "exactly ONE new log line is attributable to the command: "
                      "SSystemFactory::syncCameraReference's WARNING \"New path "
                      "has no body '' to re-reference the camera on\" "
                      "[ssystem_factory.cpp:1084], which names the EMPTY NAME "
                      "and not the absent selection. NO log line says nothing is "
                      "selected, names the fictitious anchor, or states a "
                      "consequence or a user action - i.e. nothing on this path "
                      "is §2(f)-grade.",
        "onBody": "oldView.observer.onBody flips true -> FALSE "
                  "[observer.cpp:121-123; AnchorPointObservatory is not a body]",
        "homePlanet": "oldView.observer.homePlanet becomes the EMPTY STRING "
                      "[observer.cpp:323-326, getHomeBody() null off a body]",
        "distanceFromCenter": "drops from ~4.26e-5 AU (one Earth radius) to "
                              "< 1e-8 AU, because the body-radius term "
                              "disappears and only altitude/(1000*AU) is left "
                              "[observer.cpp:81-91]; config altitude is 75 m "
                              "=> ~5.0e-10 AU",
        "latlonalt_unchanged": "oldView.observer longitude/latitude/altitude are "
                               "UNCHANGED to the last digit - "
                               "Observer::setAnchorPoint's non-eye-relative "
                               "branch only swaps the anchor pointer "
                               "[observer.cpp:445-459]",
        "get_status_position": "lat/lon/alt in `get status position` are "
                               "UNCHANGED by the teleport (same getters, "
                               "coreLink.cpp:625-628) => THE CHANNEL THE ROW'S "
                               "OWED CLAUSE NAMES IS BLIND TO A ONE-AU "
                               "DISPLACEMENT. Stated as a prediction so a match "
                               "is evidence and a mismatch is a finding.",
        "distance_Earth": "after the teleport, `select planet Earth` -> "
                          "selected_distance = 1 AU to within one Earth radius: "
                          "the anchor is at observer + 1 AU exactly "
                          "[anchor_manager.cpp:381] and the observer was 4.26e-5 "
                          "AU from Earth's centre, so the value lies in "
                          "[0.99995, 1.00005]. BAND GATE.",
        "distance_others_move": "the Mars and Sun distances CHANGE (bounded "
                                "above by 1 AU, the triangle inequality on a "
                                "1 AU displacement). No exact value predicted; "
                                "no change on any of the three would refute the "
                                "displacement.",
        "latch": "the SECOND `set home_planet selected`, still with nothing "
                 "selected, finds the anchor CACHED under \"\" "
                 "[anchor_manager.cpp:364-371 + :382] and re-installs it, so the "
                 "observer does NOT move again: all three printed distances and "
                 "the whole oldView.observer block are BYTE-IDENTICAL to the "
                 "first invocation's. A rebuild would move the observer a "
                 "further AU and change them. DISCRIMINATING CHECK.",
        "restore": "`set home_planet Earth` restores homePlanet=='Earth', "
                   "onBody==true and distanceFromCenter to its pre value - the "
                   "positive control that the anchor machinery itself works",
    },
    "B": {
        "control_dark": "with every DARK_FLAGS flag off and object_coordinates "
                        "OFF, the frame carries < 5000 lit pixels (max channel "
                        "> 8). If it does not, the labels-off control frame is "
                        "differenced instead and the claim is unaffected.",
        "positive": "with a real star selected, four labels are drawn and their "
                    "ra/dec read the STAR's own RA/DEC, which the app reports "
                    "independently as selected_star_ra / selected_star_de "
                    "[app_command_eval.cpp:317-320]. This is the control for the "
                    "channel AND for the reading of the crops.",
        "drawn_with_nothing_selected": "with NOTHING selected the labels are "
                                       "STILL drawn in at least one step - "
                                       "SkyCoords::draw gates on its fader alone "
                                       "[skyDisplay.cpp:421-422]",
        "ra_dec_constant": "those labels read EXACTLY `ra :00h00m` and "
                           "`dec:+00°00'` - rectToSphe((1,0,0)) is (0,0) and the "
                           "formatting is truncf with a leading zero "
                           "[skyDisplay.cpp:429-435, :535-544, :565-579]",
        "alt_az_move": "the same labels' `alt:`/`az :` DIFFER between sim-time "
                       "steps, and the label block's screen centroid MOVES by "
                       "> 20 px - earthEquToLocal((1,0,0)) turns with the clock "
                       "[skyDisplay.cpp:437]. A static overlay would give an "
                       "immobile centroid and identical text.",
        "off": "`flag object_coordinates off` removes them",
    },
    "C": {
        "no_value_predicted": "the ra/de handed to loadIlluminate are "
                              "INDETERMINATE by construction "
                              "[illuminate_mgr.cpp:143 `double ra, de;` + "
                              "object.cpp:40-42 empty getRaDeValue]. No value is "
                              "predicted; a predicted value here would be "
                              "unfalsifiable dressing.",
        "accepted": "`illuminate hp 0 display on size 5` is ACCEPTED - zero new "
                    "'Could not execute' lines",
        "no_diagnostic": "ZERO new log lines name HIP 0, an absent catalogue id, "
                         "or an uninitialised value - the guard mismatch "
                         "(`num > NR_OF_HIP` [illuminate_mgr.cpp:128] vs "
                         "`0 < _HP` [hip_star_mgr.cpp:1054]) is silent",
        "positive": "`illuminate hp <V> display on size 5` for a catalogue-"
                    "answering V produces a visible blob (> 500 px changed by "
                    "> 8) at the centred star's position, at fov 2 - the same "
                    "channel answering a real value",
        "size1_control": "`illuminate hp 0 display on size 1` inserts NOTHING - "
                         "load returns at :136-139 BEFORE the indeterminate read "
                         "- so 'the command ran' and 'an illuminate was "
                         "inserted' are separated by an in-run control",
        "branch_b1": "STALE-STACK: after `illuminate hp <V> display off` a blob "
                     "REMAINS at V's position, and `illuminate hp 0 display off` "
                     "then REMOVES it => an illuminate named 0 was inserted at "
                     "V's coordinates, i.e. the indeterminate slots held the "
                     "values V's own call had just written. Positively "
                     "identified by name, not inferred.",
        "branch_b2": "ELSEWHERE: no blob remains at V => hp 0's illuminate is "
                     "outside the fov-2 cone. The wide-sky leg (fov 180, "
                     "size 600) then either finds it above the horizon or does "
                     "not, and if neither leg finds it the insertion is NOT "
                     "witnessed and the report must say so rather than assert "
                     "it.",
        "branch_b3": "CRASH: if the residue happens to be non-finite, "
                     "`spheToRect` gives a NaN XYZ, and `SphereGrid::getNearest` "
                     "leaves `best` nullptr (every `>= bestDot` against NaN is "
                     "false) and dereferences it on the next subdivision level "
                     "[SphereGrid.hpp:369-388] => the shipped command KILLS THE "
                     "APP. Named as a live branch, not as the expected one.",
        "history": "hp 0 issued EARLY (before any valid load) and LATE (after "
                   "one) - a DIFFERENCE between them is positive evidence of "
                   "stack residue; sameness is consistent with residue but does "
                   "not demonstrate it, and will be reported as such.",
        "colour_secondary": "the no-colour overload reads getRGB() "
                            "[illuminate_mgr.cpp:78], white (1,1,1) for the "
                            "singleton [object_base.hpp:94-96] against a real "
                            "star's own colour. Secondary observation, not a "
                            "gate - illuminate_mgr.cpp:175 passes (r,b,g) into a "
                            "ctor declared (r,g,b) [illuminate.cpp:33], so the "
                            "drawn colour is channel-swapped either way.",
    },
    "scope3_read_only": "searchAround CANNOT push a null ObjectBaseP into "
                        "cleverFind's candidate vector - answered by reading "
                        "before the run, five producers, no launch involved.",
}


# ---------------------------------------------------------------- the channels
LINE_RE = re.compile(r"\[(f53leg|f53v_[a-z_]+)\]\s*(\S.*?)\s*$")


def script_log_text(sess):
    """The SCRIPT log ONLY - `commandPrint` writes the same string twice
    [app_command_interface.cpp:1979-1980], so a reader that concatenates the log
    directory doubles every reading (F50's recorded hazard)."""
    logdir = sess.farm / ".spacecrafter" / "log"
    hits = sorted(logdir.glob("script*.log"))
    if len(hits) != 1:
        fail(f"expected exactly one script log in {logdir}, found {[h.name for h in hits]}")
    return "".join(h.read_text(errors="replace") for h in hits)


def parse_legs(text):
    legs, cur = {}, None
    for line in text.splitlines():
        m = LINE_RE.search(line)
        if not m:
            continue
        key, val = m.group(1), m.group(2)
        if key == "f53leg":
            cur = val
            legs.setdefault(cur, {})
        elif cur is not None:
            legs[cur][key[len("f53v_"):]] = val
    return legs


def read_vars(c, leg):
    c.send(f"print f53leg {leg}", 0.30)
    for v in VARS:
        c.send(f"print f53v_{v} {v}", 0.30)
    time.sleep(0.5)


def dump(sess, c, tag):
    """The dual dump's header line, NaN-tolerantly parsed.  NB the harness's own
    grammar authority is `dumpread.sanitize_nonfinite`; `f27_reply.dump` still
    calls `b24_equivalence._NONFINITE`, which no longer exists - recorded, not
    fixed here."""
    p = sess.outdir / f"f53_{tag}.json"
    p.unlink(missing_ok=True)
    c.send(f"body action dual_dump filename {p}", 2.2)
    for _ in range(20):
        if p.exists() and p.stat().st_size > 0:
            break
        time.sleep(0.3)
    if not p.exists():
        fail(f"{tag}: dual_dump wrote nothing")
        return None
    try:
        return json.loads(dumpread.sanitize_nonfinite(
            open(p, errors="replace").readline()))
    except json.JSONDecodeError as e:
        fail(f"{tag}: dump header is not JSON ({e})")
        return None


def obs_block(hdr):
    return ((hdr or {}).get("oldView") or {}).get("observer") or {}


def settled(hdr):
    plans = (((hdr or {}).get("oldView") or {}).get("nav") or {}).get("plans") or {}
    return (not plans.get("flagAutoMove")) and (not plans.get("flagChangeHeading"))


def getpos(c):
    c.read(0.3)
    c.send("get status position", 0)
    rep, _lat, _raw = c.poll_for_reply(6.0)
    return rep


def shot(sess, c, name):
    p = sess.outdir / f"{name}.png"
    p.unlink(missing_ok=True)
    c.send(f"body action screenshot filename {p}", 2.2)
    for _ in range(15):
        if p.exists() and p.stat().st_size > 0:
            break
        time.sleep(0.3)
    return p if p.exists() else None


def lit(png, thr=8):
    a = np.asarray(Image.open(png).convert("RGB"))
    return (a.max(axis=2) > thr)


def diffcount(p, q):
    if not (p and q):
        return None
    return int((lit(p) ^ lit(q)).sum())


def bbox_centroid(mask):
    ys, xs = np.nonzero(mask)
    if len(xs) == 0:
        return None
    return {"n": int(len(xs)), "cx": float(xs.mean()), "cy": float(ys.mean()),
            "x0": int(xs.min()), "x1": int(xs.max()),
            "y0": int(ys.min()), "y1": int(ys.max())}


def crop(png, box, out, pad=26, scale=4):
    """Save an upscaled crop so the label TEXT is readable and the reading is
    checkable by anyone who opens the artifact."""
    im = Image.open(png).convert("RGB")
    x0 = max(0, box["x0"] - pad)
    y0 = max(0, box["y0"] - pad)
    x1 = min(im.width, box["x1"] + pad)
    y1 = min(im.height, box["y1"] + pad)
    c = im.crop((x0, y0, x1, y1))
    c = c.resize((max(1, c.width * scale), max(1, c.height * scale)),
                 Image.LANCZOS)
    c.save(out)
    return out


def sky_dark(c):
    for f in DARK_FLAGS:
        c.send(f"flag {f} off", 0.12)


def alive(sess, where):
    if sess.proc.poll() is not None:
        fail(f"the app DIED at {where} (rc={sess.proc.returncode}) - "
             f"prediction C/branch_b3's shape; everything after this point is "
             f"not measured")
        return False
    return True


# --------------------------------------------------------------------- checks
def check_A(sess, c, res):
    """The observer teleport, its diagnostic, and the empty-name latch."""
    A = res["A"] = {"legs": []}

    def sample(tag, do_settle=False):
        if do_settle:
            for _ in range(12):
                h = dump(sess, c, f"A_{tag}_settle")
                if settled(h):
                    break
                time.sleep(1.0)
        h = dump(sess, c, f"A_{tag}")
        rec = {"tag": tag, "observer": obs_block(h), "settled": settled(h),
               "pos": getpos(c), "dist": {}}
        for b in BODIES:
            c.send(f"select planet {b} pointer off", 1.0)
            read_vars(c, f"a_{tag}_{b.lower()}")
        c.send("deselect", 0.6)
        rec["objinfo_after_deselect"] = f32.first_line(f32.obj_info(c))
        A["legs"].append(rec)
        print(f"  A/{tag}: homePlanet={rec['observer'].get('homePlanet')!r} "
              f"onBody={rec['observer'].get('onBody')} "
              f"dfc={rec['observer'].get('distanceFromCenter')}", flush=True)
        return rec

    c.send("deselect", 0.8)
    sample("pre")

    for n, tag in ((1, "post1"), (2, "post2")):
        pre_info = f32.first_line(f32.obj_info(c))
        mark = sess.logmark()
        c.send("set home_planet selected", 3.0)
        A[f"objinfo_before_{n}"] = pre_info
        A[f"log_{n}"] = [l.strip() for l in sess.lognew(mark).splitlines() if l.strip()]
        if pre_info != "EOL":
            fail(f"A: `get status object` was {pre_info!r}, not 'EOL', before "
                 f"invocation {n} - something WAS selected and the leg measures "
                 f"the wrong thing")
        if not alive(sess, f"set home_planet selected #{n}"):
            return
        sample(tag, do_settle=True)

    mark = sess.logmark()
    c.send("set home_planet Earth", 3.0)
    A["log_restore"] = [l.strip() for l in sess.lognew(mark).splitlines() if l.strip()]
    sample("restored", do_settle=True)
    A["alive"] = sess.proc.poll() is None


def check_B(sess, c, res, hip):
    """The overlay drawn for nobody, and whether it moves with the clock."""
    B = res["B"] = {"steps": [], "hip": hip}
    c.send("deselect", 0.6)
    sky_dark(c)
    c.send("zoom fov 180 duration 0", 1.5)
    c.send("look_at azimuth 0 altitude 90 duration 0", 1.5)
    c.send("flag object_coordinates off", 0.8)
    c.send(f"date jday {JD}", 1.2)
    time.sleep(1.5)

    p = shot(sess, c, "B_dark_control")
    B["dark_control_lit"] = int(lit(p).sum()) if p else None
    note(f"B dark control: {B['dark_control_lit']} lit px with every sky flag "
         f"off and no overlay")

    # --- positive control: a real star, its labels against its own RA/DEC ----
    c.send(f"select hp {hip} pointer off", 1.2)
    c.send("flag track_object on", 3.0)
    time.sleep(2.0)
    read_vars(c, "b_star")
    c.send("flag object_coordinates on", 2.5)
    time.sleep(2.0)
    ps = shot(sess, c, "B_pos_on")
    c.send("flag object_coordinates off", 1.5)
    time.sleep(1.2)
    pso = shot(sess, c, "B_pos_off")
    B["pos_on"], B["pos_off"] = str(ps), str(pso)
    if ps and pso:
        bb = bbox_centroid(lit(ps) & ~lit(pso))
        B["pos_labels"] = bb
        if bb:
            crop(ps, bb, sess.outdir / "B_pos_crop.png")
            ok(f"B positive control: {bb['n']} label px at "
               f"({bb['cx']:.0f},{bb['cy']:.0f}) with a star selected")
        else:
            fail("B positive control: object_coordinates on/off changed NOTHING "
                 "with a star selected - the channel cannot show the overlay at "
                 "all, so no negative leg is evidence")

    # --- the negative legs: nothing selected, the clock stepped --------------
    c.send("flag track_object off", 1.0)
    c.send("deselect", 1.0)
    B["objinfo"] = f32.first_line(f32.obj_info(c))
    if B["objinfo"] != "EOL":
        fail(f"B: `get status object` is {B['objinfo']!r}, not 'EOL' - the "
             f"negative legs do not measure 'nothing selected'")
    c.send("look_at azimuth 0 altitude 90 duration 0", 1.5)

    for i in range(B_STEPS):
        jd = JD + i / B_STEPS
        c.send(f"date jday {jd:.6f}", 1.0)
        time.sleep(0.8)
        off = shot(sess, c, f"B_s{i:02d}_off")
        c.send("flag object_coordinates on", 1.2)
        time.sleep(1.2)
        on = shot(sess, c, f"B_s{i:02d}_on")
        c.send("flag object_coordinates off", 1.0)
        rec = {"i": i, "jd": jd, "on": str(on), "off": str(off)}
        if on and off:
            rec["labels"] = bbox_centroid(lit(on) & ~lit(off))
            rec["lit_off"] = int(lit(off).sum())
            if rec["labels"]:
                crop(on, rec["labels"], sess.outdir / f"B_s{i:02d}_crop.png")
        B["steps"].append(rec)
        print(f"  B/{i:02d} jd={jd:.4f}: "
              f"{(rec.get('labels') or {}).get('n')} label px", flush=True)


def check_C(sess, c, res, hip):
    """Indeterminate memory in the illuminate grid, identified by name."""
    C = res["C"] = {"hip": hip}
    c.send("flag object_coordinates off", 1.0)
    c.send("illuminate action clear", 1.5)

    # centre the star, then narrow the field so a 5' quad is ~55 px
    c.send(f"select hp {hip} pointer off", 1.2)
    c.send("flag track_object on", 3.0)
    time.sleep(2.0)
    c.send("flag track_object off", 1.0)
    read_vars(c, "c_star")
    c.send("zoom fov 2 duration 0", 2.5)
    time.sleep(2.0)
    c.send("deselect", 0.8)
    base = shot(sess, c, "C0_base")

    # (1) hp 0 EARLY - before any valid load has written those stack slots
    mark = sess.logmark()
    c.send("illuminate hp 0 display on size 5", 2.0)
    C["log_early"] = [l.strip() for l in sess.lognew(mark).splitlines() if l.strip()]
    if not alive(sess, "illuminate hp 0 (early)"):
        return
    time.sleep(1.0)
    early = shot(sess, c, "C1_hp0_early")
    C["early_delta"] = diffcount(early, base)
    c.send("illuminate hp 0 display off", 1.5)
    c.send("illuminate action clear", 1.5)
    time.sleep(0.8)
    C["cleaned_delta"] = diffcount(shot(sess, c, "C2_cleaned"), base)

    # (2) the size<=1 control: the command runs, load returns before the read
    c.send("illuminate hp 0 display on size 1", 1.5)
    time.sleep(0.8)
    C["size1_delta"] = diffcount(shot(sess, c, "C3_hp0_size1"), base)
    c.send("illuminate action clear", 1.5)

    # (3) the positive control: a real, catalogue-answering id
    c.send(f"illuminate hp {hip} display on size 5", 2.0)
    time.sleep(1.2)
    withv = shot(sess, c, "C4_hpV")
    C["V_delta"] = diffcount(withv, base)
    C["V_box"] = bbox_centroid(lit(withv) & ~lit(base)) if (withv and base) else None
    if withv and C["V_box"]:
        crop(withv, C["V_box"], sess.outdir / "C4_hpV_crop.png", pad=12, scale=2)

    # (4) hp 0 LATE - the next call into the same frame
    mark = sess.logmark()
    c.send("illuminate hp 0 display on size 5", 2.0)
    C["log_late"] = [l.strip() for l in sess.lognew(mark).splitlines() if l.strip()]
    if not alive(sess, "illuminate hp 0 (late)"):
        return
    time.sleep(1.2)
    both = shot(sess, c, "C5_hpV_plus_hp0")
    C["both_vs_V"] = diffcount(both, withv)

    # (5) remove V BY NAME, then remove 0 BY NAME - the identification
    c.send(f"illuminate hp {hip} display off", 2.0)
    time.sleep(1.2)
    afterv = shot(sess, c, "C6_after_removing_V")
    C["afterV_vs_base"] = diffcount(afterv, base)
    C["afterV_vs_withV"] = diffcount(afterv, withv)
    C["afterV_box"] = (bbox_centroid(lit(afterv) & ~lit(base))
                       if (afterv and base) else None)
    if afterv and C["afterV_box"]:
        crop(afterv, C["afterV_box"], sess.outdir / "C6_after_crop.png",
             pad=12, scale=2)
    c.send("illuminate hp 0 display off", 2.0)
    time.sleep(1.2)
    C["after0_vs_base"] = diffcount(shot(sess, c, "C7_after_removing_0"), base)

    # (6) the wide-sky leg: if hp 0 went elsewhere, this is the channel that
    #     could see it (size 600 = 10 deg, visible anywhere on the hemisphere)
    c.send("illuminate action clear", 1.5)
    c.send("zoom fov 180 duration 0", 2.5)
    c.send("look_at azimuth 0 altitude 90 duration 0", 1.8)
    time.sleep(1.5)
    wbase = shot(sess, c, "C8_wide_base")
    c.send("illuminate hp 0 display on size 600", 2.0)
    if not alive(sess, "illuminate hp 0 size 600 (wide)"):
        return
    time.sleep(1.5)
    wide = shot(sess, c, "C9_wide_hp0")
    C["wide_delta"] = diffcount(wide, wbase)
    C["wide_box"] = bbox_centroid(lit(wide) & ~lit(wbase)) if (wide and wbase) else None
    c.send("illuminate action clear", 1.5)
    C["alive"] = sess.proc.poll() is None


# ----------------------------------------------------------------------- main
def gates(res):
    L = res.get("readings", {})
    A = res.get("A", {})
    for leg in A.get("legs", []):
        leg["dist"] = {b: L.get(f"a_{leg['tag']}_{b.lower()}", {}).get(
            "selected_distance") for b in BODIES}
        leg["mag"] = {b: L.get(f"a_{leg['tag']}_{b.lower()}", {}).get(
            "selected_magnitude") for b in BODIES}
    legs = {l["tag"]: l for l in A.get("legs", [])}
    pre, p1, p2, rst = (legs.get(k) for k in ("pre", "post1", "post2", "restored"))

    # G0 the probe can answer real values at all (F50's channel, re-controlled)
    if pre:
        if pre["dist"].get("Mars") not in (None, "1"):
            ok(f"G0 control: on the untouched observer the channel answers "
               f"{pre['dist']} - not the singleton's 1")
        else:
            fail(f"G0 control: baseline distances {pre['dist']} - the channel is "
                 f"not answering real values, so nothing below is evidence")

    if A.get("log_1") is not None:
        warn = [l for l in A["log_1"] if "re-reference the camera" in l]
        (ok if warn else fail)(
            f"G1a diagnostic: {len(warn)} syncCameraReference warning(s): "
            f"{warn[:1]}")
        told = [l for l in A["log_1"]
                if re.search(r"nothing.*select|no.*selection|not selected", l, re.I)]
        if told:
            fail(f"G1b: a line DOES report the absent selection: {told[:2]} - "
                 f"the prediction that nothing on this path is §2(f)-grade fails")
        else:
            ok("G1b: no log line reports that nothing was selected, names the "
               "fictitious anchor, or states a consequence or an action")

    for tag, l in (("post1", p1), ("post2", p2)):
        if not l:
            continue
        o = l["observer"]
        (ok if o.get("onBody") is False else fail)(
            f"G2 {tag}: observer.onBody = {o.get('onBody')} (predicted False)")
        (ok if o.get("homePlanet") == "" else fail)(
            f"G3 {tag}: observer.homePlanet = {o.get('homePlanet')!r} (predicted '')")
        d = o.get("distanceFromCenter")
        (ok if (isinstance(d, (int, float)) and d < 1e-8) else fail)(
            f"G4 {tag}: distanceFromCenter = {d} (predicted < 1e-8 AU)")
        de = l["dist"].get("Earth")
        try:
            v = float(de)
        except (TypeError, ValueError):
            v = None
        (ok if (v is not None and 0.99995 <= v <= 1.00005) else fail)(
            f"G5 {tag}: selected_distance(Earth) = {de} "
            f"(predicted 1 AU +/- one Earth radius)")

    if pre and p1:
        for k in ("longitude", "latitude", "altitude"):
            a0, a1 = pre["observer"].get(k), p1["observer"].get(k)
            (ok if a0 == a1 else fail)(
                f"G6 {k} across the teleport: {a0} -> {a1} (predicted unchanged)")
        q0, q1 = pre.get("pos"), p1.get("pos")
        if q0 and q1:
            same = all(abs(q0[k] - q1[k]) < 1e-6 for k in ("lat", "lon", "alt"))
            (ok if same else fail)(
                f"G7 `get status position` across the teleport: {q0} -> {q1} "
                f"(predicted lat/lon/alt unchanged => the channel the row's own "
                f"owed clause names is BLIND to a 1 AU displacement)")
        else:
            fail(f"G7: no `get status position` reply to compare "
                 f"({q0} / {q1}) - the row's named channel produced nothing")
        moved = [b for b in BODIES if pre["dist"].get(b) != p1["dist"].get(b)]
        (ok if moved else fail)(
            f"G8 the distances that moved: {moved} "
            f"(pre {pre['dist']} -> post1 {p1['dist']})")

    if p1 and p2:
        same_d = p1["dist"] == p2["dist"]
        (ok if same_d else fail)(
            f"G9 LATCH: post2 {p2['dist']} vs post1 {p1['dist']} - "
            + ("byte-identical: the anchor cached under the empty name was reused"
               if same_d else
               "DIFFERENT: a new anchor was built, so the cache claim FAILS"))
        same_o = p1["observer"] == p2["observer"]
        (ok if same_o else fail)(
            f"G10 LATCH: the whole oldView.observer block is "
            f"{'identical' if same_o else 'DIFFERENT'} across the two invocations")

    if rst:
        o = rst["observer"]
        good = o.get("homePlanet") == "Earth" and o.get("onBody") is True
        (ok if good else fail)(
            f"G11 restore control: homePlanet={o.get('homePlanet')!r} "
            f"onBody={o.get('onBody')} dfc={o.get('distanceFromCenter')}")

    B = res.get("B")
    if B:
        steps = [s for s in B["steps"] if (s.get("labels") or {}).get("n")]
        (ok if steps else fail)(
            f"G12 the overlay IS drawn with nothing selected in "
            f"{len(steps)}/{len(B['steps'])} steps")
        cs = [(s["labels"]["cx"], s["labels"]["cy"]) for s in steps]
        if len(cs) >= 2:
            spread = max(max(abs(a[0] - b[0]) for a in cs for b in cs),
                         max(abs(a[1] - b[1]) for a in cs for b in cs))
            B["centroid_spread_px"] = spread
            (ok if spread > 20 else fail)(
                f"G13 the label block MOVES with the clock: centroid spread "
                f"{spread:.1f} px over {len(cs)} steps (predicted > 20)")
        elif steps:
            note("G13 only one step carries labels - motion not testable")

    C = res.get("C")
    if C:
        (ok if (C.get("V_delta") or 0) > 500 else fail)(
            f"G14 positive control: `illuminate hp {C.get('hip')} size 5` "
            f"changed {C.get('V_delta')} px (predicted > 500)")
        (ok if C.get("size1_delta") == 0 else fail)(
            f"G15 size-1 control: `illuminate hp 0 display on size 1` changed "
            f"{C.get('size1_delta')} px (predicted 0 - load returns at :136)")
        for k in ("log_early", "log_late"):
            bad = [l for l in C.get(k, [])
                   if re.search(r"illuminate|hipparcos|uninitial|\bHP\b", l, re.I)]
            (ok if not bad else fail)(
                f"G16 {k}: {len(bad)} line(s) mention the absent id: {bad[:2]}")
        av, a0 = C.get("afterV_vs_base"), C.get("after0_vs_base")
        if av is not None and a0 is not None:
            if av > 500 and a0 == 0:
                ok(f"G17 branch b1 STALE-STACK, IDENTIFIED BY NAME: {av} px "
                   f"survive removing V and {a0} px survive removing 0 - an "
                   f"illuminate NAMED 0 sat at V's coordinates")
            elif av <= 500:
                note(f"G17 branch b2 ELSEWHERE: only {av} px differ from base "
                     f"after removing V - hp 0's illuminate is not in this cone")
            else:
                note(f"G17 neither branch cleanly: afterV {av} px, after0 {a0} px")
        note(f"G18 history: hp 0 EARLY changed {C.get('early_delta')} px, "
             f"LATE (after a valid load) left {C.get('both_vs_V')} px of "
             f"difference against the with-V frame")
        note(f"G19 wide sky (fov 180, size 600): {C.get('wide_delta')} px, "
             f"box {C.get('wide_box')}")

    if res.get("refused"):
        fail(f"the app refused command(s): {res['refused'][:5]}")
    else:
        ok("the app refused nothing (its own §2(f) channel)")
    if res.get("exit_rc") != 0:
        fail(f"app exit rc={res.get('exit_rc')}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--bin", default=DEFAULT_BIN)
    ap.add_argument("--port-wait", type=int, default=90)
    a = ap.parse_args()
    out = Path(a.outdir).resolve()
    out.mkdir(parents=True, exist_ok=True)
    (out / "f53_predict.json").write_text(json.dumps(PRED, indent=1))

    print("=== F53 §5.113 - the three reaches of the unguarded singleton ===",
          flush=True)
    print(f"binary : {a.bin}\nmd5    : {f27.md5(a.bin)}\nwall   : "
          f"{time.strftime('%F %T %Z')}", flush=True)

    sess = f27.Session(out, "f53", a.bin, port_wait=a.port_wait)
    c = sess.client("driver")
    res = {"bin": str(a.bin), "md5": f27.md5(a.bin), "pred": PRED,
           "wall": time.strftime("%F %T %Z")}
    try:
        c.send("timerate rate 0", 1.2)
        c.send(f"date jday {JD}", 1.2)
        c.send("flag track_object off", 0.8)
        c.send("deselect", 0.8)

        hip = None
        res["hip_probe"] = []
        for h in HIP_CANDIDATES:
            c.send(f"select hp {h} pointer off", 1.0)
            info = f32.first_line(f32.obj_info(c))
            res["hip_probe"].append({"hp": h, "info": info})
            if info and info != "EOL":
                hip = h
                break
        c.send("deselect", 0.8)
        res["hip"] = hip
        if hip is None:
            fail("no HIP candidate answers on this catalogue - checks B and C "
                 "have no positive control and are not run")

        print("\n--- check A: set home_planet selected ---", flush=True)
        check_A(sess, c, res)
        if hip is not None and sess.proc.poll() is None:
            print("\n--- check B: flag object_coordinates on ---", flush=True)
            check_B(sess, c, res, hip)
        if hip is not None and sess.proc.poll() is None:
            print("\n--- check C: illuminate hp 0 ---", flush=True)
            check_C(sess, c, res, hip)
    finally:
        try:
            text = script_log_text(sess)
            (out / "f53_script.log").write_text(text)
            res["readings"] = parse_legs(text)
            res["refused"] = sess.refused()
        except Exception as e:                    # the logs are on disc either way
            fail(f"reading the script log failed: {e!r}")
        try:
            res["exit_rc"] = sess.stop(c)
        except Exception as e:
            res["exit_rc"] = sess.proc.poll()
            fail(f"shutdown failed ({e!r}); process rc={res['exit_rc']}")
        note(f"app exit rc={res['exit_rc']}")

    print("\n--- gates ---", flush=True)
    gates(res)
    res["fails"], res["notes"] = FAILS, NOTES
    (out / "f53_result.json").write_text(json.dumps(res, indent=1, default=str))
    print(f"\n{len(FAILS)} FAIL, {len(NOTES)} note -> {out}/f53_result.json",
          flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
