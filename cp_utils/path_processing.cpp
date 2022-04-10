#include "path_processing.h"

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

void path_processing::recoursive_create(const fs::path &p) {
    if (!fs::exists(p)) {
        if (!p.parent_path().empty()) {
            fs::create_directories(p.parent_path());
        }
    }
}