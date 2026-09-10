import math

import pytest

import uv_conan_nanobind_demo as g


def test_point_repr_uses_fmt():
    assert repr(g.Point(1.5, -2)) == "Point(1.5, -2)"


def test_distance():
    assert g.distance(g.Point(0, 0), g.Point(3, 4)) == 5.0


def test_fmt_linked():
    assert g.fmt_version >= 120000


# --- Polygon class ----------------------------------------------------------

def unit_square():
    return g.Polygon([g.Point(0, 0), g.Point(1, 0), g.Point(1, 1), g.Point(0, 1)])


def test_polygon_methods():
    sq = unit_square()
    assert len(sq) == 4
    assert sq.area() == 1.0
    assert math.isclose(sq.perimeter(), 4.0)
    assert sq.centroid() == g.Point(0.5, 0.5)
    assert repr(sq) == "Polygon(n=4, area=1.000, perimeter=4.000)"


def test_polygon_mutation_and_iteration():
    poly = g.Polygon()
    for p in unit_square():  # __getitem__ + IndexError drives iteration
        poly.add_vertex(p)
    assert [p.x for p in poly] == [0, 1, 1, 0]
    assert poly[3] == g.Point(0, 1)
    with pytest.raises(IndexError):
        poly[4]


def test_polygon_exceptions_translate():
    with pytest.raises(RuntimeError, match="empty polygon"):
        g.Polygon().centroid()


# --- Stats<T> class template ------------------------------------------------

def test_stats_double_instantiation():
    s = g.StatsF64()
    s.extend([1.0, 2.5, 4.0])
    assert (s.count, s.min, s.max) == (3, 1.0, 4.0)
    assert math.isclose(s.mean, 2.5)
    assert repr(s) == "Stats<double>(n=3, min=1, max=4, mean=2.500)"


def test_stats_int_instantiation_keeps_integer_sum():
    s = g.StatsI64()
    for v in (10, 20, 31):
        s.push(v)
    assert s.sum == 61 and isinstance(s.sum, int)
    assert isinstance(s.mean, float)
    with pytest.raises(TypeError):
        s.push(1.5)  # int instantiation rejects floats


def test_stats_empty_raises():
    with pytest.raises(RuntimeError, match="empty"):
        g.StatsF64().min


def test_stats_of_dispatches_by_type():
    assert isinstance(g.stats_of([1, 2, 3]), g.StatsI64)
    assert isinstance(g.stats_of([1, 2.0]), g.StatsF64)


# --- clamp_all<T> function template -> Python overloads ----------------------

def test_clamp_all_overloads():
    ints = g.clamp_all([-5, 3, 99], 0, 10)
    assert ints == [0, 3, 10] and all(isinstance(v, int) for v in ints)
    floats = g.clamp_all([-0.5, 0.25, 2.0], 0.0, 1.0)
    assert floats == [0.0, 0.25, 1.0]
    with pytest.raises(ValueError, match="lo"):
        g.clamp_all([1.0], 2.0, 1.0)
