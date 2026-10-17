#!/usr/bin/env bash
# Usage: ./c17-check.sh [format|format-fix|warn|tidy|fix|cppcheck|spatch|spatch-apply|all]
# Env: SRC, INC, CC, BUILD
set -euo pipefail

SRC="${SRC:-src include tests}"
INC="${INC:--Iinclude -Isrc -Isrc/moshier -Isrc/erfa -Isrc/erfa/vendor -Itests}"
CC="${CC:-cc}"
BUILD="${BUILD:-build}"
ROOT="$(cd "$(dirname "$0")" && pwd)"

skip() {
    case "$1" in
        src/erfa/*|tests/fixtures/*|src/moshier_full.c|src/moshier/moonlat404.c|src/moshier/moonlr404.c)
            return 0
            ;;
    esac
    return 1
}

collect() {
    local f
    FILES=()
    # shellcheck disable=SC2086
    while IFS= read -r f; do
        f="${f#./}"
        if skip "$f"; then
            continue
        fi
        FILES+=("$f")
    done < <(find $SRC -type f \( -name '*.c' -o -name '*.h' \) 2>/dev/null | sort)
    CFILES=()
    for f in "${FILES[@]}"; do
        case "$f" in
            *.c) CFILES+=("$f") ;;
        esac
    done
    if [ "${#FILES[@]}" -eq 0 ]; then
        echo "No C files found in: $SRC" >&2
        exit 2
    fi
}

# isolate-declaration, simplify-boolean-expr, and math-missing-parentheses can
# change numeric C. They stay in the tidy report and out of the rewrite list.
SAFE_FIX='-*,readability-braces-around-statements,readability-uppercase-literal-suffix,readability-redundant-control-flow,readability-else-after-return'

WARN=(-std=c17 -pedantic -Wall -Wextra -Wconversion -Wshadow -Wstrict-prototypes
      -Wmissing-prototypes -Wold-style-definition -Wformat=2 -Wundef -Wcast-qual
      -Wpointer-arith -Wvla)

SPATCH_INC=(-I include -I src -I src/moshier -I src/erfa -I src/erfa/vendor -I tests)

# clang-tidy interpolates a missing file onto a similar path, so test_julian.c
# would be parsed with src/julian.c's flags. Give every file its own command.
prepare_tidy_db() {
    local db_dir="$BUILD/c17-db"
    mkdir -p "$db_dir"
    python3 - "$ROOT" "$CC" "$BUILD/compile_commands.json" "$db_dir/compile_commands.json" "${CFILES[@]}" <<'PY'
import json
import os
import shlex
import sys

root, cc, src_db, out_db = sys.argv[1:5]
files = sys.argv[5:]
by_real = {}
if os.path.isfile(src_db):
    with open(src_db, encoding="utf-8") as fh:
        cmds = json.load(fh)
    for cmd in cmds:
        path = cmd.get("file", "")
        if not os.path.isabs(path):
            path = os.path.join(cmd.get("directory") or root, path)
        by_real[os.path.realpath(path)] = cmd

includes = [
    "-std=c17",
    "-I" + os.path.join(root, "include"),
    "-I" + os.path.join(root, "src"),
    "-I" + os.path.join(root, "src", "moshier"),
    "-I" + os.path.join(root, "src", "erfa"),
    "-I" + os.path.join(root, "src", "erfa", "vendor"),
    "-I" + os.path.join(root, "tests"),
]
entries = []
for path in files:
    abs_path = path if os.path.isabs(path) else os.path.join(root, path)
    real = os.path.realpath(abs_path)
    known = by_real.get(real)
    if known is not None:
        entries.append(known)
        continue
    entries.append(
        {
            "directory": root,
            "file": abs_path,
            "command": shlex.join([cc, *includes, "-c", abs_path]),
        }
    )
with open(out_db, "w", encoding="utf-8") as fh:
    json.dump(entries, fh)
PY
    TIDY_DB="$db_dir"
}

fingerprint() {
    cksum "${FILES[@]}"
}

cd "$ROOT"
collect

case "${1:-all}" in
    format)
        clang-format --style="file:$ROOT/.clang-format" --dry-run --Werror "${FILES[@]}"
        ;;
    format-fix)
        clang-format --style="file:$ROOT/.clang-format" -i "${FILES[@]}"
        ;;
    warn)
        for f in "${CFILES[@]}"; do
            # shellcheck disable=SC2086
            "$CC" "${WARN[@]}" $INC -fsyntax-only "$f"
        done
        ;;
    tidy)
        prepare_tidy_db
        clang-tidy "${CFILES[@]}" -p "$TIDY_DB"
        ;;
    fix)
        # else-after-return removes one else per invocation. Repeat until a
        # pass edits nothing, then format even when tidy returns non-zero.
        prepare_tidy_db
        status=0
        pass=1
        max_passes=32
        before="$(fingerprint)"
        while [ "$pass" -le "$max_passes" ]; do
            set +e
            clang-tidy --fix --checks="$SAFE_FIX" "${CFILES[@]}" -p "$TIDY_DB"
            status=$?
            set -e
            after="$(fingerprint)"
            if [ "$before" = "$after" ]; then
                break
            fi
            echo "c17-fix: pass ${pass} edited files" >&2
            before="$after"
            pass=$((pass + 1))
        done
        if [ "$pass" -gt "$max_passes" ]; then
            echo "c17-fix: clang-tidy kept editing after ${max_passes} passes" >&2
            status=1
        fi
        clang-format --style="file:$ROOT/.clang-format" -i "${FILES[@]}"
        exit "$status"
        ;;
    cppcheck)
        # cppcheck 2.19 has c89, c99, and c11. C17 adds no syntax over c11.
        # legacyUninitvar on vobs and geo_b is a false positive: the callee
        # writes them when the pointer is passed. Do not zero the arrays.
        # Style-only checks (constness, variable scope) are covered by
        # clang-tidy; cppcheck runs the defect/performance/portability classes.
        # shellcheck disable=SC2086
        cppcheck --std=c11 --enable=warning,performance,portability \
            --inline-suppr --error-exitcode=1 -q \
            --suppress=legacyUninitvar:src/calc.c \
            $INC \
            -i src/erfa -i tests/fixtures \
            -i src/moshier_full.c -i src/moshier/moonlat404.c -i src/moshier/moonlr404.c \
            src include tests
        ;;
    spatch)
        # No --include-headers: that rewrites headers included from these
        # files, including src/erfa.
        spatch --sp-file "$ROOT/modernize.cocci" "${SPATCH_INC[@]}" "${CFILES[@]}"
        ;;
    spatch-apply)
        set +e
        spatch --sp-file "$ROOT/modernize.cocci" "${SPATCH_INC[@]}" --in-place "${CFILES[@]}"
        status=$?
        set -e
        clang-format --style="file:$ROOT/.clang-format" -i "${FILES[@]}"
        exit "$status"
        ;;
    all)
        "$0" format
        "$0" warn
        "$0" tidy
        "$0" cppcheck
        ;;
    *)
        echo "Unknown mode: $1" >&2
        exit 2
        ;;
esac
