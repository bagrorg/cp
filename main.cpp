#include "cp_utils/cp_utils.h"
#include "cp_utils/path_processing.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include "wrappers/posix_helpers.h"
#include <csignal>

#define CHECK_TERM() {          \
    if (terminated()) {         \
        return EXIT_FAILURE;    \
    }                           \
}

namespace {
    volatile std::sig_atomic_t gSignalStatus = 0;
}

void signal_handler(int signal) {
    gSignalStatus = 1;
}

bool terminated() {
    return gSignalStatus == 1;
}


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
    std::signal(SIGINT, signal_handler);
    std::signal(SIGABRT, signal_handler);

    fs::path src = argv[1];
    fs::path dst = argv[2];
    src = path_processing::normal_view(src);
    dst = path_processing::normal_view(dst);
    std::unique_ptr<my_cp::BackUper> backup = std::make_unique<my_cp::BackUper>();
    CHECK_TERM();

    try {
        validate_input(src, dst);
        dst = path_processing::process_path(src, dst);
        if (src == dst) {
            throw std::runtime_error("Can't do copy at the same folder with the same name");
        }

        CHECK_TERM();

        my_cp::Copyier cp(src, dst);
        backup = cp.prepare();

        CHECK_TERM();

        backup->process(true);

        CHECK_TERM();

        cp.copy();

        CHECK_TERM();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        backup->onDelete();

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

