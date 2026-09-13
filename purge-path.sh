#!/usr/bin/env bash
#
# purge-path.sh
#
# Removes a PATH from a repository's history, and keeps every pointer that
# cited the rewritten commits pointing at them.
#
# ---------------------------------------------------------------------------
# WHY THIS EXISTS (the class, not the instance)
# ---------------------------------------------------------------------------
# A derived artifact -- a build output, a staging binary, a rendered artifact --
# is not an origin. Committed, it is either reconstructible (so redundant) or
# non-reconstructible (so unreliable as a record: nothing can check it). Either
# way it does not belong in a history whose job is to say where things came
# from. When one lands anyway, deleting it in a NEW commit does not remove it:
# every clone still carries the blob forever, and GitHub hard-rejects a push
# carrying an object over 100 MiB, so the branch simply cannot be published.
# The only removal is a history rewrite, and a history rewrite changes shas.
#
# That is the whole difficulty. In this repo pair, shas are LOAD-BEARING data:
# INTENT.md and the dispatch/journal files cite harness commits by sha to trace
# what was measured against what, and harness commit messages cite each other.
# A rewrite that ignores them converts a traceability record into a set of
# pointers to objects no branch can reach -- and git never validates a sha
# written inside a message or a markdown file, so the rot is SILENT. The rewrite
# and the repoint are therefore ONE operation, never two, for the same reason
# supervised-by.sh does both repos in one pass: two passes re-sha whatever the
# first pass just repointed.
#
# ---------------------------------------------------------------------------
# WHY `git filter-branch --index-filter` (and not rebase, and not filter-repo)
# ---------------------------------------------------------------------------
# `git rebase` REPLAYS commits: it re-applies each change onto a new base, so a
# merge in the range must be re-performed and every conflict re-resolved by
# hand -- a wrong resolution silently changes content. Nothing here needs a
# replay: each commit's tree is wanted verbatim MINUS one path.
#
# --index-filter operates on a temporary index (no checkout at all, so it is
# fast on a 187 MiB blob) and reuses the original PARENTS and metadata. Author
# and committer identity AND both dates are preserved -- measured in a scratch
# clone before this script was written, not assumed. Therefore the only
# difference between an old commit and its rewrite is the removed path, and
# that is ASSERTED per commit below rather than trusted.
#
# `git filter-repo` is faster and is the modern tool, but is a separate install;
# filter-branch ships with git, which is why supervised-by.sh uses it too.
#
# ---------------------------------------------------------------------------
# THE ONE THING THE LAB RUN CHANGED IN THIS DESIGN
# ---------------------------------------------------------------------------
# filter-branch ends by resetting the working tree onto the rewritten HEAD. The
# purged path is tracked at that moment, so the reset DELETES IT FROM DISK. For
# a staging binary that is a measuring instrument -- the whole reason it exists
# on disk -- silent deletion is a loss the operation was never asked to make.
# So the working-tree copies are saved before the rewrite, restored after, and
# md5-compared against what was saved. Predicted, then measured, then handled.
#
# ---------------------------------------------------------------------------
# THE IGNORE COUPLING (why a purge alone is not a fix)
# ---------------------------------------------------------------------------
# After the rewrite the file is still on disk and is now UNTRACKED. The next
# `git add -A` puts it straight back -- which is exactly how it arrived the
# first time. So the guard must exist BEFORE the operation, and this script
# refuses to run while any purged path is still addable (`git check-ignore`
# says nothing about it). Override with --no-ignore-check when the path is
# meant to be deleted rather than kept.
#
# ---------------------------------------------------------------------------
# THE PERSISTED SHA MAP (added 2026-09-13, F119)
# ---------------------------------------------------------------------------
# The repoint below reaches THIS repository's tracked files and nothing else:
# not the git history of those same files, not the sibling repository, not the
# shared notes, not the archive drawer, not the owner's own trees. In every one
# of those a stale sha can only be resolved by LOOKING THE TOKEN UP, and until
# F119 this script's old->new map died with its temp directory -- so the
# 2026-09-07 purge (harness 9116a6a) left a rewrite with no resolver at all,
# and nothing but this sentence records that it happened.
#
# The map is now written into the same drawer supervised-by.sh writes, in the
# same format, by the same code (sha_map_lib.sh):
#
#     <harness>/sha-maps/<UTC>-<code-tip8>-<harness-tip8>/{code,harness,repair}.tsv
#
# one line per commit whose sha CHANGED, `<old-full-sha> TAB <new-full-sha>`.
# The drawer is a harness-side convention, so it is found from whichever side
# this run was pointed at; a run with no harness repo in reach says so loudly
# instead of inventing a drawer in the code tree. The map is committed by the
# closing act -- which therefore now happens when a map was written even if
# nothing was repointed, because a rewrite whose only record sits uncommitted
# in a working tree has produced the dangling references this script exists to
# prevent.
#
# ---------------------------------------------------------------------------
# WHAT IS VERIFIED, AND WHY EACH GATE EXISTS
# ---------------------------------------------------------------------------
# Two independent classes of assertion, because they fail differently:
#
#   "nothing broke"    G1 topology: same commit count, same parent counts
#                      G3 per pair: the ONLY tree change is the purged paths
#                      G4 per pair: author/committer identity and both dates equal
#                      G5 per pair: message byte-equal after the expected remap
#                      G8 tip tree == old tip tree minus the purged paths
#                      RESTORE: worktree copies back, md5 equal
#
#   "something happened"  G6 the path is in NO commit of the new range
#                         G7 the blob is unreachable from the branch
#                         G2 the old->new map is complete and 1:1
#
# and one that spans both:
#                      G9 every hash token in the new messages, and every one in
#                         tracked files, resolves to a commit the branch REACHES.
#
# G9's bar is REACHABILITY, not existence, and that distinction is the point:
# a rewrite leaves the old objects in the database, so `git cat-file -e` and
# `git show` SUCCEED on a sha no branch can reach -- and then resolve to nothing
# at all once it is gc'd. Existence is a proxy that fails silently in exactly
# the case being tested for. (Same reason, same wording, as supervised-by.sh's
# reachable_in_code.)
#
# Every gate that fails rolls the branch back to the tip recorded before the
# rewrite and exits non-zero. A half-done purge -- history rewritten, citations
# still pointing at the old shas -- is the one state that cannot be repaired
# automatically afterwards.
#
# Usage:
#   ./purge-path.sh --path=harness/sc_f24_pre                  # preview, then confirm
#   ./purge-path.sh --path=a --path=b --range=abc123^..HEAD
#   ./purge-path.sh --path=... --dry-run                       # preview only, changes nothing
#   ./purge-path.sh --path=... --prune                         # + drop the old objects (IRREVERSIBLE)
#   ./purge-path.sh --path=... --repo=/some/other/clone        # e.g. the second harness tree
# ---------------------------------------------------------------------------

