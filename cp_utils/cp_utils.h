#pragma once

#include <cstddef>
#include <filesystem>
#include <vector>

static constexpr size_t READ_CHUNK_SIZE = 4 * 1024;
static constexpr size_t WRITE_CHUNK_SIZE = 4 * 1024;

namespace fs = std::filesystem;

namespace path_processing {
    fs::path process_existed_path(const fs::path &src, const fs::path &dst);

    fs::path process_unexisted_path(const fs::path &src, const fs::path &dst);

    fs::path process_path(const fs::path &src, const fs::path &dst);
}

namespace my_cp {
    namespace detail {

    }

    std::vector<char> get_content(const fs::path &p);

    void write_content(const fs::path &p, const std::vector<char> &content);

    void copy_content(const fs::path &src, const fs::path &dst);

    void symlink_copy(const fs::path &src, const fs::path &dst);
    //todo: naming
    void hardlink_copy(const fs::path &src, const fs::path &dst);

    void copy_main(const fs::path &src, const fs::path &dst);
}