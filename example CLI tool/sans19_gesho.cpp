// Get Expected haSH Outputs
#include <iostream>
#include <cstring>
#include "sans19hash.h"
int main() {
    for (int i = 0; i < NUM_TEST_VECTORS; ++i) {
        s19h h;
        h.update(reinterpret_cast<const uint8_t*>(test_vectors[i].input), strlen(test_vectors[i].input));
        std::string hex = h.hexdigest();
        std::cout << hex << std::endl;
    }

    return 0;
}
