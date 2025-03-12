#include <iostream>

#include "logging.hpp"

void Net::Logging::LogInfo(const char* msg) { std::cerr << msg << '\n'; }

void Net::Logging::LogFatal(const char* msg)
{
    int err = errno;
    std::cerr << "[errno: " << err << "]: " << msg << '\n';
    abort();
}
