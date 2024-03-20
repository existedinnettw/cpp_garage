// ping
// from sync_publish.cpp
// sync_consume_v5.cpp

#include "mqtt/client.h"
#include "util.hpp"
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono;
using namespace std;

// --------------------------------------------------------------------------

int
main(int argc, char* argv[])
{
  init_config();
  std::cout << "BROKER_ADDRESS:" << BROKER_ADDRESS << "\n";
  std::cout << "TEST_NAME:" << TEST_NAME << "\n";

  std::cout << "Initialzing..." << std::endl;
  sample_mem_persistence persist;
  mqtt::client client(BROKER_ADDRESS, PONG_CLIENT_ID, &persist);
  user_callback cb;
  client.set_callback(cb);

  mqtt::connect_options connOpts;
  connOpts.set_keep_alive_interval(20);
  connOpts.set_clean_session(true);
  std::cout << "...OK" << std::endl;

  try {
    std::cout << "\nConnecting..." << std::endl;
    mqtt::connect_response rsp = client.connect(connOpts);
    std::cout << "...OK" << std::endl;

    // subscribe
    if (!rsp.is_session_present()) {
      std::cout << "Subscribing to topics..." << std::flush;
      client.subscribe(PING_TOPIC, QOS);
      std::cout << "OK" << std::endl;
    } else {
      cout << "Session already present. Skipping subscribe." << std::endl;
    }

    // actual publish
    uint64_t ping_num = 0;
    uint64_t pong_num = 0;
    auto now = std::chrono::system_clock::now();
    while (true) {
      // publish message
      now = std::chrono::system_clock::now();

      auto msg = client.consume_message(); // this is blocking function
      if (msg) {
        if (msg->get_topic() == PING_TOPIC) {
          // cout << "Exit command received" << endl;
          ping_num = stoul(msg->to_string());
          cout << msg->get_topic() << ": " << ping_num << endl;
          pong_num = ping_num;

          std::cout << "\nSending pong message:" << std::to_string(pong_num) << std::endl;
          client.publish(mqtt::make_message(PONG_TOPIC, std::to_string(pong_num), QOS, false));
          std::cout << "...OK" << std::endl;
        }

      } else if (!client.is_connected()) {
        cout << "Lost connection" << endl;
        break;
      }

      std::this_thread::sleep_until(now + chrono::milliseconds(500));
    } // end of ping

    // Disconnect
    std::cout << "\nDisconnecting..." << std::endl;
    client.disconnect();
    std::cout << "...OK" << std::endl;
  } catch (const mqtt::persistence_exception& exc) {
    std::cerr << "Persistence Error: " << exc.what() << " [" << exc.get_reason_code() << "]" << std::endl;
    return 1;
  } catch (const mqtt::exception& exc) {
    std::cerr << exc.what() << std::endl;
    return 1;
  }

  std::cout << "\nExiting" << std::endl;
  return 0;
}