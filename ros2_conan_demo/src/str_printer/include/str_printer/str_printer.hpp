#pragma once

#include <string>

namespace str_printer {

/// Returns "<greeting>, <name>! (#<count>)" formatted with fmt.
std::string greet(const std::string& greeting, const std::string& name, int count);

/// The FMT_VERSION the library was compiled against, as "major.minor.patch".
std::string fmt_version();

}  // namespace str_printer
