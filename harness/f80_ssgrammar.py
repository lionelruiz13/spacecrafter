#!/usr/bin/env python3
"""f80_ssgrammar.py -- emit util/scedit/grammar/ss-grammar.json, the SECOND
contract file: the stellar-system file's grammar, in two regimes.

WHY A GENERATOR AND NOT A HAND-WRITTEN FILE. Two of the three things in the
output must not be typed by a human: the `file:line` ANCHORS (typed anchors go
stale silently -- the whole reason `anchor_gate` exists, scedit journal
2026-09-01a) and the COUNTS (a count that is asserted rather than derived is a
criterion that cannot fail -- scedit INTENT S5 item 20). Both come from
f80_census.py, which reads the loader. What IS hand-authored is the third thing:
the per-key JUDGMENT -- the doc line at the zero-knowledge bar (C6), the value
domain read at the code's own enumeration (never a parallel invention, I2), the
default read at the site, and REQUIRED/optional. Every one of those is entered
below beside the key it describes, and a key the census finds with no entry here
is an ERROR, never a default (the f70/f76 pattern).

THE TWO REGIMES ARE NOT TWO DIALECTS OF ONE FORMAT (F80 mandate (1)/(3)):
  legacy    -- `ssystem.ini` and `data/default_ssystem.ini`, read by
               ProtoSystem::addBody (the frozen comparison baseline, S11.52(b))
               and, when the experimental path also loads the file, by
               ModularSystem::loadBody;
  composed  -- `~/.spacecrafter/modularSystem/<Name>.ini`, the B24 format, read
               by ModularSystem::loadComposedSystem -> loadBody.
A key read by only ONE of them is a finding and is marked `regimes` accordingly.
That column is what pays the base-D census residual (capability-surface.md S2:
"DONE for the new path; old-path-only keys (bodyModule/) not swept").

D16-D19 ARE RATIFIED, NOT PROPOSED [vixy 2026-07-23, DECISIONS_PENDING.md:149-164
-> INTENT S11.79(j)-(m)], and D16's respell LANDED (S11.89, code f911646b). The
contract says so in `_meta.decisions`. The ONE thing still transitional is
recorded as such with its veto point, not smoothed over: S11.89(c).

Usage:  python3 f80_ssgrammar.py [--out PATH] [--print-counts]
"""
import json
import os
import sys
import subprocess

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
OUT = os.path.join(ROOT, "util", "scedit", "grammar", "ss-grammar.json")

# The commit every `file:line` in this file resolves at. One fact, stated once,
# exactly as sc-grammar.json states it (README S Authority chain).
ANCHOR_PIN = "master-beta @ ba7a32a8"

# --- value types -------------------------------------------------------------
# The conversion each key goes through, named once. The two BOOLEANS are the
# sharp edge of this contract and the reason this table exists at all: the two
# loaders do not agree on what "true" is.
VALUE_TYPES = {
    "bool_legacy": {
        "reader": "Utility::strToBool",
        "source": "tools/utility.cpp:strToBool",
        "true_set": ["true", "1"],
        "case_insensitive": True,
        "doc": "Legacy boolean. TRUE only for 'true' or '1' (any case); EVERY other "
               "value -- including 'on', 'yes' and a misspelling -- reads as FALSE, "
               "silently. There is no accepted FALSE set, so a typo is indistinguishable "
               "from a deliberate 'off'.",
    },
    "bool_composed": {
        "reader": "Utility::isTrue",
        "source": "tools/utility.hpp:160",
        "true_set": ["true", "on", "1"],
        "case_insensitive": True,
        "doc": "Composed-path boolean. TRUE for 'true', 'on' or '1' (any case); every "
               "other value reads as FALSE, silently. NOTE it accepts 'on' where the "
               "legacy reader does not -- the same authored value can mean different "
               "things on the two paths.",
    },
    "double": {"reader": "Utility::strToDouble", "source": "tools/utility.cpp:strToDouble",
               "doc": "A number, parsed by std::stod: leading blanks are allowed and a "
                      "trailing non-numeric tail is IGNORED ('12abc' reads as 12). "
                      "Unparseable or absent falls back to the key's default."},
    "float": {"reader": "Utility::strToFloat", "source": "tools/utility.cpp:strToFloat",
              "doc": "As `double`; stored as a float."},
    "string": {"reader": "(raw)", "doc": "Taken verbatim, including any high bytes."},
    "vec3f": {"reader": "Utility::strToVec3f",
              "doc": "Three comma-separated numbers, 'r,g,b'. NB a space after a comma "
                     "is not a separator the legacy positional reader forgives."},
    "enum": {"reader": "(per key)", "doc": "One of a fixed set the code enumerates; see the key's `domain`."},
}

# The `coord_func` domain, in the three groups the code itself keeps it in.
# The 32 `*_special` names are extracted from orbit.cpp rather than typed:
# `calisto_special` has one 'l' and a transcription would silently fix it.
COORD_FUNC_INTERCEPTED = ["earth_custom", "lunar_custom", "still_orbit", "location_orbit"]
COORD_FUNC_CHAIN = ["barycenter", "ell_orbit", "comet_orbit"]
COORD_FUNC_COMPOSED_ONLY = ["surface_point"]


def special_names():
    import re as _re
    path = os.path.join(ROOT, "src", "bodyModule", "orbit.cpp")
    with open(path, encoding="utf-8", errors="surrogateescape") as fh:
        return _re.findall(r'ephemerisName *== *"([a-z_0-9]+)"', fh.read())


