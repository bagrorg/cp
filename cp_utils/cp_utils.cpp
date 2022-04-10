#include <iostream>
#include <unistd.h>
#include <cstring>
#include "cp_utils.h"
#include "../file/FileUtils.h"
#include <fcntl.h>
#include "content_copy.h"

void my_cp::copy_symlink(const fs::path &src, const fs::path &dst) {        //is it solves problem with diff fss?
    errno = 0;
    char* orig_file_path = realpath(src.string().c_str(), NULL);
    if (orig_file_path == NULL) {
        throw std::runtime_error(strerror(errno));
    }

    errno = 0;
    int symlink_ret = symlink(orig_file_path, dst.string().c_str());
    if (symlink_ret != 0) {
        throw std::runtime_error(strerror(errno));
    }
}

void my_cp::copy_hardlink(const fs::path &src, const fs::path &dst) {
    auto src_str = src.string();
    auto dst_str = dst.string();

    errno = 0;
    int res = linkat(0, src_str.c_str(), 0, dst_str.c_str(), 0);
    if (res != 0) {
        switch (errno) {
            case EXDEV:
                std::cout << "Different file systems! Copying content." << std::endl;
                detail::copy_content(src, dst);                                                            
                break;

            default:
                throw std::runtime_error("Something wrong with file: " + std::string(strerror(errno)));
        }
    } else {
        std::cout << "File successfully hard-linked!" << std::endl;
    }
}

void my_cp::copy_main(const fs::path &src, const fs::path &dst) {
    fs::file_status s = fs::status(src);

    switch (s.type()) {
        case fs::file_type::symlink:
            my_cp::copy_symlink(src, dst);
            break;
        case fs::file_type::regular:
            my_cp::copy_hardlink(src, dst);
            break;
        
        default:
            throw std::runtime_error("Unsupported file type");
    }
}


