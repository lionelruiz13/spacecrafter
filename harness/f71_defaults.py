#!/usr/bin/env python3
"""F71 item 11: the `default_value` backfill, as DATA.

WHAT THIS IS.  scedit's editor arms D31's greyed-default ghost from an explicit
`default_value` string in an argument spec (`util/scedit/src/sc_docindex.cpp:89`,
`sc_editcore.cpp:576`).  Until now no spec carried one: all 324 state their
default as an English SENTENCE, and scedit's constraint C2 forbids guessing a
literal out of prose.  This script writes the literals that were verified AT
SOURCE, one by one, into both the merged contract and the extraction fragment
that owns each entry (the validator compares them -- see f71_patch.py).

THE CRITERION EVERY ROW HAD TO PASS -- stated so it can be argued with, and so a
row can be shown wrong.  A `default_value` is written only when BOTH hold:

  (1) EQUIVALENCE.  Typing `<key> <literal>` produces exactly the same engine
      behaviour as omitting the key.  This is what the ghost promises: it is
      offered as a completion, so accepting it puts those bytes in the buffer.
  (2) NON-MISLEADING AT THE ZERO-KNOWLEDGE BAR (C6).  The literal must not
      imply membership in a recognised vocabulary that does not exist.

Rule (2) is what excludes the false side of every boolean.  `Utility::isTrue`
accepts EXACTLY "TRUE"/"ON"/"1" case-folded and calls everything else false
(`src/tools/utility.hpp:160-171`); `Utility::strToBool` accepts EXACTLY
"true"/"1" lowercased (`src/tools/utility.cpp:432-437`).  So a TRUE default has
a real spelling to offer and a FALSE default has none -- any literal offered for
the false side teaches a word the engine does not know.  `camera value` is the
sharp case: its prose default reads `absent -> no`, and `no` does satisfy (1),
but the handler is a bare `valueStr == W_TRUE` string compare
(`app_command_interface.cpp:4572`), so offering `no` would teach the author that
`no` is a word here -- and `yes` would then read as true, which it does not.
Excluded, deliberately.  The prose default stays; only the machine-readable
literal is withheld.

Also excluded, each for a reason that is itself a fact about the engine:
  * `landscape fov`      - conditional (`fov` falls back to `texturefov`, then
                           180): no single literal reproduces absence.
  * `font size`          - the 12 is applied AFTER the screen scaling, not
                           before (`fontFactory.cpp:144-146`: size =
                           strToDouble(v) * fontFactor, and only THEN `if
                           (size == 0) size = 12`).  Typing `size 12` yields
                           12 * fontFactor, which is not what omitting it does
                           unless fontFactor happens to be 1.  Fails (1).
  * per-branch defaults  - `camera duration`, `zoom duration` (5 on the
                           `center` form, app_command_interface.cpp:3422),
                           `timerate step`, `dso3d color_depth`, `media speed`,
                           `look_at duration`, `camera filename`: the default
                           depends on which form the line takes, so one literal
                           in the spec would be wrong on the other branches.

EVERY LINE NUMBER BELOW WAS READ AT CODE master-beta @ d64fd437.  This matters:
the `source` anchors already in the grammar are pinned by `_meta.merged` to
`b12c8cdd`, and `app_command_interface.cpp` has grown 4747 -> 4888 lines since,
so those anchors no longer resolve at HEAD.  Rows written here carry their own
`default_value_source` with the sha in it, so they are self-pinning.
"""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from f71_patch import patch_file   # noqa: E402

import json   # noqa: E402

ROOT = "/home/claude/spacecrafter/util/scedit/grammar"
HEAD = "d64fd437"

# The zero that is not written anywhere: an unguarded `args[KEY]` lookup yields
# "" for an absent key, evalDouble/evalInt fall through to Utility::strToDouble,
# whose body is `try { return std::stod(str); } catch (...) { return
# default_value; }` with default_value = 0.
EVAL0 = ("app_command_interface.cpp:%s @ " + HEAD + " (unguarded args[] lookup) -> "
         "app_command_eval.cpp:116-133 -> src/tools/utility.cpp:399-406 "
         "(strToDouble catches and returns default_value 0, utility.hpp:143)")

# dsoNavigator declares a local, initialises it, and only overwrites it when the
# key is present: `#define EXTRACT(var, key) it = args.find(key); if (it !=
# args.end()) var = std::stof(it->second)` (dsoNavigator.cpp:264-265).
NAV = "src/inGalaxyModule/dsoNavigator.cpp:%s @ " + HEAD + " (initialiser; EXTRACT/IEXTRACT overwrite only when the key is present, :264-265)"

LAND = "src/coreModule/landscape.cpp:%s @ " + HEAD

