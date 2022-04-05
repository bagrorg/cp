#include "FileUtils.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdexcept>
#include <fcntl.h>
#include <sys/stat.h>

size_t FileDescriptor::getFileSize() const {
    struct stat statbuf = {};
    errno = 0;
    int ret_code = fstat(descr, &statbuf);

    if (ret_code != 0) {
        throw std::runtime_error(strerror(errno));
    }

    return statbuf.st_size;
}

int FileDescriptor::getDescriptor() const {
    return descr;
}

FileDescriptor::~FileDescriptor() {
    close(descr);
}

FileDescriptor::FileDescriptor(const char *fn, int flags) {
    errno = 0;
    descr = open(fn, flags, 0666);                                              //TODO!!
    if (descr < 0) {
        throw std::runtime_error(strerror(errno));
    }
}
