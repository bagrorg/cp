#pragma once

#include <cstddef>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace my_cp {
    namespace detail {

    }

    class BackUper {
    public:
        virtual ~BackUper() = 0;
        virtual void process() = 0;
        virtual void onDelete() = 0;
    };

    class FileBackUper : public BackUper {
    public:
        FileBackUper(const fs::path &p);
        virtual ~FileBackUper() override;
        virtual void onDelete() override;
        virtual void process() override;
    private:
        fs::path backup_file;
        fs::path original_file;
    };

    class CreatedDirsBackUper : public BackUper {
    public:
        CreatedDirsBackUper(const fs::path &p);
        virtual ~CreatedDirsBackUper() override;
        virtual void onDelete() override;
        virtual void process() override;
    private:
        fs::path to_delete;
    };

    void copy_symlink(const fs::path &src, const fs::path &dst);
    void copy_hardlink(const fs::path &src, const fs::path &dst);

    void copy_main(const fs::path &src, const fs::path &dst);
}