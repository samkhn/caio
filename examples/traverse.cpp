#include <cerrno>
#include <dirent.h>
#include <sys/stat.h>

#include <iostream>
#include <string>
#include <string_view>

static constexpr std::string_view kHelp = R"(
traverse - traverses and prints directory/file tree

Usage:
  traverse [ -r | -- ] pathnames

Arguments/switches
  -r          - (optional) recurse
  -(int)      - max depth (default 0)
  -pathnames  - list of paths, not ending with / e.g. /home/user, not /home/user/

)";

// possible optimization: replace w/ a map or table lookup
auto file_type(mode_t mode) -> std::string_view
{
    switch (mode & S_IFMT) {
    case S_IFSOCK:
        return "socket";
    case S_IFLNK:
        return "symbolic link";
    case S_IFIFO:
        return "FIFO";
    case S_IFREG:
        return "regular file";
    case S_IFBLK:
        return "block device";
    case S_IFCHR:
        return "char device";
    case S_IFDIR:
        return "directory";
    default:
        return "unknown";
    }
}

auto dot_name(const char* name) -> bool
{
    return name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0));
}

void list_directory(std::string file_path, int max_depth)
{
    DIR* dir_handle = opendir(file_path.data());
    if (dir_handle) {
        struct dirent* dir_entry;
        while ((dir_entry = readdir(dir_handle))) {
            if (dot_name(dir_entry->d_name))
                continue;
            struct stat stat_buffer;
            std::string child_path = file_path + '/';
            child_path += dir_entry->d_name;
            if ((stat(child_path.c_str(), &stat_buffer))) {
                std::cerr << "stat of " << file_path << "failed, errno = " << errno
                          << '\n';
            } else {
                std::cout << child_path
                          << ", type = " << file_type(stat_buffer.st_mode);
                if ((stat_buffer.st_mode & S_IFMT) == S_IFDIR) {
                    std::cout << '\n';
                    if (!dot_name(dir_entry->d_name) && (max_depth > 0 || max_depth == -1))
                        list_directory(child_path, max_depth < 0 ? -1 : max_depth - 1);
                } else {
                    std::cout << ", size = " << stat_buffer.st_size << '\n';
                }
            }
        }
        closedir(dir_handle);
    } else {
        std::cerr << "open handle to " << file_path << " failed, errno = " << errno
                  << '\n';
    }
}

auto main(int argc, char** argv) -> int
{
    if (argc < 2) {
        std::cerr << kHelp << '\n';
        return -1;
    }
    argv++;
    argc--;
    int max_depth = 0;
    if (argv[0][0] == '-') {
        if (argv[0][1] == 'r' && argv[0][2] == 0) {
            max_depth = -1;
        } else {
            max_depth = atoi(argv[0] + 1);
        }
        argv++;
        argc--;
    }
    for (int i = 0; i < argc; i++) {
        struct stat stat_buf;
        if ((stat(argv[i], &stat_buf))) {
            std::cerr << "stat of " << argv[i] << " failed, errno = " << errno
                      << '\n';
        } else {
            std::cout << argv[i] << ", type = " << file_type(stat_buf.st_mode);
            if ((stat_buf.st_mode & S_IFMT) == S_IFDIR) {
                std::cout << '\n';
                list_directory(argv[i], max_depth < 0 ? -1 : max_depth - 1);
            } else {
                std::cout << ", size = " << stat_buf.st_size << '\n';
            }
        }
    }
    return 0;
}