# Read sites the mechanical scan CANNOT see, each anchored by hand and each
# saying why the regex misses it. Keeping them here rather than widening the
# regex is deliberate: a wider regex would also swallow things that are not key
# reads, and this list is short, dated and reviewable.
EXTRA_SITES = [
    # SSystemFactory::reloadColors -- a THIRD reader of ssystem.ini (after
    # ProtoSystem::load and, via IniLine, ModularSystem::loadSystem). It does not
    # use a parameter map at all: it tests raw line PREFIXES, so no `param["k"]`
    # exists to find. Sole call site core.cpp:340, with the literal "ssystem.ini".
    ("name", "src/bodyModule/ssystem_factory.cpp", 230, "starts_with", "legacy"),
    ("name", "src/bodyModule/ssystem_factory.cpp", 248, "==", "legacy"),
    ("color", "src/bodyModule/ssystem_factory.cpp", 229, "starts_with", "legacy"),
    ("color", "src/bodyModule/ssystem_factory.cpp", 280, "==", "legacy"),
    ("label_color", "src/bodyModule/ssystem_factory.cpp", 226, "starts_with", "legacy"),
    ("orbit_color", "src/bodyModule/ssystem_factory.cpp", 227, "starts_with", "legacy"),
    ("trail_color", "src/bodyModule/ssystem_factory.cpp", 228, "starts_with", "legacy"),
]

# --- the key table -----------------------------------------------------------
# (doc, type, domain, default, required, notes)
# `domain` None = the type's own domain. `default` is the value the body gets
# when the key is ABSENT, read at the site. `required` = the body is not created
# without it.
S = "string"
D = "double"
F = "float"
BL = "bool_legacy"
BC = "bool_composed"
V3 = "vec3f"

