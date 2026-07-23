set pagination off
set confirm off
handle SIGUSR1 nostop noprint pass
# Swallow-guard (B11 precedent, INTENT 11.56): a WRONG command spelling is
# silently swallowed. The breakpoints fire inside the running process at the
# routing seam the correct spelling reaches (SSystemFactory::setBody*Radius),
# so the hit count is the ground truth for "the spelling landed", independent
# of any log the handler prints. A bogus spelling never reaches here.
break SSystemFactory::setBodyDatumRadius
commands
silent
printf "PROBE setBodyDatumRadius ENTERED\n"
continue
end
break SSystemFactory::setBodyGroundRadius
commands
silent
printf "PROBE setBodyGroundRadius ENTERED\n"
continue
end
run
