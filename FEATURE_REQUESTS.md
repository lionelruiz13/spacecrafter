# External feature requests

Single place for feature requests coming from outside the development
process itself (users, testers, anyone not already tracking work through
`src/experimentalModule/INTENT.md`). This file exists so a request made
today isn't lost before someone with context gets to triage it — it does
not imply a request will be accepted, only that it's recorded and will be
looked at.

See `INTENT.md` §13.D for how entries here connect to the internal tracker
once triaged.

## How to add a request

Append an entry at the bottom of the log below, in this form:

```
### [YYYY-MM-DD] Short title
- **From:** who's asking (name, or "anonymous" is fine)
- **Request:** what you want, in your own words — the more concrete the
  use case, the easier it is to evaluate (what were you trying to do when
  you noticed this was missing?)
- **Status:** new
```

Don't edit or remove existing entries other than to update their `Status`
field once triaged (`new` → `under consideration` / `accepted — tracked as
<INTENT.md ref>` / `declined — <reason>` / `duplicate of <entry>`).

## Log

### [2026-07-21] Fly to another star and arrive in its system
- **From:** tester (answering `USER_QUESTIONS.md` Q22, 2026-07-21 round)
- **Request:** *"We would like the possibility to select a star and get there
  by switching to the 'galaxy' mode and then to the 'stellar_system' mode
  (if existing or created from scratch like it is if there is no specific
  one)."* — i.e. a star should be a destination: select it, travel out
  through the galactic view, and arrive in a system around it, with a
  default/generated system when no authored one exists. Recorded because it
  is larger than the question that produced it (which only asked whether a
  star should have a visible surface — that half is answered and tracked
  separately).
- **Status:** accepted — tracked as `INTENT.md` §13.A **A30** (2026-07-21:
  it turned out this was already planned but never written down; the design
  is now stated — stars promote to a real system on demand, authored ones
  come from per-system files. What you asked for is in; the remaining
  decisions are internal)

### [2026-07-21] Command to save the current camera position into anchor.ini
- **From:** tester (answering `USER_QUESTIONS.md` Q5, 2026-07-21 round)
- **Request:** anchors are edited by hand and created from scripts, and
  cross-session persistence is *not* needed — but *"it could be useful to
  have a command that saves it in the anchor.ini"*, i.e. capture the
  camera where it currently is and write it out as a named anchor instead
  of hand-computing the coordinates.
- **Status:** under consideration — folded into `INTENT.md` §13.B **B4**
  (CameraAnchors port design); recorded here so it is not silently absorbed
  into that row's scope without a decision
