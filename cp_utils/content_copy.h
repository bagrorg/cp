#pragma once

#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

namespace my_cp {
    namespace detail {
        static constexpr size_t READ_CHUNK_SIZE = 4 * 1024;
        static constexpr size_t WRITE_CHUNK_SIZE = 4 * 1024;

        std::vector<char> get_content(const fs::path &p);

        void write_content(const fs::path &p, const std::vector<char> &content);

        void copy_content(const fs::path &src, const fs::path &dst);
    }
}