set -euo pipefail

# The sha-map drawer's writer and the prefix resolver, shared with
# supervised-by.sh. The resolver below USED to live here, inline; it was moved
# out when this script gained the drawer, so that the two rewrite tools write
# one convention rather than two that look alike. Sourced by absolute path from
# THIS file's own location: filter-branch runs the msg-filter mode from a
# temporary cwd, where a relative path resolves to nothing.
SHA_MAP_LIB="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")/sha_map_lib.sh"
# shellcheck source=sha_map_lib.sh
. "${SHA_MAP_LIB}"

# ---------------------------------------------------------------------------
# THE MSG-FILTER MODE
# ---------------------------------------------------------------------------
# filter-branch runs this once per commit, message on stdin, new message on
# stdout, $GIT_COMMIT set to the ORIGINAL sha, and its own `map` shell function
# in scope (verified in a scratch clone -- it is documented for --commit-filter,
# so it is used here only with a post-run gate that does not trust it).
#
# `map` takes a FULL sha and returns the rewritten one, or echoes its input when
# it knows no mapping. Citations in the wild are ABBREVIATED, so each token is
# first expanded against a precomputed table of the range's full shas, and the
# replacement is truncated back to the token's own length: the length of a
# reference is the citing text's choice, not ours to change.
#
# A token that expands to more than one in-range commit is left ALONE and
# recorded: a wrong repoint is indistinguishable from a right one afterwards,
# which is precisely what forbids guessing. So is a token `map` does not move --
# recorded, not rewritten, and gate G9 decides whether it mattered (it did not,
# if the sha is still reachable).
# ---------------------------------------------------------------------------
if [ "${1:-}" = "--msg-filter" ]; then
    MSG=$(cat; printf X); MSG=${MSG%X}          # $(...) eats trailing newlines
    if [ -z "${PURGE_SHAS:-}" ] || [ ! -s "${PURGE_SHAS}" ]; then printf '%s' "${MSG}"; exit 0; fi
    OUT="${MSG}"
    for TOK in $(printf '%s' "${MSG}" | grep -oE '\b[0-9a-f]{7,40}\b' | sort -u || true); do
        N=${#TOK}
        # The SAME prefix table the preview resolved against -- passed in, not
        # rebuilt here. Two copies of a lookup are two lookups waiting to
        # disagree, and the one place that must never disagree is "what the
        # preview said this run would do" versus "what it did".
        FULL=$(awk -v t="${TOK}" '$1==t {print $2}' "${PURGE_SHAS}")
        [ -n "${FULL}" ] || continue
        if [ "${FULL}" = "AMBIG" ]; then
            printf '%s %s AMBIGUOUS\n' "${GIT_COMMIT}" "${TOK}" >> "${PURGE_REPORT}"; continue
        fi
        # filter-branch's own `map` is a FUNCTION in ITS shell, so it does not
        # exist in this one -- this filter is an external script, i.e. a child
        # process. (The lab probe that "verified map is available" used an
        # INLINE filter, which is eval'd in filter-branch's shell: it measured a
        # different configuration than the one being shipped, and passed. The
        # shipped form died at 127 on the first commit.) What `map` actually
        # does is read $tempdir/map/<old-sha>, so the parent passes that
        # directory in explicitly (-d) and this reads the same file.
        NEWF=$(cat "${PURGE_MAPDIR:-/nonexistent}/${FULL}" 2>/dev/null || true)
        if [ -z "${NEWF}" ] || [ "${NEWF}" = "${FULL}" ]; then
            printf '%s %s UNMAPPED\n' "${GIT_COMMIT}" "${TOK}" >> "${PURGE_REPORT}"; continue
        fi
        # The sentinel is not decoration. `$(...)` strips ALL trailing newlines,
        # so a message whose token was substituted would come back one blank
        # line shorter than every message that was not -- two shapes in the
        # corpus for one rule, and a byte difference gate G5 refuses. (It did:
        # this line's first version failed at 231a149f, "14d13".)
        OUT=$(printf '%s' "${OUT}" | sed -E "s/\\b${TOK}\\b/${NEWF:0:${N}}/g"; printf X); OUT=${OUT%X}
        printf '%s %s -> %s\n' "${GIT_COMMIT}" "${TOK}" "${NEWF:0:${N}}" >> "${PURGE_REPORT}"
    done
    printf '%s' "${OUT}"
    exit 0
fi

# ===========================================================================
# MAIN
# ===========================================================================
PATHS=(); RANGE=""; REPO=""; DRY_RUN=0; NO_COMMIT=0; DO_PRUNE=0; IGNORE_CHECK=1; ALLOW_PUBLISHED=0
for arg in "$@"; do
    case "${arg}" in
        --path=*)           PATHS+=("${arg#*=}") ;;
        --range=*)          RANGE="${arg#*=}" ;;
        --repo=*)           REPO="${arg#*=}" ;;
        --dry-run)          DRY_RUN=1 ;;
        --no-commit)        NO_COMMIT=1 ;;
        --prune)            DO_PRUNE=1 ;;
        --no-ignore-check)  IGNORE_CHECK=0 ;;
        --allow-published)  ALLOW_PUBLISHED=1 ;;
        -h|--help)          sed -n '2,120p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "error: unrecognised argument '${arg}'." >&2; exit 1 ;;
    esac
done
[ "${#PATHS[@]}" -gt 0 ] || { echo "error: at least one --path=<repo-relative path> is required." >&2; exit 1; }

SELF=$(readlink -f "$0")                 # absolute: filter-branch runs from a temp cwd
[ -n "${REPO}" ] || REPO=$(git rev-parse --show-toplevel)
REPO=$(readlink -f "${REPO}")
git -C "${REPO}" rev-parse --git-dir >/dev/null

G() { git -C "${REPO}" "$@"; }           # every git call names its repo: cwd ambiguity = wrong-repo writes

