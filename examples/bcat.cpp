#include <fcntl.h>
#include <unistd.h>

#include <array>
#include <iostream>
#include <string_view>

static constexpr std::string_view kHelp = R"(
bcat - basic cat clone

Usage:
  bcat [filename]

Arguments:
  filename - (optional) path to a file

)";

static constexpr ssize_t kDefaultBufferSize = 4096;

auto main(int argc, char** argv) -> int
{
    int fd;
    bool is_file = false;
    if (argc > 2) {
        std::cerr << kHelp << '\n';
        return -1;
    } else if (argc == 2) {
        fd = open(argv[1], O_RDONLY);
        if (fd < 0) {
            std::cerr << "Could not open file " << argv[1] << '\n';
            return -1;
        }
        is_file = true;
    } else {
        fd = 0; // stdin
    }
    std::array<char, kDefaultBufferSize> buffer;
    ssize_t bytes_read, bytes_written;
    do {
        bytes_read = read(fd, buffer.data(), buffer.max_size());
        if (bytes_read > 0) {
            bytes_written = write(1, buffer.data(), bytes_read);
        }
    } while (bytes_read > 0 && bytes_written == bytes_read);
    if (is_file)
        close(fd);
    return 0;
}
