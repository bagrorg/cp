#pragma once

#include <stddef.h>

class FileDescriptor {
public:
    FileDescriptor() = default;

    explicit FileDescriptor(const char* fn, int flags);

    ~FileDescriptor();

    int getDescriptor() const;

    size_t getFileSize() const;

private:
    int descr;
};