# (command, key, literal, source)
ROWS = [
    # --- the unguarded-evalDouble/evalInt zeros -----------------------------
    ("body", "pos_x", "0", EVAL0 % "4087"),
    ("body", "pos_y", "0", EVAL0 % "4087"),
    ("body", "pos_z", "0", EVAL0 % "4087"),
    ("body", "scale", "0", EVAL0 % "4089"),
    ("body", "value", "0", EVAL0 % "4270"),
    ("color", "index", "0", EVAL0 % "2014"),
    ("dso", "ra", "0", EVAL0 % "1577"),
    ("dso", "de", "0", EVAL0 % "1577"),
    ("dso", "magnitude", "0", EVAL0 % "1577"),
    ("dso", "angular_size", "0", EVAL0 % "1578"),
    ("dso", "rotation", "0", EVAL0 % "1578"),
    ("dso", "texture_luminance_adjust", "0", EVAL0 % "1579"),
    ("dso", "distance", "0", EVAL0 % "1580"),
    ("dso2d", "index", "0", EVAL0 % "1670"),
    ("dso2d", "size", "0", EVAL0 % "1670"),
    ("dso2d", "ra", "0", EVAL0 % "1670"),
    ("dso2d", "de", "0", EVAL0 % "1670"),
    ("dso2d", "distance", "0", EVAL0 % "1670"),
    ("dso2d", "xyz", "0", EVAL0 % "1670"),
    ("heading", "duration", "0", EVAL0 % "2673, 2679"),
    ("illuminate", "rotation", "0", EVAL0 % "2025"),
    ("illuminate", "size", "0", EVAL0 % "2024"),
    ("image", "duration", "0", EVAL0 % "3120, 3140-3187"),
    ("media", "value", "0", EVAL0 % "3874"),
    ("moveto", "duration", "0", EVAL0 % "3587"),

    # --- explicit ternary with the literal in it ----------------------------
    ("meteors", "ra", "0",
     "app_command_interface.cpp:2705 @ " + HEAD + " (`!args[W_RA].empty() ? evalDouble(args[W_RA]) : 0.`)"),
    ("meteors", "de", "0",
     "app_command_interface.cpp:2706 @ " + HEAD + " (`!args[W_DE].empty() ? evalDouble(args[W_DE]) : 0.`)"),

    # --- local initialisers behind a presence guard (dsoNavigator) ----------
    ("dso3d", "index", "0", NAV % "273-274"),
    ("dso3d", "pos_x", "0", (NAV % "269, 296") + " -> src/tools/vecmath.hpp:182 (`T v[3]{}` value-initialises, so a bare `Vec3f position;` is zeroed)"),
    ("dso3d", "pos_y", "0", (NAV % "269, 296") + " -> src/tools/vecmath.hpp:182"),
    ("dso3d", "pos_z", "0", (NAV % "269, 296") + " -> src/tools/vecmath.hpp:182"),
    ("dso3d", "yaw", "0", NAV % "270, 297"),
    ("dso3d", "pitch", "0", NAV % "270, 297"),
    ("dso3d", "roll", "0", NAV % "270, 297"),
    ("dso3d", "xscale", "1", NAV % "271, 298"),
    ("dso3d", "yscale", "1", NAV % "271, 298"),
    ("dso3d", "zscale", "1", NAV % "271, 298"),
    ("dso3d", "scale", "1", NAV % "272, 299"),
    ("dso3d", "rate", "0", NAV % "302, 307"),
    ("dso3d", "color_depth_column", "0", NAV % "301, 320"),

    # --- literal passed to the converter, or assigned on the empty branch ---
    ("body", "keep_time", "10",
     "app_command_interface.cpp:4157-4158 @ " + HEAD + " (`Utility::strToInt(kt, 10)`) -> src/tools/utility.cpp:459-461 (returns the default on an empty string)"),
    ("landscape", "limited_shade", "0",
     (LAND % "186-189") + " (`float limitedShadeValue = 0;`, overwritten only when the key is non-empty)"),
    ("landscape", "texturefov", "180",
     (LAND % "203") + " (`Utility::strToDouble(param[\"texturefov\"], 180)`)"),
    ("landscape", "rotate_z", "0",
     (LAND % "204, 211") + " (`Utility::strToDouble(param[\"rotate_z\"], 0.)`, both the fisheye and the spherical branch)"),
    ("landscape", "base_altitude", "-90",
     (LAND % "210") + " (`Utility::strToDouble(param[\"base_altitude\"], -90)`; spherical scenery only)"),
    ("landscape", "top_altitude", "90",
     (LAND % "211") + " (`Utility::strToDouble(param[\"top_altitude\"], 90)`; spherical scenery only)"),
    ("landscape", "mipmap", "on",
     (LAND % "197") + " (`param[L_MIPMAP].empty() ? true : Utility::isTrue(param[L_MIPMAP])`); `on` is one of the three spellings isTrue accepts (src/tools/utility.hpp:160-171), so it reproduces the absent behaviour without teaching a word the engine does not know"),
    ("personal", "filename", "personal.txt",
     "app_command_interface.cpp:1540-1541 @ " + HEAD + " (`if (fileName.empty()) fileName = \"personal.txt\";`)"),
    ("personeq", "filename", "personeq.txt",
     "app_command_interface.cpp:1689-1690 @ " + HEAD + " (`if (fileName.empty()) fileName = \"personeq.txt\";`)"),
    ("search", "maxobject", "5",
     "app_command_interface.cpp:1461-1464 @ " + HEAD + " (empty takes the one-argument overload) -> src/coreModule/core.hpp:345 (`unsigned int maxNbItem=5`)"),
    ("session", "filename", "session",
     "src/experimentalModule/SessionFile.cpp:45 @ " + HEAD + " (`name.empty() ? DEFAULT_NAME : name`) -> SessionFile.hpp:207 (`DEFAULT_NAME = \"session\"`)"),
    ("image", "intensity", "0.05",
     "app_command_interface.cpp:3214-3218 @ " + HEAD + " (empty takes the one-argument overload) -> src/mediaModule/media.hpp:299 (`float intensity = 0.05`)"),
    ("media", "intensity", "0.05",
     "app_command_interface.cpp:3827-3842 @ " + HEAD + " (empty takes the one-argument overload) -> src/mediaModule/media.hpp:128, 299 (`float intensity = 0.05`)"),
    ("transition", "duration", "3600",
     "app_command_interface.cpp:4872-4874 @ " + HEAD + " (`float duration = evalDouble(args[W_DURATION]); if (!duration) duration = 3600;`)"),

    # --- not-found falls back to a named member of the value domain ---------
    ("body", "type", "Asteroid",
     "src/bodyModule/protosystem.cpp:542 @ " + HEAD + " (log: \"No valid body type specified ..., assume 'Asteroid'\"); the accepted spelling is protosystem.cpp:493 `CASE(\"Asteroid\", ASTEROID)`"),
    ("dso", "type", "GENRC",
     "src/coreModule/core.cpp:2564-2566 @ " + HEAD + " (`if (tmp_type == \"\") tmp_type = \"GENRC\";`)"),
    ("image", "coordinate_system", "viewport",
     "app_command_interface.cpp:3088 @ " + HEAD + " -> src/mediaModule/image_mgr.cpp:73-82 (lowercases, and an unknown or empty name returns POS_VIEWPORT)"),
    ("media", "position", "viewport",
     "src/mediaModule/image_mgr.cpp:73-82 @ " + HEAD + " (lowercases, and an unknown or empty name returns POS_VIEWPORT)"),
    ("text", "align", "LEFT",
     "src/mediaModule/text_mgr.cpp:109-116 @ " + HEAD + " (`TEXT_ALIGN textAlign = TEXT_ALIGN::LEFT;`, consulted only when the key is non-empty); the map is case-sensitive and holds \"LEFT\" (:47)"),
    ("text", "size", "MEDIUM",
     "src/mediaModule/text_mgr.cpp:71-78 @ " + HEAD + " (`FONT_SIZE textSize = FONT_SIZE::T_MEDIUM;`, consulted only when the key is non-empty); the map is case-sensitive and holds \"MEDIUM\" (:42)"),
]


