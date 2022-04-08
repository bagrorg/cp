#include <iostream>
#include <unistd.h>
#include <cstring>
#include "cp_utils.h"
#include "../file/FileUtils.h"
#include <fcntl.h>

fs::path path_processing::process_existed_path(const fs::path &src, const fs::path &dst) {
    fs::file_status s = fs::status(dst);
    std::cout << dst << std::endl;

    switch (s.type()) {
        case fs::file_type::regular:
        case fs::file_type::symlink:
            fs::remove(dst);
            return dst;

        case fs::file_type::directory:
            if (fs::exists(dst / src.filename())) {
                fs::remove(dst / src.filename());
            }
            return dst / src.filename();

        default:
            throw std::runtime_error("Unsupported file status");
    }
}

fs::path path_processing::process_unexisted_path(const fs::path &src, const fs::path &dst) {
    if (dst.filename().empty()) {
        if (!fs::exists(dst)) {
            fs::create_directories(dst);
        }

        return dst / src.filename();
    } else if (!dst.parent_path().empty()) {
        if (!fs::exists(dst.parent_path())) {
            fs::create_directories(dst.parent_path());
        }
    }

    return dst;
}

fs::path path_processing::process_path(const fs::path &src, const fs::path &dst) {
    if (fs::exists(dst)) {
        return process_existed_path(src, dst);
    } else {
        return process_unexisted_path(src, dst);
    }
}

std::vector<char> my_cp::get_content(const fs::path &p) {
    FileDescriptor fd(p.c_str(), O_RDONLY);
    size_t fsize = fd.getFileSize();
    size_t current_size = 0;

    std::vector<char> file(fsize);

    while(current_size < fsize) {
        size_t want_to_read = std::min(READ_CHUNK_SIZE, fsize - current_size);
        errno = 0;
        ssize_t readed = read(fd.getDescriptor(), file.data() + current_size, want_to_read);

        if (readed == -1) {
            switch (errno) {
                case EAGAIN:
                case EINTR:
                    continue;

                default:
                    throw std::runtime_error("Something wrong with file (READ): " + std::string(strerror(errno)));
            }
        } else if (readed == 0) {
            throw std::runtime_error("File trunkated! Current size: " + std::to_string(current_size) + ", expected size: " + std::to_string(fsize));
        }

        current_size += readed;

        if (readed != want_to_read) {
            std::cout << "WARNING: Readed " << readed << std::endl;
        }
    }
    return file;
}

void my_cp::write_content(const fs::path &p, const std::vector<char> &content) {
    FileDescriptor fd(p.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
    size_t current_size = 0;
    size_t content_size = content.size();

    while(current_size < content_size) {
        size_t want_to_write = std::min(WRITE_CHUNK_SIZE, content_size - current_size);
        errno = 0;
        ssize_t writed = write(fd.getDescriptor(), content.data() + current_size, want_to_write);

        if (writed == -1) {
            switch (errno) {
                case EAGAIN:
                case EINTR:
                    continue;

                default:
                    throw std::runtime_error("Something wrong with file (WRITE): " + std::string(strerror(errno)));
            }
        } else if (writed == 0) {
            throw std::runtime_error("File trunkated! Current size: " + std::to_string(current_size) + ", expected size: " + std::to_string(content_size));
        }

        current_size += writed;

        if (writed != want_to_write) {
            std::cout << "WARNING: Writed " << writed << std::endl;
        }
    }
}

void my_cp::copy_content(const fs::path &src, const fs::path &dst) {
    write_content(dst, get_content(src));
}

void my_cp::symlink_copy(const fs::path &src, const fs::path &dst) {
    errno = 0;
    int ret_code = symlink(fs::absolute(src).string().c_str(), fs::absolute(dst).string().c_str());
    if (ret_code != 0) {
        throw std::runtime_error(strerror(errno));
    }
}

void my_cp::hardlink_copy(const fs::path &src, const fs::path &dst) {
    const char *src_abs = fs::absolute(src).string().c_str();
    const char *dst_abs = fs::absolute(dst).string().c_str();

    errno = 0;
    int res = linkat(0, src_abs, 0, dst_abs, 0);        //todo: flags?
    if (res != 0) {
        switch (errno) {
            case EXDEV:
                std::cout << "Different file systems! Copying content." << std::endl;
                copy_content(src, dst);
                break;

            default:
                throw std::runtime_error("Something wrong with file: " + std::string(strerror(errno)));
        }
    }
}

void my_cp::copy_main(const fs::path &src, const fs::path &dst) {
    if (fs::is_symlink(src)) {
        symlink_copy(src, dst);
    } else {
        hardlink_copy(src, dst);
    }
}


