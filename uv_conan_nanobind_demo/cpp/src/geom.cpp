#include "geom/geom.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>

#include <fmt/format.h>

namespace geom {

double distance(const Point& a, const Point& b) {
    return std::hypot(a.x - b.x, a.y - b.y);
}

std::string to_string(const Point& p) {
    return fmt::format("Point({}, {})", p.x, p.y);
}

Polygon::Polygon(std::vector<Point> vertices) : vertices_(std::move(vertices)) {}

void Polygon::add_vertex(const Point& p) { vertices_.push_back(p); }

const Point& Polygon::at(std::size_t i) const {
    if (i >= vertices_.size())
        throw std::out_of_range(fmt::format("index {} out of range (size {})", i, vertices_.size()));
    return vertices_[i];
}

double Polygon::area() const {
    const std::size_t n = vertices_.size();
    if (n < 3) return 0.0;
    double acc = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        const Point& p = vertices_[i];
        const Point& q = vertices_[(i + 1) % n];
        acc += p.x * q.y - q.x * p.y;
    }
    return std::abs(acc) / 2.0;
}

double Polygon::perimeter() const {
    const std::size_t n = vertices_.size();
    if (n < 2) return 0.0;
    double acc = 0.0;
    for (std::size_t i = 0; i < n; ++i) acc += distance(vertices_[i], vertices_[(i + 1) % n]);
    return acc;
}

Point Polygon::centroid() const {
    if (vertices_.empty()) throw std::logic_error("centroid of empty polygon");
    Point c;
    for (const Point& p : vertices_) { c.x += p.x; c.y += p.y; }
    c.x /= static_cast<double>(vertices_.size());
    c.y /= static_cast<double>(vertices_.size());
    return c;
}

std::string Polygon::describe() const {
    return fmt::format("Polygon(n={}, area={:.3f}, perimeter={:.3f})", size(), area(), perimeter());
}

}  // namespace geom