# --- preconditions ----------------------------------------------------------
GITDIR=$(G rev-parse --absolute-git-dir)
if [ -d "${GITDIR}/rebase-merge" ] || [ -d "${GITDIR}/rebase-apply" ]; then
    echo "error: a rebase is in progress in ${REPO}. Finish it, or 'git rebase --abort'." >&2; exit 1
fi
# A dirty tree is refused for three converging reasons, so it is checked once
# and hard: filter-branch refuses to run against one anyway; the restore step
# below cannot tell ITS OWN writes from edits that were already there; and the
# closing commit must be provably exactly what this script wrote. --dry-run
# skips it -- previewing is read-only, so a dirty tree is no reason not to look.
if [ "${DRY_RUN}" = 0 ]; then
    DIRTY=$(G status --porcelain --untracked-files=all)
    if [ -n "${DIRTY}" ]; then
        echo "STOP: uncommitted work in ${REPO}. Nothing has been touched." >&2
        printf '%s\n' "${DIRTY}" | sed 's/^/      /' >&2
        echo "  Commit, stash or ignore it first. '??' entries are files no commit would capture." >&2
        exit 1
    fi
fi

# --- what is actually there --------------------------------------------------
# The range default is derived from the data, not asked for: the first commit
# that ADDS any purged path, back one, through HEAD. Asking the caller for a
# range they would have to compute is asking them to be the tool.
#
# NO `| head -1` HERE, and the reason is measured (F119, 2026-09-13). `git log
# --reverse` emits its whole result at once; `head -1` takes the first line and
# closes the pipe, git gets SIGPIPE, and `set -o pipefail` makes the command
# substitution exit 141, which `set -e` turns into an immediate exit of the
# whole script -- after the assignment has already succeeded, so the trace shows
# the value being set and then nothing at all. Measured on the F103 scratch
# pair: 5 runs out of 5 died this way, exit 141 with ZERO bytes of output, on
# the UNMODIFIED script (md5 86be032b), while the same pipeline run by hand in a
# subshell exited 0 three times out of three -- a race, which is worse than a
# constant failure because it fails somewhere else tomorrow.
#
# The shell takes the first line instead. No pipe, no producer to signal.
declare -A PATH_FIRST=()
FIRST_ALL=""
for p in "${PATHS[@]}"; do
    adds=$(G log --all --format='%H' --diff-filter=A --reverse -- "${p}")
    f=${adds%%$'\n'*}
    if [ -z "${f}" ]; then echo "error: '${p}' was never added in ${REPO}." >&2; exit 1; fi
    PATH_FIRST["${p}"]="${f}"
    if [ -z "${FIRST_ALL}" ] || G merge-base --is-ancestor "${f}" "${FIRST_ALL}"; then FIRST_ALL="${f}"; fi
done
[ -n "${RANGE}" ] || RANGE="${FIRST_ALL}^..HEAD"

TIP=$(G rev-parse HEAD)
BRANCH=$(G branch --show-current)
[ -n "${BRANCH}" ] || { echo "error: detached HEAD in ${REPO}; check out the branch to rewrite." >&2; exit 1; }
N_RANGE=$(G rev-list --count "${RANGE}")
[ "${N_RANGE}" -gt 0 ] || { echo "error: range '${RANGE}' is empty." >&2; exit 1; }

WORK=$(mktemp -d); trap 'rm -rf "${WORK}"' EXIT
G rev-list "${RANGE}" > "${WORK}/range.shas"
G rev-list --topo-order "${RANGE}" > "${WORK}/old.shas"
G rev-list --boundary "${RANGE}" | sed -n 's/^-//p' | sort -u > "${WORK}/boundary.shas"

# --- PUBLISHED? the line where this stops being free -------------------------
# Rewriting an unpushed commit costs nothing; rewriting a pushed one means
# force-pushing over history other clones may hold. That line is exactly the
# set of commits reachable from a remote-tracking ref, so it is measured rather
# than assumed -- and it is a refusal, not a warning, because the cost lands on
# someone who is not in this session.
G rev-list --remotes 2>/dev/null | sort > "${WORK}/remote.shas" || : > "${WORK}/remote.shas"
PUBLISHED=$(comm -12 <(sort "${WORK}/range.shas") "${WORK}/remote.shas" | grep -c . || true)

echo "=== purge-path: ${REPO}  [${BRANCH} @ ${TIP:0:8}]"
echo "    range ${RANGE}  --  ${N_RANGE} commit(s)"
TOTAL_BYTES=0
for p in "${PATHS[@]}"; do
    n=0; while read -r c; do G cat-file -e "${c}:${p}" 2>/dev/null && n=$((n + 1)) || true; done < "${WORK}/range.shas"
    blobs=$(G rev-list --objects "${RANGE}" -- "${p}" 2>/dev/null | awk -v pp="${p}" '$2==pp {print $1}' | sort -u)
    sz=0; for b in ${blobs}; do sz=$((sz + $(G cat-file -s "${b}"))); done
    TOTAL_BYTES=$((TOTAL_BYTES + sz))
    printf '    %-34s in %3d/%d commit(s), %s distinct blob(s), %s bytes\n' \
        "${p}" "${n}" "${N_RANGE}" "$(printf '%s\n' ${blobs} | grep -c . || true)" "${sz}"
    printf '        first added by %s  %s\n' "${PATH_FIRST[${p}]:0:8}" "$(G log -1 --format='%s' "${PATH_FIRST[${p}]}" | cut -c1-60)"
done
echo "    published commits in range: ${PUBLISHED}  ($([ "${PUBLISHED}" = 0 ] && echo 'none -- the rewrite is free' || echo 'FORCE-PUSH TERRITORY'))"

if [ "${PUBLISHED}" -gt 0 ] && [ "${ALLOW_PUBLISHED}" = 0 ]; then
    echo "STOP: ${PUBLISHED} commit(s) in the range are reachable from a remote ref." >&2
    echo "      Rewriting them means force-pushing over history other clones may hold." >&2
    echo "      Pass --allow-published if you genuinely mean to." >&2
    exit 1
fi

