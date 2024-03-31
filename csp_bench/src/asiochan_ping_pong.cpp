#include <asiochan/asiochan.hpp>
#include <benchmark/benchmark.h>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <optional>
#include <ranges>
#include <span>
#include <vector>

#ifdef ASIOCHAN_USE_STANDALONE_ASIO
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>
#include <asio/thread_pool.hpp>
#include <asio/use_future.hpp>
#else
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/use_future.hpp>
namespace asio = boost::asio;
#endif

// std::future<int>
// auto
/**
 * @return
 * I don't know how to get future with void type, so change to return 0
 */
auto
pinger(auto ping_out_pong_in, auto ping_in_pong_out, benchmark::State& state) -> asio::awaitable<int>
{
  co_await ping_out_pong_in.write(1);
  while (auto value = co_await ping_in_pong_out.read()) {
    if (state.KeepRunning()) {
      co_await ping_out_pong_in.write(value.value() + 1);
    } else {
      std::cout << "done\n";
      break;
    }
  }
  co_await ping_out_pong_in.write(std::nullopt); // poison
  co_return 0;
}
auto
ponger(auto ping_out_pong_in, auto ping_in_pong_out) -> asio::awaitable<void>
{
  while (auto value = co_await ping_out_pong_in.read()) {
    // chan.pop()
    auto pong_message = value;
    co_await ping_in_pong_out.write(pong_message);
  } // break if be posisoned

  co_await ping_in_pong_out.write(std::nullopt); // poison
  co_return;
}

/**
 * The result is about 900ns each ping pong --> 450ns each switch.
 * Other show asio is about 100ns context switch.
 * I am not sure if switch time will lower, if coroutine increase.
 */
static void
bench_pingpong(benchmark::State& state)
{
  //   auto executor = co_await asio::this_coro::executor;
  // Spawn N child routines, sharing the same in/out channels
  auto in = asiochan::channel<std::optional<int>, 0>{}; // queue size lower, better
  auto out = asiochan::channel<std::optional<int>, 0>{};

  auto tp = asio::thread_pool{}; //higher , worse
  // asio::io_context tp;
  auto task = asio::co_spawn(tp, pinger(in, out, state), asio::use_future);
  //   co_await asio::co_spawn(executor, ponger(in, out), asio::use_awaitable);
  asio::co_spawn(tp, ponger(in, out), asio::detached); // detached, use_future...
  // tp.run();
  [[maybe_unused]] auto result = task.get(); // use_future
  //   co_await task;
}
BENCHMARK(bench_pingpong);
BENCHMARK_MAIN();