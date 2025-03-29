// mwc, leveraging mmap to read file

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <string_view>

static constexpr std::string_view kHelp = R"(
mwc - word count using mmap()

Usage:
  mwc [filename1] [filename2] ... [filenamen]

Arguments:
  filename[n] - path to a file. at least 1 file required; n must be >= 1

)";

auto FileSize(int fd) -> size_t
{
    struct stat file_info;
    fstat(fd, &file_info);
    return file_info.st_size;
}

auto main(int argc, char** argv) -> int
{
    if (argc < 2) {
        std::cerr << kHelp << '\n';
        return -1;
    }
    int total = 0;
    for (int i = 1; i < argc; ++i) {
        int words = 0;
        int fd = open(argv[i], O_RDONLY);
        if (fd != -1) {
            size_t file_size = FileSize(fd);
            char* map = static_cast<char*>(
                mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0));
            if (map != MAP_FAILED) {
                bool mid_word = false;
                char* end = map + file_size;
                for (char* c = map; c < end; c++) {
                    switch (*c) {
                    case ' ':
                    case '\t':
                    case '\n':
                    case '\r':
                        if (mid_word) {
                            mid_word = false;
                            words += 1;
                        }
                        break;
                    default:
                        mid_word = true;
                    }
                }
                if (mid_word)
                    words += 1;
            }
        }
        close(fd);
        std::cout << words << '\t' << argv[i] << '\n';
        total += words;
    }
    if (argc > 2) {
        std::cout << total << '\t' << "Total" << '\n';
    }
}
