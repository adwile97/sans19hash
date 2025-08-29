// Get Expected haSH Outputs
#include <iostream>
#include <cstring>
#include "sans19hash.h"
int main() {
    const char* test_strings[] = {
        "",
        "sans19",
        "YOUR TAKING TOO LONG",
        "YOUR         LONG",
        "your taking too long :)",
        "your taking too long :) IS TAKING TOO LONG",
        "YOUR TOO BRIGHT",
        "YOUR TAKING TOO TOO",
        "YOUR         TOO TOO"
    };

    for (const char* s : test_strings) {
        Sans19Hash h;
        h.update(reinterpret_cast<const uint8_t*>(s), strlen(s));
        std::string hex = h.hexdigest();
        std::cout << hex << std::endl;
    }

    return 0;
}
