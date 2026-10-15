.DEFAULT_GOAL := help

.PHONY: help build test \
	c17-all c17-format c17-format-fix c17-warn c17-tidy \
	c17-cppcheck c17-fix c17-spatch c17-spatch-apply

help:
	@printf '%s\n' \
		'make build            configure and build with CMake (ISO C17 / C18)' \
		'make test             build, then ctest' \
		'make c17-all          read-only format, warnings, clang-tidy, cppcheck' \
		'make c17-format       list files that differ from .clang-format' \
		'make c17-format-fix   apply .clang-format' \
		'make c17-warn         syntax-check first-party C with -std=c17' \
		'make c17-tidy         clang-tidy report' \
		'make c17-cppcheck     cppcheck report (cppcheck standard c11)' \
		'make c17-fix          braces, literal suffix, redundant flow, else-after-return' \
		'make c17-spatch       Coccinelle dry run' \
		'make c17-spatch-apply Coccinelle rewrite'

build:
	cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cmake --build build

test: build
	ctest --test-dir build --output-on-failure

c17-all:
	./c17-check.sh all

c17-format:
	./c17-check.sh format

c17-format-fix:
	./c17-check.sh format-fix

c17-warn:
	./c17-check.sh warn

c17-tidy:
	./c17-check.sh tidy

c17-cppcheck:
	./c17-check.sh cppcheck

c17-fix:
	./c17-check.sh fix

c17-spatch:
	./c17-check.sh spatch

c17-spatch-apply:
	./c17-check.sh spatch-apply
