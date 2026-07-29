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
  into that row's scope without a decision.
  **NOT DELIVERED by B4's implementation wave (2026-07-25, §11.111(j))** — and
  the reason is worth the request's own record: writing an anchor section is a
  SERIALIZATION path, and the B31 design pass has already ruled that the one
  existing "save my camera to a file" surface (`camera action save`, defective
  as shipped — §5.41) *"must be re-expressed on the session serializer rather
  than extended"* (`b31-design.md` §3.4(d)), under the §11.66(b) preserve-the-
  file writer contract. Minting a second serialization path to save nine lines
  would be exactly the duplication I2 forbids, and would pre-empt D30–D36. So
  the request is ALIVE and now has a home: it lands with B31's writer, as one
  more output format of the one serializer. Everything it needs already exists
  on the new path — the runtime anchor carries its own declaration (kind,
  parent/body, orbit or coordinates), which IS the anchor.ini section to write.

### [2026-07-29] Script-editor TUI: autocomplete, direct TCP, static analysis, inline key documentation with defaults
- **From:** Vixy (side-note carried by the `DECISIONS_PENDING.md` D31 answer,
  2026-07-26; recorded here at the §11.113 propagation pass)
- **Request:** *"a side project, writing a tui with mouse support - a script
  editor with autocomplete, direct tcp mode, static analysis to report errors
  early and showing documentation of the currently edited call + attribute key
  documentation corresppnding to the key/value the cursor is on with default
  value shown greyed out and candidate for autocomplete when the value field is
  empty"*. Not a request against spacecrafter itself — an external tool — but it
  consumes three surfaces this project owns, which is why it is recorded rather
  than left in a decision file.
- **Status:** new — recorded as design INPUT, no commitment. Three consequences
  worth carrying into the work that touches those surfaces:
  (1) the **command grammar** and the **data-key vocabulary** become
  MACHINE-consumed, not only human-read — B38's command census
  (`capability-surface.md`) and B24/B25/B27's key inventory are the raw material,
  and §2.0 D10's documentation gate acquires a consumer that cannot infer intent
  from prose;
  (2) the **TCP channel** becomes an editor-facing API (completion/validation
  round-trips), a use case beyond running a show — its stability and error
  reporting are judged against that too;
  (3) *"default value shown greyed out"* wants **defaults to be declared data
  rather than code constants** — the same authority question §2.0 D12's
  acting-default logging raises from the other end (a default that must be
  displayed and a default that must be logged both need a single place that
  knows what it is).
