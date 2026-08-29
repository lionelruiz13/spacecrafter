#!/usr/bin/env python3
"""f52_probes.py — the attribution probes F52's primary run made necessary.

f52_strata.py implements the committed metric definitions verbatim. Its output
raised three attribution questions that the pre-declared limits L1/L2/L3/L4/L5
anticipated by name, and each is answered HERE rather than by silently editing
the primary instrument:

  Q1 (L2/L3) — EntityCore's E_mid deletion ratio is 0.65. Is that premise rework,
       or the one repository-separation commit that removed a game, a sister
       library and a vendored header?  -> per-commit deletion attribution +
       a library-only recomputation with the exclusion set stated.
  Q2 (L1/L4) — experimentalModule's HEAD-reachable history starts 2025-09-20 at a
       single-parent commit whose subject says "Merge". Is there a flattened
       pre-history?  -> the branch-side measurement.
  Q3 (L5)    — who actually authored the commits in each scope?  -> identity
       normalisation and per-person partition of every scope.

Read-only on both repositories. Output: artifacts/f52/f52_probes.json + .txt
"""

import json
import os
import re
import subprocess
from datetime import datetime, timezone

CODE = "/home/claude/spacecrafter"
EC = "/home/claude/spacecrafter/src/EntityCore"
EXP = ["src/experimentalModule/"]
PIN = "7ce58350"
CODE_HEAD = "d6aec251"

# Q1's exclusion set: content that belongs to OTHER products sharing the early
# repository, plus non-code text. Stated so the filter is challengeable.
NON_LIBRARY = re.compile(r"^(LaserBombon/|EntityLib/|cmake/|LICENSE|README|\.gitignore)"
                         r"|(^|/)stb_image\.h$")

# Q4's exclusion set: this project's own reasoning artifacts, which lived inside
# src/experimentalModule/ until 2026-07-22. Not product code.
NON_CODE = re.compile(r"\.(md|txt)$|/harness/|\.py$")

# Q3's identity map (by email, then by name).
def person(an, ae):
    if ae.endswith("@anthropic.com"):
        return "claude"
    if "calvin" in ae.lower() or "Calvin" in an:
        return "vixy"
    return "other:" + an


def git(repo, *a):
    p = subprocess.run(["git", "-C", repo] + list(a), capture_output=True,
                       text=True, errors="replace")
    if p.returncode != 0:
        raise RuntimeError("%s -> %s" % (a, p.stderr.strip()))
    return p.stdout


SEP, RECMK = "\x1f", "\x01"


def collect(repo, revargs, pathspec=None):
    fmt = RECMK + SEP.join(["%H", "%at", "%an", "%ae", "%s"])
    cmd = ["log"] + revargs + ["--no-merges", "-M50%", "--numstat", "--format=" + fmt]
    if pathspec:
        cmd += ["--"] + pathspec
    out, cur = [], None
    for line in git(repo, *cmd).split("\n"):
        if line.startswith(RECMK):
            f = line[1:].split(SEP)
            cur = {"sha": f[0], "at": int(f[1]), "an": f[2], "ae": f[3],
                   "subject": SEP.join(f[4:]), "add": 0, "del": 0,
                   "add_lib": 0, "del_lib": 0, "files": []}
            cur["person"] = person(cur["an"], cur["ae"])
            out.append(cur)
            continue
        p = line.split("\t")
        if len(p) < 3 or p[0] == "-":
            continue
        a, d, path = int(p[0]), int(p[1]), "\t".join(p[2:])
        cur["add"] += a
        cur["del"] += d
        cur["files"].append((a, d, path))
        if not NON_LIBRARY.search(path):
            cur["add_lib"] += a
            cur["del_lib"] += d
    out.sort(key=lambda c: c["at"])
    return out


def iso(ts):
    return datetime.fromtimestamp(ts, timezone.utc).strftime("%Y-%m-%d")


