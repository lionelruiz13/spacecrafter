#!/usr/bin/env bash
#
# supervised-by.sh
#
# Rewrites the trailers
#     Co-Authored-By: Claude Fable 5 ...
#     Co-Authored-By: Claude Opus 4.8 ...
# into
#     Supervised-By: <string you type in>
# across a range of commits, WITHOUT touching a single byte of code.
#
# ---------------------------------------------------------------------------
# WHY NOT `git rebase` (the trap that just cost you an afternoon)
# ---------------------------------------------------------------------------
# `git rebase` REPLAYS commits: it re-applies each one onto a new base. If the
# range contains a MERGE commit, `--rebase-merges` emits a `merge -C <sha>` step
# that re-PERFORMS the merge -- so every conflict must be resolved again by hand,
# and a wrong resolution silently changes the code. Renaming a message needs no
# replay at all. Rebase is simply the wrong instrument for this job.
#
# ---------------------------------------------------------------------------
# THE COMMAND THIS TEACHES: `git filter-branch --msg-filter`
# ---------------------------------------------------------------------------
# filter-branch walks a range and rebuilds each commit, letting you filter parts
# of it. With --msg-filter it pipes ONLY the commit message through your command
# (message on stdin, new message on stdout) and reuses the original TREE and the
# original PARENTS verbatim. Therefore:
#   * merges keep both parents -- nothing is re-merged, no conflicts possible
#   * content cannot change    -- we assert this at the end rather than trust it
#   * a commit whose message is untouched rebuilds byte-identical and keeps its
#     SHA; only rewritten commits and their descendants get new SHAs
#
# (Modern alternative: `git filter-repo --message-callback`, which is faster but
#  is a separate install. filter-branch is used here because it ships with git.)
#
# ---------------------------------------------------------------------------
# WHY STEP 6 EXISTS (the .md re-point that stops the recurrence)
# ---------------------------------------------------------------------------
# The rewrite gives every touched commit a NEW sha. The .md trackers in this
# repo (INTENT.md, the dispatch logs, ...) cite commits BY sha to trace work --
# so after a rewrite every one of those citations points at a dangling object,
# and the traceability the trackers exist for silently rots. Step 6 detects it
# and PROPOSES the fix: it builds the old->new sha map from the two sides of the
# rewrite (matched by a fingerprint the rewrite preserves) and offers to repoint
# every abbreviated hash it finds. It touches ONLY *.md -- those are the tracing
# docs; code is never edited here -- and it never applies without your y/N.
# (Requires bash 4+ for associative arrays.)
#
# Usage:
#   ./supervised-by.sh                  # range defaults to HEAD (all history)
#   ./supervised-by.sh 93a14377..HEAD   # narrower range = much faster
# ---------------------------------------------------------------------------

set -euo pipefail

RANGE="${1:-HEAD}"

# Matched in the message body. sed (line-based) anchors with ^ itself further
# down; `git log --grep` sees the whole message, so no anchor here.
MATCH='Co-Authored-By: Claude (Fable 5|Opus 4\.8)'

cd "$(git rev-parse --show-toplevel)"

# --- 0. refuse to run on a repo that is mid-operation -----------------------
if [ -d .git/rebase-merge ] || [ -d .git/rebase-apply ]; then
    echo "error: a rebase is in progress. Finish it, or 'git rebase --abort' first." >&2
    exit 1
fi

# --- 1. look before you leap: show exactly what will be touched -------------
echo "Range: ${RANGE}"
echo "Commits carrying the trailer:"
git log --oneline -E --grep="${MATCH}" "${RANGE}" || true

COUNT=$(git log --format='%H' -E --grep="${MATCH}" "${RANGE}" | wc -l)
if [ "${COUNT}" -eq 0 ]; then
    echo "Nothing to do."
    exit 0
fi
echo "(${COUNT} commit(s) match; their descendants also get new SHAs.)"
echo

