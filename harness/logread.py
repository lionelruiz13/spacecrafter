#!/usr/bin/env python3
"""F108 -- THE LOG CHANNEL'S SINGLE READER (the shape `dumpread.py` has for the
dump channel).

    python3 logread.py selftest                 # 25 checks, both directions
    python3 logread.py <logdir> [channel]       # what this dir holds, now
    python3 logread.py --live <logdir> [chan]   # the live file's path, for sh

WHY IT EXISTS.  Until INTENT Sec.11.230 the five log channels had two different
file layouts: four of them truncated `<channel>.log` at every launch, and the
SCRIPT channel alone appended into a per-day `script-YY.MM.DD.log` that nothing
ever capped (Sec.5.115).  Since Sec.11.230 every channel keeps the last
LOG_RETENTION_LAUNCHES launches by numbered rotation at open -- the current
launch is always `<channel>.log`, the previous ones are `<channel>.1.log` ...
`<channel>.7.log`, and the dated files are legacy: never written, never read
and never deleted by the application.

So a harness reader now has THREE kinds of file to tell apart, and twelve
harness files used to tell them apart by hand.  Nine of them read a log for
real; `sorted(glob("script-*.log"))[-1]` is what six of those nine had, and it
returns NOTHING in the new layout (the glob needs a dash).  The rule lives here
once instead:

  * `live(logdir, channel)` is the file THIS launch wrote.  It is
    `<channel>.log`, except in a directory written by a pre-Sec.11.230 binary,
    where the newest dated file is newer -- landed artifacts keep working
    without an era flag, because mtime answers the question directly.
  * `history` is the live file followed by the numbered archives, newest first.
  * `legacy` is the dated pile, name-sorted (which is date-sorted).
  * the archives are NEVER returned by `live`: a reader that wants the running
    launch must not be handed the one before it.
"""
import os
import re
import sys
from pathlib import Path

CHANNELS = ("spacecrafter", "script", "tcp", "shader", "vulkan")
EXT = ".log"
#: `<channel>.<k>.log`, k >= 1 -- the numbered archives of earlier launches.
ARCHIVE_RE = r"^%s\.([0-9]+)\.log$"
#: `<channel>-YY.MM.DD.log` -- the pre-Sec.11.230 per-day layout (script only).
LEGACY_RE = r"^%s-.+\.log$"


def _mtime(p):
    try:
        return p.stat().st_mtime
    except OSError:
        return -1.0


def legacy(logdir, channel="script"):
    """The dated files of the pre-rotation layout, oldest first.  Name order is
    date order for `YY.MM.DD`, and that is the order the old readers used."""
    d = Path(logdir)
    if not d.is_dir():
        return []
    rx = re.compile(LEGACY_RE % re.escape(channel))
    return sorted((p for p in d.iterdir()
                   if p.is_file() and rx.match(p.name)), key=lambda p: p.name)


def archives(logdir, channel="script"):
    """The numbered archives, NEWEST FIRST (`.1.log` is the previous launch)."""
    d = Path(logdir)
    if not d.is_dir():
        return []
    rx = re.compile(ARCHIVE_RE % re.escape(channel))
    out = []
    for p in d.iterdir():
        m = rx.match(p.name) if p.is_file() else None
        if m:
            out.append((int(m.group(1)), p))
    return [p for _, p in sorted(out)]


def live(logdir, channel="script"):
    """The file the CURRENT launch wrote, or None if the channel has none.

    `<channel>.log` wins unless a legacy dated file is strictly newer, which
    only happens in a directory last written by a pre-Sec.11.230 binary."""
    d = Path(logdir)
    cur = d / (channel + EXT)
    cands = []
    if cur.is_file():
        cands.append((_mtime(cur), 1, cur))          # 1: the live name wins ties
    for p in legacy(d, channel):
        cands.append((_mtime(p), 0, p))
    if not cands:
        return None
    return max(cands)[2]


def history(logdir, channel="script"):
    """The live file then the numbered archives -- the retention window, in
    order, newest first.  Legacy dated files are NOT in it: they are outside
    the window by construction."""
    out = []
    p = live(logdir, channel)
    if p is not None:
        out.append(p)
    out.extend(a for a in archives(logdir, channel) if a != p)
    return out


def text(logdir, channel="script", encoding="latin-1"):
    """The live file's text, or "" -- the two-line idiom the nine readers had.

    latin-1 by default: the log carries whatever byte the engine wrote and the
    readers that came before never wanted a decode error (Sec.11.188)."""
    p = live(logdir, channel)
    if p is None:
        return ""
    return p.read_bytes().decode(encoding, errors="replace")


def window(hpp=None):
    """LOG_RETENTION_LAUNCHES, read from the source of truth rather than
    recalled: an instrument that hard-codes 8 cannot notice the constant
    moving."""
    if hpp is None:
        hpp = Path(__file__).resolve().parent.parent.parent / "src/tools/log.hpp"
    m = re.search(r"LOG_RETENTION_LAUNCHES\s*=\s*([0-9]+)\s*;",
                  Path(hpp).read_text(encoding="latin-1"))
    if not m:
        raise RuntimeError("LOG_RETENTION_LAUNCHES not found in %s" % hpp)
    return int(m.group(1))


