// sans19_hash.h
#ifndef SANS19_HASH_H
#define SANS19_HASH_H

#include <string>
#include <cstddef>
#include <cstdint>

class Sans19Hash {
public:
    Sans19Hash();
    void update(const uint8_t* data, size_t len);
    std::string finalize(); // Raw binary
    std::string hexdigest(); // Hex string

private:
    uint64_t state[4];
    uint64_t tail;
    size_t length;
    uint16_t kromer;
    uint16_t mybrotherhasavery;
    uint8_t SPECIALATTACK;
};

#endif // SANS19_HASH_H
