#include <filesystem>
#include <iostream>
#include <sys/vfs.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/file.h>
#include <unistd.h>
#include <memory>
#include <string.h>

namespace fs = std::filesystem;

struct FileSystem {
    bool operator==(const FileSystem &other_fs) const {
        return true;
    }
};

class FileDescriptor {
public:
    FileDescriptor() = default;

    explicit FileDescriptor(const char* fn, int flags) {
        errno = 0;
        descr = open(fn, flags);
        if (descr < 0) {
            throw std::runtime_error(strerror(errno));
        }
    }

    ~FileDescriptor() {
        close(descr);
    }

    int getDescriptor() const {
        return descr;
    }

    size_t getFileSize() const {
        struct stat statbuf = {};
        errno = 0;
        int ret_code = fstat(descr, &statbuf);

        if (ret_code != 0) {
            throw std::runtime_error(strerror(errno));
        }

        return statbuf.st_size;
    }
    
private:
    int descr;
};

class FileManager;

class File {
public:
    explicit File(const fs::path &p, int flags) : p(p) {
        if (!fs::exists(p)) {
            throw std::runtime_error("No such file: " + p.string());
        }

        fd = FileDescriptor(p.c_str(), flags);
        s = fs::status(p);
    }

    virtual ~File() {};

    std::string getName() const {
        return p.filename().string();
    }

    FileSystem getFileSystem() const {
        return fs;
    }

    const fs::path &getPath() const {
        return p;
    }

    std::vector<char> getContent(bool verbose) const {
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
                    throw std::runtime_error("Something wrong with file: " + std::string(strerror(errno)));
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

    void writeToFile(const std::vector<char> &content) {
        size_t current_size = 0;
        size_t content_size = content.size();

        while(current_size < content_size) {
            size_t want_to_write = std::min(READ_CHUNK_SIZE, content_size - current_size);
            errno = 0;
            ssize_t writed = write(fd.getDescriptor(), content.data() + current_size, want_to_write);
            
            if (writed == -1) {
                switch (errno) {
                case EAGAIN:
                case EINTR:
                    continue;
                
                default:
                    throw std::runtime_error("Something wrong with file: " + std::string(strerror(errno)));
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

    virtual void copyFrom(const File &src) = 0;

    fs::file_type getStatus() {
        return s.type();
    }

protected:
    FileDescriptor fd;
    fs::path p;
    fs::file_status s;
    FileSystem fs;

    static constexpr size_t READ_CHUNK_SIZE = 4 * 1024;
    static constexpr size_t WRITE_CHUNK_SIZE = 4 * 1024;
};

class RegularFile : public File {
public:
    explicit RegularFile(const fs::path &p, int flags) : File(p, flags) {};

    virtual void copyFrom(const File &src) override {
        if (fs == src.getFileSystem()) {
            link(src.getPath().string().c_str(), p.string().c_str());
        } else {
            auto content = src.getContent(true);
            writeToFile(content);
        }
    }
};

class SymlinkFile : public File {
public:
    explicit SymlinkFile(const fs::path &p, int flags) : File(p, flags) {};

    virtual void copyFrom(const File &src) override {
        symlink(src.getPath().string().c_str(), p.string().c_str());
    }
};

class FileManager {
public:
    FileManager(const fs::path &p) : p(p) {};

    void buildCreateFileIfNecessary() {
        if (!fs::exists(p)) {
            fs::create_directories(p);
        }
    }

    std::unique_ptr<SymlinkFile> buildSymlinkFile(int flags) {
        return std::make_unique<SymlinkFile>(p, flags);
    }
    
    std::unique_ptr<RegularFile> buildRegularFile(int flags) {
        return std::make_unique<RegularFile>(p, flags);
    }

    std::unique_ptr<File> build(int flags) {
        if (!fs::exists(p)) {
            throw std::runtime_error("File does not exists");
        }

        fs::file_status s = fs::status(p);

        switch(s.type()) {
        case fs::file_type::regular:
            return buildRegularFile(flags);
        case fs::file_type::symlink:
            return buildSymlinkFile(flags);
        default:
            throw std::runtime_error("Unsupported file status");
            break;
        }
    }

private:
    fs::path p;
};


class CopyManager {
public:
    CopyManager(const fs::path src, const fs::path dst) : src(src), dst(dst) {};

    void copy() {
        FileManager fm_src(src);
        FileManager fm_dst(dst);

        auto src_file = fm_src.build(O_RDONLY);
        std::unique_ptr<File> dst_file;

        switch (src_file->getStatus()) {
            case fs::file_type::regular: {
                dst_file = fm_dst.buildRegularFile(O_WRONLY);
                break;
            }

            case fs::file_type::symlink: {
                dst_file = fm_dst.buildSymlinkFile(O_WRONLY);
                break;
            }

            default:
                throw std::runtime_error("Unsupported file status");
                break;
        }
        dst_file->copyFrom(*src_file);
    }

private:
    fs::path src;
    fs::path dst;
};


int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Not enough arguments" << std::endl;
        return EXIT_FAILURE;
    }

    CopyManager cm(argv[1], argv[2]);
    cm.copy();
}

