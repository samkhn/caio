#include <iostream>

#include "logging.hpp"

void log_info(const char *msg) { std::cerr << msg << '\n'; }

void log_fatal(const char *msg) {
  int err = errno;
  std::cerr << "[errno: " << err << "]: " << msg << '\n';
  abort();
}
