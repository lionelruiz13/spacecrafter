set pagination off
set confirm off
handle SIGUSR1 nostop noprint pass
# "Did the command reach the NEW-path Camera" evidence that does NOT come from
# a log the handler prints: a breakpoint inside Camera::setSkyLock, fired in
# the running process (b16/b26 probe pattern, INTENT 11.53/11.55). The mirror
# lives in Core::setFlagLockSkyPosition (core.cpp), which every channel of the
# `flag lock_sky_position` command routes through; the bogus spelling must
# produce ZERO fires (INTENT 11.58 flag-spelling trap).
break Camera::setSkyLock
commands
silent
printf "PROBE setSkyLock ENTERED b=%d\n", (int)b
continue
end
run
