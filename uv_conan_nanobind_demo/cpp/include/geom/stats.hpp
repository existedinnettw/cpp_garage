#pragma once
// Header-only templates. Python cannot see a template itself, only concrete
// instantiations, so bindings/module.cpp binds Stats<double>, Stats<long> etc.

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include <fmt/format.h>

namespace geom {

/// Running statistics over values of arithmetic type T.
template <typename T>
class Stats {
    static_assert(std::is_arithmetic_v<T>, "Stats<T> requires an arithmetic T");

public:
    void push(T v) {
        if (n_ == 0) {
            min_ = max_ = v;
        } else {
            min_ = std::min(min_, v);
            max_ = std::max(max_, v);
        }
        sum_ += v;
        ++n_;
    }

    void extend(const std::vector<T>& vs) {
        for (T v : vs) push(v);
    }

    std::size_t count() const noexcept { return n_; }
    T sum() const noexcept { return sum_; }
    T min() const { require_nonempty(); return min_; }
    T max() const { require_nonempty(); return max_; }

    /// Mean is always floating point, even for integer T.
    double mean() const { require_nonempty(); return static_cast<double>(sum_) / n_; }

    std::string describe() const {
        if (n_ == 0) return fmt::format("Stats<{}>(empty)", type_name());
        return fmt::format("Stats<{}>(n={}, min={}, max={}, mean={:.3f})",
                           type_name(), n_, min_, max_, mean());
    }

    static constexpr const char* type_name() {
        if constexpr (std::is_same_v<T, double>) return "double";
        else if constexpr (std::is_same_v<T, float>) return "float";
        else if constexpr (std::is_integral_v<T>) return "int";
        else return "T";
    }

private:
    void require_nonempty() const {
        if (n_ == 0) throw std::logic_error("Stats is empty");
    }

    std::size_t n_{0};
    T sum_{};
    T min_{std::numeric_limits<T>::max()};
    T max_{std::numeric_limits<T>::lowest()};
};

/// Function template: clamp every element into [lo, hi].
template <typename T>
std::vector<T> clamp_all(std::vector<T> values, T lo, T hi) {
    if (lo > hi) throw std::invalid_argument(fmt::format("lo ({}) > hi ({})", lo, hi));
    for (T& v : values) v = std::clamp(v, lo, hi);
    return values;
}

}  // namespace geom
