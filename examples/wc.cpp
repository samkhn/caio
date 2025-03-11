// wc, simple word counter

#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <string_view>

static constexpr std::string_view kHelp = R"(
wc - word counter

Usage:
  mwc [filename1] [filename2] ... [filenamen]

Arguments:
  filename[n] - path to a file. at least 1 file required; n must be >= 1

)";

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << kHelp << '\n';
    return -1;
  }
  int total = 0;
  for (int i = 1; i < argc; ++i) {
    int words = 0;
    int fd = open(argv[i], O_RDONLY);
    if (fd != -1) {
      bool mid_word = false;
      ssize_t bytes;
      char buffer[4096];
      while ((bytes = read(fd, buffer, sizeof(buffer)))) {
        for (char *c = buffer; c < buffer + bytes; c++) {
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
        if (mid_word) words += 1;
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
