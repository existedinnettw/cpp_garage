#pragma once

#include <vector>
#include <string>


#ifdef _WIN32
  #define PAHOMQTT_PINGPONG_EXPORT __declspec(dllexport)
#else
  #define PAHOMQTT_PINGPONG_EXPORT
#endif

PAHOMQTT_PINGPONG_EXPORT void pahomqtt_pingpong();
PAHOMQTT_PINGPONG_EXPORT void pahomqtt_pingpong_print_vector(const std::vector<std::string> &strings);
