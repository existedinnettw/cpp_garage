
#include <iostream>
#include <cstdint>
#include <cmath>
#include <tuple>

namespace static_poly
{
    template <class T>
    class Shape
    {
    public:
        double area(double len) const
        {
            return static_cast<const T *>(this)->area_impl(len);
        }
    };

    class Square : public Shape<Square>
    {
        friend Shape<Square>;

    protected:
        double area_impl(double len) const
        {
            return len * len;
        }
    };

    class Triangle : public Shape<Triangle>
    {
        friend Shape<Triangle>;

    protected:
        double area_impl(double len) const
        {
            return (len * sqrt(3.0) / 2.0) * len / 2.0;
        }
    };

    template <typename... Types>
    class Shapes
    {
    public:
        std::tuple<Types...> items;
        Shapes(std::tuple<Types...> items)
        {
            this->items = items;
        }
        void show_areas(double len)
        {
            std::apply(
                [&](auto &&...shape)
                {
                    ((std::cout << shape.area(len) << ", "), ...);
                    // std::cout << shape.area(len) << ", ";
                },
                items);
        }
        double tot_areas(double len)
        {
            return std::apply(
                [&](auto &&...shape)
                {
                    return (shape.area(len) + ...);
                    // std::cout << shape.area(len) << ", ";
                },
                items);
        }
    };
}