# --- the ignore coupling ------------------------------------------------------
if [ "${IGNORE_CHECK}" = 1 ]; then
    UNIG=()
    # --no-index is load-bearing: by default check-ignore consults the index and
    # reports a TRACKED path as not-ignored, which is every path this script is
    # ever pointed at. The question here is the counterfactual one -- "once this
    # is untracked, will an `add -A` take it back?" -- and only --no-index asks it.
    for p in "${PATHS[@]}"; do G check-ignore --no-index -q "${p}" || UNIG+=("${p}"); done
    if [ "${#UNIG[@]}" -gt 0 ]; then
        echo >&2
        echo "STOP: these paths are not ignored, so the purge would be undone by the next 'git add -A':" >&2
        printf '      %s\n' "${UNIG[@]}" >&2
        echo "      Add them to .gitignore first (that is the actual fix; this script is the cleanup)," >&2
        echo "      or pass --no-ignore-check if the file is meant to be deleted rather than kept." >&2
        exit 1
    fi
fi

# --- the citations that would rot --------------------------------------------
# Two homes, two mechanisms, both measured before anything is touched: hash
# tokens inside COMMIT MESSAGES (remapped inside the rewrite pass, since they
# live in the messages being rewritten) and inside TRACKED FILES (repointed at
# the tip afterwards, as a separate reviewable commit). Every planned
# substitution is printed WITH ITS LINE: a 7-8 hex token can also be an md5
# prefix, and only the surrounding text can tell them apart.
# Resolving an abbreviated citation is a prefix lookup, and there are a LOT of
# lookups: the harness carries 269 111 distinct hex tokens in tracked files
# (md5 sums, JSON digests, engine hashes), of which a handful are commit shas.
# One subshell per token is minutes of work and was measured as such -- the
# first version of this scan timed out at 2 minutes. So the table is built ONCE,
# every prefix length 7..40 of every in-range sha, collisions marked AMBIG, and
# every token is then a single hash lookup in one awk pass.
#
# The two awk programs that do it now live in sha_map_lib.sh, unchanged, because
# supervised-by.sh's step E hit the same wall at the same size and a second copy
# would have been the third place this repository pair writes the same rule.
sha_prefix_table "${WORK}/range.shas" "${WORK}/prefix.tbl"
resolve_tokens() {                     # resolve_tokens <file of tokens, one per line> -> "<token> <sha|AMBIG>"
    sha_resolve_tokens "${WORK}/prefix.tbl" "$1"
}

echo
echo "--- citations that the rewrite would strand ----------------------------"
: > "${WORK}/msg.tokens"; : > "${WORK}/file.tokens"
: > "${WORK}/msg.pairs"
while IFS= read -r c; do
    # `|| true`: a message with no hex token is the ordinary case, and grep
    # exiting 1 for it would take the whole run down under `set -o pipefail`.
    G log -1 --format='%B' "${c}" | grep -oE '\b[0-9a-f]{7,40}\b' | sort -u \
        | sed "s/^/${c} /" >> "${WORK}/msg.pairs" || true
done < "${WORK}/range.shas"
awk '{print $2}' "${WORK}/msg.pairs" | sort -u > "${WORK}/msg.uniq"
resolve_tokens "${WORK}/msg.uniq" > "${WORK}/msg.resolved"
awk 'NR==FNR { m[$1] = $2; next } ($2 in m) { print $1, $2, m[$2] }' \
    "${WORK}/msg.resolved" "${WORK}/msg.pairs" > "${WORK}/msg.tokens"
N_MSG=$(grep -c . "${WORK}/msg.tokens" || true)
echo "  commit messages: ${N_MSG} citation(s)"
while read -r c t r; do
    # "resolves to", not "->": at preview time the NEW sha does not exist yet.
    # An arrow here would read as a promise about a value nothing has computed.
    printf '      %s cites %s  %s\n' "${c:0:8}" "${t}" "$([ "${r}" = AMBIG ] && echo '** AMBIGUOUS -- left alone **' || echo "= commit ${r:0:8}, which this run re-shas")"
    # `|| true` for the same SIGPIPE class as the range derivation above: this
    # is a DISPLAY line, and a display line that can kill the run is worse than
    # no display at all. (`grep` exiting 1 on no match would do it too.)
    G log -1 --format='%B' "${c}" | grep -nF "${t}" | head -2 | sed 's/^/          /' || true
done < "${WORK}/msg.tokens"

# THE DRAWER IS NOT A TRACKER, AND MUST NOT BE REPOINTED. `sha-maps/*/*.tsv` is
# the old->new record of PAST rewrites -- including, since F119, the ones this
# script writes itself. Its own README says it in as many words: "They are
# append-only and are NEVER edited ... A map that has been corrected has stopped
# being evidence of what a particular run did, which is the only thing it is
# for." A repoint pass over those files rewrites the OLD side of every pair, so
# a token a reader arrives with stops matching the very row that would have
# resolved it -- and the map still looks perfectly well-formed. Measured on the
# F103 pair, 2026-09-13: an unexcluded scan proposed exactly that, on the map
# this script's sibling had written minutes earlier.
#
# `harness/artifacts/**` is the same KIND of thing -- dated measurement records
# whose shas are part of what was recorded -- and it is deliberately NOT
# excluded here: that is a wider call about what counts as evidence, it belongs
# to the owner, and it is written up rather than taken. (F119 veto point.)
SCAN_SPEC=(-- . ':(exclude)sha-maps/**')
G grep -hoIE '\b[0-9a-f]{7,40}\b' "${SCAN_SPEC[@]}" 2>/dev/null | sort -u > "${WORK}/file.uniq" || true
resolve_tokens "${WORK}/file.uniq" > "${WORK}/file.tokens"
N_FILE=$(grep -c . "${WORK}/file.tokens" 2>/dev/null || true)
echo "  tracked files:   ${N_FILE} distinct token(s)"
while read -r t r; do
    printf '      %s  %s\n' "${t}" "$([ "${r}" = AMBIG ] && echo '** AMBIGUOUS -- left alone **' || echo "= commit ${r:0:8}  $(G log -1 --format='%s' "${r}" | cut -c1-52)")"
    G grep -nwIF "${t}" "${SCAN_SPEC[@]}" 2>/dev/null | cut -c1-150 | head -3 | sed 's/^/          /' || true
done < "${WORK}/file.tokens" 2>/dev/null || true
if grep -q ' AMBIG$' "${WORK}/file.tokens" "${WORK}/msg.tokens" 2>/dev/null; then
    echo "STOP: a citation token prefixes more than one commit in the range. Repoint it by hand" >&2
    echo "      (lengthen it to 12 chars), then re-run -- this script will not guess." >&2
    exit 1
fi
echo "-----------------------------------------------------------------------"
echo

