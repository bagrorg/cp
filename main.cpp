#include "cp_utils/cp_utils.h"
#include <iostream>
#include <unistd.h>
#include <cstring>

void validate_input(const fs::path &src, const fs::path &dst) {
    if (!fs::exists(src)) throw std::runtime_error("No such `src` file");
    if (src.empty()) throw std::runtime_error("Empty `src`");
    if (dst.empty()) throw std::runtime_error("Empty `dst`");
}

class BackupFile {
public:
    BackupFile() = default;
    void build(const fs::path &p) {
        backup_path = p.parent_path() / fs::path(p.filename().string() + ".bk");

        errno = 0;
        int res = linkat(0, p.string().c_str(), 0, backup_path.string().c_str(), 0);
        if (res != 0) {
            throw std::runtime_error("Something wrong with file (backup): " + std::string(strerror(errno)));
        } else {
            std::cout << "Successfully created backup!" << std::endl;
        }

        is_active = true;
    }

    ~BackupFile() {
        std::cout << "LOL" << std::endl;
        if (is_active) {
            fs::remove(backup_path);
        }
    }

    bool isActive() const {
        return is_active;
    }

    void copyTo(const fs::path &other) {
        if (!is_active) throw std::runtime_error("Trying to copy backup file while it is not exists");

        errno = 0;
        int res = linkat(0, backup_path.string().c_str(), 0, other.string().c_str(), 0);
        if (res != 0) {
            std::cerr << "Error while recovering file: " << strerror(errno) << std::endl;
        }
    }

private:
    fs::path backup_path;
    bool is_active = false;
};

void process_dst(const fs::path &p) {
    if (!fs::exists(p)) {
        if (!p.parent_path().empty()) {
            fs::create_directories(p.parent_path());
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Not enough arguments" << std::endl;
        return EXIT_FAILURE;
    }

    fs::path src = argv[1];
    fs::path dst = argv[2];
    src = fs::absolute(src);
    dst = fs::absolute(dst);
    BackupFile backup;

    try {
        validate_input(src, dst);
        dst = path_processing::process_path(src, dst);
        
        if (!fs::exists(dst)) {
            process_dst(dst);    
        } else {
            backup.build(dst);
            fs::remove(dst);
        }

        my_cp::copy_main(src, dst);
    } catch(const std::exception &e) {
        std::cerr << e.what() << std::endl;
        if (backup.isActive()) {
            backup.copyTo(dst);
        }
        
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

