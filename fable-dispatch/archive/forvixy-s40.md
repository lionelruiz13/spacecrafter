- **Session-40 decision items (2026-09-27, Sunday — THE STRUCTURE / IMPLEMENTATION SPLIT, on your two answers of 13:42; F132-F138 minted, record §11.293):**

  ```
  YOURS — ONE LIST: `claude/DECISIONS_PENDING.md` SECTION 17 (each answerable in one sentence; the page form is on b42's page)
  THE TWO THAT UNBLOCK F137's CUBIC (F133, §11.295): F-1 which axes are cubed (all, as ruled, kills the joystick's
      eye-relative view axes under turnHorizontal's 0.3 threshold) · F-2 SENSITIVITY 64000 -> 32767 on the cubed axes
      (at 64000 full deflection normalises to 0.48: the cubic is 4.3x SLOWER at full, 22x at half). A sentence + a value.
  THE FOV SLICE (F132 §11.294 + F139 §11.301): the UBO alone puts the seam INSIDE the sky (worse for a grid-or-band
      zoom); the star field visits 26 561 stars per frame at fov >= 90 on the worker thread, so its shape is decided by
      COST: template policy (free) for the star field, the interface for the eight other layers (< 50 us, ~90
      declarations, Projector implementing it); the star field alone closes row 5 but leaves the constellation LINES
      behind their stars -> both steps in one delivery (my default) or the star field first; FIVE NAMES yours.
      F136 LANDED (code 2fccb470: the reader, the readouts, the leak - no drawn change); F140 waits on this.
  H6 MEASURED after F136: a key tap during a timed zoom now rests the Camera at ITS OWN value (the jump gone, the
      rest gap 23 -> 36 deg; a step AGAINST the plan re-times it to 36.75 s through the never-reached branch) -
      the ONE open item on the key-zoom line; F136 is one commit to revert if you want the tap kept as it was.
  SLICE 5 LANDED (code 618cf046 + 0d151c8b): the canary's new arm back in band on the boot it was red, by code.
  Q-F (NEW): where does the SENDER get the immediate-movement law — R1 the Camera's (your words; the 5.184 class in
      relative form on the old observer) · R2 each engine on its own (the old has none: old pin unchanged) · R3 through
      the B42 gate, two implementations (my default; F133 argued it from the code; B44 = B42's first client).
  F-3 per-axis or 2-vector · F-4 the anchored lateral's LAW ((i) velocityScaling / distance, my default) · F-5 the zero
      floor on a zero-radius point (fail loud?) · F-10 the recorder · M3.
  b42 Q2 scope · Q3 the screen (its premise refuted: your module already holds a second screen holder) · Q6 the mode
      holder's NAME · Q9 dragView's frame · H6 the step-during-plan law · H7 the fov limits · H8 `oldOk || newOk` ·
      H10 the partial moveto.
  Section 16: Q-B's LAW (legacy thresholds first, or the area of influence at once?) · Q-C temp_point · Q-D the wait.
  VETO POINTS OF MY READING (§11.293(b)(c)(e)): requests keep both writes forever; shape C; Q4's shared-law default
      excluded by your "two implementations"; the executor split read as a METHOD, not a one-round call.
  FOR THE PAGE (F135 §11.297): observation 1 is SEASONAL (half the year the new pin shows stars with the defect
      intact); in the race-WON state the first lat/lon key still throws the old observer 80.8 deg.
  THE CANARY: eighth boot re-banked at warm-up; the FULL run = 5.186 to the digit on this boot — slice 5's before,
      F138 in flight on it.
  ```

