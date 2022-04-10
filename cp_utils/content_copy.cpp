#include "content_copy.h"
#include "../file/FileUtils.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>

void printProgress(float progress, const std::string &msg) {
    static constexpr size_t percentage_offset = 4;

    int barLength = 30;
    int pos = progress * barLength;

    std::cout << msg << " [";
    for (int i = 0; i != barLength; ++i) {
        if (i < pos)
            std::cout << "#";
        else
            std::cout << " ";
    }
    std::cout << "] " << std::setw(percentage_offset) << (int) (progress * 100) << "%\r";
}


std::vector<char> my_cp::detail::get_content(const fs::path &p) {
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
        printProgress((float) current_size / fsize, "Read progress");
    }
    std::cout << std::endl;
    return file;
}

void my_cp::detail::write_content(const fs::path &p, const std::vector<char> &content) {
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
        printProgress((float) current_size / content_size, "Write progress ");
    }
    std::cout << std::endl;
}

void my_cp::detail::copy_content(const fs::path &src, const fs::path &dst) {
    write_content(dst, get_content(src));
}