#include <benchmark/benchmark.h>
#include <memory>
#include <random>
#include <so_5/all.hpp>

using namespace so_5;

struct ping
{
  int counter_;
  /**
   * only const function(pure function) is allowed.
   * which maybe useless
   */
  void world_func()
  {
    // std::cout << "world\n";
  };
};

struct pong
{
  int counter_ = 0;
  std::shared_ptr<int64_t> int_ptr;
  //   pong() = default;
  pong(int counter, std::shared_ptr<int64_t>& int_ptr)
    : counter_(counter)
    , int_ptr(int_ptr){};
};

void
pinger_proc(so_5::mchain_t self_ch, so_5::mchain_t ping_ch, benchmark::State& state)
{
  so_5::send<so_5::mutable_msg<ping>>(ping_ch, 1000);


  // Read all message until channel will be closed.
  so_5::receive(so_5::from(self_ch).handle_all(), [&](so_5::mutable_mhood_t<pong> cmd) {
    // if (cmd->counter_ > 0){
    if (state.KeepRunning()) {
      so_5::send<so_5::mutable_msg<ping>>(ping_ch, cmd->counter_ - 1);
      //   ++state;
    } else {
      // Channels have to be closed to break `receive` calls.
      so_5::close_drop_content(so_5::exceptions_enabled, self_ch);
      so_5::close_drop_content(so_5::exceptions_enabled, ping_ch);
    }
  });
}

void
ponger_proc(so_5::mchain_t self_ch, so_5::mchain_t pong_ch)
{
  int pings_received{};

  // Read all message until channel will be closed.
  so_5::receive(so_5::from(self_ch).handle_all(), [&](so_5::mutable_mhood_t<ping> cmd) {
    ++pings_received;
    cmd->world_func();
    auto new_int_ptr = std::make_shared<int64_t>(5);
    // auto new_pong = std::make_shared<pong>(cmd->counter_, new_int_ptr);
    auto new_pong = pong(cmd->counter_, new_int_ptr);
    // if want to direct pass in, need to be mhood_t<mutable_msg<second_msg>>
    so_5::send<so_5::mutable_msg<pong>>(pong_ch, new_pong);
  });

  std::cout << "pings received: " << pings_received << std::endl;
}

/**
 * @see [The Real Solution: Mutable
 * Messages](https://sourceforge.net/p/sobjectizer/wiki/so-5.5%20In-depth%20-%20Mutable%20Messages/) probably set
 * process in coroutine is possible
 */
static void
bench_pingpong(benchmark::State& state)
{
  so_5::wrapped_env_t sobj;

  //   std::cout << "range 0 val:" << state.range(0) << "\n";

  auto pinger_ch = so_5::create_mchain(sobj); // this is unlimited channel
  auto ponger_ch = so_5::create_mchain(sobj);
  //   auto pinger_ch =
  //     so_5::create_mchain(sobj,
  //                         so_5::infinite_wait,
  //                         1,
  //                         so_5::mchain_props::memory_usage_t::preallocated,
  //                         so_5::mchain_props::overflow_reaction_t::throw_exception); // this is unlimited channel
  //   auto ponger_ch = so_5::create_mchain(sobj,
  //                                        so_5::infinite_wait,
  //                                        1,
  //                                        so_5::mchain_props::memory_usage_t::preallocated,
  //                                        so_5::mchain_props::overflow_reaction_t::throw_exception);

  // As you seen, process run at seperated thread rather than coroutine.
  //   state.PauseTiming();
  std::thread pinger{ pinger_proc, pinger_ch, ponger_ch, std::ref(state) };
  std::thread ponger{ ponger_proc, ponger_ch, pinger_ch };

  ponger.join();
  pinger.join();

  //   state.ResumeTiming();

  //   so_5::launch([](so_5::environment_t& env) {
  // raw_receive_case(env);
  // prepared_receive_case(env);
  //   });

  //   so_5::mchain_props::overflow_reaction_t::drop_newest;
}
BENCHMARK(bench_pingpong);

BENCHMARK_MAIN();