def main():
    merged_path = os.path.join(ROOT, "sc-grammar.json")
    with open(merged_path, encoding="utf-8") as f:
        g = json.load(f)
    cmds = g["families"]["commands"]

    # Group the edits by the file that owns them: the merged contract always,
    # plus the extraction fragment named by the command's own `unit` field.
    by_file = {merged_path: []}
    for cn, key, value, src in ROWS:
        if cn not in cmds:
            raise SystemExit("no such command: " + cn)
        if key not in (cmds[cn].get("args") or {}):
            raise SystemExit("no such key: %s.%s" % (cn, key))
        unit = cmds[cn]["unit"]
        frag = os.path.join(ROOT, "args", "unit-%d.json" % unit)
        by_file.setdefault(frag, [])
        path_merged = ["families", "commands", cn, "args", key]
        path_frag = ["commands", cn, "args", key]
        by_file[merged_path].append((path_merged, "default", "default_value", value))
        by_file[merged_path].append((path_merged, "default_value", "default_value_source", src))
        by_file[frag].append((path_frag, "default", "default_value", value))
        by_file[frag].append((path_frag, "default_value", "default_value_source", src))

    total = 0
    for path, edits in sorted(by_file.items()):
        n = patch_file(path, edits)
        total += n
        print("%-52s %3d field insertions" % (os.path.basename(path), n))
    print("rows: %d, field insertions: %d" % (len(ROWS), total))


if __name__ == "__main__":
    main()
