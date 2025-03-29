cmake_minimum_required(VERSION 3.6)

project(caio
  DESCRIPTION "C++ library for async I/O ([e]poll, iouring)"
  LANGUAGES CXX
)

if (MSVC)
  add_compile_options(/W4 /w14640)
else()
  add_compile_options(-Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -g)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# NOTE: anything that depends on liburing needs threads
find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_library(IOURING_LIB liburing.so
  PATHS /usr/lib /usr/local/lib)

find_path(IOURING_HEADER liburing.h
  PATHS /usr/include /usr/local/include)

if(NOT IOURING_LIB OR NOT IOURING_HEADER)
  message(FATAL_ERROR "Cannot find liburing in system headers. Specify -DIOURING_HEADER and -DIOURING_LIB to point to header and library respectively")
else()
  message("iouring header at ${IOURING_HEADER} library at ${IOURING_LIB}")
endif()

find_program(CLANG_TIDY_BIN NAMES clang-tidy)
if(NOT CLANG_TIDY_BINARY)
  message(FATAL_ERROR "Could not find the program clang-tidy")
else()
  message("clang tidy found at ${CLANG_TIDY_BINARY}")
endif()
set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_BINARY}")

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

add_executable(wc examples/wc.cpp)
add_executable(mwc examples/mwc.cpp)
add_custom_target(mwc_loadtest
  python3 ${CMAKE_SOURCE_DIR}/examples/mwc_loadtest.py $<TARGET_FILE:mwc> $<TARGET_FILE:wc>
  DEPENDS mwc
  DEPENDS wc
)

add_executable(traverse examples/traverse.cpp)

add_executable(ioucp examples/ioucp.cpp)
target_link_libraries(ioucp ${IOURING_LIB})
target_include_directories(ioucp PUBLIC ${IOURING_HEADER} Threads::Threads)