# --- 2. acquire the string --------------------------------------------------
# `read` pulls one line from stdin into a variable.
#   -r : raw mode -- a backslash stays a backslash instead of escaping the next
#        character. For free text this is essentially always what you want.
#   -p : print a prompt first (a bash extension; not in POSIX sh).
read -r -p "Supervised-By string (e.g. 'Vixy <vixy@example.com>'): " SUPERVISOR
[ -n "${SUPERVISOR}" ] || { echo "error: empty string; aborting." >&2; exit 1; }

read -r -p "Rewrite ${COUNT} commit(s) in ${RANGE}? [y/N] " CONFIRM
case "${CONFIRM}" in
    [yY]*) ;;
    *) echo "aborted."; exit 0 ;;
esac

# sed's replacement treats \ and & specially, and # is our delimiter -- escape
# all three so whatever you typed is inserted literally.
SUPERVISOR_ESC=$(printf '%s' "${SUPERVISOR}" | sed -e 's/[\\&#]/\\&/g')
export SUPERVISOR_ESC

# --- 3. remember the tip so we can PROVE only text changed ------------------
ORIGINAL_TIP=$(git rev-parse HEAD)
BRANCH=$(git branch --show-current || true)

# --- 4. rewrite -------------------------------------------------------------
# -f : overwrite the refs/original/ backup left behind by a previous run.
# The filter runs once per commit, in a subshell, so ${SUPERVISOR_ESC} must be
# exported (done above) rather than merely set.
FILTER_BRANCH_SQUELCH_WARNING=1 \
git filter-branch -f --msg-filter '
    sed -E "s#^Co-Authored-By: Claude (Fable 5|Opus 4\.8).*\$#Supervised-By: ${SUPERVISOR_ESC}#"
' -- "${RANGE}"

# --- 5. the proof: content must be byte-identical ---------------------------
# Only messages were supposed to change, so the old tip and the new tip must
# have identical content. Asserting beats assuming: if this ever fails, some-
# thing replayed the code and we roll straight back.
if git diff --quiet "${ORIGINAL_TIP}" HEAD; then
    echo
    echo "VERIFIED: content identical to ${ORIGINAL_TIP} -- only messages changed."
else
    echo "STOP: content differs from ${ORIGINAL_TIP}. Rolling back." >&2
    git reset --hard "${ORIGINAL_TIP}"
    exit 1
fi

# --- 6. repoint .md tracker commit references (old SHA -> new SHA) -----------
# Only .md is scanned: those are the docs used for tracing. Code is never
# touched by this script, and nothing is written without an explicit y/N.
echo
echo "--- .md tracker commit references -------------------------------------"

# The two SIDES of the rewrite. ORIGINAL_TIP still resolves (its objects are
# dangling but present), so its history is the OLD side; HEAD is the NEW side.
# `--not` on each gives the symmetric difference: exactly the commits whose sha
# changed. A commit that rebuilt byte-identical (unchanged message, unchanged
# ancestors) appears in NEITHER list, so it is correctly left alone.
OLD_SHAS=(); NEW_SHAS=()
mapfile -t NEW_SHAS < <(git rev-list HEAD --not "${ORIGINAL_TIP}" 2>/dev/null || true)
mapfile -t OLD_SHAS < <(git rev-list "${ORIGINAL_TIP}" --not HEAD 2>/dev/null || true)

# old full sha -> new full sha, matched by a fingerprint the rewrite preserves:
# tree + author-date + author + SUBJECT. Only the trailer LINE of the message
# changed, so the subject (first line) is identical -- and tree/author are never
# touched. This is what makes the 1:1 pairing reliable even across merges.
declare -A NEW_OF_OLD=()
if [ "${#NEW_SHAS[@]}" -gt 0 ] && [ "${#OLD_SHAS[@]}" -gt 0 ]; then
    declare -A NEW_BY_FP=()
    for ns in "${NEW_SHAS[@]}"; do
        NEW_BY_FP["$(git log -1 --format='%T|%at|%an|%s' "${ns}")"]="${ns}"
    done
    for os in "${OLD_SHAS[@]}"; do
        nf="${NEW_BY_FP[$(git log -1 --format='%T|%at|%an|%s' "${os}")]:-}"
        [ -n "${nf}" ] && NEW_OF_OLD["${os}"]="${nf}"
    done
