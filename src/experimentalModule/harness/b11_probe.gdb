set pagination off
set confirm off
handle SIGUSR1 nostop noprint pass

# --- probe 1: THE ACCUMULATION CODE ITSELF --------------------------------
# TrailModule::accumulate is the only writer of the trail buffer, and it is
# called OUT OF LINE from TrailModule::update (verified in the shipped binary:
# `call <_ZN11TrailModule10accumulateEP11ModularBody>` inside update), so a
# symbol breakpoint sees every entry.
# Kept DISABLED and armed for ONE shot by probe 2: at 60 fps x ~10 trailed
# bodies an always-on breakpoint would stop the inferior ~600 times a second.
# Armed, it answers exactly one question - "did the accumulation code run at
# all after this flag change?" - which is the DoD-4 question.
break TrailModule::accumulate
commands
silent
printf "PROBE accumulate RAN\n"
disable 1
continue
end
disable 1

# --- probe 2: THE COMMAND PATH -------------------------------------------
# Fires only if a command actually reaches the trail-flag handler. This is the
# spelling check that does NOT come from a log line the handler itself prints:
# a non-existent command is swallowed silently by this interface (B16
# carry-over), so "no probe line" is the positive evidence that a misspelling
# reached nothing. Arms probe 1 on every flag change.
break CoreLink::planetsSetFlagTrails
commands
silent
printf "PROBE planetsSetFlagTrails b=%d\n", b
if b != 0
  enable 1
end
continue
end

# --- probe 3: the hide path (same spelling logic, other axis) -------------
break CoreLink::setPlanetHidden
commands
silent
printf "PROBE setPlanetHidden name=%s hidden=%d\n", name._M_dataplus._M_p, planethidden
continue
end

run
