#!/usr/bin/env python3
"""f58_census.py — enumerate the diagnostic-emission sites of the F58 universe.

The universe is the §11.169 audit's operational boundary (fable-dispatch F58 scope 1):
the mandatory scope floor of §11.169(b) is USER-FACING surfaces — script errors and
data errors.  Operationally that is

  arm A — the script/TCP command surface: the files a script line or a TCP command
          travels through from arrival to refusal/acceptance;
  arm B — the five NAMED data/config loader families (ssystem, config, star, nebula,
          sky-culture): the files that read the field's data and config files;
  arm C — the ledger-banked instances (§11.170(f), the illuminate clamp, §5.115).

Everything else is OUT and stated as such: internal/debug channels (renderer, Vulkan,
shader, EntityCore), the media/UI/ojm subsystems, and the data loaders OUTSIDE the five
named families (landscape, milkyway, tully, dso3d, in-galaxy star manager/navigator,
starLines, textures, fonts) — those are the named-not-swept remainder, inventoried by
this same instrument under --adjacent so the follow-on is sized rather than guessed.

The instrument is DELIBERATELY syntactic and complete over its file list: it finds every
emission site by construction, so the boundary is checkable by re-running it.  It cannot
find SILENT sites — no grep finds an absent line — which is why the silent half of the
audit is enumerated by a stated read method and carries its own boundary note.

Channels enumerated (the three a user can actually read):
  * cLog::get()->write(...)            — the log files (script/internal/tcp/...)
  * debug_message = ... / +=           — the script command surface's refusal channel,
                                         emitted by AppCommandInterface::executeCommandStatus
  * std::cerr << ... / std::cout << ...— the pre-log / fatal console channel

Usage:  python3 f58_census.py [--root <code repo>] [--adjacent] [--tsv]
Default output is TSV on stdout (header + one row per site).
"""
import argparse
import os
import re
import sys

# ---------------------------------------------------------------------------
# The universe, file by file, with its arm and family.  This list IS the
# boundary: adding or removing a file here changes the audit's scope, so it is
# committed with the entry and cited by it.
# ---------------------------------------------------------------------------
UNIVERSE = [
    # arm A — script/TCP command surface
    ("A", "command",     "src/interfaceModule/app_command_interface.cpp"),
    ("A", "command",     "src/interfaceModule/app_command_eval.cpp"),
    ("A", "command",     "src/interfaceModule/app_command_init.cpp"),
    ("A", "script",      "src/scriptModule/script.cpp"),
    ("A", "script",      "src/scriptModule/script_mgr.cpp"),
    ("A", "script",      "src/scriptModule/script_interface.cpp"),
    ("A", "tcp",         "src/tools/io.cpp"),
    # the TCP/mkfifo command INTAKE: App::updateFromSharedData is literally the
    # "TCP command handling" arm's entry point, so the arm is incomplete without it
    # (the dispatch's "app_command_interface/app_command_eval" is a naming example,
    # not an exhaustive file list).  app.cpp also consumes config, so its acting
    # defaults belong to arm B's question as well; it is listed once, under A.
    ("A", "intake",      "src/appModule/app.cpp"),
    ("A", "intake",      "src/appModule/mkfifo.cpp"),
    # arm B — the five named data/config loader families
    ("B", "ssystem",     "src/bodyModule/solarsystem.cpp"),
    ("B", "ssystem",     "src/bodyModule/ssystem_factory.cpp"),
    ("B", "ssystem",     "src/bodyModule/protosystem.cpp"),
    ("B", "config",      "src/mainModule/checkConfig.cpp"),
    ("B", "config",      "src/tools/init_parser.cpp"),
    ("B", "config",      "src/tools/app_settings.cpp"),
    ("B", "star",        "src/starModule/hip_star_mgr.cpp"),
    ("B", "star",        "src/starModule/zone_array.cpp"),
    ("B", "nebula",      "src/coreModule/nebula_mgr.cpp"),
    ("B", "skyculture",  "src/coreModule/sky_localizer.cpp"),
    ("B", "skyculture",  "src/coreModule/constellation_mgr.cpp"),
    # arm C — ledger-banked instance sites not already covered above
    ("C", "illuminate",  "src/coreModule/illuminate_mgr.cpp"),
    ("C", "logchannel",  "src/tools/log.cpp"),
    ("C", "logchannel",  "src/main.cpp"),   # §5.115's lifecycle: openLog(keepHistory) + the write_log gate
]

