   **Definition-drift assert
   (2026-08-29, §11.161(g)):** ~~`md5sum claude/agents/opus-xhigh.md
   .claude/agents/opus-xhigh.md` must MATCH — the tracked file is the authority, the
   `.claude` one its deployed projection; on mismatch, regenerate the projection from
   the authority before any dispatch.~~ **[SUPERSEDED 2026-10-10, §11.302, on the owner's
   word (the executor runs under his system prompt): `bash claude/agents/deploy.sh --check`
   must exit 0 — the projection is GENERATED (frontmatter + `/home/claude/shared/system-prompt-agent.txt`,
   the AGENT form of his prompt — the orchestrator-only part dropped, the agent preamble appended,
   same day — + the bridge + the authority's body) and compared byte-exact; on drift, `bash
   claude/agents/deploy.sh` before any dispatch. The md5 pair is no longer equal by
   construction, so the old line would always fail.]**
