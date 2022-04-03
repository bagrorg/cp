#include <filesystem>
#include <iostream>
#include <sys/vfs.h>

namespace fs = std::filesystem;

bool same_fs(const fs::path& p1, const fs::path& p2) {
    //TODO
    return true;
}

void get_status(const fs::path& p) {
    if (!fs::exists(p)) {
        std::cout << "File doesn't exists" << std::endl;
        return;
    }
    fs::file_status s = fs::status(p);
    switch(s.type()) {
        case fs::file_type::directory:
            std::cout << "File is a directory" << std::endl;
            break;
        case fs::file_type::regular:
            std::cout << "File is a regular file" << std::endl;
            break;
        case fs::file_type::symlink:
            std::cout << "File is a symlink" << std::endl;
            break;
        default:
            std::cout << "File has unsupported status" << std::endl;
            break;
    }
}



int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Not enough arguments" << std::endl;
        return EXIT_FAILURE;
    }

    fs::path p1 = argv[1];
    fs::path p2 = argv[2];

    get_status(p1);
    get_status(p2);

    struct statfs fs_info = {};
    statfs(p1.c_str(), &fs_info);
    std::cout << fs_info.f_fsid.__val[0] << ' ' << fs_info.f_fsid.__val[1] << std::endl;

    statfs(p2.c_str(), &fs_info);
    std::cout << fs_info.f_fsid.__val[0] << ' ' << fs_info.f_fsid.__val[1] << std::endl;
}

