#include <iostream>
#include <unistd.h>
#include <cstring>
#include "cp_utils.h"
#include "../file/FileUtils.h"
#include <fcntl.h>
#include "content_copy.h"
#include "../wrappers/posix_helpers.h"

void my_cp::copy_symlink(const fs::path &src, const fs::path &dst) {        //is it solves problem with diff fss?
    std::string orig_file_path = posix_helpers::realpath(src.string().c_str());
    posix_helpers::symlink(orig_file_path, dst.string().c_str());
}

void my_cp::copy_hardlink(const fs::path &src, const fs::path &dst) {
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


