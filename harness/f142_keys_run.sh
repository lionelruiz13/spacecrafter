#!/bin/bash
# F149 key leg - one fresh launch under the FIFO-fed gdb of f124_run.sh, driving f142_keys.py.
#
#   ./f142_keys_run.sh <label> --phase before|after [--baseline <label of a before run>]
#     records under artifacts/f142/<label> (drive.log, gdb.log, meta.txt, N_reading.txt, diag.txt,
#     readings.json); the fixtures under artifacts/f142/run are rewritten by the driver each run
#     SC_BIN another binary; F124_TIMEOUT the driver's bound in seconds (default 500 here: a hang must end
#     inside the Bash tool's 600 s foreground window so the runner's own teardown runs - F157, INTENT 11.330)
#
# The launch asserts (binary md5, instance probe, GPU gate, config.ini / ssystem.ini md5 in == out,
# teardown by this launch's own pids) are f124_run.sh's; its exit code is passed through.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
LABEL=${1:?label}
shift
F124_OUT_ROOT="$HERE/artifacts/f142" F124_TIMEOUT="${F124_TIMEOUT:-500}" \
    exec bash "$HERE/f124_run.sh" "$LABEL" f142_keys.py "$@"
