#!/usr/bin/env python3
"""
Update counter definitions from Intel perfmon repository.

This tool downloads Intel's performance monitoring event JSON files and
generates C++ counter definitions for CounterDefinitions.cpp.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any
from urllib.request import urlopen

# Base URL for Intel perfmon repository
PERFMON_BASE = "https://raw.githubusercontent.com/intel/perfmon/main"

# Architecture mapping: (family_enum, model_list, json_path)
ARCHITECTURES = [
    ("INTEL_BROADWELL", [0x3D, 0x47, 0x4F, 0x56], "BDW/events/broadwell_core.json"),
    ("INTEL_SKYLAKE", [0x4E, 0x5E, 0x55], "SKL/events/skylake_core.json"),
    ("INTEL_KABYLAKE", [0x8E, 0x9E, 0xA5, 0xA6], "SKL/events/skylake_core.json"),  # Kaby/Coffee/Comet use Skylake events
    ("INTEL_ICELAKE", [0x7D, 0x7E, 0x6A, 0x6C], "ICL/events/icelake_core.json"),
    ("INTEL_TIGERLAKE", [0x8C, 0x8D], "TGL/events/tigerlake_core.json"),
]

# Events we care about (event_name -> counter_name mapping)
INTERESTING_EVENTS = {
    "INST_RETIRED.ANY": "Instruct",
    "CPU_CLK_UNHALTED.THREAD": "Core cyc",
    "BR_MISP_RETIRED.ALL_BRANCHES": "BrMispred",
    "BR_INST_RETIRED.ALL_BRANCHES": "BrTaken",
    "BACLEARS.ANY": "BaClrAny",
    "INT_MISC.CLEAR_RESTEER_CYCLES": "ClrRestr",
    "INT_MISC.CLEARS_COUNT": "ClrCount",
}


def download_json(url: str) -> dict[str, Any]:
    """Download and parse JSON from URL."""
    print(f"Downloading {url}")
    with urlopen(url) as response:
        data = response.read()
    return json.loads(data)


def parse_event(event: dict[str, Any]) -> tuple[str, int, int] | None:
    """Parse event and return (name, event_code, umask) if interesting."""
    event_name = event.get("EventName", "")
    if event_name not in INTERESTING_EVENTS:
        return None

    try:
        event_code = int(event.get("EventCode", "0x0"), 16)
        umask = int(event.get("UMask", "0x0"), 16)
        return (event_name, event_code, umask)
    except (ValueError, TypeError):
        return None


def generate_counter_definitions(arch_name: str, events: list[tuple[str, int, int]]) -> list[str]:
    """Generate C++ counter definition lines for an architecture."""
    lines = []

    # Process events and sort: fixed counters first, then programmable
    sorted_events: list[tuple[int, str, int, int, int, int]] = []

    for event_name, event_code, umask in events:
        counter_name = INTERESTING_EVENTS[event_name]

        # Determine counter ID and register type
        if event_name == "INST_RETIRED.ANY":
            counter_id = 9
            counter_first = 0x40000000  # Fixed counter
            sort_order = 0
        elif event_name == "CPU_CLK_UNHALTED.THREAD":
            counter_id = 1
            counter_first = 0x40000002  # Fixed counter
            sort_order = 1
        elif "BR_MISP_RETIRED" in event_name:
            counter_id = 207
            counter_first = 0
            sort_order = 100
        elif "BR_INST_RETIRED" in event_name:
            counter_id = 201
            counter_first = 0
            sort_order = 101
        elif "BACLEARS" in event_name:
            counter_id = 410
            counter_first = 0
            sort_order = 102
        elif "CLEAR_RESTEER" in event_name:
            counter_id = 411
            counter_first = 0
            sort_order = 103
        elif "CLEARS_COUNT" in event_name:
            counter_id = 412
            counter_first = 0
            sort_order = 104
        else:
            continue

        sorted_events.append((sort_order, event_name, counter_id, counter_first, event_code, umask))

    # Sort and generate lines
    sorted_events.sort()
    for _, event_name, counter_id, counter_first, event_code, umask in sorted_events:
        counter_name = INTERESTING_EVENTS[event_name]

        # For fixed counters, event/umask are 0
        if counter_first >= 0x40000000:
            event_code = 0
            umask = 0
            counter_last = 0
            event_reg = 0
        else:
            counter_last = 3
            event_reg = 0

        # Format counter_first as hex if > 0
        if counter_first > 0:
            counter_first_str = f"0x{counter_first:x}"
        else:
            counter_first_str = "0"

        line = f"    {{{counter_id}, S_ID3, {arch_name}, {counter_first_str}, {counter_last}, {event_reg}, 0x{event_code:02x}, 0x{umask:02x}, \"{counter_name}\"}}, // {event_name}"
        lines.append(line)

    return lines


def update_counter_definitions_file(all_definitions: dict[str, list[str]], counter_file: Path) -> bool:
    """Update CounterDefinitions.cpp with new counter definitions."""
    if not counter_file.exists():
        print(f"Error: {counter_file} not found", file=sys.stderr)
        return False

    content = counter_file.read_text()

    # Find the section to replace (everything between the comment and Intel Atom)
    marker_start = "// Intel Broadwell, Skylake, Kaby/Coffee/Comet Lake, Ice Lake, Tiger Lake:"
    marker_end = "    // Intel Atom:"

    start_idx = content.find(marker_start)
    end_idx = content.find(marker_end)

    if start_idx == -1 or end_idx == -1:
        print(f"Error: Could not find markers in {counter_file}", file=sys.stderr)
        return False

    # Generate new section with per-architecture definitions
    new_lines = [
        "// Intel Broadwell, Skylake, Kaby/Coffee/Comet Lake, Ice Lake, Tiger Lake:",
        "    // These architectures share the same PMC architecture (S_ID3) and similar event encodings",
        "    // Based on Intel perfmon JSON data from https://github.com/intel/perfmon",
        "    // Auto-generated with: make update-counters",
        "    //  id   scheme  cpu         countregs eventreg event  mask   name",
    ]

    # Output all architectures' definitions
    for arch_name, lines in all_definitions.items():
        models = [m for name, m, _ in ARCHITECTURES if name == arch_name][0]
        new_lines.append(f"    // {arch_name} (Models: {', '.join(f'0x{m:02X}' for m in models)})")
        new_lines.extend(lines)

    new_section = "\n".join(new_lines) + "\n\n    "

    # Replace the section
    new_content = content[:start_idx] + new_section + content[end_idx:]

    # Write back
    counter_file.write_text(new_content)
    print(f"Updated {counter_file}")
    return True


def main() -> int:
    parser = argparse.ArgumentParser(description="Update counter definitions from Intel perfmon")
    parser.add_argument("--dry-run", action="store_true", help="Print output without modifying files")
    parser.add_argument("--arch", help="Only process specific architecture (e.g., INTEL_SKYLAKE)")
    args = parser.parse_args()

    all_definitions: dict[str, list[str]] = {}

    for arch_name, models, json_path in ARCHITECTURES:
        if args.arch and arch_name != args.arch:
            continue

        url = f"{PERFMON_BASE}/{json_path}"
        try:
            data = download_json(url)
        except Exception as e:
            print(f"Error downloading {arch_name}: {e}", file=sys.stderr)
            continue

        events: list[tuple[str, int, int]] = []
        for event in data.get("Events", []):
            parsed = parse_event(event)
            if parsed:
                events.append(parsed)

        if events:
            lines = generate_counter_definitions(arch_name, events)
            all_definitions[arch_name] = lines

    if not all_definitions:
        print("No counter definitions generated", file=sys.stderr)
        return 1

    if args.dry_run:
        # Just print the output
        for arch_name, lines in all_definitions.items():
            models = [m for name, m, _ in ARCHITECTURES if name == arch_name][0]
            print(f"\n// {arch_name} (Models: {', '.join(f'0x{m:02X}' for m in models)})")
            for line in lines:
                print(line)
    else:
        # Update the file
        counter_file = Path(__file__).parent.parent / "src" / "CounterDefinitions.cpp"
        if not update_counter_definitions_file(all_definitions, counter_file):
            return 1
        print("Counter definitions updated successfully!")
        print("Run 'git diff src/CounterDefinitions.cpp' to see changes")

    return 0


if __name__ == "__main__":
    sys.exit(main())
