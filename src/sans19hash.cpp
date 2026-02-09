#include "sans19hash.h"
#include <iomanip>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <sstream>

#include <limits> // compatibility

#define rotateleft(bits,word) \
                (((word) << (bits)) | ((word) >> (64-(bits))))

uint64_t rotl64(int bits, uint64_t word) {
    if (bits == 0) return rotl64(word & 63, word+1); // we use the edge case to our advantage
    return rotateleft(bits, word);
}

constexpr uint64_t SANS19_PRIME = 9953261ULL;    // (2^19 - 1) * 19 - 2^13
constexpr uint64_t SANS19_CONST = 0x772FAD1EULL; // based on something i don't remember

// The big-endian writes for serialization

inline void write_be64(uint8_t* out, uint64_t v) { // man i wonder what could this function called "write_be64" possibly do
    for (int i = 7; i >= 0; --i) {
        out[i] = v & 0xFF;
        v >>= 8;
    }
}

inline void write_be48(uint8_t* out, uint64_t v) {
    for (int i=5; i >= 0; --i) {
        out[i] = v & 0xFF;
        v >>= 8;
    }
}

// mIXING Functions

uint64_t colorpuzzle(uint64_t v, uint64_t k) {
    v ^= v >> 27;
    v ^= v << k % 30;
    v ^= rotl64(31, v);
    v *= k;
    v ^= rotl64(k % 64, v);
    return v;
}

uint16_t blend4(uint64_t sans, uint64_t deltarune) {
    sans ^= deltarune >> 32;
    sans ^= sans >> 35;
    sans = ( ~( sans ^ (sans >> ((deltarune * sans) % 32)))
           * ((sans << 7) | (sans % 10000) ) & 0xFFFF );
    sans ^= sans >> (deltarune % 63);
    return sans & 0xFFFF;
}

    ///////////////////////////
   // --------------------- //
  // --- Class methods --- //
 // --------------------- //
///////////////////////////

s19h::s19h() {
    state[0] = (SANS19_CONST * SANS19_CONST) & UINT64_MAX;
    state[1] = SANS19_PRIME;
    state[2] = ~SANS19_CONST & UINT64_MAX;
    state[3] = (SANS19_CONST * SANS19_PRIME) & UINT64_MAX;
    tail = 19;
    length = 0;
    kromer = 3;
    SPECIALATTACK = (state[2] - 1) %16; 
    unlock();
}

void s19h::update(const uint8_t* data, size_t len) { // Stage 1
    length += len;
    kromer = (kromer + state[(tail ^ SPECIALATTACK) % 4] + len) % UINT16_MAX;

    for (size_t i = 0; i < len; ++i) {
        uint8_t b = data[i];

        // kromer for better avalanche
        uint64_t kmix = kromer ^ (state[3 - (i % 4)] << (i % 53));

        state[0] = colorpuzzle(
                state[0] ^ (b ^ (kmix & 0xFF)),
                SANS19_CONST + (kromer % 19)
            );
        state[1] = colorpuzzle(
                state[1] + b + (kmix >> 8),
                SANS19_PRIME ^ (kromer & 0xF)
            );
        state[2] = colorpuzzle(
                ~(state[2] ^ (static_cast<uint64_t>(b + (kromer * 19)) << (i % 64))),
                SANS19_CONST + i + (kromer % 13)
        );
        state[3] = colorpuzzle(
                state[3] ^ (b + i + ((kromer + 3) ^ SANS19_CONST)),
                SANS19_CONST ^ SANS19_PRIME ^ (kromer & 0x1F)
            );
        mybrotherhasavery = blend4(state[(b ^ kmix) % 4], SANS19_PRIME);
        SPECIALATTACK = (SPECIALATTACK ^ (b + kromer)) & 0x0F;
        tail ^= (b | (mybrotherhasavery >> SPECIALATTACK));
    }
}

std::string s19h::finalize() { // stage 2
    if (stage2flag) return cachedhash; // if finalize was already called, return the cached hash
    for (int i = 0; i < 4; ++i) {
        state[i] ^= length;
        state[i] = colorpuzzle(state[i], SANS19_CONST + i * 31);

    }

    tail ^= length & 0xFFFF;
    tail = colorpuzzle(tail, SANS19_CONST ^ SANS19_PRIME);

    std::string out;
    char buf[8];
    for (int i = 0; i < 4; ++i) {
        write_be64(reinterpret_cast<uint8_t*>(buf), state[i]);
        out.append(buf, 8);
    }
    tail ^= (tail >> 48) ^ ((tail >> 56) << 8); // making the tail 6 bytes
    write_be48(reinterpret_cast<uint8_t*>(buf), tail);
    out.append(buf, 6);
    stage2flag = true;
    cachedhash = out;
    return out; // 38 bytes
}

