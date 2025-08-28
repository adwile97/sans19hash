#include "sans19hash.h"
#include <iomanip>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <sstream>

#include <limits> // compatibility

#define SANS19_HASH_SIZE 38
constexpr uint64_t SANS19_PRIME = 9953261ULL;
constexpr uint64_t SANS19_CONST = 0x772FAD1EULL;

// The big-endian writes because serialization

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

    ///////////////////////////
   // --------------------- //
  // --- Class methods --- //
 // --------------------- //
///////////////////////////

Sans19Hash::Sans19Hash() {
    state[0] = (SANS19_CONST * SANS19_CONST) & UINT64_MAX;
    state[1] = SANS19_PRIME;
    state[2] = ~SANS19_CONST & UINT64_MAX;
    state[3] = (SANS19_CONST * SANS19_PRIME) & UINT64_MAX;
    tail = 0;
    length = 0;
    kromer = 3;
    SPECIALATTACK = state[2] - 1;
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
        state[3] = colorpuzzle(state[3] ^ (b + i + ((kromer + 3) ^ SANS19_CONST)), SANS19_CONST ^ SANS19_PRIME ^ (kromer & 0x1F));

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
        write_be64(reinterpret_cast<uint8_t*>(buf), state[i]);
        out.append(buf, 8);
    }
    tail ^= (tail >> 48) ^ ((tail >> 56) << 8); // making the tail 6 bytes
    write_be48(reinterpret_cast<uint8_t*>(buf), tail);
    out.append(buf, 6);
    return out; // 32 + 6 = 38 bytes

}

std::string Sans19Hash::hexdigest() {
    std::string RAWdata = finalize();
    std::ostringstream oss;
    for (unsigned char c : RAWdata) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    return oss.str();
}

// Self-test function to verify the implementation
bool Sans19Hash::self_test() {
    struct TestVector {
        const char* input;
        const char* expected_hex;
    };

    static std::string copypasta = "Can y'all stop hating phonk? Seriously, half the time people trash the genre without even understanding what it is. They hear a distorted bassline or a gritty Memphis sample and immediately write it off as noise. But phonk isn't just a style—it's a vibe, a whole culture rooted in underground hip-hop, street racing, and raw emotion. It's the soundtrack of rebellion, of late nights drifting through neon-lit streets, of headphones blasting while the world fades away. From classic phonk with its lo-fi, eerie aesthetic to drift phonk with its aggressive energy, there's variety and depth if you take a second to actually listen. Artists pour their soul into this music, blending nostalgia, grit, and adrenaline into something that's more than just sound—it's atmosphere. Dismissing it just because it doesn't sound like chart-toppers or your favorite genre is lazy. Music is meant to challenge, to evoke, to make you feel something real, even if it's dark or chaotic. And phonk does exactly that. So before you keep riding the hate train, maybe dive a little deeper into the scene. Watch a drift montage synced to KSLV Noh or DVRST and feel the momentum. Because let's be real—you can't hate something you don't know.";
    // Request from my friend to add this specific test vector
    TestVector test_vectors[] = {
        {copypasta.c_str(), "d287fe1cdd5a0eeac660894d98ce04fdfdfa81512cd14218135b1b59fd8de42e59d517c8bf3f"},
        {"", "a257dd11eb709ae60046b5600e25dec95187b0696bc5d9f4d153fa777cece258000000000000"},
        {"sans19", "015bcf00a01f80d806b559e2b379dd10f69672a34716d18921a36d1067292d4504b07a88a0fd"},

        // Test vectors inspired by Jackenstein of DELTARUNE
        {"YOUR TAKING TOO LONG", "85005b25482225a830005bdf0b15ab4ffab32b4859147cf8cbcbfdc2ee407d8902a52b4a2f2a"},
        {"YOUR         LONG", "ec029b19b1ca738ac2372fd5654b0ae82a78e53c035ac57237f656016afbae65017b4554276c"},
        {"your taking too long :)", "3f3aaf9131bdf3166bd9b11ec82d10c1b004a8b52f05083aed8ddd5c0d5ef98b22b14c1749c5"},
        {"your taking too long :) IS TAKING TOO LONG", "359e2532348f9bcf221b6a8eca7f6dfe338a5d67eb188f2bfb727ece4ebc4ca20bd416436ddb"},
        {"YOUR TOO BRIGHT", "39551d87195b64e14905908fc7446931000a5cbcf89c3c92653a8e7f8e10f36d029097966dfc"},
        {"YOUR TAKING TOO TOO", "b5a66da0ff4e1418d743e121117dd2dfd44f3f59e6f0885c3a17bb11dabb4ce866242c8a20b9"},
        {"YOUR         TOO TOO", "8ee0775ec64acc1561613a90de9dab0ebe1b69a901ab9aeccaa6a1602796f14410055cd1e0a4"}
    };

    for (const auto& test : test_vectors) {
        Sans19Hash hasher;
        hasher.update(reinterpret_cast<const uint8_t*>(test.input), std::strlen(test.input));
        std::string result = hasher.hexdigest();
        if (result != test.expected_hex) {
            return false; // Test failed
        }
    }
    return true; // All tests passed
}