KEYS = {
    # ---- identity and placement ---------------------------------------------
    "name": ("The body's English name. THIS IS THE IDENTITY: it must be unique across "
             "every loaded system, and a second body with the same name is refused, not "
             "merged. Without it the section is skipped entirely.",
             S, None, None, True,
             "D34/S11.109(c). Both loaders refuse a duplicate AND SAY SO at L_WARNING "
             "(protosystem.cpp:546-549, ModularSystem.cpp:1057-1060) -- the drop is "
             "logged, not silent, which scedit INTENT C4's wording predates."),
    "parent": ("The English name of the body this one orbits. 'none' makes it the centre "
               "of its system. Absent is NOT the same as 'none': it warns and adopts the "
               "centre body anyway. A name that names nothing skips the body.",
               S, None, "the centre body, with a warning", False,
               "Order matters: a parent must appear EARLIER in the file "
               "(ModularSystem.cpp:773-778 resolves by name against what is loaded)."),
    "type": ("What the body IS -- the body class. Drives which object is built and, in a "
             "legacy file, a set of capabilities that the composed format states as keys "
             "of their own. An unrecognised value falls back to 'Asteroid' WITH a warning.",
             "enum",
             ["Sun", "Star", "Planet", "Moon", "Dwarf", "Asteroid", "KBO", "Comet",
              "Artificial", "Observer", "Center"],
             "Asteroid (logged)", False,
             "MATCHED ON THE FIRST FOUR BYTES (protosystem.cpp:477-501): "
             "`casify` packs 4 bytes and the switch compares them, so any value sharing a "
             "4-byte prefix with a canonical name is ACCEPTED -- 'Planetoid' loads as "
             "Planet, 'Moonlet' as Moon. A value shorter than 3 bytes is UNKNOWN. The "
             "3-byte names ('Sun', 'KBO') match exactly, their 4th byte being the "
             "terminator. The composed loader does NOT share this: it compares the whole "
             "string (ModularSystem.cpp:1103,1113), so 'Moonlet' is a Moon to the old path "
             "and not a Moon to the new one."),
    "coord_func": ("Which orbit family computes this body's position. Getting it wrong is "
                   "not cosmetic: an unknown value means NO ORBIT, and the body is not "
                   "added to the old render path at all.",
                   "enum", "@coord_func", None, False,
                   "Four values are intercepted before the creator chain "
                   "(protosystem.cpp:561,575,594,598); three more are handled BY the chain "
                   "(Bary -> Eliptic -> Comet -> Special, protosystem.cpp:65-68); the 32 "
                   "`*_special` names are matched inside SpecialOrbit's constructor "
                   "(orbit.cpp:770-901) and a body declaring one has EVERY orbital element "
                   "IGNORED -- OrbitCreatorSpecial reads no key but this one. "
                   "`surface_point` belongs to the experimental path only: the legacy reader "
                   "names it in its own failure message as the one value it deliberately "
                   "leaves to the other path (protosystem.cpp:636-645). NB the Callisto "
                   "ephemeris is spelled `calisto_special`, with one 'l'."),
    "radius": ("The body's radius in KILOMETRES. Absent reads as 0, which is a body with no "
               "size rather than an error.", D, None, "0", False, None),
    "oblateness": ("Polar flattening, 0 = a sphere.", D, None, "0", False, None),
    "albedo": ("How much light the surface reflects, 0..1.", D, None, "0", False, None),
    "halo": ("Draw a halo around the body.", BL, None, "false", False,
             "READ BY A DIFFERENT PREDICATE ON EACH PATH: strToBool (legacy, "
             "protosystem.cpp:685) vs isTrue (composed, ModularSystem.cpp:1243), so "
             "`halo = on` draws nothing on the old path and a halo on the new one."),
    "hidden": ("Load the body but do not show it.", BL, None, "false", False,
               "Same two-predicate split as `halo` (protosystem.cpp:983 vs "
               "ModularSystem.cpp:1400)."),
    "sol_local_day": ("Length of the body's solar day, in the units the body's own day is "
                      "counted in.", D, None, "1.0", False, None),
    "close_orbit": ("Whether the orbit is drawn as a closed curve.", BL, None, "true", False,
                    "LEGACY ONLY -- the composed loader's read is commented out "
                    "(ModularSystem.cpp:1074)."),
    "orbit_bounding_radius": ("Radius used for visibility/bounding tests, when the orbit "
                              "cannot supply one.", D, None, "-1 (meaning unused)", False,
                              "For `ell_orbit` the value is OVERWRITTEN by the orbit's own "
                              "computed radius (protosystem.cpp:647-649), so authoring it "
                              "there has no effect. Composed read is commented out (:1075)."),
    "hardcoded": ("Marks a body the engine itself created, not the file.", S, None, None, False,
                  "COMPOSED ONLY; a provenance marker, not authoring surface."),
    "replace": ("Allow this section to REPLACE an already-loaded body of the same name "
                "instead of being skipped.", BC, None, "false", False, "COMPOSED path read."),
    # ---- colours and textures ----------------------------------------------
    "color": ("The body's own colour, 'r,g,b' in 0..1.", V3, None, "path-dependent", False, None),
    "label_color": ("Colour of the body's name label.", V3, None, None, False, None),
    "orbit_color": ("Colour of the drawn orbit.", V3, None, None, False, None),
    "trail_color": ("Colour of the trail the body leaves.", V3, None, None, False,
                    "LEGACY ONLY (protosystem.cpp:655)."),
    "tex_map": ("Surface texture file name.", S, None, "none", False, None),
    "tex_normal": ("Normal (bump) map file name.", S, None, "none", False, None),
    "tex_night": ("Night-side texture file name.", S, None, "none", False, None),
    "tex_specular": ("Specular map file name.", S, None, "none", False, None),
    "tex_heightmap": ("Height map file name.", S, None, "none", False, None),
    "tex_skin": ("Skin texture file name.", S, None, "none", False, None),
    "tex_big_halo": ("Texture for the large halo drawn around a star. Setting it is what "
                     "ENABLES the big halo.", S, None, "none", False, None),
    "big_halo_size": ("Size of the big halo. Only read when `tex_big_halo` is set.",
                      D, None, "50", False, None),
    "path": ("Directory the big-halo texture is looked up in.", S, None, None, False, None),
    "model_name": ("Name of the 3D model to draw instead of a sphere.", S, None,
                   "the default model", False, None),
    # ---- rings ---------------------------------------------------------------
    "rings": ("Give this body a ring system. The three ring keys are only read when this "
              "is true.", BL, None, "false", False, "LEGACY ONLY (protosystem.cpp:813)."),
    "ring_inner_size": ("Inner ring radius, KILOMETRES.", D, None, "0", False, None),
    "ring_outer_size": ("Outer ring radius, KILOMETRES.", D, None, "0", False, None),
    "tex_ring": ("Ring texture file name.", S, None, "none", False, None),
    # ---- rotation ------------------------------------------------------------
    "rot_periode": ("How long the body takes to turn once, in HOURS.", D, None,
                    "`orbit_period` if present, else 24 hours", False,
                    "The 24-hour fallback is an ACTING DEFAULT: the composed loader logs it "
                    "(ModularSystem.cpp:1342-1350, D12), the legacy loader does not "
                    "(protosystem.cpp:960)."),
    "rot_obliquity": ("Tilt of the body's equator relative to its PARENT's equator, DEGREES.",
                      D, None, "0", False, None),
    "rot_equator_ascending_node": ("Where the body's equator crosses the parent's, DEGREES.",
                                   D, None, "0", False, None),
    "rot_epoch": ("Julian day the rotation angles are measured at.", D, None, "J2000", False, None),
    "rot_rotation_offset": ("Prime-meridian angle at the epoch, DEGREES.", D, None, "0", False, None),
    "rot_precession_rate": ("Rate the rotation axis precesses, degrees per Julian century.",
                            D, None, "0", False, None),
    "rot_pole_ra": ("Right ascension of the body's north pole in the ABSOLUTE J2000 frame, "
                    "DEGREES. Writing either pole key switches the whole orientation to the "
                    "absolute frame and REPLACES rot_obliquity/rot_equator_ascending_node.",
                    D, None, "0", False, None),
    "rot_pole_de": ("Declination of the body's north pole, absolute J2000, DEGREES. See "
                    "`rot_pole_ra`.", D, None, "0", False, None),
    "rot_pole_w0": ("IAU prime meridian W0 for an absolute-pole body, DEGREES; converted by "
                    "the loader into the internal referential.",
                    F, None, "rot_rotation_offset, used raw", False, "COMPOSED ONLY (B28/S11.79(a))."),
    "rot_frame": ("Which frame the rotation keys above are authored in. Absent, it is "
                  "DERIVED: pole keys present means absolute, otherwise parent-relative.",
                  "enum", ["absolute_pole", "parent_relative"],
                  "derived from which rotation keys are present", False,
                  "COMPOSED ONLY. An invalid value falls back to the derived default with an "
                  "actionable diagnostic (ModularSystem.cpp:944-952)."),
    "axial_tilt": ("Extra tilt applied to the drawn body.", D, None, "0", False, None),
    "orbit_visualization_period": ("Period used when drawing the orbit curve, days.",
                                   D, None, "0", False, None),
    "sidereal_time": ("Which sidereal-time model drives the body's rotation phase.",
                      S, None, "none", False, "COMPOSED ONLY."),
    # ---- atmosphere ----------------------------------------------------------
    "has_atmosphere": ("Give this body an atmosphere. Presence of this key OR of "
                       "`atmosphere_lim_landscape` is what opens the whole atmosphere block.",
                       BL, None, "false", False, None),
    "atmosphere_model": ("Which built-in atmosphere model to use. An unrecognised value "
                         "silently means NO model.",
                         "enum", ["earth_model", "venus_model", "mars_model"],
                         "no model", False, "protosystem.cpp:setAtmosphere, silent fallback."),
    "atmosphere_ext_model": ("File name of an external atmosphere table.", S, None, "none", False, None),
    "atmosphere_radius_factor": ("Atmosphere radius as a multiple of the body's radius.",
                                 D, None, "1.05", False, None),
    "atmosphere_lim_inf": ("Lower altitude bound of the atmosphere, METRES.", F, None, "40000", False, None),
    "atmosphere_lim_sup": ("Upper altitude bound of the atmosphere, METRES.", F, None, "80000", False, None),
    "atmosphere_lim_landscape": ("Altitude below which the landscape is drawn, METRES.",
                                 F, None, "10000", False, None),
    "atmosphere_ambient_r": ("Red component of the atmosphere's ambient light.", F, None, "0", False, None),
    "atmosphere_ambient_g": ("Green component of the atmosphere's ambient light.", F, None, "0", False, None),
    "atmosphere_ambient_b": ("Blue component of the atmosphere's ambient light.", F, None, "0", False, None),
    "atmosphere_sun_deviation": ("Angular spread of the sunlight through the atmosphere, DEGREES.",
                                 F, None, "0", False, None),
    "atmosphere_ambient_deviation": ("Angular spread of the ambient light, DEGREES.",
                                     F, None, "0", False, None),
    # ---- small bodies and tails ---------------------------------------------
    "apparent_magnitude": ("Brightness parameter of a small body. THIS KEY AND `slope` "
                           "TOGETHER gate the entire tail system: without both, no tail key "
                           "on the body is read at all.",
                           F, None, None, False,
                           "The setter it feeds is named setAbsoluteMagnitudeAndSlope "
                           "(protosystem.cpp:840) while the key is spelled `apparent_` -- "
                           "and `absolute_magnitude`, which the shipped file uses once, is "
                           "read by nothing."),
    "slope": ("Brightness slope parameter of a small body. See `apparent_magnitude`.",
              F, None, None, False, None),
    "halo_alpha_override": ("Override the computed halo opacity. Read only together with "
                            "`halo_scale_override`.", F, None, None, False, None),
    "halo_scale_override": ("Override the computed halo size. Read only together with "
                            "`halo_alpha_override`.", F, None, None, False, None),
    # ---- new-path capability keys -------------------------------------------
    "light_source": ("Declares that this body EMITS light. In a legacy file the same fact "
                     "is inferred from `type`; in a composed file it must be stated.",
                     BC, None, "from `type` (legacy) / false (composed)", False, "COMPOSED ONLY (B27)."),
    "primary": ("Declares this body the primary star of its system.", BC, None,
                "from `type` (legacy) / false (composed)", False, "COMPOSED ONLY (B27, D27)."),
    "shadow_exempt": ("Declares that this body neither casts nor receives shadows.",
                      BC, None, "from `type` (legacy) / false (composed)", False, "COMPOSED ONLY (B27)."),
    "shadow_color": ("How much light of each channel survives this body's shadow, 'r,g,b'.",
                     V3, None, "1,1,1 (an opaque shadow)", False, "COMPOSED ONLY."),
    "surface_model": ("Which lighting lineage the surface uses. In a legacy file it is "
                      "inferred from `type = Moon`.", S, None,
                      "'lunar' for type = Moon (legacy) / 'planet' (composed)", False,
                      "COMPOSED ONLY (B27, A6)."),
    "trail_length": ("How many samples of trail the body keeps.", "int", None,
                     "from `type` (legacy: Planet/Dwarf 1460, Comet 2920, else the default)",
                     False, "COMPOSED ONLY (B27, A7)."),
    "brightness": ("Extra brightness applied to the body.", F, None, "0", False, "COMPOSED ONLY."),
    "datum_radius": ("Radius the altitude datum is measured from, KILOMETRES.", F, None,
                     "`radius`", False, "COMPOSED ONLY."),
    "ground_radius": ("Radius the ground is drawn at, KILOMETRES.", F, None, "`radius`", False,
                      "COMPOSED ONLY."),
    "display_scale": ("Per-body display scaling.", S, None, "none", False, "COMPOSED ONLY."),
    "system_star": ("Declares this body the star that defines the system.", BC, None, "false",
                    False, "COMPOSED ONLY."),
    "planet_grid": ("Draw a coordinate grid on this body.", BC, None, "false", False,
                    "COMPOSED ONLY; installs a CUSTOM module in the named GRID slot."),
    # ---- B24 structural keys -------------------------------------------------
    "relation": ("How this body is attached to its parent. Supersedes `bound_to_surface`, "
                 "and is the only way to reach 'inner' from a file.",
                 "enum", ["orbiting", "grounded", "inner"],
                 "'grounded' when bound_to_surface is true, else 'orbiting'", False,
                 "D16 RATIFIED [vixy 2026-07-23, S11.79(j)]. NEW-FORMAT ONLY: writing it "
                 "into a legacy file breaks downgrade (D13). Declaring it AND a "
                 "disagreeing bound_to_surface is a diagnosed conflict, explicit wins "
                 "(ModularSystem.cpp:1301-1310)."),
    "compose": ("Whether this body's modules are deduced from its keys or come only from "
                "explicit BodyModule declarations.",
                "enum", ["deduced", "explicit"], "deduced", False,
                "D16 RATIFIED. NEW-FORMAT ONLY (D13)."),
    "bound_to_surface": ("Legacy spelling of `relation = grounded`.", BC, None, "false", False,
                         "COMPOSED-PATH READ ONLY: the legacy loader never reads this key -- "
                         "it derives the same state from `coord_func = location_orbit` "
                         "(protosystem.cpp:598-609). Kept as an alias; prefer `relation`."),
    "body": ("In a BodyModule section, the name of the node this module attaches to. Its "
             "PRESENCE is what makes a section a module declaration rather than a body.",
             S, None, None, True,
             "D16 RATIFIED. The `type=` disambiguator (ModularSystem.cpp:1615-1626). "
             "NEW-FORMAT ONLY (D13)."),
    "slot": ("Name of the slot a module is installed into, when a body carries more than "
             "one module of a family.", S, None, "the family's default slot", False,
             "NEW-FORMAT ONLY (D13)."),
    # ---- orbit sub-grammar ---------------------------------------------------
    "orbit_period": ("Time the body takes to go once round its parent, DAYS.", D, None,
                     "0 under ell_orbit; treated as absent under comet_orbit", False,
                     "Under `ell_orbit` an ABSENT period is not an error: it defaults to 0 and "
                     "the orbit then divides by it (orbit.cpp:450,569), so the body gets an "
                     "infinite or NaN mean anomaly silently. Under `comet_orbit` it is "
                     "required only when the parent body itself has a parent "
                     "(orbit_creator_cor.cpp:182-186). It is ALSO the fallback for "
                     "`rot_periode`, so editing it changes how fast the body SPINS unless "
                     "`rot_periode` is present (protosystem.cpp:960)."),
    "orbit_epoch": ("Julian day the orbital elements are given at.", D, None, "J2000", False, None),
    "orbit_eccentricity": ("How elongated the orbit is; 0 is a circle.", D, None, "0", False, None),
    "orbit_semimajoraxis": ("Half the orbit's long axis. THE UNIT DEPENDS ON `coord_func`: "
                            "KILOMETRES under `ell_orbit`, ASTRONOMICAL UNITS under "
                            "`comet_orbit`.", D, None,
                            "0 under ell_orbit; treated as absent under comet_orbit", False,
                            "One key, one file, two units a factor of 1.496e8 apart: "
                            "orbit_creator_cor.cpp:78 divides by AU, orbit_creator_cor.cpp:159 "
                            "does not. The composed loaders reproduce the asymmetry "
                            "(ElipticOrbitLoader.hpp divides, CometOrbitLoader.hpp does not), "
                            "so it is the data grammar and not a one-path defect. Under "
                            "`comet_orbit` it is REQUIRED unless `orbit_pericenterdistance` "
                            "is given (orbit_creator_cor.cpp:158-164)."),
    "orbit_inclination": ("Tilt of the orbit plane, DEGREES.", D, None, "0", False, None),
    "orbit_ascendingnode": ("Where the orbit crosses the reference plane going north, DEGREES.",
                            D, None, "0", False, None),
    "orbit_longofpericenter": ("Direction of closest approach, measured from the reference "
                               "direction, DEGREES. `ell_orbit` only.", D, None, "0", False,
                               "See `orbit_argofpericenter` -- the comet spelling is a "
                               "DIFFERENT angle."),
    "orbit_argofpericenter": ("Direction of closest approach, measured from the ascending "
                              "node, DEGREES. `comet_orbit` only.", D, None, "0", False,
                              "NOT interchangeable with `ell_orbit`'s "
                              "`orbit_longofpericenter`: the two differ by the ascending node "
                              "(orbit_creator_cor.cpp:85 vs :219). Copying a value between an "
                              "ell_orbit and a comet_orbit section under the obviously "
                              "corresponding name is wrong by that angle."),
    "orbit_meanlongitude": ("Where the body is on its orbit at the epoch, measured from the "
                            "reference direction, DEGREES. `ell_orbit` only.", D, None, "0",
                            False,
                            "Offset from `comet_orbit`'s `orbit_meananomaly` by the longitude "
                            "of pericentre (orbit_creator_cor.cpp:86)."),
    "orbit_meananomaly": ("Where the body is on its orbit at the epoch, measured from "
                          "pericentre, DEGREES. `comet_orbit` only.", D, None,
                          "treated as absent", False,
                          "Required TOGETHER with `orbit_epoch` when `orbit_timeatpericenter` "
                          "is absent (orbit_creator_cor.cpp:207-210)."),
    "orbit_meanmotion": ("How fast the body goes round, DEGREES PER DAY. `comet_orbit` only; "
                        "an alternative to `orbit_period`, and it wins when both are present.",
                        D, None, "treated as absent", False,
                        "orbit_creator_cor.cpp:178,195,199."),
    "orbit_pericenterdistance": ("Closest approach distance, ASTRONOMICAL UNITS. `comet_orbit` "
                                 "only.", D, None, "treated as absent", False,
                                 "Required unless `orbit_semimajoraxis` is given. The absence "
                                 "test is `<= 0.0` (orbit_creator_cor.cpp:158), so an explicit "
                                 "0 is indistinguishable from not writing the key."),
    "orbit_timeatpericenter": ("Julian day of a pericentre passage. `comet_orbit` only, and "
                               "the alternative to giving `orbit_epoch` + `orbit_meananomaly`.",
                               D, None, "treated as absent", False,
                               "orbit_creator_cor.cpp:202,213."),
    "orbit_x": ("Fixed position, X, for a body that does not move.", D, None, "0", False, None),
    "orbit_y": ("Fixed position, Y, for a body that does not move.", D, None, "0", False, None),
    "orbit_z": ("Fixed position, Z, for a body that does not move.", D, None, "0", False, None),
    "orbit_lon": ("Longitude on the parent's surface, DEGREES.", D, None, "0", False, None),
    "orbit_lat": ("Latitude on the parent's surface, DEGREES.", D, None, "0", False, None),
    "orbit_alt": ("Altitude above the parent's surface.", D, None, "0", False, None),
    "parent_rot_obliquity": ("The parent's obliquity, in RADIANS, used only when `parent` "
                             "names nothing that exists.", D, None, "0", False,
                             "RADIANS, not degrees -- every `orbit_*` angle in the same "
                             "section is degrees, these three are not "
                             "(orbit_creator_cor.cpp:50,125 apply no conversion; the writer "
                             "at orbit.cpp:608 emits them raw while orbit.cpp:597 converts "
                             "`orbit_inclination`). IGNORED whenever `parent` resolves: the "
                             "values then come from the parent body itself, so editing this "
                             "on a body with a real parent changes nothing and says nothing "
                             "(orbit_creator_cor.cpp:49,124)."),
    "parent_rot_asc_node": ("The parent's ascending node, in RADIANS, used only when "
                            "`parent` names nothing that exists.", D, None, "0", False,
                            "RADIANS; ignored when `parent` resolves. See "
                            "`parent_rot_obliquity`."),
    "parent_rot_J2000_longitude": ("The parent's J2000 longitude, in RADIANS, used only "
                                   "when `parent` names nothing that exists.", D, None, "0",
                                   False,
                                   "RADIANS; ignored when `parent` resolves. See "
                                   "`parent_rot_obliquity`."),
    "body_A": ("English name of the first body of a barycentre pair -- the point the "
               "position is measured FROM. `coord_func = barycenter` only.", S, None, None,
               True,
               "REQUIRED: empty, or naming no loaded body, drops the body "
               "(orbit_creator_cor.cpp:291-302). The shipped anchor.ini states an intended "
               "rule the code does NOT enforce -- that `parent` and `body_A` should be the "
               "same body for the orbit to mean anything."),
    "body_B": ("English name of the second body of a barycentre pair -- the point the "
               "position is measured TOWARDS. `coord_func = barycenter` only.", S, None, None,
               True, "REQUIRED; see `body_A`."),
    "a": ("Weight of `body_A` in the barycentre split. `coord_func = barycenter` only.",
          D, None, None, True,
          "REQUIRED: an empty value drops the body (orbit_creator_cor.cpp:286-289). Only the "
          "RATIO is observable -- the position is body_A + (body_B - body_A) * b/(a+b) "
          "(orbit.cpp:1159) -- so a = b = 1 means the midpoint. Parsed by a BARE std::stod "
          "(orbit_creator_cor.cpp:304), not the guarded helper every other numeric key uses, "
          "so a non-empty non-numeric value throws out of the loader."),
    "b": ("Weight of `body_B` in the barycentre split. `coord_func = barycenter` only.",
          D, None, None, True, "REQUIRED; see `a`."),
    # ---- comet tails ---------------------------------------------------------
}

