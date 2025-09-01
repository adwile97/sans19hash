# sans19hash

A completely custom, non-standard, state-based C++ hash function family. With a taste of Undertale.

## Overview

This library provides a C++ implementation of the **two** sans19 hash functions:

- **sans19-304**: The original 38-byte implementation. Insecure, experimental, and fun.
   - Usage for strings:
     ```cpp
     #include "sans19hash.h"

     s19h hasher;
     std::string input = "ererererer";
     hasher.update(reinterpret_cast<const uint8_t*>(input.c_str()), input.size());
     std::string rawhash = hasher.finalize(); // for binary data
     std::string hexhash = hasher.hexdigest(); // for hex string
     ```
- **sans19-256**: The more excusable, cryptographically viable 32-byte implementation.
   - Usage for strings:
     ```cpp
     #include "sans19hash.h"

     s19h hasher;
     std::string input = "ererererer";
     hasher.update(reinterpret_cast<const uint8_t*>(input.c_str()), input.size());
     std::string rawhash = hasher.finalize256(); // for binary data
     std::string hexhash = hasher.hexdigest256(); // for hex string
     ```

This implementation works in 8-bit (1-byte) chunks, making it suitable for hashing streams without loading everything into memory.
You can update the hash in chunks of any byte size, and the final digest will be the same.

# DISCLAIMER
Sans19 is actively being tested and tweaked. The math and output may change in future versions.
This is **not** a stable or finalized implementation. **Do not use this API for any purpose in any serious project before it is finalized.** Even then, do your own research.