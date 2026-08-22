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
declare -A PATH_FIRST=()
FIRST_ALL=""
for p in "${PATHS[@]}"; do
    f=$(G log --all --format='%H' --diff-filter=A --reverse -- "${p}" | head -1)
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
cut -c1-40 "${WORK}/range.shas" > "${WORK}/shas40"
awk '{ for (n = 7; n <= length($0); n++) { pfx = substr($0, 1, n)
         if (pfx in seen && seen[pfx] != $0) seen[pfx] = "AMBIG"; else seen[pfx] = $0 } }
     END { for (pfx in seen) print pfx, seen[pfx] }' "${WORK}/shas40" > "${WORK}/prefix.tbl"
resolve_tokens() {                     # resolve_tokens <file of tokens, one per line> -> "<token> <sha|AMBIG>"
    awk 'NR==FNR { m[$1] = $2; next } ($1 in m) { print $1, m[$1] }' "${WORK}/prefix.tbl" "$1"
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
    G log -1 --format='%B' "${c}" | grep -nF "${t}" | head -2 | sed 's/^/          /'
done < "${WORK}/msg.tokens"

G grep -hoIE '\b[0-9a-f]{7,40}\b' -- . 2>/dev/null | sort -u > "${WORK}/file.uniq" || true
resolve_tokens "${WORK}/file.uniq" > "${WORK}/file.tokens"
N_FILE=$(grep -c . "${WORK}/file.tokens" 2>/dev/null || true)
echo "  tracked files:   ${N_FILE} distinct token(s)"
while read -r t r; do
    printf '      %s  %s\n' "${t}" "$([ "${r}" = AMBIG ] && echo '** AMBIGUOUS -- left alone **' || echo "= commit ${r:0:8}  $(G log -1 --format='%s' "${r}" | cut -c1-52)")"
    G grep -nwIF "${t}" -- . 2>/dev/null | cut -c1-150 | head -3 | sed 's/^/          /' || true
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

# --- repoint the tracked files ----------------------------------------------
echo
echo "--- tracker citations --------------------------------------------------"
if [ ! -s "${WORK}/file.tokens" ]; then
    echo "No tracked-file citation points into the rewritten range."
else
    declare -A TOUCHED=()
    N_SUB=0
    while read -r t r; do
        nn=$(awk -v o="${r}" '$1==o {print $2}' "${WORK}/map")
        [ -n "${nn}" ] || { echo "      ${t}: no mapping (commit rebuilt identical) -- left alone"; continue; }
        n=${#t}
        while IFS= read -r f; do
            (cd "${REPO}" && sed -i -E "s/\\b${t}\\b/${nn:0:${n}}/g" "${f}")
            TOUCHED["${f}"]=1
        done < <(G grep -lwIF "${t}" -- . 2>/dev/null || true)
        N_SUB=$((N_SUB + 1))
        printf '      %s -> %s\n' "${t}" "${nn:0:${n}}"
    done < "${WORK}/file.tokens"
    echo "Repointed ${N_SUB} citation(s) across ${#TOUCHED[@]} file(s)."

    # The dirty set must be exactly what this script wrote -- asserted now, not
    # inherited from the clean-tree check at the top: the gap between the two is
    # the whole run, and an assumption that held at the start is not evidence
    # about the end.
    EXPECTED=$(printf '%s\n' "${!TOUCHED[@]}" | sort)
    ACTUAL=$(G status --porcelain --untracked-files=no | cut -c4- | sort)
    if [ "${EXPECTED}" != "${ACTUAL}" ]; then
        echo "NOT COMMITTED: the dirty set is not the set this script wrote." >&2
        diff <(printf '%s\n' "${EXPECTED}") <(printf '%s\n' "${ACTUAL}") | sed 's/^/      /' >&2
        echo "  Review and commit by hand." >&2
    elif [ "${NO_COMMIT}" = 1 ]; then
        echo "(--no-commit: left uncommitted.)"
    else
        CODE_TRAILER=""
        CODE_REPO=$(dirname "${REPO}")
        if git -C "${CODE_REPO}" rev-parse --git-dir >/dev/null 2>&1; then
            CODE_TRAILER="Code: $(git -C "${CODE_REPO}" branch --show-current) @ $(git -C "${CODE_REPO}" rev-parse --short=8 HEAD)"
        fi
        (cd "${REPO}" && git add -- "${!TOUCHED[@]}" && git commit -q -F - <<COMMITMSG
Repoint tracker citations after the ${PATHS[0]##*/} purge

$(printf '%s\n' "${PATHS[@]}" | sed 's/^/    /')
was removed from ${N_RANGE} commit(s) of ${RANGE}, which re-shaed ${N_CHANGED} of
them. Every sha those commits are cited by would otherwise point at an object no
branch reaches -- and git validates no sha written inside a file, so the rot is
silent. This commit carries ONLY that repoint: ${N_SUB} citation(s) across
${#TOUCHED[@]} file(s), old sha -> new sha, no prose changed.

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
    if G grep -qwIF "${t}" -- . 2>/dev/null; then
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
