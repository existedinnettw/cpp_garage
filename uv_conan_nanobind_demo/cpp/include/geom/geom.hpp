#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace geom {

struct Point {
    double x{0.0};
    double y{0.0};
};

/// Euclidean distance between two points.
double distance(const Point& a, const Point& b);

/// Human-readable form, e.g. "Point(1.5, -2)". Uses fmt.
std::string to_string(const Point& p);

/// A class with state, invariants and methods (not just a data bag).
class Polygon {
public:
    Polygon() = default;
    explicit Polygon(std::vector<Point> vertices);

    void add_vertex(const Point& p);
    std::size_t size() const noexcept { return vertices_.size(); }
    const Point& at(std::size_t i) const;           // throws std::out_of_range
    const std::vector<Point>& vertices() const noexcept { return vertices_; }

    double area() const;        // shoelace formula, 0 if < 3 vertices
    double perimeter() const;
    Point centroid() const;     // throws std::logic_error if empty

    std::string describe() const;  // uses fmt

private:
    std::vector<Point> vertices_;
};

}  // namespace geom
