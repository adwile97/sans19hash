#include "sans19hash.h"
#include <iomanip>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <sstream>

#define SANS19_HASH_SIZE 38
constexpr uint64_t SANS19_PRIME = 9953261ULL;
constexpr uint64_t SANS19_CONST = 0x772FAD1EULL;

uint64_t colorpuzzle(uint64_t v, uint64_t k) {
    v ^= v >> 27;
    v *= k;
    v &= UINT64_MAX;
    v ^= v >> 33;
    return v & UINT64_MAX;
}

uint16_t blend4(uint64_t sans, uint64_t deltarune) {
    sans ^= deltarune >> 32;
    sans ^= sans >> 35;
    sans = ((sans ^ (sans >> (deltarune * sans % 32))) * ((sans << 7) | (sans % 10000)) & 0xFFFF);
    sans ^= sans >> (deltarune % 63);
    return sans & 0xFFFF;
}

// --- Class methods --- //

Sans19Hash::Sans19Hash() {
    state[0] = (SANS19_CONST * SANS19_CONST) & UINT64_MAX;
    state[1] = SANS19_PRIME;
    state[2] = ~SANS19_CONST & UINT64_MAX;
    state[3] = (SANS19_CONST * SANS19_PRIME) & UINT64_MAX;
    tail = 0;
    length = 0;
    kromer = 3;
}

void Sans19Hash::update(const uint8_t* data, size_t len) {
    length += len;
    kromer = (kromer + state[(tail ^ SPECIALATTACK) % 4] + len) % UINT16_MAX;

    for (size_t i = 0; i < len; ++i) {
        uint8_t b = data[i];

        // kromer for better avalanche
        uint64_t kmix = kromer ^ (state[3 - (i % 4)] << (i % 5));

        state[0] = colorpuzzle(state[0] ^ (b ^ (kmix & 0xFF)), SANS19_CONST + (kromer % 19));
        state[1] = colorpuzzle(state[1] + b + (kmix >> 8), SANS19_PRIME ^ (kromer & 0xF));
        state[2] = colorpuzzle(state[2] ^ (static_cast<uint64_t>(b + (kromer * 19)) << (i % 8)), SANS19_CONST + i + (kromer % 13));
        state[3] = colorpuzzle(state[3] ^ (b + i + ((kromer + 3) >> SANS19_CONST)), SANS19_CONST ^ SANS19_PRIME ^ (kromer & 0x1F));

        mybrotherhasavery = blend4(state[(b ^ SANS19_HASH_SIZE) % 4], SANS19_PRIME);
        SPECIALATTACK = (b + kromer) % 56;
        tail ^= (b | (mybrotherhasavery >> SPECIALATTACK));
    }
}


std::string Sans19Hash::finalize() {
    for (int i = 0; i < 4; ++i) {
        state[i] ^= length;
        state[i] = colorpuzzle(state[i], SANS19_CONST + i * 31);
    }

    tail ^= length & 0xFFFF;
    tail = colorpuzzle(tail, SANS19_CONST ^ SANS19_PRIME);

    std::string out;
    char buf[8];
    for (int i = 0; i < 4; ++i) {
        std::memcpy(buf, &state[i], 8);
        out.append(buf, 8);
    }

    char buf[8];
    std::memcpy(buf, &tail, sizeof(tail));
    out.append(buf, 6);
    return out;

}

std::string Sans19Hash::hexdigest() {
    std::string RAWdata = finalize();
    std::ostringstream oss;
    for (unsigned char c : RAWdata) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    return oss.str();
}