#pragma once

#include <filesystem>

namespace fs = std::filesystem;

namespace path_processing {
    fs::path process_existed_path(const fs::path &src, const fs::path &dst);

    fs::path process_unexisted_path(const fs::path &src, const fs::path &dst);

    fs::path process_path(const fs::path &src, const fs::path &dst);

    fs::path normal_view(const fs::path &p) {
        return fs::absolute(p).lexically_normal();
    }
}