# The named-not-swept remainder: data loaders outside the five named families and the
# log-lifecycle owner.  Inventoried, not classified.
ADJACENT = [
    "src/coreModule/landscape.cpp",
    "src/coreModule/milkyway.cpp",
    "src/coreModule/tully.cpp",
    "src/coreModule/starLines.cpp",
    "src/inGalaxyModule/dso3d.cpp",
    "src/inGalaxyModule/starManager.cpp",
    "src/inGalaxyModule/starNavigator.cpp",
    "src/mediaModule/media.cpp",
    "src/mediaModule/audio.cpp",
    "src/mediaModule/text_mgr.cpp",
    "src/mediaModule/video_player.cpp",
    "src/tools/s_texture.cpp",
    "src/tools/s_font.cpp",
    "src/tools/call_system.cpp",
    "src/ojmModule/ojm.cpp",
    "src/ojmModule/ojm_mgr.cpp",
    "src/navModule/anchor_manager.cpp",
    "src/navModule/anchor_creator_cor.cpp",
    "src/bodyModule/orbit_creator_cor.cpp",
    "src/uiModule/ui.cpp",
]

LOGCALL = "cLog::get()->write"


def balanced(text, start):
    """Return (inner, end_index) for the parenthesised group opening at text[start]=='('."""
    depth = 0
    i = start
    instr = False
    esc = False
    while i < len(text):
        c = text[i]
        if instr:
            if esc:
                esc = False
            elif c == "\\":
                esc = True
            elif c == '"':
                instr = False
        else:
            if c == '"':
                instr = True
            elif c == "(":
                depth += 1
            elif c == ")":
                depth -= 1
                if depth == 0:
                    return text[start + 1:i], i
        i += 1
    return text[start + 1:], len(text)


def split_top(arg):
    """Split a C++ argument list on top-level commas.

    `<` and `>` are NOT bracket characters here.  Counting them as such makes every
    `a->b()` in an argument drive the depth negative, after which no comma is ever seen
    as top-level again — measured: 6 sites swallowed their own LOG_TYPE argument into
    the message text and were scored at the default severity instead of the written one
    (`ssystem_factory.cpp:967` is L_ERROR and was read as L_INFO).  A cLog call carrying
    a template argument list with a top-level comma would be the opposite risk; there is
    none in the universe.
    """
    out, depth, cur, instr, esc = [], 0, [], False, False
    for c in arg:
        if instr:
            cur.append(c)
            if esc:
                esc = False
            elif c == "\\":
                esc = True
            elif c == '"':
                instr = False
            continue
        if c == '"':
            instr = True
            cur.append(c)
        elif c in "([{":
            depth += 1
            cur.append(c)
        elif c in ")]}":
            depth -= 1
            cur.append(c)
        elif c == "," and depth == 0:
            out.append("".join(cur).strip())
            cur = []
        else:
            cur.append(c)
    if cur:
        out.append("".join(cur).strip())
    return out


def stmt_end(text, start):
    """Index of the statement-terminating ';' at/after start, skipping string and char
    literals.  A bare text.find(';') truncates any message that CONTAINS a semicolon —
    measured: four `debug_message` sites in app_command_interface.cpp lost their tail."""
    i = start
    instr = char = esc = False
    while i < len(text):
        c = text[i]
        if esc:
            esc = False
        elif c == "\\":
            esc = True
        elif instr:
            if c == '"':
                instr = False
        elif char:
            if c == "'":
                char = False
        elif c == '"':
            instr = True
        elif c == "'":
            char = True
        elif c == ";":
            return i
        i += 1
    return -1


def flat(s):
    return re.sub(r"\s+", " ", s).strip()


