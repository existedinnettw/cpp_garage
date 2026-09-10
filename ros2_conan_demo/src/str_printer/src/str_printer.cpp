#include "str_printer/str_printer.hpp"

#include <fmt/format.h>

namespace str_printer {

std::string greet(const std::string& greeting, const std::string& name, int count) {
  return fmt::format("{}, {}! (#{})", greeting, name, count);
}

std::string fmt_version() {
  return fmt::format("{}.{}.{}", FMT_VERSION / 10000, FMT_VERSION / 100 % 100, FMT_VERSION % 100);
}

}  // namespace str_printer
