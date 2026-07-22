set pagination off
set confirm off
# App's stall watchdog raises SIGUSR1 for its own stacktrace dump (fps.cpp:128,
# §11.5) - pass it through so it does not stop us.
handle SIGUSR1 nostop noprint pass
# We WANT gdb to stop on SIGSEGV (the §11.15d fire) and give us the backtrace -
# that is the context the exit-code instrument cannot capture. gdb's default is
# stop+print for SIGSEGV; make it explicit.
handle SIGSEGV stop print
run
# Control returns here when the inferior STOPS (a signal) or EXITS. If it exited
# cleanly, the following stack commands print "No stack" (harmless, batch). If it
# SIGSEGV'd at teardown, they print exactly where the §11.15d fire is.
echo \n==== POST-RUN STACK (empty => clean exit; non-empty => a stop/fire) ====\n
bt
echo \n==== ALL THREADS ====\n
thread apply all bt
