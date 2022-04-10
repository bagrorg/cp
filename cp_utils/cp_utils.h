#pragma once

#include <cstddef>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace path_processing {
    fs::path process_existed_path(const fs::path &src, const fs::path &dst);

    fs::path process_unexisted_path(const fs::path &src, const fs::path &dst);

    fs::path process_path(const fs::path &src, const fs::path &dst);
}

namespace my_cp {
    namespace detail {

    }

    void copy_symlink(const fs::path &src, const fs::path &dst);
    void copy_hardlink(const fs::path &src, const fs::path &dst);

    void copy_main(const fs::path &src, const fs::path &dst);
}