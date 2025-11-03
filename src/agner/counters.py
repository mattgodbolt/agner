"""Performance counter management and validation."""

from __future__ import annotations

import csv
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from collections.abc import Sequence


@dataclass(frozen=True)
class CounterInfo:
    """Information about a PMC counter."""

    counter_id: int
    name: str
    supported: bool
    scheme: int
    family: int


class CounterDB:
    """Database of available performance counters for this CPU."""

    def __init__(self) -> None:
        """Initialize by querying the list-counters tool."""
        self._counters_by_id: dict[int, list[CounterInfo]] = {}
        self._counters_by_name: dict[str, list[CounterInfo]] = {}
        self._load_counters()

    def _load_counters(self) -> None:
        """Load counter definitions from the C++ tool."""
        src_dir = Path(__file__).parent.parent  # src/agner/ -> src/
        list_counters = src_dir / "out" / "list-counters"

        if not list_counters.exists():
            # Build it
            subprocess.check_call(["make", "out/list-counters"], cwd=str(src_dir), stdout=subprocess.DEVNULL)

        result = subprocess.check_output([str(list_counters)], text=True, stderr=subprocess.DEVNULL)

        reader = csv.DictReader(result.splitlines())
        for row in reader:
            counter = CounterInfo(
                counter_id=int(row["counter_id"]),
                name=row["name"],
                supported=bool(int(row["supported"])),
                scheme=int(row["scheme"], 16),
                family=int(row["family"], 16),
            )

            self._counters_by_id.setdefault(counter.counter_id, []).append(counter)
            self._counters_by_name.setdefault(counter.name, []).append(counter)

    def get_counter(self, id_or_name: int | str) -> CounterInfo | None:
        """Get a supported counter by ID or name.

        Args:
            id_or_name: Counter ID (int) or name (str)

        Returns:
            CounterInfo if the counter exists and is supported, None otherwise
        """
        if isinstance(id_or_name, int):
            counters = self._counters_by_id.get(id_or_name, [])
        else:
            counters = self._counters_by_name.get(id_or_name, [])

        # Return the first supported counter, or None if none are supported
        for counter in counters:
            if counter.supported:
                return counter
        return None

    def is_supported(self, id_or_name: int | str) -> bool:
        """Check if a counter is supported on this CPU.

        Args:
            id_or_name: Counter ID (int) or name (str)

        Returns:
            True if supported, False otherwise
        """
        return self.get_counter(id_or_name) is not None

    def get_all_counters(self, id_or_name: int | str) -> list[CounterInfo]:
        """Get all counter variants (supported and unsupported) by ID or name.

        Args:
            id_or_name: Counter ID (int) or name (str)

        Returns:
            List of all CounterInfo objects for this ID/name
        """
        if isinstance(id_or_name, int):
            return self._counters_by_id.get(id_or_name, [])
        else:
            return self._counters_by_name.get(id_or_name, [])

    def validate_counters(self, counters: Sequence[int | str]) -> tuple[list[int], list[str]]:
        """Validate a list of counter IDs or names.

        Args:
            counters: List of counter IDs (int) or names (str)

        Returns:
            Tuple of (valid_ids, error_messages)
            - valid_ids: List of counter IDs that are supported
            - error_messages: List of human-readable error messages for unsupported counters
        """
        valid_ids: list[int] = []
        errors: list[str] = []

        for counter in counters:
            info = self.get_counter(counter)
            if info:
                valid_ids.append(info.counter_id)
            else:
                # Counter not supported - provide helpful error message
                all_variants = self.get_all_counters(counter)
                if not all_variants:
                    if isinstance(counter, int):
                        errors.append(f"Counter ID {counter} does not exist")
                    else:
                        errors.append(f"Counter '{counter}' does not exist")
                else:
                    # Counter exists but isn't supported on this CPU
                    variant = all_variants[0]
                    if isinstance(counter, int):
                        errors.append(
                            f"Counter ID {counter} ('{variant.name}') is not supported on this CPU "
                            f"(requires scheme=0x{variant.scheme:x}, family=0x{variant.family:x})"
                        )
                    else:
                        errors.append(
                            f"Counter '{counter}' (ID {variant.counter_id}) is not supported on this CPU "
                            f"(requires scheme=0x{variant.scheme:x}, family=0x{variant.family:x})"
                        )

        return valid_ids, errors

    def list_supported_counters(self) -> list[CounterInfo]:
        """Get a list of all supported counters on this CPU.

        Returns:
            List of CounterInfo objects for supported counters
        """
        supported = []
        seen_ids = set()

        for counters in self._counters_by_id.values():
            for counter in counters:
                if counter.supported and counter.counter_id not in seen_ids:
                    supported.append(counter)
                    seen_ids.add(counter.counter_id)

        return sorted(supported, key=lambda c: c.counter_id)


# Global instance
_counter_db: CounterDB | None = None


def get_counter_db() -> CounterDB:
    """Get the global CounterDB instance."""
    global _counter_db
    if _counter_db is None:
        _counter_db = CounterDB()
    return _counter_db
