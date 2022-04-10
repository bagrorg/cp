#pragma once

#include <exception>
#include <string>

namespace posix_helpers {
    enum ERROR_TYPE {
        LINKAT_EXDEV = 0,
        LINKAT_OTHER,
        SYMLINK_EXCEPTION,
        REALPATH_EXCEPTION
    };

    class WrapperException : public std::exception {
    public:
        explicit WrapperException(ERROR_TYPE flag_, const std::string &strerror_input);

        const char *what() const noexcept override;

        ERROR_TYPE get_error() const;

    private:
        ERROR_TYPE flag_;
        std::string strerror_;
    };

    void symlink(const std::string &__from, const std::string &__to);

    void linkat(int __fromfd, const std::string &__from, int __tofd, const std::string &__to, int __flags);

    std::string realpath(const std::string &__name);
}