#!/usr/bin/env python3
"""f52_strata.py — the INT-7 stratigraphy signature, measured from git alone.

Implements EXACTLY the metric definitions committed in
  harness/artifacts/f52/f52_predictions.json
(written and committed before the first log read; see §11.168 / §11.161(f)).

Read-only on both repositories: it runs `git log` / `git show` and nothing else.
No code, no data, no launches, no builds.

Scopes
  A  EntityCore     — every commit reachable from the sha the code tree pins
  B  experimentalModule — code-repo commits touching src/experimentalModule/,
                          diffs restricted to that pathspec
  C  control        — same repo/window, src/ minus experimentalModule minus EntityCore

Outputs (under harness/artifacts/f52/):
  f52_measured.json   every number this task cites
  f52_report.txt      the same, human-readable

Usage:  python3 harness/f52_strata.py [--out DIR]
"""

import argparse
import json
import os
import re
import subprocess
import sys
from datetime import datetime, timezone

CODE_REPO = "/home/claude/spacecrafter"
EC_REPO = "/home/claude/spacecrafter/src/EntityCore"

# --- era boundaries, from the committed predictions file (derived from §11.161(b),
#     anchored at today = 2026-08-30). Epoch seconds, UTC.
E_LEARN_END = datetime(2021, 8, 30, tzinfo=timezone.utc).timestamp()   # ">5 years ago"
E_RECENT_START = datetime(2024, 8, 30, tzinfo=timezone.utc).timestamp()  # "~1-2 yr"
MIN_BUCKET_N = 5            # buckets below this are reported, excluded from the trend
SPIKE_REGEX = re.compile(r"asmooth|taskable", re.I)   # fixed by §11.161(f)'s own words
ROLL_N = 11                 # centered rolling window, commits

SEP = "\x1f"
REC = "\x01"


def git(repo, *args):
    p = subprocess.run(["git", "-C", repo] + list(args),
                       capture_output=True, text=True, errors="replace")
    if p.returncode != 0:
        raise RuntimeError("git %s -> rc=%d: %s" % (" ".join(args), p.returncode, p.stderr.strip()))
    return p.stdout


def collect(repo, ref, pathspec=None):
    """Non-merge commits reachable from ref, with numstat + name-status, -M50%."""
    fmt = REC + SEP.join(["%H", "%at", "%ct", "%an", "%ae", "%cn", "%s"])
    base = ["log", ref, "--no-merges", "-M50%", "--format=" + fmt]
    ps = (["--"] + pathspec) if pathspec else []

    def run(extra):
        return git(repo, *(base + extra + ps))

    commits = {}
    order = []
    cur = None
    for line in run(["--numstat"]).split("\n"):
        if line.startswith(REC):
            f = line[1:].split(SEP)
            cur = {"sha": f[0], "at": int(f[1]), "ct": int(f[2]), "an": f[3],
                   "ae": f[4], "cn": f[5], "subject": SEP.join(f[6:]),
                   "add": 0, "del": 0, "nfiles": 0, "binary": 0,
                   "files": [], "status": []}
            commits[cur["sha"]] = cur
            order.append(cur["sha"])
            continue
        if not line.strip() or cur is None:
            continue
        parts = line.split("\t")
        if len(parts) < 3:
            continue
        a, d, path = parts[0], parts[1], "\t".join(parts[2:])
        cur["nfiles"] += 1
        if a == "-" or d == "-":
            cur["binary"] += 1
            continue
        cur["add"] += int(a)
        cur["del"] += int(d)
        cur["files"].append((int(a), int(d), path))

    cur = None
    for line in run(["--name-status"]).split("\n"):
        if line.startswith(REC):
            cur = commits.get(line[1:].split(SEP)[0])
            continue
        if not line.strip() or cur is None:
            continue
        parts = line.split("\t")
        cur["status"].append((parts[0], "\t".join(parts[1:])))

    for sha in order:
        c = commits[sha]
        tot = c["add"] + c["del"]
        c["r"] = (c["del"] / tot) if tot else None
        st = [s[0] for s in c["status"]]
        c["has_DR"] = any(s and s[0] in "DRC" for s in st)
        c["pure_add"] = bool(st) and all(s and s[0] == "A" for s in st) and c["del"] == 0
    out = [commits[s] for s in order]
    out.sort(key=lambda c: c["at"])          # chronological by AUTHOR time (FM-10)
    return out


def merge_count(repo, ref, pathspec=None):
    ps = (["--"] + pathspec) if pathspec else []
    o = git(repo, *(["log", ref, "--merges", "--format=%H"] + ps))
    return len([x for x in o.split("\n") if x.strip()])


