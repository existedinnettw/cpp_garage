"""Python API on top of the nanobind extension `_core` (which wraps libgeom)."""

from ._core import (
    Point,
    Polygon,
    StatsF64,
    StatsI64,
    clamp_all,
    distance,
    fmt_version,
)

__all__ = [
    "Point",
    "Polygon",
    "StatsF64",
    "StatsI64",
    "clamp_all",
    "distance",
    "fmt_version",
    "stats_of",
]


def stats_of(values):
    """Pick the right C++ template instantiation from the Python element types."""
    s = StatsI64() if all(isinstance(v, int) for v in values) else StatsF64()
    s.extend(values)
    return s
