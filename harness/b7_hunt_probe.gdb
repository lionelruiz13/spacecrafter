# B7 §11.15d shutdown-segfault ACTIVE-HUNT probe (2026-07-24, dispatch row B7).
# Superset of b7_probe.gdb: also PASSES the teardown signals (SIGINT/SIGTERM/
# SIGQUIT) through to the app's own handler (main.cpp:340-343 -> NSSigTERM ->
# app.flag(ALIVE,false) == the `shutdown action now` teardown), so the hunt can
# exercise the SIGNAL entry path -- the ONE true §11.15d fire (B23, §11.74(c))
# fired on a SIGINT teardown, not on a command teardown.
set pagination off
set confirm off
# App's stall watchdog raises SIGUSR1 for its own stacktrace dump (fps.cpp:128,
# §11.5) - pass it through, it is NOT the teardown segv.
handle SIGUSR1 nostop noprint pass
# Teardown triggers: let them reach the app's signal handler untouched. gdb's
# default is stop for these, which would freeze the inferior instead of tearing
# it down; nostop noprint pass routes them to NSSigTERM.
handle SIGINT  nostop noprint pass
handle SIGTERM nostop noprint pass
handle SIGQUIT nostop noprint pass
# The fires we WANT to catch: a segfault (§11.15d) or an abort (glibc double-free
# / assert) DURING teardown. Stop + print so the post-run stack captures WHERE.
handle SIGSEGV stop print
handle SIGABRT stop print
run
# Control returns here when the inferior STOPS (a caught fire) or EXITS. Clean
# exit => the stack commands print "No stack" (harmless, batch). A fire => they
# print exactly where the §11.15d teardown crash is + all thread states.
echo \n==== POST-RUN STACK (empty => clean exit; non-empty => a stop/fire) ====\n
bt
echo \n==== ALL THREADS ====\n
thread apply all bt
echo \n==== REGISTERS (only meaningful on a fire) ====\n
info registers rip rsp rbp
echo \n==== END PROBE ====\n