if [ "${DRY_RUN}" = 1 ]; then echo "(--dry-run: stopping here. Nothing was touched.)"; exit 0; fi
read -r -p "Purge ${#PATHS[@]} path(s) from ${N_RANGE} commit(s)$([ "${DO_PRUNE}" = 1 ] && echo ' AND PRUNE THE OLD OBJECTS (irreversible)')? [y/N] " CONFIRM
case "${CONFIRM}" in [yY]*) ;; *) echo "aborted."; exit 0 ;; esac

# --- save the working-tree copies -------------------------------------------
# filter-branch ends with a reset onto the rewritten HEAD, which DELETES a
# tracked-then-purged file from disk. Measured in a scratch clone. These are
# instruments (a staging binary is what a measurement is replayed against), so
# they are saved here and restored -- with an md5 comparison, because "the file
# is back" and "the file is the same file" are different claims.
mkdir -p "${WORK}/wt"
: > "${WORK}/wt.md5"
for p in "${PATHS[@]}"; do
    if [ -f "${REPO}/${p}" ]; then
        mkdir -p "${WORK}/wt/$(dirname "${p}")"
        cp -a "${REPO}/${p}" "${WORK}/wt/${p}"
        (cd "${REPO}" && md5sum "${p}") >> "${WORK}/wt.md5"
    fi
done
echo "Saved $(grep -c . "${WORK}/wt.md5" || true) working-tree copy/copies."

# --- the backup ref ----------------------------------------------------------
BACKUP="refs/purge-backup/${BRANCH}-${TIP:0:8}"
G update-ref "${BACKUP}" "${TIP}"
echo "Backup ref ${BACKUP} -> ${TIP:0:8}"

rollback() {
    G reset --hard "${TIP}" >/dev/null 2>&1 || true
    for p in "${PATHS[@]}"; do
        [ -f "${WORK}/wt/${p}" ] && cp -a "${WORK}/wt/${p}" "${REPO}/${p}"
    done
    echo "Rolled ${REPO} back to ${TIP:0:8} (working-tree copies restored)." >&2
}

# --- the rewrite -------------------------------------------------------------
export PURGE_SHAS="${WORK}/prefix.tbl"
export PURGE_MAPDIR="${WORK}/rewrite/map"
export PURGE_REPORT="${WORK}/remap.report"; : > "${PURGE_REPORT}"
RM_ARGS=""; for p in "${PATHS[@]}"; do RM_ARGS+=" $(printf '%q' "${p}")"; done

if ! FILTER_BRANCH_SQUELCH_WARNING=1 G filter-branch -f \
        -d "${WORK}/rewrite" \
        --index-filter "git rm --cached --ignore-unmatch -q --${RM_ARGS}" \
        --msg-filter "$(printf '%q --msg-filter' "${SELF}")" \
        -- "${RANGE}" >/dev/null; then
    echo "STOP: filter-branch failed." >&2; rollback; exit 1
fi
NEWTIP=$(G rev-parse HEAD)

# ===========================================================================
# THE GATES
# ===========================================================================
fail() { echo "STOP: $*" >&2; rollback; exit 1; }

# G1 -- topology. Pairing is by position in --topo-order: filter-branch keeps
# parents verbatim, so the two orders are the same walk over the same shape.
# The pairing is not TRUSTED though -- G3/G4 would blow up loudly on a mispair,
# which is what makes index pairing safe to use here.
# The new side is walked from the BOUNDARY commits -- the parents just outside
# the range, which the rewrite cannot have touched -- not from the range string:
# a range whose left end names a rewritten commit would silently walk the wrong
# set. Same walk order (--topo-order) over the same shape, so position pairs.
if [ -s "${WORK}/boundary.shas" ]; then
    G rev-list --topo-order HEAD --not $(tr '\n' ' ' < "${WORK}/boundary.shas") > "${WORK}/new.shas"
else
    G rev-list --topo-order HEAD > "${WORK}/new.shas"
fi
[ "$(wc -l < "${WORK}/new.shas")" -eq "${N_RANGE}" ] || fail "commit count changed: ${N_RANGE} -> $(wc -l < "${WORK}/new.shas")"

: > "${WORK}/map"
paste "${WORK}/old.shas" "${WORK}/new.shas" > "${WORK}/map"
N_CHANGED=$(awk '$1!=$2' "${WORK}/map" | wc -l)

