# B5 diagnosis probe - nested-draw liveness (INTENT 6.9 draw-half).
# Launch: gdb -batch -x b5_probe.gdb --args <binary>   (cwd = app cwd)
# Prints one line per drawNested/drawStarProxy/Renderer::drawHalo hit.
set pagination off
set confirm off
handle SIGUSR1 nostop noprint pass
break ModularSystem::drawNested
commands
silent
printf "NESTED %s d=%g sub=%g vis=%d bodyvis=%d\n", this->englishName._M_dataplus._M_p, this->distance, this->subsystemRadius, this->isVisible, this->isBodyVisible
continue
end
break ModularSystem::drawStarProxy
commands
silent
printf "PROXY %s d=%g\n", this->englishName._M_dataplus._M_p, this->distance
continue
end
break Renderer::drawHalo
commands
silent
printf "HALO x=%g y=%g rmag=%g\n", pos.first, pos.second, rmag
continue
end
run
