#include "cp_utils/cp_utils.h"
#include <iostream>
#include <unistd.h>
#include <cstring>

void validate_input(const fs::path &src, const fs::path &dst) {
    if (!fs::exists(src)) throw std::runtime_error("No such `src` file");
    if (src.empty()) throw std::runtime_error("Empty `src`");
    if (dst.empty()) throw std::runtime_error("Empty `dst`");
}

fs::path create_backup(const fs::path &p) {
    fs::path backup = p.parent_path() / fs::path(p.filename().string() + ".bk");

    errno = 0;
    int res = linkat(0, p.string().c_str(), 0, backup.string().c_str(), 0);             //todo: copypaste???
    if (res != 0) {
        throw std::runtime_error("Something wrong with file: " + std::string(strerror(errno)));
    } else {
        std::cout << "File successfully hard-linked!" << std::endl;
    }

    return backup;
}

fs::path process_dst(const fs::path &p) {
    if (!fs::exists(p)) {
        if (!p.parent_path().empty()) {
            fs::create_directories(p.parent_path());
        }

        return "";
    } else {
        auto backup = create_backup(p);
        fs::remove(p);
        return backup;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Not enough arguments" << std::endl;
        return EXIT_FAILURE;
    }

    fs::path src = argv[1];
    fs::path dst = argv[2];
    src = fs::absolute(src);
    dst = fs::absolute(dst);
    fs::path backup;

    try {
        validate_input(src, dst);
        dst = path_processing::process_path(src, dst);
        backup = process_dst(dst);

        my_cp::copy_main(src, dst);
    } catch(const std::exception &e) {
        if (!backup.empty()) {
            errno = 0;
            int res = linkat(0, backup.string().c_str(), 0, dst.string().c_str(), 0);
            if (res != 0) {
                throw std::runtime_error("Something wrong with file: " + std::string(strerror(errno)));         //todo: throw in catch?
            }
        }

        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}

