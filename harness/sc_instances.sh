#!/bin/bash
# sc_instances.sh -- the SHELL FRONT END of the one concurrent-instance probe.
# F112, INTENT Sec.11.238.
#
#   . "$HERE/sc_instances.sh" is NOT how this is used; call it:
#     bash "$HERE/sc_instances.sh" --assert <label> || exit 2
#
# usage (every flag is the authority's; this file adds none):
#   sc_instances.sh                      list hits, exit 0 clear / 2 hit
#   sc_instances.sh --assert [LABEL]     one summary line, exit 0 / 2
#   sc_instances.sh --quiet              silent, exit 0 / 2
#   sc_instances.sh --json FILE          write the hits as json too
#   sc_instances.sh --kill               SIGTERM then SIGKILL every hit, BY PID
#   sc_instances.sh --self-test          THIS file's suite (see below)
#   exit 0 clear | 2 an engine is live | 4 the probe could not run
#
# WHY THIS FILE HOLDS NO CRITERION.  The mandate asked for the probe "in two
# languages".  Two IMPLEMENTATIONS of one criterion is the duplication I2 names
# as a pending silent desync, and this corpus is here BECAUSE of that class: 44
# harness files carried a copy-pasted `comm == "spacecrafter"` test, every one of
# them blind to a renamed binary, and no single edit could fix them
# (Sec.11.231(j2)).  So there are two ENTRY POINTS and exactly one criterion:
# sc_instances.py is the authority, this file delegates to it and owns nothing
# but the delegation.  A shell caller still gets one line; a desync is now
# impossible rather than merely unlikely.
#
# The cost is a python3 process per probe (~40 ms).  python3 is already a hard
# dependency of every path that would call this: f56_canary.sh runs
# f56_manifest.py, f90_rehearsal_run.sh runs f90_rehearsal.py, the D14 gate is
# f70_ascii.py.  If python3 is missing this exits 4 -- a probe that cannot run is
# a STOP, never a silent pass.
#
# SC_INSTANCES_PY overrides the authority's path (used by --self-test only).
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
PY="${SC_INSTANCES_PY:-$HERE/sc_instances.py}"

# ---------------------------------------------------------------- --self-test
# This suite tests the FRONT END -- delegation, exit-code mapping, and the one
# behaviour a wrapper can silently get wrong: swallowing a failure.  The
# criterion's own suite is `sc_instances.py --self-test [--mutant NAME]`.
if [ "${1:-}" = "--self-test" ]; then
    shift || true
    P=0; F=0
    ok()   { P=$((P+1)); echo "  PASS $1  $2"; }
    bad()  { F=$((F+1)); echo "  FAIL $1  $2"; }
    echo "sc_instances.sh --self-test"

    # s0: the authority is reachable and its own suite is green.  A front end
    #     whose authority is broken must not report a clean host.
    if out=$(timeout 60 python3 "$PY" --self-test 2>&1) && \
       echo "$out" | grep -q ' 0 FAIL'; then
        ok s0 "the authority's own criterion suite is green ($(echo "$out" | tail -1 | tr -s ' '))"
    else
        bad s0 "the authority's criterion suite is not green"
    fi

    # s1: delegation returns the authority's verdict verbatim on this host.
    a=$(timeout 60 python3 "$PY" --assert selftest 2>&1); arc=$?
    b=$(bash "$0" --assert selftest 2>&1); brc=$?
    if [ "$a" = "$b" ] && [ "$arc" = "$brc" ]; then
        ok s1 "front end and authority agree, text and exit code ($arc)"
    else
        bad s1 "front end $brc [$b] vs authority $arc [$a]"
    fi

    # s2: a CLEAR host maps to exit 0 (this host is clear right now; if it is
    #     not, this case says so rather than passing).
    bash "$0" --quiet; rc=$?
    if [ "$rc" = 0 ]; then
        ok s2 "no engine live on this host -> exit 0"
    elif [ "$rc" = 2 ]; then
        bad s2 "an engine IS live -- run this suite on a clear host (exit 2)"
    else
        bad s2 "unexpected exit $rc"
    fi

    # s3: a MISSING authority exits 4, NOT 0.  This is the case that matters:
    #     the failure mode of a wrapper is to report success when its tool is
    #     gone, which is exactly how an unverified run reads like a verified one.
    SC_INSTANCES_PY=/nonexistent/sc_instances.py bash "$0" --quiet > /dev/null 2>&1
    rc=$?
    [ "$rc" = 4 ] && ok s3 "a missing authority exits 4 (a probe that cannot run STOPS)" \
                  || bad s3 "a missing authority exited $rc, expected 4"

    # s4: a BROKEN authority (present, but exits non-zero for a harness reason)
    #     propagates 4 rather than being read as "clear".
    tmp=$(mktemp -d)
    printf '#!/usr/bin/env python3\nimport sys\nsys.stderr.write("boom\\n")\nsys.exit(4)\n' \
        > "$tmp/sc_instances.py"
    SC_INSTANCES_PY="$tmp/sc_instances.py" bash "$0" --quiet > /dev/null 2>&1
    rc=$?
    [ "$rc" = 4 ] && ok s4 "a broken authority propagates 4" \
                  || bad s4 "a broken authority gave $rc, expected 4"

    # s5: an authority that reports a HIT propagates 2, not 0.
    printf '#!/usr/bin/env python3\nimport sys\nprint("HIT")\nsys.exit(2)\n' \
        > "$tmp/sc_instances.py"
    SC_INSTANCES_PY="$tmp/sc_instances.py" bash "$0" --quiet > /dev/null 2>&1
    rc=$?
    [ "$rc" = 2 ] && ok s5 "a hit propagates 2" || bad s5 "a hit gave $rc, expected 2"

    # s6: SHOWN ABLE TO FAIL -- the mutant front end, the one real wrapper bug:
    #     `python3 "$PY" "$@" > /dev/null; exit 0`, i.e. the exit code swallowed.
    #     s3/s4/s5 must all refuse it.
    cat > "$tmp/mutant.sh" <<'MUT'
