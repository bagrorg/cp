#include "path_processing.h"
#include <iostream>

fs::path path_processing::process_existed_path(const fs::path &src, const fs::path &dst) {
    fs::file_status s = fs::status(dst);

    if (s.type() == fs::file_type::directory) {
        return dst / src.filename();
    }
    return dst;
}

fs::path path_processing::process_unexisted_path(const fs::path &src, const fs::path &dst) {
    if (dst.filename().empty()) {
        return dst / src.filename();
    }
    return dst;
}

fs::path path_processing::process_path(const fs::path &src, const fs::path &dst) {
    if (fs::exists(dst)) {
        return process_existed_path(src, dst);
    } else {
        return process_unexisted_path(src, dst);
    }
}

void path_processing::recoursive_create(const fs::path &p, bool verbose) {
    fs::path rec = "";

    for (auto part: p.parent_path()) {
        rec /= part;
        if (!fs::exists(rec)) {
            if (verbose) std::cout << "Creating " << rec << std::endl;
            fs::create_directory(rec);
        }
    }
}