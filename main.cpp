#include "cp_utils/cp_utils.h"
#include <iostream>

void validate_input(const fs::path &src, const fs::path &dst) {
    if (!fs::exists(src)) throw std::runtime_error("No such `src` file");
    if (src.empty()) throw std::runtime_error("Empty `src`");
    if (dst.empty()) throw std::runtime_error("Empty `dst`");
}

bool process_dst(const fs::path &p) {
    if (!fs::exists(p)) {
        if (!p.parent_path().empty()) {
            fs::create_directories(p.parent_path());
        }

        return false;
    } else {
        fs::remove(p);
        return true;
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
    bool exists;

    try {
        validate_input(src, dst);
        dst = path_processing::process_path(src, dst);
        exists = process_dst(dst);

        my_cp::copy_main(src, dst);
    } catch(const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}