def st(cs, lib=False):
    ka, kd = ("add_lib", "del_lib") if lib else ("add", "del")
    a = sum(c[ka] for c in cs)
    d = sum(c[kd] for c in cs)
    rs = sorted(c[kd] / (c[ka] + c[kd]) for c in cs if c[ka] + c[kd])
    med = None if not rs else (rs[len(rs) // 2] if len(rs) % 2 else
                               (rs[len(rs) // 2 - 1] + rs[len(rs) // 2]) / 2)
    return {"n": len(cs), "n_lines": len(rs), "add": a, "del": d,
            "R": (d / (a + d)) if a + d else None, "median_r": med,
            "first": iso(cs[0]["at"]) if cs else None,
            "last": iso(cs[-1]["at"]) if cs else None}


def by_year(cs, lib=False):
    y = {}
    for c in cs:
        y.setdefault(datetime.fromtimestamp(c["at"], timezone.utc).year, []).append(c)
    return {k: st(v, lib) for k, v in sorted(y.items())}


def by_person(cs, lib=False):
    p = {}
    for c in cs:
        p.setdefault(c["person"], []).append(c)
    return {k: st(v, lib) for k, v in sorted(p.items(), key=lambda kv: -len(kv[1]))}


def spearman(xs, ys):
    n = len(xs)
    if n < 3:
        return None
    def rk(v):
        idx = sorted(range(n), key=lambda i: v[i])
        r = [0.0] * n
        i = 0
        while i < n:
            j = i
            while j + 1 < n and v[idx[j + 1]] == v[idx[i]]:
                j += 1
            for k in range(i, j + 1):
                r[idx[k]] = (i + j) / 2.0 + 1
            i = j + 1
        return r
    rx, ry = rk(xs), rk(ys)
    mx, my = sum(rx) / n, sum(ry) / n
    num = sum((rx[i] - mx) * (ry[i] - my) for i in range(n))
    dx = sum((rx[i] - mx) ** 2 for i in range(n)) ** .5
    dy = sum((ry[i] - my) ** 2 for i in range(n)) ** .5
    return num / (dx * dy) if dx and dy else None


def main():
    out_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "artifacts", "f52")
    os.makedirs(out_dir, exist_ok=True)
    R = {"generated": datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M:%SZ"),
         "non_library_regex": NON_LIBRARY.pattern}
    T = []
    def p(s=""):
        T.append(s)

    # ---------------- Q1: EntityCore, deletion attribution + library-only
    A = collect(EC, [PIN])
    tot_del = sum(c["del"] for c in A)
    top = sorted(A, key=lambda c: -c["del"])[:8]
    R["Q1_top_deleting_commits"] = [
        {"sha": c["sha"][:8], "date": iso(c["at"]), "add": c["add"], "del": c["del"],
         "share_of_all_deletions": c["del"] / tot_del, "subject": c["subject"],
         "del_non_library": sum(d for a, d, f in c["files"] if NON_LIBRARY.search(f)),
         "del_library": c["del_lib"]} for c in top]
    R["Q1_all_deletions"] = tot_del
    R["Q1_full"] = {"whole": st(A), "years": by_year(A)}
    R["Q1_library_only"] = {"whole": st(A, True), "years": by_year(A, True)}
    ylib = {k: v for k, v in by_year(A, True).items() if v["n"] >= 5 and v["R"] is not None}
    R["Q1_library_only"]["spearman_rho_year_vs_R"] = spearman(list(ylib), [v["R"] for v in ylib.values()])
    R["Q1_library_only"]["trend_years"] = list(ylib)
    # library-only, birth-relative
    t0, t1 = A[0]["at"], A[-1]["at"]
    R["Q1_library_only"]["first_365d"] = st([c for c in A if c["at"] < t0 + 365 * 86400], True)
    R["Q1_library_only"]["last_365d"] = st([c for c in A if c["at"] > t1 - 365 * 86400], True)
    R["Q1_library_only"]["first_730d"] = st([c for c in A if c["at"] < t0 + 730 * 86400], True)
    R["Q1_library_only"]["after_730d"] = st([c for c in A if c["at"] >= t0 + 730 * 86400], True)
    R["Q1_persons"] = by_person(A)

    p("=== Q1  EntityCore: is E_mid's R=0.65 premise rework? ===")
    p("all deletions over 325 non-merge commits: %d" % tot_del)
    for c in R["Q1_top_deleting_commits"]:
        p("  %s %s +%-5d -%-6d (%5.1f%% of all deletions; non-library %d) %s"
          % (c["sha"], c["date"], c["add"], c["del"], 100 * c["share_of_all_deletions"],
             c["del_non_library"], c["subject"][:56]))
    p("")
    p("  library-only scope excludes /%s/" % NON_LIBRARY.pattern)
    p("  full     whole: +%d/-%d R=%.4f" % (R["Q1_full"]["whole"]["add"],
                                            R["Q1_full"]["whole"]["del"], R["Q1_full"]["whole"]["R"]))
    p("  lib-only whole: +%d/-%d R=%.4f" % (R["Q1_library_only"]["whole"]["add"],
                                            R["Q1_library_only"]["whole"]["del"],
                                            R["Q1_library_only"]["whole"]["R"]))
    p("  year   full-R   lib-R    n     lib +add/-del")
    for y, s in R["Q1_full"]["years"].items():
        L = R["Q1_library_only"]["years"][y]
        p("  %d  %s  %s  %-4d  +%d/-%d" % (y, f4(s["R"]), f4(L["R"]), s["n"], L["add"], L["del"]))
    p("  lib-only spearman rho over %s = %s" % (R["Q1_library_only"]["trend_years"],
                                                f4(R["Q1_library_only"]["spearman_rho_year_vs_R"])))
    for k in ("first_365d", "last_365d", "first_730d", "after_730d"):
        s = R["Q1_library_only"][k]
        p("  lib-only %-10s n=%-4d +%-6d -%-6d R=%s median_r=%s"
          % (k, s["n"], s["add"], s["del"], f4(s["R"]), f4(s["median_r"])))
    p("  persons: %s" % {k: v["n"] for k, v in R["Q1_persons"].items()})

    # ---------------- Q2: experimentalModule's flattened pre-history
    B = collect(CODE, [CODE_HEAD], EXP)
    Bbranch = collect(CODE, ["body-refactoring", "^" + CODE_HEAD], EXP)
    Ball = collect(CODE, ["--all"], EXP)
    R["Q2_head_reachable"] = {"whole": st(B), "years": by_year(B), "persons": by_person(B)}
    R["Q2_branch_only"] = {"whole": st(Bbranch), "years": by_year(Bbranch),
                           "persons": by_person(Bbranch),
                           "commits": [{"sha": c["sha"][:8], "date": iso(c["at"]),
                                        "person": c["person"], "add": c["add"],
                                        "del": c["del"], "subject": c["subject"][:70]}
                                       for c in Bbranch]}
    R["Q2_all_refs"] = {"whole": st(Ball), "years": by_year(Ball), "persons": by_person(Ball)}
    imp = [c for c in B if c["sha"].startswith("da858612")]
    R["Q2_import_commit"] = ({"sha": imp[0]["sha"][:8], "date": iso(imp[0]["at"]),
                              "add": imp[0]["add"], "del": imp[0]["del"],
                              "subject": imp[0]["subject"],
                              "parents": git(CODE, "log", "-1", "--format=%P", imp[0]["sha"]).split()}
                             if imp else None)
    p("")
    p("=== Q2  experimentalModule: the flattened pre-history ===")
    p("  HEAD-reachable : n=%d  %s..%s  +%d/-%d R=%s"
      % (B and st(B)["n"], st(B)["first"], st(B)["last"], st(B)["add"], st(B)["del"], f4(st(B)["R"])))
    p("  branch-only    : n=%d  %s..%s  +%d/-%d R=%s   (reachable from body-refactoring, NOT from %s)"
      % (st(Bbranch)["n"], st(Bbranch)["first"], st(Bbranch)["last"], st(Bbranch)["add"],
         st(Bbranch)["del"], f4(st(Bbranch)["R"]), CODE_HEAD))
    for c in R["Q2_branch_only"]["commits"]:
        p("     %s %s %-6s +%-5d -%-5d %s" % (c["sha"], c["date"], c["person"],
                                              c["add"], c["del"], c["subject"][:60]))
    p("  import commit  : %s" % R["Q2_import_commit"])

    # ---------------- Q3: identity partition of every scope
    CTRL = ["src/", ":(exclude)src/experimentalModule/", ":(exclude)src/EntityCore"]
    C = collect(CODE, [CODE_HEAD], CTRL)
    t_lo, t_hi = B[0]["at"], B[-1]["at"]
    Cw = [c for c in C if t_lo <= c["at"] <= t_hi]
    R["Q3_persons"] = {
        "A_EntityCore": by_person(A),
        "B_expMod_head": by_person(B),
        "B_expMod_all_refs": by_person(Ball),
        "C_control_window": by_person(Cw),
    }
    # the vixy-only subsets, which is what the testimony is about
    R["Q3_vixy_only"] = {
        "A_EntityCore": st([c for c in A if c["person"] == "vixy"]),
        "A_EntityCore_library_only": st([c for c in A if c["person"] == "vixy"], True),
        "B_expMod_head": st([c for c in B if c["person"] == "vixy"]),
        "B_expMod_all_refs": st([c for c in Ball if c["person"] == "vixy"]),
        "B_expMod_all_refs_years": by_year([c for c in Ball if c["person"] == "vixy"]),
        "B_expMod_all_refs_commits": [
            {"sha": c["sha"][:8], "date": iso(c["at"]), "add": c["add"], "del": c["del"],
             "r": (c["del"] / (c["add"] + c["del"])) if c["add"] + c["del"] else None,
             "subject": c["subject"][:70]}
            for c in Ball if c["person"] == "vixy"],
        "C_control_window": st([c for c in Cw if c["person"] == "vixy"]),
    }
    R["Q3_claude_only"] = {
        "B_expMod_head": st([c for c in B if c["person"] == "claude"]),
        "B_expMod_all_refs": st([c for c in Ball if c["person"] == "claude"]),
        "C_control_window": st([c for c in Cw if c["person"] == "claude"]),
        "A_EntityCore": st([c for c in A if c["person"] == "claude"]),
    }
    p("")
    p("=== Q3  who wrote each scope ===")
    for k, v in R["Q3_persons"].items():
        p("  %-20s %s" % (k, {p2: s["n"] for p2, s in v.items()}))
    p("  VIXY-ONLY subsets:")
    for k, s in R["Q3_vixy_only"].items():
        if isinstance(s, dict) and "n" in s:
            p("    %-32s n=%-4d %s..%s +%-6d -%-6d R=%s median_r=%s"
              % (k, s["n"], s["first"], s["last"], s["add"], s["del"], f4(s["R"]), f4(s["median_r"])))
    p("    expMod all-refs, vixy commits:")
    for c in R["Q3_vixy_only"]["B_expMod_all_refs_commits"]:
        p("      %s %s +%-5d -%-5d r=%s %s" % (c["sha"], c["date"], c["add"], c["del"],
                                               f4(c["r"]), c["subject"][:56]))
    p("  CLAUDE-ONLY subsets:")
    for k, s in R["Q3_claude_only"].items():
        p("    %-32s n=%-4d %s..%s +%-6d -%-6d R=%s median_r=%s"
          % (k, s["n"], s["first"], s["last"], s["add"], s["del"], f4(s["R"]), f4(s["median_r"])))

    # ---------------- Q4: experimentalModule, PRODUCT CODE only
    # The module directory carried this project's own reasoning artifacts until
    # 2026-07-22 (c523e3b1 deleted 87 of them). They are not product code and
    # they dominate the line counts, so the module is re-measured without them.
    for c in Ball:
        c["add_lib"] = sum(a for a, d, f in c["files"] if not NON_CODE.search(f))
        c["del_lib"] = sum(d for a, d, f in c["files"] if not NON_CODE.search(f))
    MECH = ("da858612", "c523e3b1")
    vall = [c for c in Ball if c["person"] == "vixy"]
    R["Q4_non_code_regex"] = NON_CODE.pattern
    R["Q4_expmod_code_only"] = {
        "all": st(Ball, True), "by_person": by_person(Ball, True),
        "by_year": by_year(Ball, True),
        "vixy": st(vall, True),
        "vixy_minus_mechanical": st([c for c in vall if not c["sha"].startswith(MECH)], True),
        "vixy_minus_mechanical_full_lines": st([c for c in vall if not c["sha"].startswith(MECH)]),
        "claude": st([c for c in Ball if c["person"] == "claude"], True),
        "mechanical_excluded": [
            {"sha": c["sha"][:8], "date": iso(c["at"]), "add": c["add"], "del": c["del"],
             "add_code": c["add_lib"], "del_code": c["del_lib"], "subject": c["subject"][:64]}
            for c in vall if c["sha"].startswith(MECH)],
    }
    p("")
    p("=== Q4  experimentalModule, product code only (excl /%s/) ===" % NON_CODE.pattern)
    for k in ("all", "vixy", "vixy_minus_mechanical", "claude"):
        s = R["Q4_expmod_code_only"][k]
        p("  %-24s n=%-4d %s..%s +%-6d -%-6d R=%s median_r=%s"
          % (k, s["n"], s["first"], s["last"], s["add"], s["del"], f4(s["R"]), f4(s["median_r"])))
    p("  by person: %s" % {k: (v["n"], f4(v["R"])) for k, v in R["Q4_expmod_code_only"]["by_person"].items()})
    p("  by year  : %s" % {k: (v["n"], f4(v["R"])) for k, v in R["Q4_expmod_code_only"]["by_year"].items()})
    p("  mechanically excluded from the vixy subset: %s" % R["Q4_expmod_code_only"]["mechanical_excluded"])

    # ---------------- Q5: EntityCore library-only, commit-weighted trend + late rework
    R["Q5_lib_years_median_r"] = {y: {"n": s["n"], "R": s["R"], "median_r": s["median_r"]}
                                  for y, s in by_year(A, True).items()}
    post = [c for c in A if c["at"] > datetime(2021, 12, 20, tzinfo=timezone.utc).timestamp()]
    R["Q5_top_library_deletions_after_the_split"] = [
        {"sha": c["sha"][:8], "date": iso(c["at"]), "add": c["add_lib"], "del": c["del_lib"],
         "r": c["del_lib"] / (c["add_lib"] + c["del_lib"]) if c["add_lib"] + c["del_lib"] else None,
         "subject": c["subject"][:64]}
        for c in sorted(post, key=lambda c: -c["del_lib"])[:10]]
    p("")
    p("=== Q5  EntityCore library-only: commit-weighted trend + the real rework events ===")
    for y, s in R["Q5_lib_years_median_r"].items():
        p("  %d n=%-4d R=%s median_r=%s" % (y, s["n"], f4(s["R"]), f4(s["median_r"])))
    p("  largest LIBRARY deletions after the 2021-12-20 separation:")
    for c in R["Q5_top_library_deletions_after_the_split"]:
        p("    %s %s +%-5d -%-5d r=%s %s" % (c["sha"], c["date"], c["add"], c["del"],
                                             f4(c["r"]), c["subject"]))

    # ---------------- Q6: the committed era test, re-run on the library-only scope
    E1 = datetime(2021, 8, 30, tzinfo=timezone.utc).timestamp()
    E2 = datetime(2024, 8, 30, tzinfo=timezone.utc).timestamp()
    eras = {"E_learn": [c for c in A if c["at"] < E1],
            "E_mid": [c for c in A if E1 <= c["at"] < E2],
            "E_recent": [c for c in A if c["at"] >= E2]}
    R["Q6_library_only_eras"] = {k: st(v, True) for k, v in eras.items()}
    dense = {y: s for y, s in by_year(A, True).items() if s["n"] >= 20}
    R["Q6_dense_years"] = dense
    R["Q6_dense_spearman_R"] = spearman(list(dense), [s["R"] for s in dense.values()])
    R["Q6_dense_spearman_median_r"] = spearman(list(dense), [s["median_r"] for s in dense.values()])
    R["Q6_commit_share_by_year"] = {y: s["n"] / len(A) for y, s in by_year(A).items()}
    p("")
    p("=== Q6  the committed era test, library-only scope ===")
    for k, s in R["Q6_library_only_eras"].items():
        p("  %-9s n=%-4d +%-6d -%-6d R=%s median_r=%s" % (k, s["n"], s["add"], s["del"],
                                                          f4(s["R"]), f4(s["median_r"])))
    p("  dense years (n>=20, %.1f%% of all commits): %s"
      % (100 * sum(s["n"] for s in dense.values()) / len(A),
         {y: (s["n"], f4(s["R"]), f4(s["median_r"])) for y, s in dense.items()}))
    p("  dense-year spearman: R %s | median_r %s" % (f4(R["Q6_dense_spearman_R"]),
                                                     f4(R["Q6_dense_spearman_median_r"])))
    p("  commit share by year: %s" % {y: "%.1f%%" % (100 * v) for y, v in
                                      R["Q6_commit_share_by_year"].items()})

    # ---------------- Q7: an external scale for "is 0.16 high or low?"
    D = collect(CODE, [CODE_HEAD], ["src/"])
    R["Q7_reference_whole_src"] = {"whole": st(D), "years": by_year(D),
                                   "persons": {k: v["n"] for k, v in by_person(D).items()}}
    p("")
    p("=== Q7  reference scale: the whole code repo's src/ (the inherited tree) ===")
    s = R["Q7_reference_whole_src"]["whole"]
    p("  n=%d  %s..%s  +%d/-%d  R=%s  median_r=%s" % (s["n"], s["first"], s["last"],
                                                      s["add"], s["del"], f4(s["R"]), f4(s["median_r"])))
    p("  by year: %s" % {y: (v["n"], f4(v["R"])) for y, v in R["Q7_reference_whole_src"]["years"].items()})

    with open(os.path.join(out_dir, "f52_probes.json"), "w") as f:
        json.dump(R, f, indent=1)
        f.write("\n")
    with open(os.path.join(out_dir, "f52_probes.txt"), "w") as f:
        f.write("\n".join(T) + "\n")
    print("\n".join(T))


def f4(x):
    return "None" if x is None else "%.4f" % x


if __name__ == "__main__":
    main()