# The three tail families are generated: nine keys each, identical in shape and
# differing only in their defaults, and writing them out by hand is how a table
# acquires a copy-paste error.
TAIL_DEFAULTS = {
    "gaz":   {"trace_jd": "1",  "ejection_force": "30",  "ejection_linearity": "1",
              "radius_xx_coef": "-1", "radius_x_coef": "0.5", "radius_base_coef": "2",
              "color_red": "0.3", "color_green": "0.3", "color_blue": "0.7"},
    "dust":  {"trace_jd": "30", "ejection_force": "0.5", "ejection_linearity": "1",
              "radius_xx_coef": "-2", "radius_x_coef": "1",   "radius_base_coef": "2",
              "color_red": "0.5", "color_green": "0.5", "color_blue": "0.5"},
    "extra": {"trace_jd": "30", "ejection_force": "0.5", "ejection_linearity": "1",
              "radius_xx_coef": "-2", "radius_x_coef": "1",   "radius_base_coef": "2",
              "color_red": "0.5", "color_green": "0.5", "color_blue": "0.5"},
}
TAIL_DOC = {
    "trace_jd": "How many days of tail are traced.",
    "ejection_force": "How hard material leaves the nucleus.",
    "ejection_linearity": "How straight the ejection is.",
    "radius_xx_coef": "Quadratic term of the tail's radius profile.",
    "radius_x_coef": "Linear term of the tail's radius profile.",
    "radius_base_coef": "Constant term of the tail's radius profile.",
    "color_red": "Red component of the tail's colour.",
    "color_green": "Green component of the tail's colour.",
    "color_blue": "Blue component of the tail's colour.",
}
for fam, defs in TAIL_DEFAULTS.items():
    for suffix, dflt in defs.items():
        note = ("Read only when BOTH `apparent_magnitude` and `slope` are present "
                "(protosystem.cpp:838).")
        if fam == "extra":
            note += " The extra tail also needs `extra_tail_trace_jd` to be present at all."
        KEYS["%s_tail_%s" % (fam, suffix)] = (
            "%s (%s tail)" % (TAIL_DOC[suffix], fam), F, None, dflt, False, note)

