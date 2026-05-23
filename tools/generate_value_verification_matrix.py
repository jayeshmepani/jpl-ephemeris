#!/usr/bin/env python3
from __future__ import annotations

import csv
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
HEADERS = [ROOT / "include/jme/jme.h", ROOT / "include/jme/jme_extended.h"]
DOC_PATH = ROOT / "docs/VALUE_VERIFICATION_MATRIX.md"
CSV_PATH = ROOT / "build/value_verification_matrix.csv"


def load_functions() -> list[str]:
    text = "\n".join(path.read_text(encoding="utf-8") for path in HEADERS)
    funcs = sorted(
        set(
            re.findall(
                r"(?m)^\s*(?:const\s+char\s*\*\s*|char\s*\*\s*|void\s+|double\s+|int\s+)(jme_[a-zA-Z0-9_]+)\s*\(",
                text,
            )
        )
    )
    return funcs


EXTERNAL_WEB_NUMERIC = {
    "jme_calc",
    "jme_calc_ut",
    "jme_get_orbital_elements",
    "jme_lun_eclipse_how",
    "jme_lun_eclipse_when",
    "jme_lun_eclipse_when_loc",
    "jme_lun_occult_where",
    "jme_lun_occult_when_glob",
    "jme_lun_occult_when_loc",
    "jme_sidereal_time",
    "jme_sidereal_time0",
    "jme_sol_eclipse_how",
    "jme_sol_eclipse_when_glob",
    "jme_sol_eclipse_when_loc",
    "jme_sol_eclipse_where",
}

EMBEDDED_REFERENCE_NUMERIC = {
    "jme_elp2000_moon_state",
    "jme_meeus_moon_state",
    "jme_meeus_planet_state",
    "jme_meeus_sun_state",
    "jme_moshier_planet_state",
    "jme_vsop87_planet_state",
}

INDEPENDENT_FORMULA = {
    "jme_calendar_is_leap_year",
    "jme_centiseconds_difference",
    "jme_centiseconds_difference_signed",
    "jme_centiseconds_normalize",
    "jme_centiseconds_round_second",
    "jme_date_is_valid",
    "jme_day_of_week",
    "jme_day_of_year",
    "jme_days_in_month",
    "jme_decimal_hour",
    "jme_degree_midpoint",
    "jme_degree_normalize",
    "jme_degrees_difference",
    "jme_degrees_difference_signed",
    "jme_degrees_to_hours",
    "jme_degrees_to_radians",
    "jme_delta_t",
    "jme_delta_t_ex",
    "jme_double_to_long",
    "jme_ecliptic_to_equatorial",
    "jme_ecliptic_to_equatorial_rectangular_state",
    "jme_equatorial_to_ecliptic",
    "jme_equatorial_to_ecliptic_rectangular_state",
    "jme_equatorial_to_horizontal",
    "jme_get_ayanamsa",
    "jme_get_ayanamsa_ex",
    "jme_get_ayanamsa_ex_ut",
    "jme_get_ayanamsa_ut",
    "jme_get_nutation",
    "jme_get_nutation_matrix",
    "jme_get_obliquity",
    "jme_get_precession_matrix",
    "jme_get_topo_pos",
    "jme_horizontal_to_equatorial",
    "jme_hours_normalize",
    "jme_hours_to_degrees",
    "jme_jd_add_seconds",
    "jme_jd_difference_seconds",
    "jme_jd_to_utc",
    "jme_julian_day",
    "jme_lat_to_lmt",
    "jme_lmt_to_lat",
    "jme_matrix_identity",
    "jme_matrix_multiply",
    "jme_matrix_rotate_x",
    "jme_matrix_rotate_y",
    "jme_matrix_rotate_z",
    "jme_matrix_transform_state",
    "jme_radian_midpoint",
    "jme_radian_normalize",
    "jme_radians_difference_signed",
    "jme_radians_to_degrees",
    "jme_rectangular_to_spherical_state",
    "jme_refract",
    "jme_refract_extended",
    "jme_reverse_julian_day",
    "jme_spherical_angular_separation",
    "jme_spherical_position_angle",
    "jme_spherical_to_rectangular_state",
    "jme_split_degree",
    "jme_state_add",
    "jme_state_convert_units",
    "jme_state_distance",
    "jme_state_light_time_days",
    "jme_state_position_velocity_dot",
    "jme_state_scale",
    "jme_state_speed",
    "jme_state_subtract",
    "jme_time_equ",
    "jme_utc_time_zone",
    "jme_utc_to_jd",
}


