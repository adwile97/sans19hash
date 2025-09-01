#include "sans19hash.h"
#include <iomanip>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <sstream>

#include <limits> // compatibility

#define SANS19_HASH_SIZE 38
#define rotl64(bits,word) \
                (((word) << (bits)) | ((word) >> (64-(bits))))

constexpr uint64_t SANS19_PRIME = 9953261ULL;
constexpr uint64_t SANS19_CONST = 0x772FAD1EULL;

// The big-endian writes for serialization

inline void write_be64(uint8_t* out, uint64_t v) {
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

// Mixing functions

uint64_t colorpuzzle(uint64_t v, uint64_t k) {
    v ^= v >> 27;
    v ^= rotl64(31, v);
    v *= k;
    v ^= rotl64(k % 64, v);
    return v;
}

uint16_t blend4(uint64_t sans, uint64_t deltarune) {
    sans ^= deltarune >> 32;
    sans ^= sans >> 35;
    sans = ((sans ^ (sans >> (deltarune * sans % 32))) * ((sans << 7) | (sans % 10000)) & 0xFFFF);
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
}

void s19h::update(const uint8_t* data, size_t len) {
    length += len;
    kromer = (kromer + state[(tail ^ SPECIALATTACK) % 4] + len) % UINT16_MAX;

    for (size_t i = 0; i < len; ++i) {
        uint8_t b = data[i];

        // kromer for better avalanche
        uint64_t kmix = kromer ^ (state[3 - (i % 4)] << (i % 53));

        state[0] = colorpuzzle(state[0] ^ (b ^ (kmix & 0xFF)), SANS19_CONST + (kromer % 19));
        state[1] = colorpuzzle(state[1] + b + (kmix >> 8), SANS19_PRIME ^ (kromer & 0xF));
        state[2] = colorpuzzle(state[2] ^ (static_cast<uint64_t>(b + (kromer * 19)) << (i % 64)), SANS19_CONST + i + (kromer % 13));
        state[3] = colorpuzzle(state[3] ^ (b + i + ((kromer + 3) ^ SANS19_CONST)), SANS19_CONST ^ SANS19_PRIME ^ (kromer & 0x1F));

        mybrotherhasavery = blend4(state[(b ^ kmix) % 4], SANS19_PRIME);
        SPECIALATTACK = (SPECIALATTACK ^ (b + kromer)) & 0x0F;
        tail ^= (b | (mybrotherhasavery >> SPECIALATTACK));
    }
}


std::string s19h::finalize() {
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
    return out; // 38 bytes
}

std::string s19h::finalize256() {
    tail ^= length & 0xFFFF;
    tail = colorpuzzle(tail, SANS19_CONST ^ SANS19_PRIME);
    for (int cycles = 0; cycles < 19; ++cycles) {
        for (int i = 0; i < 4; ++i) {
            state[i] ^= length;
            state[i] = colorpuzzle(state[i], tail);
            tail <<= (state[i] >> (kromer * 17 % 64) % 64); // make the tail less of a constant mask
            tail ^= rotl64((kromer % 63) + i, state[i]);
    }
    }
    std::string out;
    char buf[8];
    for (int i = 0; i < 4; ++i) {
        write_be64(reinterpret_cast<uint8_t*>(buf), state[i]);
        out.append(buf, 8);
    }
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

// Self-test function to verify the implementation
bool s19h::self_test() {
    struct TestVector {
        const char* input;
        const char* expected_hex;
    };
    TestVector test_vectors[] = {
        {"", "6dca21463713586427a0c96f1fc654f63b0356954706e2c1268c1da51a48b28acecf42db9d9e"},
        {"sans19", "f63bfa1ca3eb70585c3c75bbc366ddf84042dbff8df66349746587e40692741c1b3cba7ccf87"},
        {"YOUR TAKING TOO LONG", "e0442e73331dc9b13eb56b36df6072e82f8c65cec10fb95dbee74480fa49fc69cc7db8e3c3ee"},
        {"YOUR         LONG", "24ac897fbb26a409d868bba5f4f66ef41a0b38a83e3b007b928dba600af03ddfcf40b7e14ca0"},
        {"your taking too long :)", "94488c6761fb5d880c5d9be31c88d89c5cad3bd3c8719da452f42aae150ce0e1558e54f4f8ec"},
        {"your taking too long :) IS TAKING TOO LONG", "5a1d3f72e8799023c1a5dfaf65ff7d4b430d3b369e538295ccc883547aecca46e9db6ce6e064"},
        {"YOUR TOO BRIGHT", "a97ce4d060da15c38623d166c281143245d537f4782e7f1c498ec2cf67371fa2d918c57618ca"},
        {"YOUR TAKING TOO TOO", "22b800309db316fba64ae6599301d0deaeaad5c758ff5e98b9d980010cd6353ee43078e687e7"},
        {"YOUR         TOO TOO", "36a6b694422fab47a9b3744b86f0bce35cb8adfd2f76f9149a979c6f46038c2c76fc95496f56"}
    };

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