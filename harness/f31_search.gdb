# F31 / INTENT §5.74 — the app runs UNDER gdb (ptrace_scope=1 blocks attach,
# B10-cmd precedent). Every breakpoint is silent, prints through f31_probe.py
# into $F31_PROBE, and continues: the inferior is never left stopped, so the
# TCP driver on the other side sees only latency, never a hang.
set pagination off
set confirm off
# 40, not 8: `print elements` caps STRING length as well as aggregate length,
# and an 8-char cap would truncate the very catalogue names this probe exists
# to read out (the prefix the driver then searches with must be the catalogue's
# own content, §11.51(d) — a truncated key would make it the probe's content).
set print elements 40
set print repeats 0
set print pretty off
set breakpoint pending on
handle SIGUSR1 nostop noprint pass

source /home/claude/spacecrafter/claude/harness/f31_probe.py

# ---- the aggregation and its four catalogues (core.cpp:2367-2399) ----------
break Core::listMatchingObjectsI18n
commands
silent
python probe_core()
continue
end

break ProtoSystem::listMatchingObjectsI18n
commands
silent
python probe_planet()
continue
end

break ConstellationMgr::listMatchingObjectsI18n
commands
silent
python probe_const()
continue
end

break NebulaMgr::listMatchingObjectsI18n
commands
silent
python probe_neb()
continue
end

break HipStarMgr::listMatchingObjectsI18n
commands
silent
python probe_star()
continue
end

# ---- the load sites and the gate above them --------------------------------
break Core::setSkyCultureDir
commands
silent
python probe_setculture()
continue
end

break core.cpp:1430
commands
silent
python probe_gate_reject()
continue
end

break Core::loadSkyCulture
commands
silent
python probe_loadsky()
continue
end

break ConstellationMgr::loadLinesAndArt
commands
silent
python probe_loadlines()
continue
end

break ConstellationMgr::loadNames
commands
silent
python probe_loadlines_done()
continue
end

break HipStarMgr::loadCommonNames
commands
silent
python probe_loadcommon()
continue
end

break HipStarMgr::updateI18n
commands
silent
python probe_updatei18n()
continue
end

info breakpoints
run
