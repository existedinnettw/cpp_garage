#include "copper.h"
#include <benchmark/benchmark.h>
#include <future>
#include <iostream>


template<typename CH>
void
pinger(CH& channel_1, CH& channel_2, benchmark::State& state)
{
  (void)channel_1.push(1);
  for (auto pong_message : channel_2) {
    // chan.pop()
    if (state.KeepRunning() && !channel_1.is_read_closed()) {
      (void)channel_1.push(pong_message + 1);
      //   std::cout << pong_message << "\n";
    } else {
      std::cout << "done\n";
      break;
    }
  }
  channel_1.close();
  //   std::cout << "pinger fin\n";
}

// auto setting channel?

template<typename CH>
void
ponger(CH& channel_1, CH& channel_2)
{
  for (auto ping_message : channel_1) {
    // chan.pop()
    auto pong_message = ping_message;
    (void)channel_2.push(pong_message);
  }
  channel_2.close();
  //   std::cout << "ponger fin\n";
}

// int main() {
static void
bench_copper_pingpong(benchmark::State& state)
{
  copper::unbuffered_channel<int> channel_1;
  copper::unbuffered_channel<int> channel_2;
  const auto p1 = std::async(pinger<decltype(channel_1)>, std::ref(channel_1), std::ref(channel_2), std::ref(state));
  const auto p2 = std::async(ponger<decltype(channel_1)>, std::ref(channel_1), std::ref(channel_2));
  return;
}
BENCHMARK(bench_copper_pingpong);
BENCHMARK_MAIN();