# --------------------------------------------------------------- the selftest
def _touch(p, body, mtime):
    p.write_text(body)
    os.utime(p, (mtime, mtime))


def selftest():
    import tempfile
    ok = fail = 0

    def nm(p):
        """The name, or None -- so a rule that returns nothing FAILS a check
        instead of crashing the suite (the mutant's own first run did)."""
        return p.name if p is not None else None

    def check(label, got, want):
        nonlocal ok, fail
        if got == want:
            ok += 1
            print("PASS  %-58s %s" % (label, got))
        else:
            fail += 1
            print("FAIL  %-58s got %r want %r" % (label, got, want))

    with tempfile.TemporaryDirectory() as td:
        d = Path(td)

        # (1) empty directory: every accessor answers, none raises
        check("empty: live", live(d), None)
        check("empty: text", text(d), "")
        check("empty: history", history(d), [])
        check("empty: archives", archives(d), [])
        check("empty: legacy", legacy(d), [])
        check("missing dir: live", live(d / "nope"), None)
        check("missing dir: text", text(d / "nope"), "")

        # (2) the PRE-Sec.11.230 era: dated files only, the newest wins
        _touch(d / "script-26.08.29.log", "old\n", 1000)
        _touch(d / "script-26.08.30.log", "newer\n", 2000)
        check("pre-era: live is the newest dated",
              nm(live(d)), "script-26.08.30.log")
        check("pre-era: text", text(d).strip(), "newer")
        check("pre-era: legacy count", len(legacy(d)), 2)
        check("pre-era: legacy is name-sorted",
              [p.name for p in legacy(d)],
              ["script-26.08.29.log", "script-26.08.30.log"])
        check("pre-era: history is just the live file",
              [p.name for p in history(d)], ["script-26.08.30.log"])

        # (3) the POST-Sec.11.230 era beside the same pile: the live name wins
        _touch(d / "script.log", "running\n", 3000)
        _touch(d / "script.1.log", "previous\n", 2500)
        _touch(d / "script.7.log", "seven back\n", 2100)
        check("post-era: live is script.log", nm(live(d)), "script.log")
        check("post-era: text", text(d).strip(), "running")
        check("post-era: an archive is never live",
              nm(live(d)) != "script.1.log", True)
        check("post-era: history order",
              [p.name for p in history(d)],
              ["script.log", "script.1.log", "script.7.log"])
        check("post-era: archives are index-sorted, not name-sorted",
              [p.name for p in archives(d)], ["script.1.log", "script.7.log"])
        check("post-era: the pile is still outside the window",
              [p.name for p in legacy(d)],
              ["script-26.08.29.log", "script-26.08.30.log"])

        # (4) THE OTHER DIRECTION -- the selection must be able to go the other
        # way: an OLD binary writing into a directory that already holds a
        # script.log from a newer one (a downgrade, Sec.2.0 D13) must read its
        # own dated file, because that file is the newer one.
        _touch(d / "script-26.09.12.log", "after a downgrade\n", 4000)
        check("downgrade: live is the dated file again",
              nm(live(d)), "script-26.09.12.log")

        # (5) the four channels that never had a dated layout
        _touch(d / "spacecrafter.log", "internal\n", 3000)
        _touch(d / "spacecrafter.3.log", "three back\n", 2000)
        check("other channel: live", nm(live(d, "spacecrafter")),
              "spacecrafter.log")
        check("other channel: no legacy", legacy(d, "spacecrafter"), [])
        check("other channel: history",
              [p.name for p in history(d, "spacecrafter")],
              ["spacecrafter.log", "spacecrafter.3.log"])
        check("channel names do not leak into each other",
              [p.name for p in history(d, "tcp")], [])

        # (6) a directory is not a log file
        (d / "script.9.log").mkdir()
        check("a directory named like an archive is skipped",
              [p.name for p in archives(d)], ["script.1.log", "script.7.log"])

        # (7) the window comes from the source, not from here
        check("window() reads the constant", window() >= 2, True)

    print("\n%d PASS, %d FAIL" % (ok, fail))
    return 1 if fail else 0


def main(argv):
    if len(argv) >= 2 and argv[1] == "selftest":
        return selftest()
    if len(argv) >= 3 and argv[1] == "--live":
        # For shell callers (f4_scriptspeed.sh): print the live file's path and
        # nothing else, so `LOG=$(logread.py --live <dir> <chan>)` gets the same
        # rule the Python readers get.  Absent -> empty output, exit 1.
        p = live(argv[2], argv[3] if len(argv) > 3 else "script")
        if p is None:
            return 1
        print(p)
        return 0
    if len(argv) < 2:
        print(__doc__)
        return 2
    d = Path(argv[1])
    chans = [argv[2]] if len(argv) > 2 else list(CHANNELS)
    for c in chans:
        lv = live(d, c)
        print("%-14s live=%-26s archives=%-2d legacy=%d"
              % (c, lv.name if lv else "-", len(archives(d, c)), len(legacy(d, c))))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