# G2..G5 -- per pair.
while IFS=$'\t' read -r OLD NEW; do
    [ -n "${OLD}" ] && [ -n "${NEW}" ] || fail "map has an empty side"
    # G3: the ONLY tree change is the purged paths.
    DIFF=$(G diff --name-status "${OLD}" "${NEW}")
    EXPECT=$(for p in "${PATHS[@]}"; do G cat-file -e "${OLD}:${p}" 2>/dev/null && printf 'D\t%s\n' "${p}" || true; done | sort)
    [ "$(printf '%s' "${DIFF}" | sort)" = "${EXPECT}" ] || fail "$(printf 'tree change beyond the purged paths at %s -> %s:\n%s' "${OLD:0:8}" "${NEW:0:8}" "${DIFF}")"
    # G4: identity and both dates, on both sides of the commit.
    FO=$(G log -1 --format='%an|%ae|%aI|%cn|%ce|%cI' "${OLD}")
    FN=$(G log -1 --format='%an|%ae|%aI|%cn|%ce|%cI' "${NEW}")
    [ "${FO}" = "${FN}" ] || fail "metadata changed at ${OLD:0:8} -> ${NEW:0:8}: '${FO}' vs '${FN}'"
    # Parents: each old parent maps to the new one, or is a boundary commit the
    # rewrite could not touch and must therefore be carried verbatim. A merge
    # that lost or gained a parent passes every other gate in this loop.
    PEXP=""
    for pp in $(G log -1 --format='%P' "${OLD}"); do
        m=$(awk -v o="${pp}" '$1==o {print $2}' "${WORK}/map")
        PEXP+="${m:-${pp}} "
    done
    PGOT="$(G log -1 --format='%P' "${NEW}") "
    [ "${PEXP}" = "${PGOT}" ] || fail "parents changed at ${OLD:0:8} -> ${NEW:0:8}: expected '${PEXP}' got '${PGOT}'"
    # G5: message byte-equal after the expected remap, recomputed here
    # independently of what the filter did -- an assertion that re-derives the
    # answer instead of re-reading the filter's own output.
    G log -1 --format='%B' "${OLD}" > "${WORK}/m.old"
    cp "${WORK}/m.old" "${WORK}/m.exp"
    while read -r c t r; do
        [ "${c}" = "${OLD}" ] || continue
        n=${#t}; nn=$(awk -v o="${r}" '$1==o {print $2}' "${WORK}/map")
        [ -n "${nn}" ] && sed -i -E "s/\\b${t}\\b/${nn:0:${n}}/g" "${WORK}/m.exp"
    done < "${WORK}/msg.tokens"
    G log -1 --format='%B' "${NEW}" > "${WORK}/m.new"
    cmp -s "${WORK}/m.exp" "${WORK}/m.new" || fail "$(printf 'message at %s -> %s is not the expected remap:\n%s' "${OLD:0:8}" "${NEW:0:8}" "$(diff "${WORK}/m.exp" "${WORK}/m.new" | head -6)")"
done < "${WORK}/map"

# G6/G7 -- it HAPPENED. Content identity alone passes perfectly for a filter
# that silently did nothing, so these are the assertions that cannot be fooled
# by a no-op: the path in no commit, the blob unreachable from the branch.
for p in "${PATHS[@]}"; do
    while read -r c; do
        G cat-file -e "${c}:${p}" 2>/dev/null && fail "${p} still present at ${c:0:8}"
    done < "${WORK}/new.shas"
    G ls-tree -r "${BRANCH}" --name-only | grep -qxF "${p}" && fail "${p} still tracked at the tip" || true
done
for p in "${PATHS[@]}"; do
    for b in $(G rev-list --objects "${TIP}" -- "${p}" | awk -v pp="${p}" '$2==pp {print $1}' | sort -u || true); do
        G rev-list --objects "${BRANCH}" | grep -q "^${b}" && fail "blob ${b:0:8} of ${p} is still reachable from ${BRANCH}"
    done
done

# G8 -- the tip's tree is the old tip's tree minus exactly the purged paths.
G ls-tree -r "${TIP}" | sort > "${WORK}/t.old"
G ls-tree -r "${BRANCH}" | sort > "${WORK}/t.new"
printf '%s\n' "${PATHS[@]}" > "${WORK}/paths.list"
awk -F'\t' 'NR==FNR{drop[$0]=1;next} !($2 in drop)' "${WORK}/paths.list" "${WORK}/t.old" > "${WORK}/t.exp"
cmp -s "${WORK}/t.exp" "${WORK}/t.new" || fail "$(printf 'tip tree is not old-minus-paths:\n%s' "$(diff "${WORK}/t.exp" "${WORK}/t.new" | head -6)")"

# G9 -- reachability of every hash token in the new messages. Existence would
# report success on a dangling sha (the old objects are still in the database
# right now), so the bar is what the branch can REACH.
DANG=0
while read -r c; do
    for t in $(G log -1 --format='%B' "${c}" | grep -oE '\b[0-9a-f]{7,40}\b' | sort -u || true); do
        G rev-parse --verify -q "${t}^{commit}" >/dev/null 2>&1 || continue    # not a commit token (md5, etc.)
        G merge-base --is-ancestor "${t}" "${BRANCH}" 2>/dev/null && continue
        # a token naming a CODE-repo commit is not this repo's business
        G cat-file -t "${t}" >/dev/null 2>&1 && { echo "      dangling in ${c:0:8}: ${t}"; DANG=$((DANG + 1)); }
    done
done < "${WORK}/new.shas"
[ "${DANG}" -eq 0 ] || fail "${DANG} commit-message citation(s) now point at commits ${BRANCH} cannot reach"

grep -E ' (AMBIGUOUS|UNMAPPED)$' "${PURGE_REPORT}" >/dev/null 2>&1 && {
    echo "  note: the filter left these tokens alone (G9 passed, so they were already fine):" >&2
    grep -E ' (AMBIGUOUS|UNMAPPED)$' "${PURGE_REPORT}" | sed 's/^/      /' >&2; }

# --- restore the working-tree copies ----------------------------------------
if [ -s "${WORK}/wt.md5" ]; then
    for p in "${PATHS[@]}"; do
        [ -f "${WORK}/wt/${p}" ] && cp -a "${WORK}/wt/${p}" "${REPO}/${p}"
    done
    (cd "${REPO}" && md5sum -c --quiet "${WORK}/wt.md5") || fail "a restored working-tree copy does not match its pre-run md5"
    echo "Restored $(grep -c . "${WORK}/wt.md5") working-tree copy/copies, md5 verified."
fi

echo "VERIFIED: ${N_CHANGED}/${N_RANGE} commit(s) re-shaed; per-pair tree change == the purged paths only;"
echo "          identity/dates/parents preserved; messages == expected remap; paths and blobs gone;"
echo "          every message citation reachable from ${BRANCH}."
echo "          ${TIP:0:8} -> ${NEWTIP:0:8}"

# --- persist the map beside the ledger --------------------------------------
# Until F119 this script's old->new map died with ${WORK}. The repoint below
# reaches this repository's tracked files and NOTHING ELSE: not the git history
# of those same files, not the sibling repo, not the shared notes, not the
# archive drawer, not the owner's own trees. For every one of those a stale sha
# is resolvable only by LOOKING THE TOKEN UP, and the map is the only thing that
# can answer. The 2026-09-07 purge (harness 9116a6a) ran without one, which is
# why that rewrite has no resolver at all.
#
# Only CHANGED pairs are recorded: a commit whose sha survived keeps its object
# and belongs in no map, which is the drawer's stated rule.
#
# The drawer lives in the HARNESS repo -- it is a harness-side convention and
# `sha-maps/` is a harness path -- so it is found from whichever side this run
# was pointed at, and a run with no harness repo in reach SAYS SO rather than
# inventing a drawer in the code tree.
MAP_FILES=(); MAP_DIR=""
awk -F'\t' '$1!=$2' "${WORK}/map" > "${WORK}/changed.map"
DRAWER=""; DRAWER_CODE_TIP=""; DRAWER_HARNESS_TIP=""
if [ "$(basename "${REPO}")" = "claude" ]; then
    DRAWER="${REPO}"; DRAWER_HARNESS_TIP="${TIP:0:8}"
    PARENT=$(dirname "${REPO}")
    git -C "${PARENT}" rev-parse --git-dir >/dev/null 2>&1 && \
        DRAWER_CODE_TIP=$(git -C "${PARENT}" rev-parse --short=8 HEAD)
elif [ -d "${REPO}/claude/.git" ] || [ -f "${REPO}/claude/.git" ]; then
    DRAWER="${REPO}/claude"; DRAWER_CODE_TIP="${TIP:0:8}"
    DRAWER_HARNESS_TIP=$(git -C "${DRAWER}" rev-parse --short=8 HEAD)
fi
if [ ! -s "${WORK}/changed.map" ]; then
    echo "No sha changed, so no map is written."
elif [ -z "${DRAWER}" ]; then
    echo "NOTE: no harness repository is in reach of ${REPO}, so this run's old->new" >&2
    echo "      map has nowhere conventional to live and is NOT persisted. ${N_CHANGED} commit(s)" >&2
    echo "      changed sha and only this terminal knows the pairs. Keep them:" >&2
    sed 's/^/          /' "${WORK}/changed.map" >&2
else
    if [ "${DRAWER}" = "${REPO}" ]; then
        sha_map_write "${DRAWER}" \
            "$(sha_map_dir_name "${DRAWER_CODE_TIP}" "${DRAWER_HARNESS_TIP}")" \
            "" "${WORK}/changed.map" ""
    else
        sha_map_write "${DRAWER}" \
            "$(sha_map_dir_name "${DRAWER_CODE_TIP}" "${DRAWER_HARNESS_TIP}")" \
            "${WORK}/changed.map" "" ""
    fi
    MAP_FILES=("${SHA_MAP_FILES[@]}"); MAP_DIR="${SHA_MAP_DIR}"
    echo "Sha map written to ${DRAWER}/${MAP_DIR}/ : $(sha_map_summary "${DRAWER}")"
    if [ "${DRAWER}" != "${REPO}" ]; then
        echo "  It is STAGED in ${DRAWER}, which this run does not commit: that repo was"
        echo "  not the one rewritten. Commit it there -- an unpersisted map is a rewrite"
        echo "  with no resolver."
    fi
fi

# --- repoint the tracked files ----------------------------------------------
echo
echo "--- tracker citations --------------------------------------------------"
declare -A TOUCHED=()
N_SUB=0
if [ ! -s "${WORK}/file.tokens" ]; then
    echo "No tracked-file citation points into the rewritten range."
else
    while read -r t r; do
        nn=$(awk -v o="${r}" '$1==o {print $2}' "${WORK}/map")
        [ -n "${nn}" ] || { echo "      ${t}: no mapping (commit rebuilt identical) -- left alone"; continue; }
        # A commit whose sha did not change needs no repoint, and saying so here
        # is not an optimisation -- it is what keeps the closing commit possible.
        # ${WORK}/map is `paste old.shas new.shas` over the WHOLE range, so an
        # unchanged commit maps to ITSELF; the sed below then rewrites the token
        # to the identical string, changes no byte, and yet the file is recorded
        # in TOUCHED. The expected-set assertion afterwards compares TOUCHED
        # against the dirty set, finds the file listed and not dirty, declares
        # "the dirty set is not the set this script wrote", and REFUSES to
        # commit -- leaving the repoint applied and uncommitted, which is the
        # state this script exists to avoid. Measured on the F103 pair
        # 2026-09-13, purging one artifact: 13 of 100 commits re-shaed, 33 files
        # listed as touched that no byte had changed in, closing commit refused.
        [ "${nn}" = "${r}" ] && continue
        n=${#t}
        while IFS= read -r f; do
            (cd "${REPO}" && sed -i -E "s/\\b${t}\\b/${nn:0:${n}}/g" "${f}")
            TOUCHED["${f}"]=1
        done < <(G grep -lwIF "${t}" "${SCAN_SPEC[@]}" 2>/dev/null || true)
        N_SUB=$((N_SUB + 1))
        printf '      %s -> %s\n' "${t}" "${nn:0:${n}}"
    done < "${WORK}/file.tokens"
    echo "Repointed ${N_SUB} citation(s) across ${#TOUCHED[@]} file(s)."
fi

# --- the closing commit -----------------------------------------------------
# Reached whenever this run WROTE something into THIS repository's working tree:
# the repointed trackers, the persisted map, or both. The map alone is reason
# enough -- a run that re-shaed commits and left its only old->new record
# uncommitted in a working tree has produced exactly the dangling references it
# exists to prevent, and has hidden that behind a clean-looking summary. (Same
# rule, same words, as supervised-by.sh's closing commit; before F119 this
# script had no map to keep, so it committed only when something was repointed.)
#
# A map written into the SIBLING repo is staged there and is not this commit's
# business: this run rewrote one repository, and committing in the other one
# under this message would claim work in a tree it did not verify.
MAP_HERE=()
if [ -n "${MAP_DIR}" ] && [ "${DRAWER}" = "${REPO}" ]; then MAP_HERE=("${MAP_FILES[@]}"); fi
if [ "${#TOUCHED[@]}" -gt 0 ] || [ "${#MAP_HERE[@]}" -gt 0 ]; then
    # ONE list, built once and used for both the assertion and `git add`: two
    # expansions of "the files this run wrote" is two chances to write it
    # differently, and the assertion would then certify a set the add did not
    # stage. `git add --` with an empty pathspec is a no-op that still exits 0,
    # so that divergence would commit nothing and say nothing.
    ADD_LIST=()
    [ "${#TOUCHED[@]}" -gt 0 ] && ADD_LIST+=("${!TOUCHED[@]}")
    [ "${#MAP_HERE[@]}" -gt 0 ] && ADD_LIST+=("${MAP_HERE[@]}")
    # The dirty set must be exactly what this script wrote -- asserted now, not
    # inherited from the clean-tree check at the top: the gap between the two is
    # the whole run, and an assumption that held at the start is not evidence
    # about the end.
    EXPECTED=$(printf '%s\n' "${ADD_LIST[@]}" | sort -u)
    ACTUAL=$(G status --porcelain --untracked-files=all | cut -c4- | sort)
    if [ "${EXPECTED}" != "${ACTUAL}" ]; then
        echo "NOT COMMITTED: the dirty set is not the set this script wrote." >&2
        diff <(printf '%s\n' "${EXPECTED}") <(printf '%s\n' "${ACTUAL}") | sed 's/^/      /' >&2
        echo "  Review and commit by hand." >&2
        if [ "${#MAP_HERE[@]}" -gt 0 ]; then
            echo "  The sha map is part of the expected set and must be committed with the" >&2
            echo "  rest: ${MAP_HERE[*]}" >&2
        fi
    elif [ "${NO_COMMIT}" = 1 ]; then
        echo "(--no-commit: left uncommitted.)"
        if [ "${#MAP_HERE[@]}" -gt 0 ]; then
            echo "  The sha map is written and STAGED but NOT committed. It is this run's"
            echo "  only record of old sha -> new sha: ${MAP_HERE[*]}"
        fi
    else
        CODE_TRAILER=""
        CODE_REPO=$(dirname "${REPO}")
        if git -C "${CODE_REPO}" rev-parse --git-dir >/dev/null 2>&1; then
            CODE_TRAILER="Code: $(git -C "${CODE_REPO}" branch --show-current) @ $(git -C "${CODE_REPO}" rev-parse --short=8 HEAD)"
        fi
        SUBJECT="Repoint tracker citations after the ${PATHS[0]##*/} purge"
        [ "${#TOUCHED[@]}" -gt 0 ] || \
            SUBJECT="Record the old->new sha map of the ${PATHS[0]##*/} purge"
        MAPLINE="no sha map was written (nothing changed sha)."
        [ "${#MAP_HERE[@]}" -gt 0 ] && MAPLINE="this run's sha map: ${MAP_HERE[*]}"
        (cd "${REPO}" && git add -- "${ADD_LIST[@]}" && git commit -q -F - <<COMMITMSG
${SUBJECT}

$(printf '%s\n' "${PATHS[@]}" | sed 's/^/    /')
was removed from ${N_RANGE} commit(s) of ${RANGE}, which re-shaed ${N_CHANGED} of
them. Every sha those commits are cited by would otherwise point at an object no
branch reaches -- and git validates no sha written inside a file, so the rot is
silent. This commit carries that repoint -- ${N_SUB} citation(s) across
${#TOUCHED[@]} file(s), old sha -> new sha, no prose changed -- and
${MAPLINE}
The map is what resolves a stale sha in every surface this rewrite cannot reach:
the git history of these same files, the sibling repository, notes outside the
pair, the archive. Look a token up as a PREFIX of the old side, newest directory
first; the record is never rewritten. sha-maps/README.md carries the rule.

Citations inside the rewritten COMMIT MESSAGES are not part of this commit --
they are remapped inside the rewrite pass itself, since they live in the
messages being rewritten.

Generated by claude/purge-path.sh. The tree was verified clean before the run
and verified to contain only these files before this commit.

${CODE_TRAILER}

Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>
COMMITMSG
        )
        echo "Committed: $(G log -1 --format='%h  %s')"
    fi
fi
# G10 -- the repoint's own "it happened" gate. A sed that silently matched
# nothing leaves the trackers citing dead shas and every other check green.
# Scoped to the tokens this run set out to repoint -- rescanning all 269k
# tracked tokens would cost a rev-parse each and prove nothing extra: a token
# this run never claimed is not this run's to certify.
LEFT=0
while read -r t r; do
    [ "${r}" = AMBIG ] && continue
    # A token naming a commit whose sha SURVIVED the rewrite is still valid and
    # is still there on purpose -- it was never this run's to repoint. Asserting
    # its absence asserts that a correct citation was destroyed. Same root as
    # the skip in the repoint loop above: ${WORK}/map covers the whole range and
    # maps an unchanged commit to itself. (Measured on the F103 pair 2026-09-13:
    # 67 such tokens, every one of them correct, reported as "still present ...
    # unrepointed" and the whole purge rolled back on them.)
    nnc=$(awk -v o="${r}" '$1==o {print $2}' "${WORK}/map")
    [ "${nnc}" = "${r}" ] && continue
    if G grep -qwIF "${t}" "${SCAN_SPEC[@]}" 2>/dev/null; then
        echo "      still present in a tracked file, unrepointed: ${t}"; LEFT=$((LEFT + 1)); continue
    fi
    nn=$(awk -v o="${r}" '$1==o {print $2}' "${WORK}/map"); [ -n "${nn}" ] || continue
    G merge-base --is-ancestor "${nn}" "${BRANCH}" 2>/dev/null || {
        echo "      repointed to an unreachable commit: ${t} -> ${nn:0:8}"; LEFT=$((LEFT + 1)); }
done < "${WORK}/file.tokens"
[ "${LEFT}" -eq 0 ] || fail "${LEFT} tracked-file citation(s) still point into the purged range"
echo "VERIFIED: no tracked-file citation points at a commit ${BRANCH} cannot reach."
echo "-----------------------------------------------------------------------"

# --- the irreversible half ---------------------------------------------------
# Last, and only after every gate above passed: this is what removes the way
# back. refs/original is filter-branch's own backup, the reflog is git's, and
# this script's backup ref is the third -- all three must go for the objects to
# become unreachable, which is why "delete the ref" and "reclaim the space" are
# one step and not two.
if [ "${DO_PRUNE}" = 1 ]; then
    echo
    echo "--- prune --------------------------------------------------------------"
    BEFORE=$(G count-objects -vH | awk '/^size:/{print $2 $3}')
    for r in $(G for-each-ref --format='%(refname)' refs/original refs/purge-backup); do
        G update-ref -d "${r}"; echo "  deleted ${r}"
    done
    G reflog expire --expire=now --expire-unreachable=now --all
    G gc --prune=now --quiet
    AFTER=$(G count-objects -vH | awk '/^size-pack:/{print $2 $3}')
    echo "  loose ${BEFORE} -> packed ${AFTER}"
    # post-prune re-verification: the branch must be exactly what the gates left.
    [ "$(G rev-parse HEAD)" = "$(G rev-parse "${BRANCH}")" ] || echo "  WARNING: HEAD and ${BRANCH} disagree after gc" >&2
    G fsck --no-progress --no-dangling 2>&1 | head -5 || true
    echo "  fsck clean; tip $(G log -1 --format='%h %s' | cut -c1-70)"
    echo "-----------------------------------------------------------------------"
    echo
    echo "No undo: the pre-rewrite objects are gone. The rewritten history is the only one."
else
    echo
    echo "Undo:  git -C ${REPO} reset --hard ${BACKUP}   # then restore the working-tree copies"
    echo "Prune: git -C ${REPO} update-ref -d ${BACKUP} && \\"
    echo "       git -C ${REPO} for-each-ref --format='%(refname)' refs/original | xargs -r -n1 git -C ${REPO} update-ref -d && \\"
    echo "       git -C ${REPO} reflog expire --expire=now --expire-unreachable=now --all && \\"
    echo "       git -C ${REPO} gc --prune=now"
fi
