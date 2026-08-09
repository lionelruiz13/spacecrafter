# F35 / §5.81 — the guard branch, OBSERVED, at the instruction that is the
# branch body rather than at the source line.
#
# `break ModularBody.hpp:508` DOES NOT WORK and the failure is silent: gdb
# resolves a line to that line's statement start, which the compiler placed on
# the COMMON path (the `ucomiss` of `distance == 0.f`, +50), so the probe fires
# for every visible body every frame — measured 22613 hits with Earth (dist
# 4.3e-05), Moon (2.4e-03) and Mercury (1.07) among them. A probe that fires on
# the branch NOT taken turns an observation into fiction, so the address is
# taken from the guard's own instruction instead:
#
#   507  if (distance == 0.f) {          +50 pxor / +54 ucomiss / +57 jp / +66 jne
#   508      screenPos.first = ... 0.f;  +72 movlps %xmm0,0x30c(%rbx)   <-- HERE
#
# `ModularBody::update` is called OUT OF LINE from `recursiveUpdate` (measured
# in the backtrace), so the single out-of-line copy is the live one. The binary
# is PIE, so the address is taken AFTER `start` and the probe prints the
# instruction it landed on — the run's own record of what was instrumented.
#
# SIGUSR1 is the app's stall watchdog; without `pass` gdb stops the inferior on
# it and a batch script then quits, killing the app mid-run (measured: the port
# never reopened for the second leg).
set pagination off
set confirm off
set breakpoint pending on
handle SIGUSR1 nostop noprint pass
start
set $guard = (char *)&'_ZN11ModularBody6updateEdRK7Matrix4IfE' + 72
printf "F35-PROBE-AT "
output $guard
printf "\n"
x/2i $guard
break *$guard
commands
silent
printf "F35-GUARD-TAKEN %s\n", this->englishName._M_dataplus._M_p
continue
end
continue
quit
