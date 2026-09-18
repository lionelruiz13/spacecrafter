#!/usr/bin/env python3
"""f120_ids.py -- ID CONSERVATION between the archived DEPLOYMENT-MAP and its regeneration.

Why this exists (F120, 2026-09-18): before the regeneration the map's tier items were markdown
LIST POSITIONS (`1.` ... `11.` under `## T1`), and eleven of them are cited from OUTSIDE the map
(INTENT.md, DECISIONS_PENDING.md, INTENT/<id>.md).  A list that loses a member silently retargets
every citation after it.  The regeneration writes every surviving item under an EXPLICIT label
(`**T1.5**`), so this script can prove mechanically that nothing was lost, renumbered or reused.

WHAT IT PROVES (tier id space = R<n>, T1.<n>, T5.<n>):
  C1  every row of the disposition table is unique          (no id in two classes)
  C2  archive tier ids == table tier ids minus the NEW ones (nothing lost, nothing invented)
  C3  table LIVE ids == the explicit labels in the live map (no live id absent from the table,
                                                             no table-LIVE id absent from the map)
  C4  CLOSED / EXCLUDED ids appear NOWHERE as a live label  (a closed id leaves a gap)
  C5  every NEW id is > max(archive, live) in its own tier  (never a reused number)

USAGE
  python3 claude/harness/f120_ids.py
  python3 claude/harness/f120_ids.py --map <live.md> --archive <pred.md> --table <disposition.tsv>
Exit 0 = all five checks pass; exit 1 = at least one failed (every failure printed with its set).
"""
import argparse
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
CLAUDE = os.path.dirname(HERE)

DEF_MAP = os.path.join(CLAUDE, "DEPLOYMENT-MAP.md")
DEF_ARCHIVE = os.path.join(CLAUDE, "DEPLOYMENT-MAP", "archive", "2026-09-18-predecessor.md")
DEF_TABLE = os.path.join(HERE, "artifacts", "f120", "disposition.tsv")

# the three tiers whose items carry a NUMBER in the predecessor (the cited id space).
# key = tier label written in the regeneration, value = regex matching the predecessor's heading.
TIERS = {
    "R": re.compile(r"^## R\b"),
    "T1": re.compile(r"^## T1\b"),
    "T5": re.compile(r"^## T5\b"),
}
ANY_HEAD = re.compile(r"^## ")
NUMBERED = re.compile(r"^(\d+)\. ")
# an explicit label in the regenerated map: "**R7**", "- **T1.5**", "#### **T5.6**" ...
LIVE_LABEL = re.compile(r"^[-*#\s]*\*\*(R\d+|T[15]\.\d+)\*\*")


def tier_of(tid):
    return tid.split(".")[0] if "." in tid else "R"


def num_of(tid):
    return int(tid.split(".")[1]) if "." in tid else int(tid[1:])


def archive_ids(path):
    """The predecessor's id set, derived from its NUMBERED LISTS -- i.e. from exactly the
    list positions that were the ids."""
    out = set()
    cur = None
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            if ANY_HEAD.match(line):
                cur = None
                for tier, rx in TIERS.items():
                    if rx.match(line):
                        cur = tier
                continue
            if cur is None:
                continue
            m = NUMBERED.match(line)
            if m:
                out.add("%s%s" % (cur, m.group(1)) if cur == "R" else "%s.%s" % (cur, m.group(1)))
    return out


def live_ids(path):
    """The explicit labels the regenerated map writes."""
    out = set()
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            m = LIVE_LABEL.match(line)
            if m:
                out.add(m.group(1))
    return out


def table_rows(path):
    """(id -> class) plus the NEW-marked set, from disposition.tsv."""
    cls, new, order = {}, set(), []
    dup = []
    with open(path, encoding="utf-8") as fh:
        for raw in fh:
            if raw.startswith("#") or not raw.strip():
                continue
            cell = raw.rstrip("\n").split("\t")
            if cell[0] == "id":
                continue
            tid = cell[0].strip()
            if tid.endswith(")") and "(new" in tid:
                base = tid.split("(")[0].strip()
                new.add(base)
                tid = base
            if tid in cls:
                dup.append(tid)
            disp = (cell[3].strip() if len(cell) > 3 else "").split()[0].rstrip(",;")
            cls[tid] = disp
            order.append(tid)
    return cls, new, order, dup


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--map", default=DEF_MAP)
    ap.add_argument("--archive", default=DEF_ARCHIVE)
    ap.add_argument("--table", default=DEF_TABLE)
    a = ap.parse_args()

    for p in (a.map, a.archive, a.table):
        if not os.path.exists(p):
            print("STOP: missing input %s" % p)
            return 2

    arch = archive_ids(a.archive)
    live = live_ids(a.map)
    cls, new, order, dup = table_rows(a.table)
    tier_tbl = {k: v for k, v in cls.items() if re.fullmatch(r"R\d+|T[15]\.\d+", k)}

    print("archive  : %d tier ids  %s" % (len(arch), " ".join(sorted(arch, key=lambda s: (tier_of(s), num_of(s))))))
    print("live map : %d tier ids  %s" % (len(live), " ".join(sorted(live, key=lambda s: (tier_of(s), num_of(s))))))
    print("table    : %d rows, %d of them tier ids, %d marked NEW" % (len(order), len(tier_tbl), len(new)))

    fails = []

    # C1 -- uniqueness
    if dup:
        fails.append("C1 duplicate table rows: %s" % sorted(set(dup)))
    else:
        print("C1 OK  every table row unique")

    # C2 -- conservation
    carried = set(tier_tbl) - new
    if carried != arch:
        fails.append("C2 archive-not-in-table: %s ; table-not-in-archive(and not NEW): %s"
                     % (sorted(arch - carried), sorted(carried - arch)))
    else:
        print("C2 OK  archive tier ids == table tier ids minus NEW (%d)" % len(arch))

    # C3 -- live set agreement
    tbl_live = {k for k, v in tier_tbl.items() if v == "LIVE"}
    if tbl_live != live:
        fails.append("C3 table-LIVE-not-in-map: %s ; map-label-not-LIVE-in-table: %s"
                     % (sorted(tbl_live - live), sorted(live - tbl_live)))
    else:
        print("C3 OK  table LIVE == explicit labels in the map (%d)" % len(live))

    # C4 -- closed ids leave a gap
    gone = {k for k, v in tier_tbl.items() if v in ("CLOSED", "EXCLUDED")}
    bad = gone & live
    if bad:
        fails.append("C4 closed/excluded id still written live: %s" % sorted(bad))
    else:
        print("C4 OK  %d closed/excluded ids leave a gap" % len(gone))

    # C5 -- new ids are max+1 over live U archive, per tier
    for tid in sorted(new, key=lambda s: (tier_of(s), num_of(s))):
        t = tier_of(tid)
        prior = [num_of(x) for x in arch if tier_of(x) == t]
        if not prior or num_of(tid) <= max(prior):
            fails.append("C5 new id %s is not above the archive's max in tier %s (%s)"
                         % (tid, t, max(prior) if prior else "none"))
    if not any(f.startswith("C5") for f in fails):
        print("C5 OK  %d new ids all above their tier's archived maximum" % len(new))

    if fails:
        print("\n== FAIL ==")
        for f in fails:
            print("  " + f)
        return 1
    print("\n== f120_ids: 5/5 PASS ==")
    return 0


if __name__ == "__main__":
    sys.exit(main())
