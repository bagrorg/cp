#include <filesystem>
#include <iostream>
#include <vector>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "file/FileUtils.h"

static constexpr size_t READ_CHUNK_SIZE = 4 * 1024;
static constexpr size_t WRITE_CHUNK_SIZE = 4 * 1024;

namespace fs = std::filesystem;

fs::path process_existed_path(const fs::path &src, const fs::path &dst) {
    fs::file_status s = fs::status(dst);
    std::cout << dst << std::endl;

    switch (s.type()) {
        case fs::file_type::regular:
        case fs::file_type::symlink:    //is for symlink too?
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

fs::path process_unexisted_path(const fs::path &src, const fs::path &dst) {
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

fs::path process_path(const fs::path &src, const fs::path &dst) {
    if (fs::exists(dst)) {
        return process_existed_path(src, dst);
    } else {
        return process_unexisted_path(src, dst);
    }
}

std::vector<char> get_content(const fs::path &p) {
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
    std::cout << "READED " << current_size << " of " << fsize << std::endl;
    return file;
}

void write_content(const fs::path &p, const std::vector<char> &content) {
    FileDescriptor fd(p.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
    size_t current_size = 0;
    size_t content_size = content.size();
    std::cout << "Written " << current_size << " of " << content_size << std::endl;

    while(current_size < content_size) {
        std::cout << "Written " << current_size << " of " << content_size << std::endl;
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

void copy_content(const fs::path &src, const fs::path &dst) {
    std::cout << src << ' ' << dst << std::endl;
    write_content(dst, get_content(src));
}

void copy_main(const fs::path &src, const fs::path &dst) {
    errno = 0;
    int res = linkat(0, fs::absolute(src).string().c_str(), 0, fs::absolute(dst).string().c_str(), 0);
    if (res != 0) {
        switch (errno) {
            case EXDEV:
                std::cout << "Different file systems!" << std::endl;
                copy_content(src, dst);
                break;

            default:
                throw std::runtime_error("Something wrong with file: " + std::string(strerror(errno)));
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
    dst = process_path(src, dst);
    copy_main(src, dst);
}

