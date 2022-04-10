#pragma once

#include <cstddef>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace my_cp {
    namespace detail {

    }

    void copy_symlink(const fs::path &src, const fs::path &dst);
    void copy_hardlink(const fs::path &src, const fs::path &dst);

    void copy_main(const fs::path &src, const fs::path &dst);
}