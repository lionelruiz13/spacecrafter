set pagination off
set confirm off
handle SIGUSR1 nostop noprint pass
break App::takeScreenshot
commands
silent
printf "PROBE file=%s draw=%d pinned=%d\n", filename._M_dataplus._M_p, core._M_ptr->ssystemFactory->drawModularSystem, core._M_ptr->ssystemFactory->pathPinned
continue
end
run
