#include <iostream>

#include "logging.hpp"

void Caio::Logging::LogInfo(const char* msg) { std::cerr << msg << '\n'; }

void Caio::Logging::LogFatal(const char* msg)
{
    int err = errno;
    std::cerr << "[errno: " << err << "]: " << msg << '\n';
    abort();
}
