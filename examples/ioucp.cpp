#include "liburing.h"
#include <iostream>
#include <string_view>

static constexpr std::string_view kHelp = R"(
ioucp - cp using iouring

Usage: ioucp src_file dest_file

Args:
  src and dest_file are required arguments

)";

static constexpr unsigned int kEntriesCount = 64;

auto FileSize(int fd) -> size_t
{
    struct stat file_info;
    fstat(fd, &file_info);
    return file_info.st_size;
}

auto SetupContext(unsigned entry_count, struct io_uring* ring) -> int
{
    int ret;
    ret = io_uring_queue_init(entry_count, ring, 0);
    if (ret < 0) {
        std::cerr << "queue init failed. returned code " << ret << '\n';
        return -1;
    }
    return 0;
}

auto Copy(struct io_uring* ring, off_t input_size) -> int
{
    unsigned long reads, writes;
    struct io_uring_cqe* cqe;
    off_t write_left, offset;
    int ret;
    write_left = input_size;
    writes = reads = offset = 0;
    while (input_size || write_left) {
        unsigned long had_reads;
        int got_comp;
        had_reads = reads;
        while (input_size) {
        }
    }
    return ret;
}

auto main(int argc, char** argv) -> int
{
    if (argc != 3) {
        std::cerr << kHelp << '\n';
        return -1;
    }
    struct io_uring ring;
    int ret;
    int input_fd = open(argv[1], O_RDONLY);
    if (input_fd < 0) {
        std::cerr << "failed to open " << argv[1] << '\n';
        return -1;
    }
    int output_fd = open(argv[2], O_WRONLY);
    if (output_fd < 0) {
        std::cerr << "failed to open " << argv[2] << '\n';
        return -1;
    }
    if ((SetupContext(kEntriesCount, &ring))) {
        return -1;
    }
    off_t input_size = FileSize(input_fd);
    ret = Copy(&ring, input_size);
    close(input_fd);
    close(output_fd);
    io_uring_queue_exit(&ring);
    return ret;
}
