#include "mqtt/client.h"
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

////////////////////////////////subscribe/////////////////////////////////////////////

// Message table function signature
using handler_t = std::function<bool(const mqtt::message&)>;

// Handler for data messages (i.e. topic "data/#")
bool
data_handler(const mqtt::message& msg)
{
  cout << msg.get_topic() << ": " << msg.to_string() << endl;
  return true;
}

// Handler for command messages (i.e. topic "command")
// Return false to exit the application
bool
command_handler(const mqtt::message& msg)
{
  if (msg.to_string() == "exit") {
    cout << "Exit command received" << endl;
    return false;
  }
  return true;
}


////////////////////////////////////publish/////////////////////////////////////////
/**
 * fake persistance
 */
class sample_mem_persistence : virtual public mqtt::iclient_persistence
{
  // Whether the store is open
  bool open_;

  // Use an STL map to store shared persistence pointers
  // against string keys.
  std::map<std::string, std::string> store_;

public:
  sample_mem_persistence()
    : open_(false)
  {
  }

  // "Open" the store
  void open(const std::string& clientId, const std::string& serverURI) override
  {
    std::cout << "[Opening persistence store for '" << clientId << "' at '" << serverURI << "']" << std::endl;
    open_ = true;
  }

  // Close the persistent store that was previously opened.
  void close() override
  {
    std::cout << "[Closing persistence store.]" << std::endl;
    open_ = false;
  }

  // Clears persistence, so that it no longer contains any persisted data.
  void clear() override
  {
    std::cout << "[Clearing persistence store.]" << std::endl;
    store_.clear();
  }

  // Returns whether or not data is persisted using the specified key.
  bool contains_key(const std::string& key) override
  {
    return store_.find(key) != store_.end();
  }

  // Returns the keys in this persistent data store.
  mqtt::string_collection keys() const override
  {
    mqtt::string_collection ks;
    for (const auto& k : store_)
      ks.push_back(k.first);
    return ks;
  }

  // Puts the specified data into the persistent store.
  void put(const std::string& key, const std::vector<mqtt::string_view>& bufs) override
  {
    std::cout << "[Persisting data with key '" << key << "']" << std::endl;
    std::string str;
    for (const auto& b : bufs)
      str.append(b.data(), b.size()); // += b.str();
    store_[key] = std::move(str);
  }

  // Gets the specified data out of the persistent store.
  std::string get(const std::string& key) const override
  {
    std::cout << "[Searching persistence for key '" << key << "']" << std::endl;
    auto p = store_.find(key);
    if (p == store_.end())
      throw mqtt::persistence_exception();
    std::cout << "[Found persistence data for key '" << key << "']" << std::endl;

    return p->second;
  }

  // Remove the data for the specified key.
  void remove(const std::string& key) override
  {
    std::cout << "[Persistence removing key '" << key << "']" << std::endl;
    auto p = store_.find(key);
    if (p == store_.end())
      throw mqtt::persistence_exception();
    store_.erase(p);
    std::cout << "[Persistence key removed '" << key << "']" << std::endl;
  }
};

/////////////////////////////////////////////////////////////////////////////
// Class to receive callbacks

class user_callback : public virtual mqtt::callback
{
  void connection_lost(const std::string& cause) override
  {
    std::cout << "\nConnection lost" << std::endl;
    if (!cause.empty())
      std::cout << "\tcause: " << cause << std::endl;
  }

  void delivery_complete(mqtt::delivery_token_ptr tok) override
  {
    std::cout << "\n\t[Delivery complete for token: " << (tok ? tok->get_message_id() : -1) << "]" << std::endl;
  }

public:
};
/////////////////////////////////////////////////////////////////////////////
