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

# isolate-declaration and simplify-boolean-expr can change numeric C. They stay
# in the tidy report and out of the rewrite list.
SAFE_FIX='-*,readability-braces-around-statements,readability-uppercase-literal-suffix,readability-redundant-control-flow,readability-else-after-return'

WARN=(-std=c17 -pedantic -Wall -Wextra -Wconversion -Wshadow -Wstrict-prototypes
      -Wmissing-prototypes -Wold-style-definition -Wformat=2 -Wundef -Wcast-qual
      -Wpointer-arith -Wvla)

tidy_args() {
    if [ -f "$BUILD/compile_commands.json" ]; then
        printf '%s\n' "-p" "$BUILD"
    else
        printf '%s\n' "--" "-std=c17" "-Iinclude" "-Isrc" "-Isrc/moshier" "-Isrc/erfa" "-Isrc/erfa/vendor" "-Itests"
    fi
}

cd "$ROOT"
collect

case "${1:-all}" in
    format)
        clang-format --dry-run --Werror "${FILES[@]}"
        ;;
    format-fix)
        clang-format -i "${FILES[@]}"
        ;;
    warn)
        for f in "${CFILES[@]}"; do
            # shellcheck disable=SC2086
            "$CC" "${WARN[@]}" $INC -fsyntax-only "$f"
        done
        ;;
    tidy)
        # shellcheck disable=SC2046
        clang-tidy "${CFILES[@]}" $(tidy_args)
        ;;
    fix)
        # shellcheck disable=SC2046
        clang-tidy --fix --checks="$SAFE_FIX" "${CFILES[@]}" $(tidy_args)
        clang-format -i "${FILES[@]}"
        ;;
    cppcheck)
        # cppcheck 2.19 has c89, c99, and c11. C17 adds no syntax over c11.
        # shellcheck disable=SC2086
        cppcheck --std=c11 --enable=warning,style,performance,portability \
            --inline-suppr --error-exitcode=1 -q \
            $INC \
            -i src/erfa -i tests/fixtures \
            -i src/moshier_full.c -i src/moshier/moonlat404.c -i src/moshier/moonlr404.c \
            src include tests
        ;;
    spatch)
        spatch --sp-file "$ROOT/modernize.cocci" --include-headers "${CFILES[@]}"
        ;;
    spatch-apply)
        spatch --sp-file "$ROOT/modernize.cocci" --include-headers --in-place "${CFILES[@]}"
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
