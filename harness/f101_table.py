#!/usr/bin/env python3
"""F101 - the leg table, re-derived from the FINAL artifacts.

    cd claude/harness && python3 f101_table.py <absRunDir> [--out <dir>]

WHY THIS EXISTS AND IS NOT A `print` INSIDE THE DRIVER.  `f101_bisect.py` reads
the applog and counts its witness patterns while the application is STILL
RUNNING (it must: the counts describe the state the dump was taken in, and the
quit comes after).  The child's stdout is a buffered file, so the driver's
counts are a LOWER BOUND on the finished file - measured here, not assumed:
`p25desc06`'s `oldpath_add` reads 259 in `result.json` and 261 in the finished
`leg.applog`, i.e. two lines the process had not yet flushed.  Every count in
the record therefore comes from THIS pass over the completed files, and the
difference is reported per leg instead of being quietly overwritten.

It also does the arithmetic that turns `06old.sts`'s 170 authored lines into the
156 bodies that actually land, by reading the engine's own refusal lines rather
than by subtracting a number somebody remembered.
"""

import argparse
import json
import re
import sys
from collections import Counter
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import dumpread                                                   # noqa: E402
from f101_bisect import WITNESS                                   # noqa: E402

ADD = re.compile(r"Loading new Stellar System object\.\.\. (.*?)\x1b")
DUP = re.compile(r"Can not add body named (.*?) because a body of that name")
NOORBIT = re.compile(r"Body '(.*?)': could not build an orbit from coord_func")

LEGS = ["p23", "p25", "p27", "p31", "p27nc", "ctlmars",
        "p25desc", "p31desc", "p25desc06", "p31desc06", "ctl_S02"]


def half_names(path):
    # require_old=False: same reason as the driver's - this table's rows
    # INCLUDE the empty-old legs and reporting them is the point.
    _h, pairs, missing_new, missing_old = dumpread.load_dump(
        path, require_old=False)
    both = {r["name"] for r in pairs}
    return both | set(missing_new), both | set(missing_old)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--out", default=str(HERE / "artifacts" / "f101"))
    a = ap.parse_args()
    run, out = Path(a.run), Path(a.out)
    out.mkdir(parents=True, exist_ok=True)

    rows, md = [], []
    md.append("| leg | pre old/new | post old/new | then old/new | "
              "witness order (after startup) |")
    md.append("|---|---|---|---|---|")
    for leg in LEGS:
        d = run / leg
        if not (d / "result.json").is_file():
            continue
        r = json.loads((d / "result.json").read_text())
        txt = (d / "leg.applog").read_text(encoding="latin-1", errors="replace")
        final = {k: txt.count(v) for k, v in WITNESS.items()}
        lag = {k: final[k] - r["witness"][k]
               for k in final if final[k] != r["witness"][k]}
        adds = ADD.findall(txt)
        row = {
            "leg": leg,
            "show_md5": r["show_md5"], "show_lines": r["show_lines"],
            "bin_md5": r["bin_md5"], "exit_status": r["exit_status"],
            "quit_exit_code": r["quit_exit_code"],
            "script_end_seen": r["script_end_seen"],
            "frozen_ok": r["frozen_ok"], "frozen_n": r["frozen_n"],
            "frozen_digest_in": r["frozen_digest_in"],
            "frozen_digest_out": r["frozen_digest_out"],
            "farm_14sts_md5": r["farm_14sts_md5"],
            "lock_before": r["lock_before"], "lock_after": r["lock_after"],
            "proc_before": r["proc_before"], "proc_after": r["proc_after"],
            "witness_final": final,
            "witness_flush_lag": lag,
            "witness_order": r["witness_order"],
            "altitude_lines": r["altitude_lines"],
            "oldpath_add_total": len(adds),
            "oldpath_dup_refusals": len(DUP.findall(txt)),
            "oldpath_noorbit_refusals": sorted(set(NOORBIT.findall(txt))),
        }
        cells = []
        for tag in ("dump_pre", "dump_post", "dump_then"):
            v = r.get(tag)
            row[tag] = ({"bodies_old": v["bodies_old"],
                         "bodies_new": v["bodies_new"],
                         "both": v["both"], "old_only": v["old_only"],
                         "new_only": v["new_only"],
                         "old_names_n": v["old_names_n"]} if v else None)
            cells.append("%d/%d" % (v["bodies_old"], v["bodies_new"])
                         if v else "-")
        # the `06old.sts` accounting, from the engine's own lines
        if r.get("then"):
            # the second show's own attempts start after the last add the
            # FIRST show made; `Solsys` marks it when there is one, and when
            # the first show authors nothing (ctl_S02) the mark is the 90
            # startup adds.
            mark = (len(adds) - adds[::-1].index("Solsys")
                    if "Solsys" in adds else r["dump_post"]["bodies_old"])
            after = adds[mark:]
            distinct = set(after)
            old_then, new_then = half_names(d / "dump_then.json")
            row["then_show"] = r["then"]
            row["then_attempts"] = len(after)
            row["then_distinct"] = len(distinct)
            row["then_repeated_names"] = sorted(
                k for k, v in Counter(after).items() if v > 1)
            row["then_absent_from_old"] = sorted(distinct - old_then)
            row["then_absent_from_new"] = sorted(distinct - new_then)
        rows.append(row)
        # run-length collapse: `06old.sts` contributes 170 consecutive
        # `oldpath_no_parent` entries and a table is not the place to spell
        # them out one by one - the COUNT is what the claim rests on and it is
        # in `witness_final`.
        seq, prev, n = [], None, 0
        for k in r["witness_order"][5:]:
            if k == prev:
                n += 1
                continue
            if prev is not None:
                seq.append(prev + (" x%d" % n if n > 1 else ""))
            prev, n = k, 1
        if prev is not None:
            seq.append(prev + (" x%d" % n if n > 1 else ""))
        md.append("| `%s` | %s | %s | %s | %s |"
                  % (leg, cells[0], cells[1], cells[2],
                     " -> ".join(seq) or "(none)"))

    (out / "legs.json").write_text(json.dumps(rows, indent=1))
    (out / "legs.md").write_text("\n".join(md) + "\n")
    print("\n".join(md))
    for r in rows:
        if r["witness_flush_lag"]:
            print("flush lag %-10s %s" % (r["leg"], r["witness_flush_lag"]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
