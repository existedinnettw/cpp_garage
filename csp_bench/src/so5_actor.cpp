// #include "sobjectizer_test.h"
#include <functional>
#include <so_5/all.hpp>
#include <string>
#include <vector>

struct Nested_struct
{
  int a = 5;
  bool c = true;
};

struct ping
{
  int counter_;
  std::string command = "hello";
  Nested_struct n;
  /**
   * only const function(pure function) is allowed.
   * which maybe useless if you prefer same machine one.
   * But switch to mutable_mhood_t, now get mutable one.
   * @see so_5/mutable_msg_agents/main.cpp
   * need to modify all so_5::mutable_msg and mutable_mhood_t
   */
  void world_func()
  {
    n.c = false;
    std::cout << "world\n";
  };
};

struct pong
{
  int counter_;
};

class pinger final : public so_5::agent_t
{
  so_5::mbox_t ponger_;

  void on_pong(mutable_mhood_t<pong> cmd)
  {
    if (cmd->counter_ > 0) {
      //   std::cout << "on_pong\n";
      so_5::send<so_5::mutable_msg<ping>>(ponger_, cmd->counter_ - 1); // minus 1 until reach 0
    } else
      so_deregister_agent_coop_normally(); // unregist self
  }

public:
  pinger(context_t ctx)
    : so_5::agent_t{ std::move(ctx) }
  {
  }

  void set_ponger(const so_5::mbox_t mbox)
  {
    ponger_ = mbox;
  }

  void so_define_agent() override
  {
    so_subscribe_self().event(&pinger::on_pong);
  }

  void so_evt_start() override
  {
    // aware mutable message and immutable message are totally difference
    so_5::send<so_5::mutable_msg<ping>>(ponger_, 1000, "hello2"); // set counter_=1000, same as constructor
  }
};

class ponger final : public so_5::agent_t
{
  const so_5::mbox_t pinger_;
  int pings_received_{};

public:
  ponger(context_t ctx, so_5::mbox_t pinger)
    : so_5::agent_t{ std::move(ctx) }
    , pinger_{ std::move(pinger) }
  {
  }

  void so_define_agent() override
  {
    so_subscribe_self().event(
      // on_ping
      //   [this](mhood_t<ping> cmd) {
      [this](mutable_mhood_t<ping> cmd) {
        ++pings_received_;
        cmd->world_func();
        so_5::send<so_5::mutable_msg<pong>>(pinger_, cmd->counter_); // ponger send back to pinger
      });
  }

  void so_evt_finish() override
  {
    std::cout << "pings received: " << pings_received_ << std::endl;
  }
};

int
main()
{
  so_5::launch([](so_5::environment_t& env) {
    env.introduce_coop([](so_5::coop_t& coop) {
      auto pinger_actor = coop.make_agent<pinger>();
      auto ponger_actor = coop.make_agent<ponger>(pinger_actor->so_direct_mbox());

      pinger_actor->set_ponger(ponger_actor->so_direct_mbox());
    });
  });

  return 0;
}
