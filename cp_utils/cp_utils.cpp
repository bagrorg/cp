#include "cp_utils.h"
#include "../wrappers/posix_helpers.h"
#include "content_copy.h"
#include <iostream>

namespace my_cp {

    FileBackUper::FileBackUper(const fs::path &p) : backup_file(
            p.parent_path() / fs::path(p.filename().string() + ".bk")), original_file(p) {}

    FileBackUper::~FileBackUper() {
        try {
            if (fs::exists(backup_file)) {
                fs::remove(backup_file);
            }
        } catch (const std::exception &e) {
            std::cerr << "Problems with removing of backup file: " << e.what() << std::endl;
        }
    }

    void FileBackUper::onDelete() {
        posix_helpers::linkat(0, backup_file.string(), 0, original_file.string(), 0);
    }

    void FileBackUper::process(bool verbose) {
        posix_helpers::linkat(0, original_file.string(), 0, backup_file.string(), 0);
        if (verbose) std::cout << "Successfully created backup!" << std::endl;
        fs::remove(original_file);
    }

    CreatedDirsBackUper::CreatedDirsBackUper(const fs::path &p) : full_path(p) {
        creating_root = "";
        for (const auto &part: p) {
            creating_root /= part;
            if (!fs::exists(creating_root)) break;
        }
    }

    CreatedDirsBackUper::~CreatedDirsBackUper() {
        try {
            if (armed) {
                if (fs::exists(creating_root)) {
                    fs::remove_all(creating_root);
                }
            }
        } catch (const std::exception &e) {
            std::cerr << "Problems with removing of created directories: " << e.what() << std::endl;
        }
    }

    void CreatedDirsBackUper::onDelete() {
        armed = true;
    }

    void CreatedDirsBackUper::process(bool verbose) {
        fs::path rec = creating_root;
        for (const auto &part: full_path.parent_path()) {
            rec /= part;
            if (!fs::exists(rec)) {
                if (verbose) std::cout << "Creating " << rec << std::endl;
                fs::create_directory(rec);
            }
        }
    }

    Copyier::Copyier(const fs::path &src, const fs::path &dst) : src(src), dst(dst) {}

    void Copyier::copy() {
        fs::file_status s = fs::status(src);

        switch (s.type()) {
            case fs::file_type::symlink:
                copy_symlink();
                break;
            case fs::file_type::regular:
                copy_hardlink();
                break;

            default:
                throw std::runtime_error("Unsupported file type");
        }
    }

    std::unique_ptr<BackUper> Copyier::prepare() {
        std::unique_ptr<BackUper> backUper;
        if (!fs::exists(dst)) {
            backUper = std::make_unique<CreatedDirsBackUper>(dst);
        } else {
            backUper = std::make_unique<FileBackUper>(dst);
        }
        return backUper;
    }

    void Copyier::copy_symlink() {
        std::string orig_file_path = posix_helpers::realpath(src.string());
        posix_helpers::symlink(orig_file_path, dst.string());
    }

    void Copyier::copy_hardlink() {
        auto src_str = src.string();
        auto dst_str = dst.string();

        try {
            posix_helpers::linkat(0, src_str, 0, dst_str, 0);
            std::cout << "File successfully hard-linked!" << std::endl;
        } catch (const posix_helpers::WrapperException &e) {
            if (e.get_error() == posix_helpers::LINKAT_EXDEV) {
                std::cout << "Different file systems! Copying content." << std::endl;
                detail::copy_content(src, dst);
                std::cout << "FIle successfully copied!" << std::endl;
            } else {
                throw e;
            }
        }
    }

    void BackUper::process(bool verbose) {
        if (verbose) std::cout << "Backuper wasn't created" << std::endl;
    }

    void BackUper::onDelete() {}

} // namespace my_cp