# Keys the census finds but that are NOT stellar-system-file authoring surface.
# Each says which grammar it really belongs to, so "not in the contract" is a
# recorded answer instead of an omission.
# Only keys the census actually READS here belong in this table -- an entry that
# never fires is a claim nobody checks, which is the class this whole file exists
# to avoid. `x`/`y`/`z` are galactic.ini keys too, but in these loaders they occur
# only as WRITES (ssystem_factory.cpp:747-749), so they are named in the prose
# below rather than listed as exclusions that never happen.
NOT_OURS = {
    "system": "galactic.ini -- names a stellar_systems/*.ini file for one system; read at "
              "ssystem_factory.cpp:736-737 inside loadSystem, whose params come from "
              "galactic.ini (opened at ssystem_factory.cpp:619). Not a body-section key.",
}
NEIGHBOURING_GRAMMARS = {
    "galactic.ini": "the system list: hip, name, system, x, y, z. Its x/y/z reach these "
                    "loaders only as values written into a synthesized orbit map "
                    "(ssystem_factory.cpp:334-336, :747-749), never read from a body section.",
    "anchor.ini": "the camera-anchor grammar (type = point|body|observatory|orbit), read by "
                  "AnchorManager; it SHARES the barycenter orbit keys a/b/body_A/body_B "
                  "through the same creator chain. B4's file, out of this contract's scope.",
    "config.ini": "moon_scale / sun_scale and the viewing mode; not a body file.",
}


