#!/usr/bin/env python3
"""Audit UI coverage of the canonical VoxP4 parameter contract.

Reads the canonical registry (src/model/ParameterRegistryData.inl) and the UI
presentation metadata (src/ui/params/UiParamModel.cpp) and reports, for every
wire parameter, where it is exposed in the UI and at which generation level.

Exit code is non-zero when a canonical parameter has no UI placement, or when a
parameter is placed more than once. This is the script counterpart of the
`test_ui_coverage_all_parameters` native test.

Usage:
    python scripts/audit_ui_coverage.py [--check]
"""
from __future__ import annotations

import argparse
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REGISTRY_INL = os.path.join(ROOT, "src", "model", "ParameterRegistryData.inl")
PARAM_IDS_H = os.path.join(ROOT, "src", "voxlink", "generated", "VoxP4ParamIds.h")
MODEL_CPP = os.path.join(ROOT, "src", "ui", "params", "UiParamModel.cpp")

EFFECT_NAMES = {
    "Gate": "GATE",
    "Compressor": "COMPRESSOR",
    "Harmony": "HARMONY",
    "Drive": "DRIVE",
    "Modulation": "MODULATION",
    "Delay": "DELAY",
    "Reverb": "REVERB",
}


def load_param_ids() -> dict[str, int]:
    ids: dict[str, int] = {}
    with open(PARAM_IDS_H, "r", encoding="utf-8") as f:
        for line in f:
            m = re.match(r"#define\s+(VOXP4_PARAM_[A-Z0-9_]+)\s+(0x[0-9A-Fa-f]+)u", line)
            if m:
                ids[m.group(1)] = int(m.group(2), 16)
    return ids


def load_registry() -> list[dict]:
    row_re = re.compile(
        r"\{\s*(\d+)u,\s*(\d+)u,\s*\"([^\"]*)\",\s*\"([^\"]*)\",\s*\"([^\"]*)\",\s*\"([^\"]*)\","
        r"\s*ParamType::(\w+),\s*([-0-9.]+)f,\s*([-0-9.]+)f,\s*([-0-9.]+)f\s*\}"
    )
    out = []
    with open(REGISTRY_INL, "r", encoding="utf-8") as f:
        for line in f:
            m = row_re.search(line)
            if m:
                out.append({
                    "id": int(m.group(1)),
                    "key": m.group(3),
                    "type": m.group(7),
                })
    return out


def parse_effect_enables(text: str) -> dict[str, str]:
    """effect name -> VOXP4_PARAM_* macro, from ui_effect_enable_wire()."""
    enabled: dict[str, str] = {}
    body = re.search(r"ui_effect_enable_wire\(UiEffectId effect\).*?\n\}", text, re.S)
    if not body:
        return enabled
    for m in re.finditer(
        r"case\s+UiEffectId::(\w+)\s*:\s*return\s+(VOXP4_PARAM_[A-Z0-9_]+)\s*;",
        body.group(0),
    ):
        enabled[m.group(1)] = m.group(2)
    return enabled


def parse_metadata_tables(text: str) -> list[dict]:
    entries: list[dict] = []
    # Capture each descriptor array: extern const UiParamMeta kUiXxxDescriptors[] = { ... };
    for arr in re.finditer(
        r"extern\s+const\s+UiParamMeta\s+(\w+)\[\]\s*=\s*\{(.*?)\n\};", text, re.S
    ):
        table_name = arr.group(1)
        body = arr.group(2)
        for m in re.finditer(r"UIP\((.*?)\),\s*(?=\n|/\*|$)", body, re.S):
            fields = [f.strip() for f in re.split(r",(?![^()]*\))", m.group(1))]
            if len(fields) < 13:
                continue
            entries.append({
                "table": table_name,
                "wire": fields[0],
                "label": fields[1].strip('"'),
                "section": fields[2].strip('"'),
                "control": fields[3],
                "format": fields[4],
                "visibility": fields[8],
            })
    return entries


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true", help="exit non-zero on gaps")
    args = parser.parse_args()

    param_ids = load_param_ids()
    registry = load_registry()
    with open(MODEL_CPP, "r", encoding="utf-8") as f:
        model_text = f.read()

    enables = parse_effect_enables(model_text)
    metadata = parse_metadata_tables(model_text)

    # Build placements keyed by dense index (registry order is dense order).
    placements: dict[int, dict] = {}
    order = {p["id"]: i for i, p in enumerate(registry)}

    def place(wire: int, loc: str, level: str, control: str) -> None:
        if wire not in order:
            raise SystemExit(f"UI references unknown wire ID 0x{wire:04X}")
        idx = order[wire]
        if idx in placements:
            raise SystemExit(
                f"duplicate UI placement for 0x{wire:04X} "
                f"({placements[idx]['loc']} / {loc})"
            )
        placements[idx] = {"loc": loc, "level": level, "control": control}

    for effect, macro in enables.items():
        name = EFFECT_NAMES.get(effect, effect.upper())
        place(param_ids[macro], f"{name} header toggle", "basic", "Toggle")

    for entry in metadata:
        wire = param_ids[entry["wire"]]
        table = entry["table"]
        # Derive the owning effect from the table name.
        owner = None
        for effect in EFFECT_NAMES:
            if f"kUi{effect}Descriptors" == table:
                owner = EFFECT_NAMES[effect]
                break
        loc = f"{owner} / {entry['section']} / {entry['label']}" if owner else \
              f"MASTER / {entry['section']} / {entry['label']}"
        level = "advanced" if entry["visibility"] == "Advanced" else (
            "basic" if entry["visibility"] == "Basic" else "global")
        place(wire, loc, level, entry["control"])

    missing = [p for i, p in enumerate(registry) if i not in placements]

    print("| wire ID | parameter | UI location | basic/advanced | control type |")
    print("|---|---|---|---|---|")
    for i, p in enumerate(registry):
        pl = placements.get(i)
        if pl is None:
            print(f"| 0x{p['id']:04X} | {p['key']} | **MISSING** | - | - |")
        else:
            print(f"| 0x{p['id']:04X} | {p['key']} | {pl['loc']} | "
                  f"{pl['level']} | {pl['control']} |")

    print()
    print(f"registry parameters : {len(registry)}")
    print(f"UI placements       : {len(placements)}")
    print(f"missing             : {len(missing)}")

    if missing:
        print("\nUnexposed parameters:")
        for p in missing:
            print(f"  - 0x{p['id']:04X} {p['key']}")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
