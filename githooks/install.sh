#!/usr/bin/env bash
#
# install.sh -- point both repos at githooks/, then PROVE the hook fires.
#
# Writing `core.hooksPath` and reporting success is reporting that a config key
# was set, which is not the claim anyone cares about. The claim is "a commit
# carrying a derivation is now refused", and the only evidence for it is a
# refusal. So this script stages synthetic blobs into a THROWAWAY INDEX
# (GIT_INDEX_FILE -- the real index and the working tree are never touched) and
# runs the hook against them:
#
#     positive 1  an ELF magic blob, 8 bytes    -> must be refused (the criterion)
#     positive 2  a 9 MiB text blob             -> must be refused (the proxy)
#     negative    a 12-byte text blob           -> must PASS
#
# The negative control is not decoration: a hook that refuses everything passes
# both positives perfectly, and would then block every commit in the repo. Two
# positives and no negative is a detector that cannot be told from a wall.
#
# Idempotent. Safe to re-run; that is how you re-check a clone.
#
# THE LIMIT, STATED: git config is per-clone and is not cloned. A fresh clone
# has no hook until this runs. Nothing in a repository can close that -- it is a
# property of git, not an oversight here -- so it is documented in README.md
# next to the layout precondition rather than left to be rediscovered.
# ---------------------------------------------------------------------------
set -euo pipefail

HOOKS_DIR=$(cd "$(dirname "$(readlink -f "$0")")" && pwd)
HARNESS=$(cd "${HOOKS_DIR}/.." && pwd)
CODE=$(cd "${HARNESS}/.." && pwd)

# The body is a SUBSHELL -- `() ( ... )`, not `() { ... }` -- so the exported
# GIT_INDEX_FILE cannot outlive it and redirect the caller's git commands at a
# throwaway index. (The first version used a RETURN trap for that cleanup; a
# RETURN trap set inside a function is not scoped to it, so it fired again on
# the NEXT function's return, where its variable no longer existed and `set -u`
# killed the run. Measured, not reasoned about: the harness leg had already
# passed when the code leg died.)
selftest() (                       # selftest <repo> <hook> -> 0 if all four legs behave
    repo=$1; hook=$2; ok=1
    idx=$(mktemp -u); export GIT_INDEX_FILE="${idx}"
    trap 'rm -f "${idx}"' EXIT
    git -C "${repo}" read-tree HEAD 2>/dev/null || git -C "${repo}" read-tree --empty

    # positive 1 -- the criterion, and TINY on purpose: 8 bytes. The point the
    # size ceiling cannot make is that there is no size at which a compiled
    # object becomes an origin, so the leg that proves the criterion works must
    # be far below any threshold. If this passes while the 9 MiB leg fails, the
    # hook has quietly reverted to being a size policy.
    blob=$(printf '\177ELF\002\001\001\000' | git -C "${repo}" hash-object -w --stdin)
    git -C "${repo}" update-index --add --cacheinfo "100755,${blob},__selftest_elf"
    rc=0; (cd "${repo}" && "${hook}" >/dev/null 2>&1) || rc=$?
    [ "${rc}" -ne 0 ] || { echo "  SELFTEST FAIL: an ELF blob was NOT refused." >&2; ok=0; }
    git -C "${repo}" update-index --force-remove "__selftest_elf"

    # positive 2 -- a derivation with NO magic number this script knows: a Java
    # class file, 16 bytes. It exercises the file(1) channel specifically; the
    # magic-byte fallback would also catch this one (cafebabe), so it is not a
    # proof that file(1) ran -- which is why the .pyc below is here too, whose
    # signature the fallback does NOT know.
    blob=$(printf '\312\376\272\276\000\000\0004\000\010' | git -C "${repo}" hash-object -w --stdin)
    git -C "${repo}" update-index --add --cacheinfo "100644,${blob},__selftest_class"
    rc=0; (cd "${repo}" && "${hook}" >/dev/null 2>&1) || rc=$?
    [ "${rc}" -ne 0 ] || { echo "  SELFTEST FAIL: a Java class blob was NOT refused." >&2; ok=0; }
    git -C "${repo}" update-index --force-remove "__selftest_class"

    # positive 3 -- the file(1) channel ALONE: a 12-byte .pyc, whose signature is
    # deliberately absent from the magic-byte fallback. If file(1) is missing or
    # stopped being consulted, this is the leg that says so.
    blob=$(printf '\003\363\r\n\000\000\000\000\000\000\000\000' | git -C "${repo}" hash-object -w --stdin)
    git -C "${repo}" update-index --add --cacheinfo "100644,${blob},__selftest_pyc"
    rc=0; (cd "${repo}" && "${hook}" >/dev/null 2>&1) || rc=$?
    [ "${rc}" -ne 0 ] || { echo "  SELFTEST FAIL: a .pyc blob was NOT refused -- the file(1) channel is dead." >&2; ok=0; }
    git -C "${repo}" update-index --force-remove "__selftest_pyc"

    # positive 4 -- the RESIDUE layer, which is a different claim: 9 MiB of text
    # (one repeated byte, so it costs a few KB on disk; cat-file -s still reports
    # the true 9437184). Nothing about it is derived as far as any detector can
    # tell -- that is exactly the case this layer exists to hand back to a human.
    blob=$(head -c 9437184 /dev/zero | tr '\0' 'a' | git -C "${repo}" hash-object -w --stdin)
    git -C "${repo}" update-index --add --cacheinfo "100644,${blob},__selftest_big"
    rc=0; (cd "${repo}" && "${hook}" >/dev/null 2>&1) || rc=$?
    [ "${rc}" -ne 0 ] || { echo "  SELFTEST FAIL: a 9 MiB blob was NOT refused." >&2; ok=0; }
    git -C "${repo}" update-index --force-remove "__selftest_big"

    # negative control -- an ordinary small text file must pass.
    blob=$(printf 'hello world\n' | git -C "${repo}" hash-object -w --stdin)
    git -C "${repo}" update-index --add --cacheinfo "100644,${blob},__selftest_ok"
    rc=0; (cd "${repo}" && "${hook}" >/dev/null 2>&1) || rc=$?
    [ "${rc}" -eq 0 ] || { echo "  SELFTEST FAIL: an ordinary 12-byte file was refused (rc=${rc})." >&2; ok=0; }
    git -C "${repo}" update-index --force-remove "__selftest_ok"

    [ "${ok}" = 1 ]
)