std::string s19h::finalize256() { // stage 2-256
    if (stage2flag256) return cachedhash256; // if finalize256 was already called, return the cached hash
    tail ^= length & 0xFFFF;
    tail = colorpuzzle(tail, SANS19_CONST ^ SANS19_PRIME);
    for (int cycles = 0; cycles < 19; ++cycles) {
        for (int i = 0; i < 4; ++i) {
            state[i] ^= length;
            state[i] = colorpuzzle(state[i], tail);
            tail <<= ((state[i] >> (kromer * 17 % 60)) % 49); // make the tail less of a constant mask
            if (cycles % 2 == 0) {
                tail >>= 5;
            }
            tail ^= rotl64((kromer % 63), state[i] ^ tail);
            kromer ^= ((tail ^ state[i]) & 0xFFFF);
        }
    }
    std::string out;
    char buf[8];
    for (int i = 0; i < 4; ++i) {
        write_be64(reinterpret_cast<uint8_t*>(buf), state[i]);
        out.append(buf, 8);
    }
    stage2flag256 = true;
    cachedhash256 = out;
        return out; // 32 bytes
}
    

std::string s19h::hexdigest() {
    std::string RAWdata = finalize();
    std::ostringstream oss;
    for (unsigned char c : RAWdata) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    return oss.str();
}

std::string s19h::hexdigest256() {
    std::string RAWdata = finalize256();
    std::ostringstream oss;
    for (unsigned char c : RAWdata) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    return oss.str();
}

TestVector test_vectors[] = {
    // Empty string hash (reference o implemenutput from originaltation)
    {"","f87e1a2c43115bfd3bfff3095b2382576ecb3a676cb5aade189803d38acf80486f920d983db3"},
    // Hash of "sans19" (reference output from original implementation)
    {"sans19","7ec9bb0e080619c88190327ec27047cd749077f1d9257b9252f1405c36e349cd0df89b92c93e"},
    // Hash of "YOUR TAKING TOO LONG" (used for regression testing)
    {"YOUR TAKING TOO LONG", "ddbae741f114f5fca6a57a87f4ca37d0fa3da531e5440eed58125373049208dfe78783f5fbf2"},
    // Hash of "YOUR         LONG" (tests handling of multiple spaces)
    {"YOUR         LONG", "af25d71dd58ab5e196dc4b082c5c75741fa3ea7285f353ab92a54abca9fc42fb3407bbbafde6"},
    // Hash of "your taking too long :)" (tests lowercase and punctuation)
    {"your taking too long :)", "bd4036fce521f4bc977bc794aa32a2463b027f3c27b7edada912a5092147aaf6627e9d62e98f"},
    // Hash of "your taking too long :) IS TAKING TOO LONG" (longer input, mixed case)
    {"your taking too long :) IS TAKING TOO LONG", "4f9bb477302dcf484861c12015c5141bb3f699c228d6bc09fdb0d2d7e58903488297967a93ff"},
    // Hash of "YOUR TOO BRIGHT" (different phrase, regression test)
    {"YOUR TOO BRIGHT", "a59b95fa0d5f390cadd50a6ff8713ec0dff7d5f3711bd1f1f9fe8c91e674c115975e9afc6164"},
    // Hash of "YOUR TAKING TOO TOO" (repeated words, regression test)
    {"YOUR TAKING TOO TOO", "d6bf0031960e84a5ceb7131049e2744471b9604632f41fbf2603b2ccaca613be98265622af08"},
    // Hash of "YOUR         TOO TOO" (multiple spaces, repeated words)
    {"YOUR         TOO TOO", "dc21a8573bde65a50d54bf14a6bd51220352f8a2027e1f2cadfcdf56d28744aaa0014604217e"}
};

// Self-test function to verify the implementation
bool s19h::self_test() { // do not use this. do not use this hash in general actually wtf are you doing here
    for (const auto& test : test_vectors) {
        s19h hasher;
        hasher.update(reinterpret_cast<const uint8_t*>(test.input), std::strlen(test.input));
        std::string result = hasher.hexdigest();
        if (result != test.expected_hex) {
            return false; // Test failed
        }
    }
    return true; // All tests passed
}

// Reallow stage 2. If called as unlock(256), only reset the 256 variables. if called as unlock(304), only reset the 304 variables.
// To reset the state entirely call s19h::s19h() instead.
void s19h::unlock(int which = 0) {
    if (!(which % 256)) {
        cachedhash256.clear();
        stage2flag256 = true;
    }
    if (!(which % 304)) {
        cachedhash.clear();
        stage2flag = true;
    }
}