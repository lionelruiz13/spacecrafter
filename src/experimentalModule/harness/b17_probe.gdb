set pagination off
set confirm off
handle SIGUSR1 nostop noprint pass
# "Did the command reach its handler" evidence that does NOT come from a log
# string the handler prints (b16/b26 pattern, INTENT 11.53/11.55): the
# breakpoint fires inside the running process. Core::setViewOffset is the ONE
# sink both channels (config [navigation] view_offset AND the runtime command
# `set zoom_offset=`) funnel into [core.cpp:494/2111]. Its body calls only the
# OLD navigator (navigation->setViewOffset + setLocalVision) - there is NO
# Camera call, so a hit here with an unchanged Camera dump is the seam.
break Core::setViewOffset
commands
silent
printf "PROBE setViewOffset ENTERED offset=%f\n", offset
continue
end
run
