# F36 / INTENT §5.77 — generated wrapper; the probe sets its own
# breakpoints through the gdb Python API (f36_probe.py), so this file
# only fixes the gdb settings the probe depends on.
set pagination off
set confirm off
set breakpoint pending on
set print elements 200
handle SIGUSR1 nostop noprint pass
source /home/claude/spacecrafter/claude/harness/f36_probe.py
run
