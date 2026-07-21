set pagination off
set confirm off
handle SIGUSR1 nostop noprint pass
# "Did the command reach its handler" evidence that does NOT come from a log
# string the handler itself prints: the breakpoint fires inside the running
# process (b26_probe.gdb pattern, INTENT 11.53).
break SSystemFactory::reloadCurrentSystem
commands
silent
printf "PROBE reloadCurrentSystem ENTERED\n"
continue
end
run