def classify(fn: str) -> tuple[str, str]:
    if fn in EXTERNAL_WEB_NUMERIC:
        return (
            "external_web_numeric",
            "Checked against authoritative web/reference values in the current test suite.",
        )
    if fn in EMBEDDED_REFERENCE_NUMERIC:
        return (
            "embedded_reference_numeric",
            "Checked against bundled known-value/reference datasets or derived reference cases.",
        )
    if fn in INDEPENDENT_FORMULA:
        return (
            "independent_formula_or_identity",
            "Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant.",
        )
    if fn.startswith("jme_jpl_"):
        return (
            "kernel_runtime_or_error_contract",
            "Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts.",
        )
    return (
        "api_or_behavior_contract",
        "Directly tested, but not yet broadly externally value-certified against an independent web oracle.",
    )


def mode_status(mode: str) -> str:
    if mode == "external_web_numeric":
        return "web-source numeric verified"
    if mode == "embedded_reference_numeric":
        return "reference numeric verified"
    if mode == "independent_formula_or_identity":
        return "formula/identity verified"
    if mode == "kernel_runtime_or_error_contract":
        return "runtime/error contract verified"
    return "contract verified only"


def main() -> int:
    funcs = load_functions()
    if len(funcs) != 204:
        raise SystemExit(f"expected 204 public functions, found {len(funcs)}")

    rows: list[tuple[str, str, str, str]] = []
    counts: dict[str, int] = {}
    for fn in funcs:
        mode, note = classify(fn)
        rows.append((fn, mode, mode_status(mode), note))
        counts[mode] = counts.get(mode, 0) + 1

    CSV_PATH.parent.mkdir(parents=True, exist_ok=True)
    with CSV_PATH.open("w", encoding="utf-8", newline="") as fh:
        writer = csv.writer(fh)
        writer.writerow(["function", "verification_mode", "status", "note"])
        writer.writerows(rows)

    lines = [
        "# Value Verification Matrix",
        "",
        "This file tracks how each public `jme_*` function is currently validated.",
        "",
        "It separates four different ideas that are often conflated:",
        "",
        "- symbol existence",
        "- direct test invocation",
        "- internal contract/identity validation",
        "- independent external numeric validation",
        "",
        "Current source of truth for inventory:",
        "",
        "- `204` public functions from `include/jme/jme.h` and `include/jme/jme_extended.h`",
        "",
        "## Summary",
        "",
        "| Verification mode | Count | Meaning |",
        "|---|---:|---|",
    ]

    meaning = {
        "external_web_numeric": "Compared against authoritative external web/reference values.",
        "embedded_reference_numeric": "Compared against bundled reference datasets or known-value source material.",
        "independent_formula_or_identity": "Validated by deterministic mathematical identities, formulae, or round-trips.",
        "kernel_runtime_or_error_contract": "Validated by CALCEPH runtime success paths or explicit unavailable/error contracts.",
        "api_or_behavior_contract": "Called directly and contract-tested, but not yet broadly externally value-certified.",
    }
    order = [
        "external_web_numeric",
        "embedded_reference_numeric",
        "independent_formula_or_identity",
        "kernel_runtime_or_error_contract",
        "api_or_behavior_contract",
    ]
    for mode in order:
        lines.append(f"| `{mode}` | {counts.get(mode, 0)} | {meaning[mode]} |")

    lines.extend(
        [
            "",
            "## Interpretation",
            "",
            "- `web-source numeric verified` is the strongest current category for public astronomical outputs.",
            "- `contract verified only` does not mean untested; it means the function is exercised and hardened, but not yet certified against an independent external numeric oracle.",
            "- Some functions are not meaningful candidates for web-value checks at all, such as setters, getters, serialization helpers, and unavailable-kernel boundary APIs.",
            "",
            "## Per-Function Matrix",
            "",
            "| Function | Verification mode | Current status | Note |",
            "|---|---|---|---|",
        ]
    )
    for fn, mode, status, note in rows:
        lines.append(f"| `{fn}` | `{mode}` | {status} | {note} |")

    DOC_PATH.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"generated {DOC_PATH}")
    print(f"generated {CSV_PATH}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
