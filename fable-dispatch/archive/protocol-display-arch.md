   - **Display architecture is part of the instrument (2026-07-31, §11.122(o) +
     §11.123(o)/(o2)):** claude renders on his OWN headless GNOME/Xwayland `:2`
     (GPU-real; the harness default); Vixy's remmina/RDP relay is view-only and its
     CPU is coupled to what WE draw. The "161.3 fps" cadence label is UNATTRIBUTED
     among three clocks that all fail to match it exactly (config `maximum_fps = 144`,
     `:2` virtual monitor 59.96, panel 164.5) — ~~sharpest hypothesis: the dwell's
     "20 s" denominator was nominal and the true cadence is EXACTLY the config cap
     (§11.123(o2), H1; discriminating check owed by the next cadence-touching task:
     wall-clock-bracketed counter reads). Until settled:~~ **[DISCHARGED 2026-08-29,
     F45 §11.159(k7), accepted by this item's owner: 4392 frames over a [30.0,
     30.5] s bracket = [144.0, 146.4] fps, 4× reproduced ⇒ H1 CONFIRMED on the
     CURRENT (post-reboot, F43-rebuilt) stack — cadence = the config cap. The
     historical "161.3" label is permanently unattributable: its stack was
     destroyed by the 2026-08-27 reboot (§11.121(m)'s retroactive shape).]**
     Standing: trust counter RATIOS and
     in-run A/B only; never absolute fps labels, never cross-session cadence; a
     stack change (compositor, streamer, headless X, screen power state) ⇒ report +
     re-baseline.
