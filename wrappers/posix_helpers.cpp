#include "posix_helpers.h"

#include <unistd.h>
#include <fcntl.h>
#include <cstring>

void posix_helpers::symlink(const std::string &__from, const std::string &__to) {
    errno = 0;
    int symlink_ret = ::symlink(__from.c_str(), __to.c_str());
    if (symlink_ret != 0) {
        throw WrapperException(SYMLINK_EXCEPTION, strerror(errno));
    }
}

void posix_helpers::linkat(int __fromfd, const std::string &__from, int __tofd, const std::string &__to, int __flags) {
    errno = 0;
    int res = ::linkat(__fromfd, __from.c_str(), __tofd, __to.c_str(), __flags);
    if (res != 0) {
        switch (errno) {
            case EXDEV:
                throw WrapperException(LINKAT_EXDEV, strerror(errno));                                                       
                break;

            default:
                throw WrapperException(LINKAT_OTHER, strerror(errno));
        }
    }
}

std::string posix_helpers::realpath(const std::string &__name) {
    errno = 0;
    char* orig_file_path = ::realpath(__name.c_str(), NULL);
    if (orig_file_path == NULL) {
        throw WrapperException(REALPATH_EXCEPTION, strerror(errno));
    }

    return std::string(orig_file_path);
}

posix_helpers::WrapperException::WrapperException(ERROR_TYPE flag_, const std::string &strerror_input) : flag_(flag_) {
    std::string header;
    switch (flag_) {
        case LINKAT_EXDEV:
        case LINKAT_OTHER:
            strerror_ = "linkat() exception: " + strerror_input;
            break;
        
        case SYMLINK_EXCEPTION:
            strerror_ = "symlink() exception: " + strerror_input;
            break;
        
        case REALPATH_EXCEPTION:
            strerror_ = "realpath() exception: " + strerror_input;
            break;
        
        default:
            break;
    }
}

const char* posix_helpers::WrapperException::what() const noexcept 
{
    return strerror_.c_str();
}


posix_helpers::ERROR_TYPE posix_helpers::WrapperException::get_error() const
{
    return flag_;
}