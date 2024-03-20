cmake_minimum_required(VERSION 3.15)
project(pahomqtt_pingpong C CXX)

# set(PAHO_WITH_SSL ON)

find_package(PahoMqttCpp REQUIRED CONFIG)
# find_package(eclipse-paho-mqtt-c REQUIRED CONFIG) #need?

set(SSL_EXECUTABLES
  ssl_publish
)

add_executable(pahomqtt_pingpong src/pahomqtt_pingpong.cpp src/main.cpp)

if(PAHO_MQTT_CPP_SHARED)
  target_link_libraries(pahomqtt_pingpong PahoMqttCpp::paho-mqttpp3)
else()
  target_link_libraries(pahomqtt_pingpong PahoMqttCpp::paho-mqttpp3-static)
endif()
# PahoMqttCpp::paho-mqttpp3-static
# if(PAHO_WITH_SSL)
#   target_compile_definitions(pahomqtt_pingpong PUBLIC OPENSSL)
# endif()

install(TARGETS pahomqtt_pingpong DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        )