def scan_file(root, arm, family, rel):
    path = os.path.join(root, rel)
    with open(path, "r", encoding="utf-8", errors="replace") as fh:
        lines = fh.readlines()
    text = "".join(lines)
    # offset -> line number
    starts = []
    off = 0
    for ln in lines:
        starts.append(off)
        off += len(ln)

    def lineno(pos):
        lo, hi = 0, len(starts) - 1
        while lo < hi:
            mid = (lo + hi + 1) // 2
            if starts[mid] <= pos:
                lo = mid
            else:
                hi = mid - 1
        return lo + 1

    # Byte map of /* ... */ block-comment interiors.  Measured need: checkConfig.cpp:512
    # and :515 are two std::cout sites sitting inside a commented-out block; a `//`-only
    # test reported them LIVE, which would have put two dead sites in the gap table.
    blockcom = bytearray(len(text))
    j = 0
    while True:
        o = text.find("/*", j)
        if o < 0:
            break
        c = text.find("*/", o + 2)
        c = len(text) if c < 0 else c + 2
        for k in range(o, c):
            blockcom[k] = 1
        j = c

    def commented(pos):
        """True if pos is commented out: inside /* */, or with // earlier on its line."""
        if blockcom[pos]:
            return True
        ls = text.rfind("\n", 0, pos) + 1
        return "//" in text[ls:pos]

    sites = []
    # channel 1: cLog writes
    for m in re.finditer(re.escape(LOGCALL) + r"\s*\(", text):
        op = text.index("(", m.end() - 1)
        inner, _ = balanced(text, op)
        args = split_top(inner)
        ln = lineno(m.start())
        sites.append(dict(arm=arm, family=family, file=rel, line=ln,
                          channel="clog",
                          commented=commented(m.start()),
                          text=flat(args[0]) if args else "",
                          level=flat(args[1]) if len(args) > 1 else "LOG_TYPE::L_INFO",
                          sink=flat(args[2]) if len(args) > 2 else "LOG_FILE::INTERNAL"))
    # channel 2: the script surface's refusal channel
    for m in re.finditer(r"debug_message\s*(\+?=)\s*", text):
        end = stmt_end(text, m.end())
        if end < 0:
            continue
        rhs = flat(text[m.end():end])
        if rhs in ("", '""'):
            continue
        ln = lineno(m.start())
        sites.append(dict(arm=arm, family=family, file=rel, line=ln,
                          channel="debug_message",
                          commented=commented(m.start()),
                          text=rhs, level="(via executeCommandStatus L_DEBUG)",
                          sink="LOG_FILE::SCRIPT"))
    # channel 4: the C console channel.  Found mid-audit: zone_array.cpp reports 14
    # star-catalogue faults through printf/fprintf and NOTHING through cLog, so an
    # iostream-only census would have scored that loader as almost silent.
    for m in re.finditer(r"(?<![\w:.>])(printf|fprintf|perror)\s*\(", text):
        op = text.index("(", m.end() - 1)
        inner, _ = balanced(text, op)
        ln = lineno(m.start())
        sites.append(dict(arm=arm, family=family, file=rel, line=ln,
                          channel=m.group(1),
                          commented=commented(m.start()),
                          text=flat(inner),
                          level="(console)", sink="(console)"))
    # channel 3: the C++ console channel
    for m in re.finditer(r"std::(cerr|cout)\s*<<", text):
        end = stmt_end(text, m.end())
        if end < 0:
            continue
        ln = lineno(m.start())
        sites.append(dict(arm=arm, family=family, file=rel, line=ln,
                          channel="std::" + m.group(1),
                          commented=commented(m.start()),
                          text=flat(text[m.end():end]),
                          level="(console)", sink="(console)"))
    sites.sort(key=lambda s: (s["line"], s["channel"]))
    return sites


COLS = ["arm", "family", "file", "line", "channel", "commented", "level", "sink", "text"]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default="/home/claude/spacecrafter")
    ap.add_argument("--adjacent", action="store_true",
                    help="inventory the named-not-swept remainder instead of the universe")
    args = ap.parse_args()

    if args.adjacent:
        print("file\tclog\tdebug_message\tconsole\ttotal")
        gt = 0
        for rel in ADJACENT:
            s = scan_file(args.root, "-", "-", rel)
            live = [x for x in s if not x["commented"]]
            c = sum(1 for x in live if x["channel"] == "clog")
            d = sum(1 for x in live if x["channel"] == "debug_message")
            o = len(live) - c - d
            gt += len(live)
            print("%s\t%d\t%d\t%d\t%d" % (rel, c, d, o, len(live)))
        print("TOTAL\t\t\t\t%d" % gt)
        return

    print("\t".join(COLS))
    total = 0
    for arm, family, rel in UNIVERSE:
        for s in scan_file(args.root, arm, family, rel):
            total += 1
            print("\t".join(str(s[c]) for c in COLS))
    print("# sites: %d (commented-out included; filter commented==False for live)" % total,
          file=sys.stderr)


if __name__ == "__main__":
    main()
