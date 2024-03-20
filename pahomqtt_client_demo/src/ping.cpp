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
  mqtt::client client(BROKER_ADDRESS, PING_CLIENT_ID, &persist); // mqtt::create_options(MQTTVERSION_5)
  //   mqtt::async_client cli_async(BROKER_ADDRESS, PING_CLIENT_ID);
  //   mqtt::client client(BROKER_ADDRESS, PING_CLIENT_ID);

  user_callback cb;
  client.set_callback(cb);

  mqtt::connect_options connOpts;
  connOpts.set_keep_alive_interval(20);
  connOpts.set_clean_session(true);
  std::cout << "...OK" << std::endl;

  // this test is try to known the raw stability of mqtt
  // auto connOpts = mqtt::connect_options_builder()
  //                    .user_name("user")
  //                    .password("passwd")
  //                    .keep_alive_interval(seconds(30))
  //                    .automatic_reconnect(seconds(2), seconds(30))
  //                    .clean_session(false)
  //                    .finalize();
  // You can install a callback to change some connection data
  // on auto reconnect attempts. To make a change, update the
  // `connect_data` and return 'true'.
  // client.set_update_connection_handler([](mqtt::connect_data& connData) {
  //   string newUserName{ "newuser" };
  //   if (connData.get_user_name() == newUserName)
  //     return false;

  //   cout << "Previous user: '" << connData.get_user_name() << "'" << endl;
  //   connData.set_user_name(newUserName);
  //   cout << "New user name: '" << connData.get_user_name() << "'" << endl;
  //   return true;
  // });

  try {
    std::cout << "\nConnecting..." << std::endl;
    mqtt::connect_response rsp = client.connect(connOpts);
    std::cout << "...OK" << std::endl;

    // subscribe
    if (!rsp.is_session_present()) {
      std::cout << "Subscribing to topics..." << std::flush;
      client.subscribe(PONG_TOPIC, QOS);
      std::cout << "OK" << std::endl;
    } else {
      cout << "Session already present. Skipping subscribe." << std::endl;
    }

    // actual publish
    uint64_t ping_num = 0;
    uint64_t pong_num = 0;
    auto now = std::chrono::system_clock::now();
    while (ping_num < 1e9) {
      // publish message
      now = std::chrono::system_clock::now();
      std::cout << "\nSending ping message:" << std::to_string(ping_num) << std::endl;
      client.publish(mqtt::make_message(PING_TOPIC, std::to_string(ping_num), QOS, false));
      std::cout << "...OK" << std::endl;

      // Consume messages
      // there is no seperated thread, after ping, it always wait until response
      // while (true) {
      auto msg = client.consume_message(); // this is blocking function
      if (msg) {
        if (msg->get_topic() == PONG_TOPIC) {
          // cout << "Exit command received" << endl;
          pong_num = stoul(msg->to_string());
          cout << msg->get_topic() << ": " << pong_num << endl;
          if (pong_num == ping_num) {
            ping_num += 1;
          } else {
            std::cout << "unexpeced pong value, bug exist\n";
            break;
          }
        }
        // cout << msg->get_topic() << ": " << msg->to_string() << endl;
        // break;
      } else if (!client.is_connected()) {
        cout << "Lost connection" << endl;
        // while (!client.is_connected()) {
        // 	this_thread::sleep_for(milliseconds(250));
        // }
        // cout << "Re-established connection" << endl;
        break;
      }
      // } // end of subscribe consume while

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