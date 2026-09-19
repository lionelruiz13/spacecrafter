#!/bin/bash
# The owner's REVIEW QUEUE (INTENT S2.1 G12 (S7), S13 B46): what changed in the code since the
# commit he last marked as read. Sessions write code whenever; he reads whenever; this decouples
# the two. The mark is a git ref of the CODE repo, refs/review/<queue>, and its reflog is the
# history of his reviews - no file to keep in sync.
#   review.sh [status]        how much is waiting: commits, files, shape numbers at both ends
#   review.sh show [path..]   the waiting diff in the pager, then asks whether to mark it read
#   review.sh added [path..]  only the added or rewritten lines (a batch that mostly deletes)
#   review.sh commits [path..]  the same, one commit at a time, oldest first
#   review.sh done [commit]   mark read up to HEAD (or up to <commit>: partial review)
#   review.sh log             the marks so far
# Scope: headers under src/ (his review surface). ALL=1: every source file and shader.
# QUEUE=<name>: another independent mark (default: headers).
set -u
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
SHAPE="$ROOT/claude/harness/shape"
REF="refs/review/${QUEUE:-headers}"
g() { git -C "$ROOT" "$@"; }
if [ "${ALL:-0}" = 1 ]; then SCOPE=(src shaders); else SCOPE=(':(glob)src/**/*.hpp' ':(glob)src/**/*.h'); fi
cmd="${1:-status}"; [ $# -gt 0 ] && shift
[ $# -gt 0 ] && [ "$cmd" != done ] && SCOPE=("$@")

mark() { g rev-parse -q --verify "$REF^{commit}" 2>/dev/null; }
need_mark() {
    mark >/dev/null && return 0
    echo "No mark yet for queue '${QUEUE:-headers}'. Set the commit you last read:  $0 done <commit>"; exit 1
}
ask_done() {
    [ -t 0 ] && [ -t 1 ] || return 0
    local head; head=$(g rev-parse --short HEAD)
    read -r -p "Mark read up to $head? [y/N] " a
    [ "$a" = y ] || [ "$a" = Y ] && "$0" done
}

case "$cmd" in
status)
    need_mark
    n=$(g rev-list --count "$REF..HEAD" -- "${SCOPE[@]}")
    echo "read up to : $(g log -1 --format='%h %ad %an  %s' --date=short "$REF" | cut -c1-110)"
    echo "HEAD       : $(g log -1 --format='%h %ad %an  %s' --date=short HEAD | cut -c1-110)"
    echo "waiting    : $n commit(s) touching the scope"
    [ "$n" = 0 ] && exit 0
    echo; g log --reverse --format='  %h %ad %<(16,trunc)%an %s' --date=short "$REF..HEAD" -- "${SCOPE[@]}" | cut -c1-140
    echo; g diff --stat=110 "$REF" HEAD -- "${SCOPE[@]}" | tail -26
    echo; echo "module shape, read -> HEAD:"
    ( cd "$ROOT" && python3 "$SHAPE/ratio.py" "$(g rev-parse --short "$REF")" | head -1 && python3 "$SHAPE/ratio.py" HEAD | head -1 ) | sed 's/^/  /'
    ;;
show)
    need_mark
    g diff --stat=110 --patch "$REF" HEAD -- "${SCOPE[@]}"
    ask_done ;;
added)   # only what was ADDED or rewritten: the cheap read when a batch is mostly deletions
    need_mark
    g diff -U0 "$REF" HEAD -- "${SCOPE[@]}" | /usr/bin/grep -E '^(\+|@@|diff --git)' | /usr/bin/grep -v '^+++' | ${PAGER:-less -R}
    ask_done ;;
commits)
    need_mark
    g log --reverse --patch --stat=110 --format='%n======== %h  %an  %ad%n%s%n%b' --date=short "$REF..HEAD" -- "${SCOPE[@]}"
    ask_done ;;
done)
    to="${1:-HEAD}"
    new=$(g rev-parse -q --verify "$to^{commit}") || { echo "not a commit: $to"; exit 1; }
    old=$(mark || echo none)
    g update-ref --create-reflog -m "read by the owner up to $(g rev-parse --short "$new")" "$REF" "$new"
    echo "queue '${QUEUE:-headers}': $( [ "$old" = none ] && echo none || g rev-parse --short "$old") -> $(g rev-parse --short "$new")"
    ;;
log)
    need_mark
    g reflog show --date=iso "$REF" ;;
*)
    sed -n '2,14p' "$0" | sed 's/^# \{0,1\}//' ;;
esac
