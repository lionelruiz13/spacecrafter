#!/bin/bash
# B14/sat6-era alias of the ONE command battery (I2).
#
# This file used to be a byte-for-byte COPY of b10_cmd_battery_run.sh whose only
# difference was the ssystem md5 it ECHOED (940cdaa4..., stale by many waves and
# compared to nothing) - i.e. the duplication existed solely to carry a value
# that was never asserted. The assert now lives once, in the runner below, and
# the corpus expectation is overridable there (SSYS_PRISTINE), so this name
# survives as an entry point and nothing is duplicated.
#
# Zero callers at the time of the merge [observed 2026-07-30: whole-tree grep];
# kept rather than deleted because ledger entries name harness files by name.
exec "$(cd "$(dirname "$0")" && pwd)/b10_cmd_battery_run.sh" "$@"