install_into() {                   # install_into <repo> <relative hooks path>
    local repo=$1 rel=$2 cur
    git -C "${repo}" rev-parse --git-dir >/dev/null
    cur=$(git -C "${repo}" config --local --get core.hooksPath || true)
    if [ "${cur}" != "${rel}" ]; then
        git -C "${repo}" config --local core.hooksPath "${rel}"
        echo "  core.hooksPath = ${rel}   (was: ${cur:-unset})"
    else
        echo "  core.hooksPath = ${rel}   (already set)"
    fi
    # Resolve it the way git will, and check the file is actually there and
    # executable: a hooksPath pointing at nothing makes git run NO hook and say
    # nothing at all -- the silent failure this whole file exists to refuse.
    local resolved="${repo}/${rel}/pre-commit"
    [ -x "${resolved}" ] || { echo "  ERROR: ${resolved} is missing or not executable." >&2; return 1; }
    if selftest "${repo}" "${resolved}"; then
        echo "  selftest: 8-byte ELF refused, Java class refused, .pyc refused (file(1) channel live),"
        echo "            9 MiB text refused by the residue layer, ordinary file passed."
    else
        echo "  ERROR: the hook is installed but does not behave. Not trusting it." >&2
        return 1
    fi
}

echo "harness: ${HARNESS}"
install_into "${HARNESS}" "githooks"
echo "code:    ${CODE}"
install_into "${CODE}" "claude/githooks"
echo
echo "Both repos refuse compiled artifacts at ANY size (the criterion), and hand back"
echo "anything >= $(git -C "${HARNESS}" config --int hooks.maxBlobBytes 2>/dev/null || echo 8388608) bytes that no detector can classify (the residue)."
echo "Override for one commit: git commit --no-verify"
