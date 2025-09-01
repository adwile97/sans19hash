// sans19hash.h
#ifndef SANS19_HASH_H
#define SANS19_HASH_H

#include <string>
#include <cstddef>
#include <cstdint>

class s19h {
public:
    s19h();
    void update(const uint8_t* data, size_t len);
    std::string finalize(); // Raw binary
    std::string finalize256();
    std::string hexdigest(); // Hex string
    std::string hexdigest256();
    static bool self_test();
    
private:
    uint64_t state[4];
    uint64_t tail;
    uint64_t length;
    uint16_t kromer;
    uint16_t mybrotherhasavery;
    uint8_t SPECIALATTACK;
};

#endif // SANS19_HASH_H
