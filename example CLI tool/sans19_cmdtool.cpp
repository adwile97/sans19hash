// sans19_cmdtool.cpp
// compile this and run in the command line: sans19_cmdtool example.txt

#include <iostream>
#include <fstream>
#include <vector>
#include "../include/sans19hash.h"

int main(int argc, char* argv[]) {

    if (Sans19Hash::self_test()) {
        std::cout << "Self-test passed\n";
    } else {
        std::cout << "Self-test failed\n";
    }

    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0] << " <filename> [--raw]\n";
        return 1;
    }
    
    bool rawMode = false;
    if (argc == 3 && std::string(argv[2]) == "--raw") {
        rawMode = true;
    }

    std::ifstream file(argv[1], std::ios::binary);
    if (!file) {
        std::cerr << "Error: Could not open file " << argv[1] << "\n";
        return 1;
    }

    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());

    Sans19Hash hasher;
    hasher.update(buffer.data(), buffer.size());
    std::string hash = hasher.hexdigest();

    std::cout << "SANS19 Hash: " << hash << "\n";
    return 0;
}