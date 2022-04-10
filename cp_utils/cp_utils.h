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
        virtual ~BackUper() = default;
        virtual void process(bool verbose);
        virtual void onDelete();
    };

    class FileBackUper : public BackUper {
    public:
        explicit FileBackUper(const fs::path &p);
        virtual ~FileBackUper();
        void onDelete() override;
        void process(bool verbose) override;
    private:
        fs::path backup_file;
        fs::path original_file;
    };

    class CreatedDirsBackUper : public BackUper {
    public:
        explicit CreatedDirsBackUper(const fs::path &p);
        virtual ~CreatedDirsBackUper();
        void onDelete() override;
        void process(bool verbose) override;
    private:
        fs::path creating_root;
        fs::path full_path;
        bool armed = false;
    };

    class Copyier {
    public:
        Copyier(const fs::path &src, const fs::path &dst);

        void copy();
        std::unique_ptr<BackUper> prepare();

    private:
        void copy_symlink();
        void copy_hardlink();

        fs::path src;
        fs::path dst;
    };

}