#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <string_view>

static constexpr std::string_view kHelp = R"(
seek - read count bytes of file at an offset

Usage:
  seek [filename] [position] [bytes]

Arguments:
  filename - path to a file
  offset   - non-negative integer offset into a file to begin reading from
  bytes    - non-negative integer count of bytes to read

)";

auto main(int argc, char** argv) -> int
{
    if (argc != 4) {
        std::cerr << kHelp << '\n';
        return -1;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        std::cerr << "could not open file " << argv[1] << '\n';
        return -1;
    }
    int position = atoi(argv[2]);
    if (position < 0) {
        std::cerr << "position must be >= 0\n";
        return -1;
    }
    int bytes = atoi(argv[3]);
    if (bytes <= 0) {
        std::cerr << "bytes must be > 0\n";
        return -1;
    }
    off_t seek_result = lseek(fd, position, SEEK_SET);
    if (seek_result == -1) {
        std::cerr << "could not seek to " << position << '\n';
        return -1;
    }
    char* buffer = (char*)malloc(bytes * sizeof(char));
    ssize_t bytes_read;
    if ((bytes_read = read(fd, buffer, bytes))) {
        write(1, buffer, bytes_read);
        std::cout << '\n';
    } else {
        std::cerr << "could not read " << bytes << " bytes at position " << position
                  << '\n';
        return -1;
    }
    free(buffer);
    close(fd);
    return 0;
}
