#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
    fs::path p = argv[1];
    std::cout << p << std::endl;
}