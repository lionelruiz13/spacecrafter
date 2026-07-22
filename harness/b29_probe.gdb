# B29 runtime-color seam probe (INTENT §11.65).
# The app runs UNDER gdb (ptrace_scope=1 blocks attach); SIGUSR1 is the app's
# stall watchdog - pass it through. The seam's two entry points (factory
# setBodyColor / setDefaultBodyColor) are inline header methods, so a breakpoint
# on them fans out to every inlined copy (some frames drop the arg symbols). We
# break instead on the concrete OLD sinks they each call EXACTLY ONCE
# (SolarSystemColor::setBodyColor / ::setDefaultBodyColor, real .cpp functions):
# the hit count is 1:1 with the factory call, the args are always in scope, and
# the hit proves the command reached the seam (the new-path routing is the
# deterministic code right after, whose effect the dump then reads).
set pagination off
set breakpoint pending on
handle SIGUSR1 nostop noprint pass

break SolarSystemColor::setBodyColor
commands
  silent
  printf "PROBE setBodyColor name=%s channel=%s\n", englishName._M_dataplus._M_p, colorName._M_dataplus._M_p
  continue
end

break SolarSystemColor::setDefaultBodyColor(std::string const&, Vec3f const&)
commands
  silent
  printf "PROBE setDefaultBodyColor channel=%s\n", colorName._M_dataplus._M_p
  continue
end

run