#!/bin/bash
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
PY="${SC_INSTANCES_PY:-$HERE/sc_instances.py}"
python3 "$PY" "$@" > /dev/null 2>&1
exit 0
MUT
    m=0
    SC_INSTANCES_PY=/nonexistent/x.py bash "$tmp/mutant.sh" --quiet >/dev/null 2>&1 || m=$?
    [ "$m" = 0 ] && m1=swallowed || m1=caught
    m=0
    SC_INSTANCES_PY="$tmp/sc_instances.py" bash "$tmp/mutant.sh" --quiet >/dev/null 2>&1 || m=$?
    [ "$m" = 0 ] && m2=swallowed || m2=caught
    if [ "$m1" = swallowed ] && [ "$m2" = swallowed ]; then
        ok s6 "the mutant front end (exit code swallowed) is refused by s3 and s5 -- this suite can fail"
    else
        bad s6 "the mutant was not distinguishable: missing=$m1 hit=$m2"
    fi
    rm -rf "$tmp"

    # s7: the front end never matches ITSELF.  Its own exe is bash; the
    #     authority reads exe, never a command line (Sec.11.231(j)(1)).
    self=$(readlink /proc/$$/exe 2>/dev/null || echo "?")
    case "${self##*/}" in
        bash|sh|dash) ok s7 "this wrapper's own exe is ${self##*/}, which no rule matches" ;;
        *)            bad s7 "this wrapper's exe is $self -- check E1-E5" ;;
    esac

    echo "  $P PASS $F FAIL"
    [ "$F" = 0 ] || exit 1
    exit 0
fi

# ------------------------------------------------------------------ delegate
if [ ! -r "$PY" ]; then
    echo "sc_instances.sh: HARNESS ERROR: the criterion's authority is not" >&2
    echo "  readable at: $PY" >&2
    echo "  A probe that cannot run is a STOP, not a clear host: exiting 4." >&2
    exit 4
fi
if ! command -v python3 > /dev/null 2>&1; then
    echo "sc_instances.sh: HARNESS ERROR: python3 not on PATH; exiting 4." >&2
    exit 4
fi
exec python3 "$PY" "$@"
