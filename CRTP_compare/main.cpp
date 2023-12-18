
#include <iostream>
#include "dyna_poly.hpp"
#include "static_poly.hpp"
#include <benchmark/benchmark.h>
#include <random>

static void bench_dyna_poly(benchmark::State &state)
{
    using namespace dyna_poly;
    using namespace std;
    random_device rd;
    mt19937 gen(rd());

    // doesn't count in initialization time.
    Square square;
    Triangle triangle;
    // cout<<square.area(2)<<", "<<triangle.area(2)<<"\n";
    Shapes shapes(&square, &triangle);
    for (auto _ : state)
    {
        // shapes.show_areas(2);
        // cout<<"\n";
        shapes.tot_areas(gen());
    }
}
BENCHMARK(bench_dyna_poly);

static void bench_static_poly(benchmark::State &state)
{
    using namespace static_poly;
    using namespace std;
    random_device rd;
    mt19937 gen(rd());
    Square square;
    Triangle triangle;
    // cout<<square.area(2)<<", "<<triangle.area(2)<<"\n";
    Shapes<Square, Triangle> shapes(make_tuple(square, triangle));
    for (auto _ : state)
    {
        // shapes.show_areas(2);
        // cout<<"\n";
        shapes.tot_areas(gen());
    }
}
BENCHMARK(bench_static_poly);

BENCHMARK_MAIN();