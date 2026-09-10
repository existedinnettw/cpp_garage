// nanobind glue only: no logic lives here, everything comes from libgeom.
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include <fmt/format.h>

#include "geom/geom.hpp"
#include "geom/stats.hpp"

namespace nb = nanobind;
using namespace nb::literals;

namespace {

// One helper binds any Stats<T> instantiation. Called once per T below.
template <typename T>
void bind_stats(nb::module_& m, const char* py_name) {
    using S = geom::Stats<T>;
    nb::class_<S>(m, py_name)
        .def(nb::init<>())
        .def("push", &S::push, "value"_a)
        .def("extend", &S::extend, "values"_a)
        .def_prop_ro("count", &S::count)
        .def_prop_ro("sum", &S::sum)
        .def_prop_ro("min", &S::min)
        .def_prop_ro("max", &S::max)
        .def_prop_ro("mean", &S::mean)
        .def("__len__", &S::count)
        .def("__repr__", &S::describe);
}

}  // namespace

NB_MODULE(_core, m) {
    m.doc() = "nanobind bindings for the geom C++ library (built via Conan).";
    m.attr("fmt_version") = FMT_VERSION;

    // --- plain struct ------------------------------------------------------
    nb::class_<geom::Point>(m, "Point")
        .def(nb::init<>())
        .def(nb::init<double, double>(), "x"_a, "y"_a)
        .def_rw("x", &geom::Point::x)
        .def_rw("y", &geom::Point::y)
        .def("__repr__", &geom::to_string)
        .def("__eq__", [](const geom::Point& a, const geom::Point& b) {
            return a.x == b.x && a.y == b.y;
        });

    m.def("distance", &geom::distance, "a"_a, "b"_a,
          "Euclidean distance between two points.");

    // --- class with methods / properties / Python protocols -----------------
    nb::class_<geom::Polygon>(m, "Polygon")
        .def(nb::init<>())
        .def(nb::init<std::vector<geom::Point>>(), "vertices"_a)
        .def("add_vertex", &geom::Polygon::add_vertex, "p"_a)
        .def("area", &geom::Polygon::area)
        .def("perimeter", &geom::Polygon::perimeter)
        .def("centroid", &geom::Polygon::centroid)
        .def_prop_ro("vertices", &geom::Polygon::vertices)
        .def("__len__", &geom::Polygon::size)
        // std::out_of_range -> IndexError automatically, which also makes
        // `for p in poly` work via the legacy iteration protocol.
        .def("__getitem__", [](const geom::Polygon& s, std::size_t i) { return s.at(i); })
        .def("__repr__", &geom::Polygon::describe);

    // --- class template: one Python class per instantiation ------------------
    bind_stats<double>(m, "StatsF64");
    bind_stats<long>(m, "StatsI64");

    // --- function template: instantiations exposed as Python overloads -------
    // nanobind tries them in order; int-only lists pick the long version,
    // anything containing a float falls through to the double version.
    m.def("clamp_all", &geom::clamp_all<long>, "values"_a, "lo"_a, "hi"_a);
    m.def("clamp_all", &geom::clamp_all<double>, "values"_a, "lo"_a, "hi"_a,
          "Clamp every element into [lo, hi]. Integer or float overload.");
}
