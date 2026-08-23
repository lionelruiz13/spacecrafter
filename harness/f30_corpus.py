#!/usr/bin/env python3
"""F30 corpus scans - INTENT §5.65 / §5.69 / §5.70 / §5.72.

Four rows each owe one corpus question. This script answers all four over ONE
explicitly enumerated corpus, so that every absence is a MAPPED NEGATIVE (the
files were opened and searched) rather than a failed lookup.

Corpus, stated with paths and file counts in the report:
  S1  installed shipped scripts   ~/.spacecrafter/scripts/**  (*.sts)
  S2  installed script-dir other  ~/.spacecrafter/scripts/**  (.fab .sh .txt .csv)
  S3  code-repo shipped scripts   <repo>/doc/**               (*.sts)
  S4  code-repo shipped data      <repo>/data/*               (all files)
  R1  installed recordings        ~/.spacecrafter/*.sts   <- ScriptMgr::recordScript's
                                  own default output dir (getConfigDir() +
                                  "record_<date>.sts", script_mgr.cpp:212-214)
  R2  installed sessions          ~/.spacecrafter/sessions/**
  T1  code-repo tools/clients     <repo>/util/**, <repo>/www/**, <repo>/sts-extension/**
  T2  installed client surfaces   ~/.spacecrafter/www/**, ~/.spacecrafter/ftp/**
  X   source tree                 <repo>/src/**  (.cpp .hpp .h .c)

Encoding: every file is read as latin-1, which cannot raise - the installed data
is ISO-8859 and the standard grep wrapper silently skips such files (standing
rule, CLAUDE.md). Reading bytes and decoding latin-1 makes the skip impossible.

Each scan carries a POSITIVE CONTROL on the SAME channel: a string known to be
present in that same corpus, searched by the same code path. A scan whose
control returns 0 is reported as INSTRUMENT FAILURE, not as a negative.
"""
import json
import os
import re
import sys

HOME = os.path.expanduser("~")
SC = os.path.join(HOME, ".spacecrafter")
REPO = "/home/claude/spacecrafter"


def walk(root, exts=None):
    out = []
    if not os.path.isdir(root):
        return out
    for dirpath, _dirnames, filenames in os.walk(root):
        for fn in filenames:
            if exts is None or os.path.splitext(fn)[1].lower() in exts:
                out.append(os.path.join(dirpath, fn))
    return sorted(out)


def globdir(root, exts):
    if not os.path.isdir(root):
        return []
    return sorted(
        os.path.join(root, fn) for fn in os.listdir(root)
        if os.path.isfile(os.path.join(root, fn))
        and (exts is None or os.path.splitext(fn)[1].lower() in exts))


CORPUS = {
    "S1_installed_scripts_sts": walk(os.path.join(SC, "scripts"), {".sts"}),
    "S2_installed_scripts_other": walk(os.path.join(SC, "scripts"),
                                       {".fab", ".sh", ".txt", ".csv"}),
    "S3_repo_doc_sts": walk(os.path.join(REPO, "doc"), {".sts"}),
    "S4_repo_data": globdir(os.path.join(REPO, "data"), None),
    "R1_installed_recordings": globdir(SC, {".sts"}),
    "R2_installed_sessions": walk(os.path.join(SC, "sessions")),
    "T1_repo_tools": (walk(os.path.join(REPO, "util"))
                      + walk(os.path.join(REPO, "www"))
                      + walk(os.path.join(REPO, "sts-extension"))),
    "T2_installed_clients": (walk(os.path.join(SC, "www"))
                             + walk(os.path.join(SC, "ftp"))),
    "X_source_tree": walk(os.path.join(REPO, "src"),
                          {".cpp", ".hpp", ".h", ".c"}),
}

# Binary blobs inside T1 (a .vsix zip, .odt, .pdf) would produce noise, not
# hits; they are kept in the corpus (so the count is honest) and searched
# anyway - latin-1 decoding makes that harmless.


def read(path):
    try:
        with open(path, "rb") as f:
            return f.read().decode("latin-1")
    except (OSError, IOError):
        return None


def scan(corpora, pattern, flags=0):
    """Return [(corpus, path, lineno, line)] for every match."""
    rx = re.compile(pattern, flags)
    hits = []
    for name in corpora:
        for path in CORPUS[name]:
            txt = read(path)
            if txt is None:
                continue
            for i, line in enumerate(txt.splitlines(), 1):
                if rx.search(line):
                    hits.append((name, path, i, line.strip()[:300]))
    return hits


REPORT = []


def emit(s=""):
    REPORT.append(s)
    print(s)


