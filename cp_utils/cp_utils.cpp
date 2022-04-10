#include "cp_utils.h"
#include "../file/FileUtils.h"
#include "../wrappers/posix_helpers.h"
#include "content_copy.h"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

void my_cp::copy_symlink(
    const fs::path &src,
    const fs::path &dst) { // is it solves problem with diff fss?
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

namespace my_cp {
FileBackUper::FileBackUper(const fs::path &p) {

}

FileBackUper::~FileBackUper() {}

void FileBackUper::onDelete() {}

void FileBackUper::process() {}

CreatedDirsBackUper::CreatedDirsBackUper(const fs::path &p) {
    to_delete = "";
    for (const auto &part: p) {
        to_delete /= part;
        if (!fs::exists(to_delete)) break;
    }
}

CreatedDirsBackUper::~CreatedDirsBackUper() {
    if (fs::exists(to_delete)) {
        fs::remove(to_delete);
    }
}

void CreatedDirsBackUper::onDelete() {
    
}

void CreatedDirsBackUper::process() {}
} // namespace my_cp