def load_sites():
    out = subprocess.check_output(
        [sys.executable, os.path.join(HERE, "f80_census.py"), "sites"], text=True)
    rows = []
    for line in out.splitlines()[1:]:
        f, n, k, form, kind, regime = line.split("\t")
        rows.append({"file": "src/" + f, "line": int(n), "key": k,
                     "form": form, "kind": kind, "regime": regime})
    return rows


def build():
    sites = load_sites()
    reads = [s for s in sites if s["kind"] == "read"]
    for key, f, n, form, regime in EXTRA_SITES:
        reads.append({"file": f, "line": n, "key": key, "form": form,
                      "kind": "read", "regime": regime})

    specials = special_names()
    bykey = {}
    for s in reads:
        bykey.setdefault(s["key"], []).append(s)

    # The count gate that cannot be satisfied by silence: every key the census
    # reads must be dispositioned here, and every disposition must correspond to
    # a key the census reads. Both directions, because either alone can rot.
    stale_exclusions = sorted(set(NOT_OURS) - set(bykey))
    if stale_exclusions:
        sys.stderr.write("ERROR: excluded keys no census site reads: %s\n"
                         % ", ".join(stale_exclusions))
        return None
    undisposed = sorted(set(bykey) - set(KEYS) - set(NOT_OURS))
    unused = sorted(set(KEYS) - set(bykey))
    if undisposed:
        sys.stderr.write("ERROR: census keys with no disposition: %s\n" % ", ".join(undisposed))
        return None
    if unused:
        sys.stderr.write("ERROR: dispositioned keys no census site reads: %s\n" % ", ".join(unused))
        return None

    keys = {}
    for k in sorted(bykey):
        if k in NOT_OURS:
            continue
        doc, vtype, domain, dflt, required, notes = KEYS[k]
        ss = sorted(bykey[k], key=lambda s: (s["file"], s["line"]))
        regimes = sorted({s["regime"] for s in ss})
        entry = {
            "doc": doc,
            "value_type": vtype,
            "default": dflt,
            "required": required,
            "regimes": regimes,
            "source": ["%s:%d" % (s["file"], s["line"]) for s in ss],
        }
        if domain == "@coord_func":
            domain = {
                "intercepted_before_the_chain": COORD_FUNC_INTERCEPTED,
                "handled_by_the_creator_chain": COORD_FUNC_CHAIN,
                "special_ephemerides": specials,
                "composed_path_only": COORD_FUNC_COMPOSED_ONLY,
            }
        if domain is not None:
            entry["domain"] = domain
        if notes:
            entry["notes"] = notes
        keys[k] = entry

    onlylegacy = sorted(k for k, v in keys.items() if v["regimes"] == ["legacy"])
    onlycomposed = sorted(k for k, v in keys.items() if v["regimes"] == ["composed"])

    g = {
        "_meta": {
            "contract": "stellar-system-file",
            "contract_note":
                "THE DISCRIMINATOR (F80 mandate (2)): one field answers 'which contract is "
                "this' for every consumer, so a reader never has to guess from a filename or "
                "a shape. `sc-grammar.json` carries `contract = command-surface`. A tool "
                "handed the wrong file says so instead of finding no commands and reporting "
                "an empty grammar.",
            "schema_version": 1,
            "seeded":
                "2026-09-04, F80 -- generated by claude/harness/f80_ssgrammar.py from the "
                "loader census (claude/harness/f80_census.py). Hand-authored per key: the doc "
                "line, the value domain, the default and REQUIRED/optional, each read at the "
                "site the `source` array names. Mechanical: every anchor and every count.",
            "anchor_pin":
                ANCHOR_PIN + " -- every `file:line` anchor in this file OUTSIDE `_meta` "
                "resolves at this commit. Gate: `anchor_gate` (ctest).",
            "authority_chain": [
                "src/bodyModule/protosystem.cpp -- ProtoSystem::addBody, the LEGACY reader "
                "and the frozen comparison baseline (INTENT S11.52(b))",
                "src/experimentalModule/ModularSystem.cpp -- ModularSystem::loadBody, the ONE "
                "composed authority (INTENT S11.78(d))",
                "src/bodyModule/orbit_creator_cor.cpp -- the coord_func families",
                "src/tools/ini_line.hpp -- the .ini LINE grammar for every reader but the "
                "legacy one (INTENT S11.115(b))",
            ],
            "decisions":
                "D16-D19 are RATIFIED, not proposed [vixy 2026-07-23, DECISIONS_PENDING.md:"
                "149-164 -> INTENT S11.79(j)-(m)]; D16 was ratified WITH AN OVERRIDE ('use "
                "type= instead of declare=') and the respell LANDED (INTENT S11.89, code "
                "f911646b): ONE `type=` key carries the declaration kind, `declare=` and "
                "`module=` are retired. The B28 'sign-off pending' state is DISCHARGED.",
            "transitional":
                "S11.89(c), the second veto point, VERBATIM: 'transitional "
                "body-type-under-`type=` on nodes (full `type=BODY` needs Tier-B emission "
                "beyond B25-emit's A1/A2)'. So a composed NODE today carries the legacy "
                "body type under `type=` (`type = Sun`), and `type = BODY` is the canonical "
                "marker for a body-type-less node. Moving every node to `type = BODY` is "
                "Vixy's decision, not a default -- and in a LEGACY file `BODY` is not a "
                "body type at all, which is why the legacy `type` domain below excludes it.",
            "counts_note":
                "Every count here is DERIVED by the generator from the census, never "
                "asserted. `seed_gate`'s ss half re-derives them from the data on every run.",
            "expected_counts": {
                "keys": len(keys),
                "read_sites": len([s for s in reads if s["key"] in keys]),
                "legacy_only_keys": len(onlylegacy),
                "composed_only_keys": len(onlycomposed),
                "both_regimes_keys": len(keys) - len(onlylegacy) - len(onlycomposed),
            },
            "regime_split": {
                "legacy_only": onlylegacy,
                "composed_only": onlycomposed,
            },
            "not_this_contract": NOT_OURS,
            "neighbouring_grammars": NEIGHBOURING_GRAMMARS,
        },
        "value_types": VALUE_TYPES,
        "regimes": {
            "legacy": {
                "doc": "`ssystem.ini` and `data/default_ssystem.ini`: the frozen field "
                       "format. Selected by FILE LOCATION, never by sniffing content.",
                "reader": "src/bodyModule/protosystem.cpp:121-140 (ProtoSystem::load)",
                "comment_rule":
                    "A '#' is a comment ONLY in column 0 (protosystem.cpp:134). A '#' "
                    "anywhere else is read as part of the key or the value. The rest of "
                    "the .ini family goes through tools/ini_line.hpp, which treats '#' as a "
                    "comment ANYWHERE on the line (ini_line.hpp:16) -- so a mid-line '#' in "
                    "a legacy file means different things to the two readers of that same "
                    "file (the INTENT S5.39 class, on the comment axis).",
                "line_rule":
                    "`key = value`, split at the first '='; the legacy reader uses "
                    "substr(0,pos-1)/substr(pos+2), which assumes exactly ONE space each "
                    "side of the '=' (protosystem.cpp:135-137). A line shorter than 2 bytes "
                    "is skipped. Keys are CASE-SENSITIVE: no reader lowercases them.",
                "section_rule":
                    "`[name]` opens a body section. A body is flushed when the NEXT header "
                    "is seen, which is why the file ends with a `[end]` sentinel -- it is "
                    "load-bearing and is not a body.",
                "forbidden":
                    "New-format constructs must never be written into a legacy file: an "
                    "older build must still read it (INTENT S2.0 D13, scedit C4).",
            },
            "composed": {
                "doc": "`~/.spacecrafter/modularSystem/<Name>.ini`, the B24 format. The "
                       "`.ini.disabled` twin is machine-owned and regenerated at every "
                       "legacy load; adopting it means dropping the extension.",
                "reader": "src/experimentalModule/ModularSystem.cpp:1037 (loadBody)",
                "comment_rule": "A '#' starts a comment anywhere on the line "
                                "(src/tools/ini_line.hpp:16).",
                "section_kinds": {
                    "node": "A body. Carries no `body =`.",
                    "module": "A BodyModule attached to a node. Carries `body = <node>`, and "
                              "its `type =` names the module FAMILY, not a body type. The "
                              "presence of `body =` is the ONLY discriminator "
                              "(src/experimentalModule/ModularSystem.cpp:1615-1626).",
                },
            },
        },
        "families": {
            "body_types": {
                "doc": "The `type =` domain in a LEGACY section, and on a composed NODE.",
                "names": KEYS["type"][2],
                "source": "src/bodyModule/protosystem.cpp:483-501",
                "matching": "first four bytes; see the `type` key's notes",
            },
            "coord_func": {
                "doc": "The `coord_func =` domain: which orbit family positions the body.",
                "intercepted_before_the_chain": COORD_FUNC_INTERCEPTED,
                "handled_by_the_creator_chain": COORD_FUNC_CHAIN,
                "special_ephemerides": specials,
                "composed_path_only": COORD_FUNC_COMPOSED_ONLY,
                "count": len(COORD_FUNC_INTERCEPTED) + len(COORD_FUNC_CHAIN)
                         + len(specials) + len(COORD_FUNC_COMPOSED_ONLY),
                "source": ["src/bodyModule/protosystem.cpp:561-618",
                           "src/bodyModule/protosystem.cpp:65-68",
                           "src/bodyModule/orbit.cpp:770-901"],
            },
        },
        "keys": keys,
    }
    return g


def main():
    out = OUT
    argv = sys.argv[1:]
    if "--out" in argv:
        out = argv[argv.index("--out") + 1]
    g = build()
    if g is None:
        return 1
    text = json.dumps(g, indent=2, ensure_ascii=True, sort_keys=False) + "\n"
    # D14: the contract is pure ASCII by construction (ensure_ascii above).
    with open(out, "w", encoding="ascii") as fh:
        fh.write(text)
    c = g["_meta"]["expected_counts"]
    sys.stderr.write("wrote %s\n  keys %d  read_sites %d  legacy-only %d  composed-only %d  both %d\n"
                     % (out, c["keys"], c["read_sites"], c["legacy_only_keys"],
                        c["composed_only_keys"], c["both_regimes_keys"]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
