cmake_minimum_required(VERSION 3.6)

project(caio
  DESCRIPTION "C++ library for async I/O ([e]poll, iouring)"
  LANGUAGES CXX)

if (MSVC)
  add_compile_options(/W4 /w14640)
else()
  add_compile_options(-Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -g)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(CLANG_TIDY_PATH NAMES clang-tidy)
if(NOT CLANG_TIDY_PATH)
  message(FATAL_ERROR "Could not find the program clang-tidy")
endif()
set(CMAKE_CXX_CLANG_TIDY "clang-tidy-18")

# TODO
# add_compile_options(-fsanitize=address,thread,memory,undefined)
# add_link_options(-fsanitize=address,thread,memory,undefined)

add_library(logging STATIC src/logging.cpp include/logging.hpp)
target_include_directories(logging PUBLIC include PRIVATE src)

add_library(buffer STATIC src/buffer.cpp include/buffer.hpp)
target_include_directories(buffer PUBLIC include PRIVATE src)
# TODO: buffer unit tests

# Example binaries
add_executable(tcp_server_simple examples/tcp_server_simple.cpp)
target_link_libraries(tcp_server_simple logging buffer)

add_executable(tcp_server_poll examples/tcp_server_poll.cpp)
target_link_libraries(tcp_server_poll logging)

add_executable(tcp_client examples/tcp_client.cpp)
target_link_libraries(tcp_client logging buffer)

add_executable(bcat examples/bcat.cpp)
add_executable(seek examples/seek.cpp)