def run_scan(title, corpora, pattern, control_pattern, control_desc,
             flags=re.IGNORECASE):
    emit("-" * 78)
    emit(title)
    nfiles = sum(len(CORPUS[c]) for c in corpora)
    emit("  corpus   : %s" % ", ".join(corpora))
    emit("  files    : %d" % nfiles)
    emit("  pattern  : %s" % pattern)
    hits = scan(corpora, pattern, flags)
    ctrl = scan(corpora, control_pattern, flags)
    emit("  CONTROL  : %s -> %d hit(s) in %d file(s)"
         % (control_desc, len(ctrl), len({h[1] for h in ctrl})))
    if not ctrl:
        emit("  *** INSTRUMENT FAILURE: control returned 0; the negative below "
             "is NOT a mapped negative ***")
    emit("  RESULT   : %d hit(s) in %d file(s)"
         % (len(hits), len({h[1] for h in hits})))
    for name, path, ln, line in hits[:80]:
        emit("    [%s] %s:%d: %s" % (name, path.replace(HOME, "~"), ln, line))
    if len(hits) > 80:
        emit("    ... %d more" % (len(hits) - 80))
    return hits, ctrl


def main():
    emit("=" * 78)
    emit("F30 corpus scans - INTENT §5.65 / §5.69 / §5.70 / §5.72")
    emit("=" * 78)
    emit("Corpus enumeration (paths + file counts):")
    for name in CORPUS:
        emit("  %-28s %5d file(s)" % (name, len(CORPUS[name])))
    emit("")

    results = {}

    # ---------------------------------------------------------------- §5.65
    # "whether any shipped script actually issues the pair
    #  [moveto + flag lock_sky_position on] in one block"
    # Spellings verified at the parser, NOT from the row's prose
    # (base_command_interface.hpp:359 ACP_CN_MOVETO="moveto";
    #  :468 ACP_FN_LOCK_SKY_POSITION="lock_sky_position").
    script_corpora = ["S1_installed_scripts_sts", "S2_installed_scripts_other",
                      "S3_repo_doc_sts", "S4_repo_data",
                      "R1_installed_recordings"]
    lock_hits, _ = run_scan(
        "§5.65 (a) - does the sky lock appear in the shipped script corpus at all?",
        script_corpora, r"lock_sky_position",
        r"^\s*flag\b", "any 'flag' command line in the same corpus")
    moveto_hits, _ = run_scan(
        "§5.65 (b) - does 'moveto' appear in the shipped script corpus?",
        script_corpora, r"^\s*moveto\b",
        r"^\s*flag\b", "any 'flag' command line in the same corpus")
    results["5.65_lock_hits"] = len(lock_hits)
    results["5.65_moveto_hits"] = len(moveto_hits)

    # The pair test: same FILE, and then same BLOCK. A block ends at any command
    # that returns a non-zero script wait - verified at source: only `wait`
    # (app_command_interface.cpp:1361), `script` (:2797), `zoom` (:3268) and
    # `camera` (:4186) ever set it, and ScriptMgr::update runs commands in a
    # `while (wait_time==0)` loop (script_mgr.cpp:303), i.e. a run of
    # zero-wait commands executes inside ONE frame.
    lock_files = {h[1] for h in lock_hits}
    moveto_files = {h[1] for h in moveto_hits}
    both = sorted(lock_files & moveto_files)
    emit("-" * 78)
    emit("§5.65 (c) - files carrying BOTH moveto and lock_sky_position: %d" % len(both))
    boundary = re.compile(r"^\s*(wait|script|zoom|camera)\b", re.IGNORECASE)
    lock_on = re.compile(r"^\s*flag\s+lock_sky_position\s+(on|1|true|toggle)\b",
                         re.IGNORECASE)
    moveto_re = re.compile(r"^\s*moveto\b", re.IGNORECASE)
    pairs = []
    for path in both:
        lines = read(path).splitlines()
        last_moveto = None
        for i, line in enumerate(lines, 1):
            if boundary.match(line):
                last_moveto = None
                continue
            if moveto_re.match(line):
                last_moveto = i
                continue
            if lock_on.match(line) and last_moveto is not None:
                pairs.append((path, last_moveto, i,
                              lines[last_moveto - 1].strip(), line.strip()))
    emit("  IN-BLOCK PAIRS (moveto ... flag lock_sky_position on, no yielding "
         "command between): %d" % len(pairs))
    for path, a, b, la, lb in pairs:
        emit("    %s: line %d %r  ->  line %d %r"
             % (path.replace(HOME, "~"), a, la, b, lb))
    results["5.65_files_with_both"] = len(both)
    results["5.65_in_block_pairs"] = len(pairs)
    # Positive control for the BLOCK analyser itself: it must find a pair in a
    # synthetic file that certainly has one.
    ctrl_path = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                             "artifacts", "f30", "control_5_65.sts")
    os.makedirs(os.path.dirname(ctrl_path), exist_ok=True)
    with open(ctrl_path, "w") as f:
        f.write("moveto lat 45 lon 5 alt 0 duration 0\n"
                "flag lock_sky_position on\n"
                "wait duration 1\n"
                "moveto lat 0 lon 0 alt 0 duration 0\n"
                "wait duration 1\n"
                "flag lock_sky_position on\n")
    lines = read(ctrl_path).splitlines()
    cpairs = []
    last_moveto = None
    for i, line in enumerate(lines, 1):
        if boundary.match(line):
            last_moveto = None
            continue
        if moveto_re.match(line):
            last_moveto = i
            continue
        if lock_on.match(line) and last_moveto is not None:
            cpairs.append(i)
    emit("  CONTROL (synthetic %s): analyser finds %d in-block pair(s), "
         "expected exactly 1 (the second pair is separated by a `wait`)"
         % (os.path.basename(ctrl_path), len(cpairs)))
    results["5.65_control_pairs"] = len(cpairs)

    # ---------------------------------------------------------------- §5.69
    # "whether any shipped show relies on today's truncated keep_time value"
    # Spellings at the parser: W_PRELOAD="preload" (:312),
    # W_KEEPTIME="keep_time" (:288).
    pre_hits, _ = run_scan(
        "§5.69 (a) - does any shipped script issue `body action preload`?",
        script_corpora, r"\bpreload\b",
        r"^\s*body\b", "any 'body' command line in the same corpus")
    kt_hits, _ = run_scan(
        "§5.69 (b) - does any shipped script set `keep_time`?",
        script_corpora, r"\bkeep_time\b",
        r"^\s*body\b", "any 'body' command line in the same corpus")
    results["5.69_preload_hits"] = len(pre_hits)
    results["5.69_keep_time_hits"] = len(kt_hits)

    # ---------------------------------------------------------------- §5.70
    # "whether any archived recording already carries the broken
    #  [look delta_az] line"
    # The emitted string is verified at source (core.cpp:2009):
    #   "look delta_az " << deltaAz << " delta_alt " << deltaAlt
    rec_corpora = ["R1_installed_recordings", "S1_installed_scripts_sts",
                   "S2_installed_scripts_other", "S3_repo_doc_sts",
                   "R2_installed_sessions"]
    look_hits, _ = run_scan(
        "§5.70 (a) - does any archived recording/script carry `look delta_az`?",
        rec_corpora, r"\blook\s+delta_az\b",
        r"^\s*(wait|flag|select|moveto|date|body)\b",
        "any recorded-command-shaped line in the same corpus")
    dz_hits, _ = run_scan(
        "§5.70 (b) - any `delta_az` token at all (wider net)?",
        rec_corpora, r"delta_az\b",
        r"^\s*(wait|flag|select|moveto|date|body)\b",
        "any recorded-command-shaped line in the same corpus")
    zoom_hits, _ = run_scan(
        "§5.70 (c) - the ramp's CORRECT sibling `zoom delta_fov` (this is the "
        "sharpest control: if recordings of interactive ramps existed in the "
        "corpus, the zoom half would be there too)",
        rec_corpora, r"\bzoom\s+delta_fov\b",
        r"^\s*(wait|flag|select|moveto|date|body)\b",
        "any recorded-command-shaped line in the same corpus")
    results["5.70_look_delta_az"] = len(look_hits)
    results["5.70_delta_az_any"] = len(dz_hits)
    results["5.70_zoom_delta_fov"] = len(zoom_hits)

    # ---------------------------------------------------------------- §5.72
    # "whether any shipped client subscribes with $LOGON and parses command
    #  answers off that stream"
    logon_corpora = ["T1_repo_tools", "T2_installed_clients", "X_source_tree",
                     "S1_installed_scripts_sts", "S2_installed_scripts_other",
                     "S4_repo_data"]
    logon_hits, _ = run_scan(
        "§5.72 - who produces or consumes `$LOGON`?",
        logon_corpora, r"\$LOGON",
        r"(?i)socket|tcp", "any socket/tcp mention in the same corpus")
    results["5.72_logon_hits"] = len(logon_hits)

    emit("-" * 78)
    emit("MACHINE SUMMARY")
    emit(json.dumps(results, indent=2, sort_keys=True))

    out = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                       "artifacts", "f30")
    os.makedirs(out, exist_ok=True)
    with open(os.path.join(out, "corpus_report.txt"), "w") as f:
        f.write("\n".join(REPORT) + "\n")
    with open(os.path.join(out, "corpus_summary.json"), "w") as f:
        json.dump({"counts": results,
                   "corpus_sizes": {k: len(v) for k, v in CORPUS.items()}},
                  f, indent=2, sort_keys=True)
    print("\nwritten: %s/corpus_report.txt , corpus_summary.json" % out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