def era_of(at):
    if at < E_LEARN_END:
        return "E_learn"
    if at < E_RECENT_START:
        return "E_mid"
    return "E_recent"


def median(xs):
    xs = sorted(xs)
    n = len(xs)
    if n == 0:
        return None
    return xs[n // 2] if n % 2 else (xs[n // 2 - 1] + xs[n // 2]) / 2.0


def stats(cs):
    """Every era-level statistic the predictions file defines."""
    add = sum(c["add"] for c in cs)
    dele = sum(c["del"] for c in cs)
    rs = [c["r"] for c in cs if c["r"] is not None]
    return {
        "n_commits": len(cs),
        "n_with_lines": len(rs),
        "zero_line_commits": len(cs) - len(rs),
        "add": add, "del": dele, "lines": add + dele,
        "R": (dele / (add + dele)) if (add + dele) else None,
        "median_r": median(rs),
        "F_struct": (sum(1 for c in cs if c["has_DR"]) / len(cs)) if cs else None,
        "F_pureadd": (sum(1 for c in cs if c["pure_add"]) / len(cs)) if cs else None,
        "binary_file_touches": sum(c["binary"] for c in cs),
        "first": iso(cs[0]["at"]) if cs else None,
        "last": iso(cs[-1]["at"]) if cs else None,
    }


def iso(ts):
    return datetime.fromtimestamp(ts, timezone.utc).strftime("%Y-%m-%d %H:%M:%SZ")


def spearman(xs, ys):
    """Rank correlation with average ranks for ties. n is small here."""
    n = len(xs)
    if n < 3:
        return None

    def ranks(v):
        idx = sorted(range(n), key=lambda i: v[i])
        r = [0.0] * n
        i = 0
        while i < n:
            j = i
            while j + 1 < n and v[idx[j + 1]] == v[idx[i]]:
                j += 1
            avg = (i + j) / 2.0 + 1
            for k in range(i, j + 1):
                r[idx[k]] = avg
            i = j + 1
        return r
    rx, ry = ranks(xs), ranks(ys)
    mx, my = sum(rx) / n, sum(ry) / n
    num = sum((rx[i] - mx) * (ry[i] - my) for i in range(n))
    dx = sum((rx[i] - mx) ** 2 for i in range(n)) ** 0.5
    dy = sum((ry[i] - my) ** 2 for i in range(n)) ** 0.5
    return (num / (dx * dy)) if dx and dy else None


def buckets(cs):
    """Calendar-year buckets on the AUTHOR timestamp, UTC."""
    by = {}
    for c in cs:
        y = datetime.fromtimestamp(c["at"], timezone.utc).year
        by.setdefault(y, []).append(c)
    out = []
    for y in sorted(by):
        s = stats(by[y])
        s["year"] = y
        s["included_in_trend"] = s["n_commits"] >= MIN_BUCKET_N
        out.append(s)
    inc = [b for b in out if b["included_in_trend"] and b["R"] is not None]
    rho = spearman([b["year"] for b in inc], [b["R"] for b in inc]) if len(inc) >= 3 else None
    return out, rho, [b["year"] for b in inc]


def rolling(cs):
    rows = []
    half = ROLL_N // 2
    for i, c in enumerate(cs):
        w = cs[max(0, i - half): i + half + 1]
        a = sum(x["add"] for x in w)
        d = sum(x["del"] for x in w)
        rows.append({"sha": c["sha"][:8], "at": iso(c["at"]),
                     "roll_R": (d / (a + d)) if (a + d) else None,
                     "subject": c["subject"][:70]})
    return rows


def authors(cs):
    a = {}
    for c in cs:
        k = "%s <%s>" % (c["an"], c["ae"])
        a[k] = a.get(k, 0) + 1
    return dict(sorted(a.items(), key=lambda kv: -kv[1]))


def top_files(cs, k=10):
    f = {}
    for c in cs:
        for a, d, p in c["files"]:
            f[p] = f.get(p, 0) + a + d
    return sorted(f.items(), key=lambda kv: -kv[1])[:k]


def rewrite_signals(cs):
    """L1: squash/rebase/import detection — attempted, never assumed."""
    diverging = [c for c in cs if c["ct"] != c["at"]]
    ct_runs = {}
    for c in cs:
        ct_runs[c["ct"]] = ct_runs.get(c["ct"], 0) + 1
    shared = sorted([(v, iso(k)) for k, v in ct_runs.items() if v > 1], reverse=True)[:10]
    adds = sorted((c["add"] for c in cs), reverse=True)
    first_add = cs[0]["add"] if cs else 0
    return {
        "n_author_ne_committer_time": len(diverging),
        "max_ct_minus_at_days": round(max((c["ct"] - c["at"]) for c in cs) / 86400.0, 2) if cs else None,
        "shared_committer_timestamps_top10": shared,
        "first_commit_add": first_add,
        "first_commit_rank_by_add": (adds.index(first_add) + 1) if cs else None,
        "largest_commit_add": adds[0] if adds else None,
        "committer_names": dict(sorted(
            ((c["cn"], 1) for c in cs), key=lambda kv: kv[0])) if cs else {},
    }


def scope_report(name, cs, merges, extra=None):
    eras = {e: [c for c in cs if era_of(c["at"]) == e]
            for e in ("E_learn", "E_mid", "E_recent")}
    bks, rho, inc_years = buckets(cs)
    rep = {
        "scope": name,
        "n_commits_non_merge": len(cs),
        "n_merges_excluded": merges,
        "date_range_author": [iso(cs[0]["at"]), iso(cs[-1]["at"])] if cs else None,
        "date_range_committer": [iso(min(c["ct"] for c in cs)),
                                 iso(max(c["ct"] for c in cs))] if cs else None,
        "first_commit": {"sha": cs[0]["sha"], "at": iso(cs[0]["at"]),
                         "ct": iso(cs[0]["ct"]), "subject": cs[0]["subject"],
                         "add": cs[0]["add"], "del": cs[0]["del"],
                         "nfiles": cs[0]["nfiles"]} if cs else None,
        "last_commit": {"sha": cs[-1]["sha"], "at": iso(cs[-1]["at"]),
                        "subject": cs[-1]["subject"]} if cs else None,
        "whole": stats(cs),
        "eras": {e: stats(v) for e, v in eras.items()},
        "year_buckets": bks,
        "spearman_rho_year_vs_R": rho,
        "trend_included_years": inc_years,
        "authors_overall": authors(cs),
        "authors_by_era": {e: authors(v) for e, v in eras.items()},
        "top_files_by_touched_lines": top_files(cs),
        "rewrite_signals": rewrite_signals(cs),
    }
    if extra:
        rep.update(extra)
    return rep


def birth_relative(cs):
    """Declared robustness binning: first 365 days vs last 365 days."""
    if not cs:
        return None
    t0, t1 = cs[0]["at"], cs[-1]["at"]
    first = [c for c in cs if c["at"] < t0 + 365 * 86400]
    last = [c for c in cs if c["at"] > t1 - 365 * 86400]
    return {"first_365d": stats(first), "last_365d": stats(last)}


def spike_check(cs, eras_stats):
    matched = [c for c in cs if SPIKE_REGEX.search(c["subject"])]
    roll = {r["sha"]: r["roll_R"] for r in rolling(cs)}
    era_R = {e: eras_stats[e]["R"] for e in eras_stats}
    era_med = {e: eras_stats[e]["median_r"] for e in eras_stats}
    rows = []
    for c in matched:
        e = era_of(c["at"])
        med = era_med.get(e)
        R = era_R.get(e)
        rr = roll.get(c["sha"][:8])
        rows.append({
            "sha": c["sha"][:8], "at": iso(c["at"]), "subject": c["subject"],
            "add": c["add"], "del": c["del"], "r": c["r"], "era": e,
            "era_median_r": med, "era_R": R, "roll_R": rr,
            "r_above_median_plus_0.10": (c["r"] is not None and med is not None
                                         and c["r"] >= med + 0.10),
            "roll_above_R_plus_0.10": (rr is not None and R is not None
                                       and rr >= R + 0.10),
        })
    hits = sum(1 for r in rows if r["r_above_median_plus_0.10"] or r["roll_above_R_plus_0.10"])
    return {"n_matched": len(matched), "n_spiking": hits,
            "share": (hits / len(matched)) if matched else None, "rows": rows}


def window_subset(cs, t_lo, t_hi):
    return [c for c in cs if t_lo <= c["at"] <= t_hi]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                                  "artifacts", "f52"))
    a = ap.parse_args()
    os.makedirs(a.out, exist_ok=True)

    code_head = git(CODE_REPO, "rev-parse", "HEAD").strip()
    ec_pin = git(CODE_REPO, "rev-parse", "HEAD:src/EntityCore").strip()
    ec_head = git(EC_REPO, "rev-parse", "HEAD").strip()

    # ---- scope A: EntityCore, whole repo, at the pinned sha
    A = collect(EC_REPO, ec_pin)
    A_rep = scope_report("A_EntityCore", A, merge_count(EC_REPO, ec_pin))
    A_rep["ref"] = ec_pin
    A_rep["birth_relative"] = birth_relative(A)
    A_rep["spikes"] = spike_check(A, A_rep["eras"])
    # L6: what lies beyond the pin
    try:
        beyond = git(EC_REPO, "log", "--format=%H", ec_pin + "..main")
        A_rep["commits_on_main_beyond_pin"] = len([x for x in beyond.split("\n") if x.strip()])
    except RuntimeError as e:
        A_rep["commits_on_main_beyond_pin"] = "unresolved: %s" % e
    A_rep["local_branches"] = git(EC_REPO, "branch", "-a", "--format=%(refname) %(objectname:short)").strip().split("\n")

    # ---- scope B: experimentalModule, path-restricted
    EXP = ["src/experimentalModule/"]
    B = collect(CODE_REPO, code_head, EXP)
    B_rep = scope_report("B_experimentalModule", B, merge_count(CODE_REPO, code_head, EXP))
    B_rep["ref"] = code_head
    B_rep["pathspec"] = EXP
    B_rep["birth_relative"] = birth_relative(B)
    B_rep["spikes"] = spike_check(B, B_rep["eras"])
    if B:
        adds = sorted((c["add"] for c in B), reverse=True)
        p90 = adds[max(0, int(round(0.10 * (len(adds) - 1))))]
        B_rep["P7_first_commit_add"] = B[0]["add"]
        B_rep["P7_p90_add"] = p90
        B_rep["P7_pass"] = B[0]["add"] >= p90

    # ---- scope C: era-matched control, same repo + same window as B
    CTRL = ["src/", ":(exclude)src/experimentalModule/", ":(exclude)src/EntityCore"]
    C_all = collect(CODE_REPO, code_head, CTRL)
    t_lo = B[0]["at"] if B else 0
    t_hi = B[-1]["at"] if B else 0
    C = window_subset(C_all, t_lo, t_hi)
    C_rep = scope_report("C_control_src_minus_expmod", C, merge_count(CODE_REPO, code_head, CTRL))
    C_rep["ref"] = code_head
    C_rep["pathspec"] = CTRL
    C_rep["window_author_time"] = [iso(t_lo), iso(t_hi)]
    C_rep["n_commits_all_time_before_window_filter"] = len(C_all)

    # EntityCore restricted to the same window (the second control of P6b)
    A_win = window_subset(A, t_lo, t_hi)
    A_win_stats = stats(A_win)

    out = {
        "generated": datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M:%SZ"),
        "instrument": os.path.basename(__file__),
        "predictions_file": "artifacts/f52/f52_predictions.json",
        "anchors": {"code_head": code_head, "entitycore_pin_at_code_head": ec_pin,
                    "entitycore_worktree_head": ec_head,
                    "pin_equals_worktree": ec_pin == ec_head},
        "era_boundaries_utc": {"E_learn_end": iso(E_LEARN_END),
                               "E_recent_start": iso(E_RECENT_START),
                               "min_bucket_n": MIN_BUCKET_N,
                               "rolling_window": ROLL_N,
                               "spike_regex": SPIKE_REGEX.pattern},
        "A_EntityCore": A_rep,
        "B_experimentalModule": B_rep,
        "C_control": C_rep,
        "A_EntityCore_in_B_window": A_win_stats,
    }

    jpath = os.path.join(a.out, "f52_measured.json")
    with open(jpath, "w") as f:
        json.dump(out, f, indent=1, sort_keys=False)
        f.write("\n")

    # ---- human-readable twin
    L = []
    def p(s=""):
        L.append(s)
    p("f52_strata.py — INT-7 stratigraphy signature, git-only")
    p("generated %s" % out["generated"])
    p("code HEAD %s | EntityCore pin %s (worktree HEAD equal: %s)"
      % (code_head[:8], ec_pin[:8], ec_pin == ec_head))
    p("eras (UTC, from §11.161(b)): E_learn < %s <= E_mid < %s <= E_recent"
      % (iso(E_LEARN_END)[:10], iso(E_RECENT_START)[:10]))
    for rep in (A_rep, B_rep, C_rep):
        p("")
        p("=" * 78)
        p("SCOPE %s   ref=%s" % (rep["scope"], rep["ref"][:8]))
        if rep.get("pathspec"):
            p("  pathspec: %s" % " ".join(rep["pathspec"]))
        p("  non-merge commits: %d   merges excluded: %d" %
          (rep["n_commits_non_merge"], rep["n_merges_excluded"]))
        p("  author-date range:    %s .. %s" % tuple(rep["date_range_author"] or ("-", "-")))
        p("  committer-date range: %s .. %s" % tuple(rep["date_range_committer"] or ("-", "-")))
        if rep["first_commit"]:
            fc = rep["first_commit"]
            p("  first commit: %s %s  +%d/-%d over %d files  %r"
              % (fc["sha"][:8], fc["at"], fc["add"], fc["del"], fc["nfiles"], fc["subject"][:60]))
        w = rep["whole"]
        p("  WHOLE: n=%d  +%d/-%d  R=%s  median_r=%s  F_struct=%s  F_pureadd=%s"
          % (w["n_commits"], w["add"], w["del"], fmt(w["R"]), fmt(w["median_r"]),
             fmt(w["F_struct"]), fmt(w["F_pureadd"])))
        for e in ("E_learn", "E_mid", "E_recent"):
            s = rep["eras"][e]
            p("  %-9s n=%-5d +%-7d -%-7d R=%-7s median_r=%-7s F_struct=%-7s F_pureadd=%s"
              % (e, s["n_commits"], s["add"], s["del"], fmt(s["R"]), fmt(s["median_r"]),
                 fmt(s["F_struct"]), fmt(s["F_pureadd"])))
        p("  year buckets (R, n; * = in trend):")
        for b in rep["year_buckets"]:
            p("    %d  R=%-7s median_r=%-7s n=%-4d F_struct=%-7s F_pureadd=%-7s %s"
              % (b["year"], fmt(b["R"]), fmt(b["median_r"]), b["n_commits"],
                 fmt(b["F_struct"]), fmt(b["F_pureadd"]),
                 "*" if b["included_in_trend"] else ""))
        p("  spearman rho(year, R) over %s = %s"
          % (rep["trend_included_years"], fmt(rep["spearman_rho_year_vs_R"], 3)))
        if rep.get("birth_relative"):
            br = rep["birth_relative"]
            p("  birth-relative: first365d R=%s (n=%d) | last365d R=%s (n=%d)"
              % (fmt(br["first_365d"]["R"]), br["first_365d"]["n_commits"],
                 fmt(br["last_365d"]["R"]), br["last_365d"]["n_commits"]))
        p("  authors: %s" % rep["authors_overall"])
        rw = rep["rewrite_signals"]
        p("  rewrite signals: author!=committer time on %d/%d commits, max lag %s d; "
          "first-commit add=%d (rank %s of %d by add, largest=%d)"
          % (rw["n_author_ne_committer_time"], rep["n_commits_non_merge"],
             rw["max_ct_minus_at_days"], rw["first_commit_add"],
             rw["first_commit_rank_by_add"], rep["n_commits_non_merge"],
             rw["largest_commit_add"]))
        p("  shared committer timestamps (count, time): %s" % rw["shared_committer_timestamps_top10"][:5])
        p("  top files by touched lines: %s" % [(n, t) for t, n in
                                                [(x[1], x[0]) for x in rep["top_files_by_touched_lines"]]][:6])
        if rep.get("spikes"):
            sp = rep["spikes"]
            p("  spike lexicon /%s/ : matched %d, spiking %d (share %s)"
              % (SPIKE_REGEX.pattern, sp["n_matched"], sp["n_spiking"], fmt(sp["share"])))
            for r in sp["rows"]:
                p("    %s %s r=%s (era %s median_r=%s, roll_R=%s) %r"
                  % (r["sha"], r["at"][:10], fmt(r["r"]), r["era"],
                     fmt(r["era_median_r"]), fmt(r["roll_R"]), r["subject"][:56]))
    p("")
    p("EntityCore restricted to B's window %s .. %s: n=%d R=%s median_r=%s"
      % (iso(t_lo), iso(t_hi), A_win_stats["n_commits"], fmt(A_win_stats["R"]),
         fmt(A_win_stats["median_r"])))
    p("EntityCore commits on main beyond the pin: %s" % A_rep["commits_on_main_beyond_pin"])

    tpath = os.path.join(a.out, "f52_report.txt")
    with open(tpath, "w") as f:
        f.write("\n".join(L) + "\n")
    print("\n".join(L))
    print("\nwrote %s\nwrote %s" % (jpath, tpath))


def fmt(x, nd=4):
    return "None" if x is None else ("%.*f" % (nd, x))


if __name__ == "__main__":
    sys.exit(main())