fi

# Every hash-shaped token in tracked .md, resolved to its new SHORT form iff it
# is an unambiguous prefix of exactly ONE rewritten old sha (same length kept,
# so `b989d4bd` -> the new sha's first 8). A token that prefixes two rewritten
# commits is flagged ambiguous and never auto-replaced.
declare -A TOK_NEW=()
declare -A TOK_AMBIG=()
MD_FILES=()
mapfile -t MD_FILES < <(git ls-files -- '*.md' 2>/dev/null || true)
if [ "${#NEW_OF_OLD[@]}" -gt 0 ] && [ "${#MD_FILES[@]}" -gt 0 ]; then
    while IFS= read -r tok; do
        [ -n "${tok}" ] || continue
        hit=""
        for os in "${!NEW_OF_OLD[@]}"; do
            if [ "${os:0:${#tok}}" = "${tok}" ]; then       # tok is a prefix of os
                [ -n "${hit}" ] && [ "${hit}" != "${os}" ] && TOK_AMBIG["${tok}"]=1
                hit="${os}"
            fi
        done
        if [ -n "${hit}" ] && [ -z "${TOK_AMBIG[${tok}]:-}" ]; then
            TOK_NEW["${tok}"]="${NEW_OF_OLD[${hit}]:0:${#tok}}"
        fi
    done < <(grep -hoE '[0-9a-f]{7,40}' "${MD_FILES[@]}" 2>/dev/null | sort -u || true)
fi

if [ "${#TOK_NEW[@]}" -eq 0 ]; then
    echo "No .md references to rewritten commits -- trackers are already consistent."
else
    echo "These .md references point at rewritten (now-dangling) commits:"
    echo
    for tok in "${!TOK_NEW[@]}"; do
        while IFS= read -r loc; do
            echo "    ${loc}  (${tok} -> ${TOK_NEW[${tok}]})"
        done < <(grep -nwF "${tok}" "${MD_FILES[@]}" 2>/dev/null | cut -d: -f1,2 || true)
    done
    echo
    if [ "${#TOK_AMBIG[@]}" -gt 0 ]; then
        echo "  (${#TOK_AMBIG[@]} token(s) prefix more than one rewritten commit -- skipped,"
        echo "   repoint those by hand.)"
        echo
    fi
    APPLY=n
    if [ -t 0 ]; then
        read -r -p "Repoint these in the working tree (uncommitted, review before commit)? [y/N] " APPLY
    else
        echo "  (non-interactive shell: proposing only, nothing applied.)"
    fi
    case "${APPLY}" in
        [yY]*)
            for tok in "${!TOK_NEW[@]}"; do
                new="${TOK_NEW[${tok}]}"
                while IFS= read -r f; do
                    # \b so a token is never matched inside a longer hex run
                    sed -i -E "s/\\b${tok}\\b/${new}/g" "${f}"
                done < <(grep -lwF "${tok}" "${MD_FILES[@]}" 2>/dev/null || true)
            done
            echo
            echo "Repointed (UNCOMMITTED -- review, then commit):"
            echo "    git diff -- '*.md'"
            echo "    git commit -am 'docs: repoint tracker commit refs after trailer rewrite'"
            ;;
        *)
            echo "Not applied. Re-run and answer y, or repoint by hand from the list above."
            ;;
    esac
fi
echo "----------------------------------------------------------------------"

echo
echo "Result:"
git log --oneline -5
echo
echo "Undo (filter-branch kept a backup):  git reset --hard ${ORIGINAL_TIP}"
if [ -n "${BRANCH}" ]; then
echo "Drop that backup once satisfied:     git update-ref -d refs/original/refs/heads/${BRANCH}"
fi
echo "Publish (SHAs changed, so force):    git push --force-with-lease"
