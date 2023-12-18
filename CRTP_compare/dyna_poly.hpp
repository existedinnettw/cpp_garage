
#include <iostream>
#include <cstdint>
#include <cmath>
#include <tuple>
#include <array>
#include <vector>

namespace dyna_poly
{
    class Shape
    {
    public:
        virtual double area(double len) = 0;
    };

    class Square : public Shape
    {
    public:
        double area(double len) override
        {
            return len * len;
        }
    };

    class Triangle : public Shape
    {
    public:
        double area(double len) override
        {
            return (len * sqrt(3.0) / 2.0) * len / 2.0;
        }
    };

    class Shapes
    {
    public:
        std::array<Shape *, 2> items;
        // std::vector<Shape *> items = std::vector<Shape *>(2);
        Shapes(Shape *s1, Shape *s2)
        {
            items[0] = s1;
            items[1] = s2;
        }
        void show_areas(double len)
        {
            for (int i = 0; i < 2; i++)
            {
                std::cout << items[i]->area(len) << ",";
            }
        }
        double tot_areas(double len)
        {
            double rst = 0;
            for (int i = 0; i < 2; i++)
            {
                rst += items[i]->area(len);
            }
            return rst;
        }
    };
}
