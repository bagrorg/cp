#include "cp_utils/cp_utils.h"
#include "cp_utils/path_processing.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include "wrappers/posix_helpers.h"

void validate_input(const fs::path &src, const fs::path &dst) {
    if (!fs::exists(src)) throw std::runtime_error("No such `src` file");
    if (src.empty()) throw std::runtime_error("Empty `src`");
    if (dst.empty()) throw std::runtime_error("Empty `dst`");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Not enough arguments" << std::endl;
        return EXIT_FAILURE;
    }

    fs::path src = argv[1];
    fs::path dst = argv[2];
    src = path_processing::normal_view(src);
    dst = path_processing::normal_view(dst);
    std::unique_ptr<my_cp::BackUper> backup = std::make_unique<my_cp::BackUper>();

    try {
        validate_input(src, dst);
        dst = path_processing::process_path(src, dst);
        if (src == dst) {
            throw std::runtime_error("Can't do copy at the same folder with the same name");
        }

        my_cp::Copyier cp(src, dst);
        backup = cp.prepare();
        backup->process(true);
        cp.copy();
    } catch(const std::exception &e) {
        std::cerr << e.what() << std::endl;
        backup->onDelete();

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

