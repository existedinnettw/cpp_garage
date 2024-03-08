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
#include <asio/thread_pool.hpp>
#include <asio/use_future.hpp>
#else
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/use_future.hpp>
namespace asio = boost::asio;
#endif

auto
sum_subtask(asiochan::read_channel<std::optional<int>> in, asiochan::write_channel<int> out) -> asio::awaitable<void>
{
  auto sum = 0;
  while (auto value = co_await in.read()) {
    sum += *value;
  }

  co_await out.write(sum);
}

auto
sum_task(std::span<int const> array, int num_tasks) -> asio::awaitable<int>
{
  auto executor = co_await asio::this_coro::executor;

  // Spawn N child routines, sharing the same in/out channels
  auto in = asiochan::channel<std::optional<int>>{};
  auto out = asiochan::channel<int>{};
  for ([[maybe_unused]] auto _ : std::views::iota(0, num_tasks)) {
    asio::co_spawn(executor, sum_subtask(in, out), asio::detached);
  }

  // Send the array to the child routines
  for (auto val : array) {
    co_await in.write(val);
  }

  for ([[maybe_unused]] auto _ : std::views::iota(0, num_tasks - 1)) {
    // Join a task
    co_await in.write(std::nullopt);
    // Retrieve its result
    auto subresult = co_await out.read();
    // Send it to another task
    co_await in.write(subresult);
  }

  // Join the last task
  co_await in.write(std::nullopt);
  // Retrieve the complete result
  co_return co_await out.read();
}

// std::future<int>
// auto
/**
 * @return
 * I don't know how to get future with void type, so change to return 0
 */
auto
pinger(asiochan::write_channel<std::optional<int>> ping_out_pong_in,
       asiochan::read_channel<std::optional<int>> ping_in_pong_out,
       benchmark::State& state) -> asio::awaitable<int>
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
ponger(asiochan::read_channel<std::optional<int>> ping_out_pong_in,
       asiochan::write_channel<std::optional<int>> ping_in_pong_out) -> asio::awaitable<void>
{
  while (auto value = co_await ping_out_pong_in.read()) {
    // chan.pop()
    auto pong_message = value;
    co_await ping_in_pong_out.write(pong_message);
  } // break if be posisoned

  co_await ping_in_pong_out.write(std::nullopt); // poison
  co_return;
}

// int
// main()
// {
//   auto tp = asio::thread_pool{};

//   auto numbers = std::vector<int>(10'000);
//   std::iota(numbers.begin(), numbers.end(), 1);

//   auto start = std::chrono::steady_clock::now();

//   auto task = asio::co_spawn(tp, sum_task(numbers, 100), asio::use_future);
//   auto result = task.get();

//   auto dur = std::chrono::steady_clock::now() - start;

//   std::cout << "The result is: " << result << "\n";
//   std::cout << "Test duration: " << std::chrono::duration<double>{ dur }.count() << "s\n";

//   return EXIT_SUCCESS;
// }
static void
bench_pingpong(benchmark::State& state)
{
  //   auto executor = co_await asio::this_coro::executor;
  auto tp = asio::thread_pool{};

  // Spawn N child routines, sharing the same in/out channels
  auto in = asiochan::channel<std::optional<int>>{};
  auto out = asiochan::channel<std::optional<int>>{};
  //   for (auto i : std::views::iota(0, num_tasks)) {
  //     asio::co_spawn(executor, sum_subtask(in, out), asio::detached);
  //   }
  //   asio::co_spawn(executor, pinger(in, out, state), asio::detached);
  //   auto task = asio::co_spawn(tp, pinger(in, out, state), asio::use_awaitable);
  auto task = asio::co_spawn(tp, pinger(in, out, state), asio::use_future);
  //   co_await asio::co_spawn(executor, ponger(in, out), asio::use_awaitable);
  asio::co_spawn(tp, ponger(in, out), asio::detached);
  auto result = task.get(); // use_future
  //   co_await task;
}
BENCHMARK(bench_pingpong);
BENCHMARK_MAIN();