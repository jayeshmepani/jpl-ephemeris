#!/usr/bin/env python3
"""Regenerate the counts and inventory tables of docs/API_REFERENCE.md from the public headers."""

from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
HEADERS = [ROOT / "include/jme/jme.h", ROOT / "include/jme/jme_extended.h"]
DOC_PATH = ROOT / "docs/API_REFERENCE.md"

FUNCTION_RE = re.compile(
    r"(?m)^\s*(?:const\s+char\s*\*\s*|char\s*\*\s*|void\s+|double\s+|int\s+)(jme_[a-zA-Z0-9_]+)\s*\("
)
CONSTANT_RE = re.compile(r"\bJME_[A-Z0-9_]+\b")
GUARD_RE = re.compile(r"(?m)^\s*#\s*ifndef\s+(JME_[A-Z0-9_]+_H)\s*$")


def inventory() -> tuple[list[str], list[str]]:
    text = "\n".join(path.read_text(encoding="utf-8") for path in HEADERS)
    guards = set(GUARD_RE.findall(text))
    functions = sorted(set(FUNCTION_RE.findall(text)))
    constants = sorted(set(CONSTANT_RE.findall(text)) - guards)
    return functions, constants


def replace_table(doc: str, heading: str, rows: list[str]) -> str:
    pattern = re.compile(
        r"("
        + re.escape(heading)
        + r"\n\n\| # \| [^\n]+\n\|---:\|---\|---\|\n)(?:\|[^\n]*\n)*"
    )
    if not pattern.search(doc):
        raise SystemExit(f"table under '{heading}' not found in {DOC_PATH}")
    return pattern.sub(
        lambda m: m.group(1) + "".join(r + "\n" for r in rows), doc, count=1
    )


def replace_count(doc: str, label: str, value: int) -> str:
    pattern = re.compile(r"(?m)^(\| " + re.escape(label) + r" \| )\d+( \|)")
    if not pattern.search(doc):
        raise SystemExit(f"count row '{label}' not found in {DOC_PATH}")
    return pattern.sub(lambda m: f"{m.group(1)}{value}{m.group(2)}", doc, count=1)


def main() -> int:
    functions, constants = inventory()
    doc = DOC_PATH.read_text(encoding="utf-8")
    doc = replace_count(doc, "Public `jme_*` functions", len(functions))
    doc = replace_count(doc, "Public `JME_*` constants", len(constants))
    doc = replace_table(
        doc,
        "## Current Public Functions",
        [
            f"| {i} | `{name}` | Declared and defined |"
            for i, name in enumerate(functions, 1)
        ],
    )
    doc = replace_table(
        doc,
        "## Current Public Constants",
        [
            f"| {i} | `{name}` | Public constant |"
            for i, name in enumerate(constants, 1)
        ],
    )
    DOC_PATH.write_text(doc, encoding="utf-8")
    print(
        f"generated {DOC_PATH}: {len(functions)} functions, {len(constants)} constants"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
