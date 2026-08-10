/*
 * Hash Functions -- Data Structure - Hashing
 * -----------------------------------------------------------------------------
 * Summary:
 *   An educational tour of common NON-cryptographic hash functions used to map
 *   keys (strings and integers) into a small, uniformly distributed range of
 *   bucket indices for hash tables. These are fast "mixing" functions, NOT
 *   secure digests -- do not use them where collision resistance matters.
 *
 * Operations & complexity:
 *   +----------------------------+------------------+---------------------------+
 *   | Function                   | Cost (len L key) | Notes                     |
 *   +----------------------------+------------------+---------------------------+
 *   | djb2 (string)              | O(L)             | one multiply/xor per byte |
 *   | FNV-1a 32-bit (string)     | O(L)             | xor-then-multiply         |
 *   | polynomial rolling (string)| O(L)             | base^i accumulation       |
 *   | splitmix64 mix (integer)   | O(1)             | fixed number of ops       |
 *   +----------------------------+------------------+---------------------------+
 *   A good hash spreads keys evenly so a hash table keeps average O(1) lookups;
 *   a degenerate hash collapses keys into few buckets, degrading ops to O(n).
 *
 * Invariants / key ideas:
 *   * Determinism: hashing the same input always yields the same value. This is
 *     the ONE hard requirement -- a table relies on re-finding a key's bucket.
 *   * Avalanche: flipping one input bit should flip about half the output bits,
 *     so similar keys ("item-1", "item-2") scatter instead of clustering.
 *   * Unsigned overflow is well-defined in C++ (modulo 2^bits), so we lean on it
 *     for the "wrap-around" arithmetic every rolling/multiplicative hash needs.
 *
 * When to use / trade-offs:
 *   * djb2 / FNV-1a: tiny, fast, great default string hashes for hash tables.
 *   * Polynomial rolling: enables O(1) substring rehashing (Rabin-Karp) but is
 *     easy to attack with crafted keys unless the base/modulus are randomized.
 *   * Integer mixers: turn low-entropy ints (0,1,2,...) into scattered values;
 *     essential because raw ints modulo a power-of-two table size cluster badly.
 *
 * Memory: purely value-based arithmetic; no owning pointers, so the Rule of Zero
 * applies -- there are no special members to hand-write here.
 */

#include <cassert>
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

// -----------------------------------------------------------------------------
// djb2 (Daniel J. Bernstein). Idea: start from the "magic" seed 5381 and, for
// each byte, do hash = hash * 33 ^ c. The odd multiplier 33 mixes bits cheaply
// (a shift-add on many CPUs) while the xor folds the new byte in. Empirically
// excellent for short ASCII strings; a classic default for symbol tables.
// -----------------------------------------------------------------------------
std::uint32_t djb2(const std::string& s) {
    std::uint32_t hash = 5381u;
    for (unsigned char c : s) {
        // Well-defined unsigned overflow keeps this within 32 bits.
        hash = hash * 33u ^ c;
    }
    return hash;
}

// -----------------------------------------------------------------------------
// FNV-1a, 32-bit (Fowler-Noll-Vo, "1a" variant). Idea: xor the byte in FIRST,
// then multiply by the FNV prime. Doing the xor before the multiply gives
// better avalanche than the original FNV-1. Widely used for hash tables and
// checksums because it is branch-free and extremely fast.
// -----------------------------------------------------------------------------
std::uint32_t fnv1a_32(const std::string& s) {
    constexpr std::uint32_t kOffsetBasis = 2166136261u; // FNV offset basis
    constexpr std::uint32_t kPrime        = 16777619u;   // FNV 32-bit prime
    std::uint32_t hash = kOffsetBasis;
    for (unsigned char c : s) {
        hash ^= c;          // fold byte in first ...
        hash *= kPrime;     // ... then diffuse via multiply (mod 2^32).
    }
    return hash;
}

// -----------------------------------------------------------------------------
// Polynomial rolling hash for strings. Idea: treat the string as digits of a
// number in base B: h = c0*B^(n-1) + c1*B^(n-2) + ... + c(n-1). Accumulated via
// Horner's rule (h = h*B + c) in a 64-bit unsigned integer, so overflow acts as
// an implicit modulus 2^64. A prime-ish base like 131 spreads letters well.
// This is the backbone of Rabin-Karp substring search, where the "rolling"
// property lets you update the hash in O(1) as a window slides.
// -----------------------------------------------------------------------------
std::uint64_t poly_rolling(const std::string& s) {
    constexpr std::uint64_t kBase = 131u;
    std::uint64_t hash = 0u;
    for (unsigned char c : s) {
        // +1 so trailing/leading spaces and repeated chars still shift the value.
        hash = hash * kBase + (static_cast<std::uint64_t>(c) + 1u);
    }
    return hash;
}

// -----------------------------------------------------------------------------
// Integer mixer: the splitmix64 finalizer. Idea: raw integer keys (0,1,2,...)
// have almost no entropy in their high bits, so hashing "key % tableSize"
// clusters terribly for power-of-two sizes. This finalizer runs the value
// through a sequence of xor-shift and odd-constant multiplies that "avalanche"
// every input bit across the full 64-bit output. (Knuth's multiplicative hash,
// key * 2654435761u, is the simpler cousin of the same idea.)
// -----------------------------------------------------------------------------
std::uint64_t splitmix64_mix(std::uint64_t x) {
    x += 0x9E3779B97F4A7C15ull;                 // golden-ratio increment
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull; // diffuse high->low
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBull; // diffuse again
    x =  x ^ (x >> 31);                          // final fold
    return x;
}

// -----------------------------------------------------------------------------
// Tests + demo
// -----------------------------------------------------------------------------
int main() {
    // --- (1) Determinism: same input -> same output, for EVERY function. -----
    const std::string sample = "hash-me";
    assert(djb2(sample)          == djb2(sample));
    assert(fnv1a_32(sample)      == fnv1a_32(sample));
    assert(poly_rolling(sample)  == poly_rolling(sample));
    assert(splitmix64_mix(42u)   == splitmix64_mix(42u));

    // Determinism must hold for integers across the whole range we care about.
    for (std::uint64_t i = 0; i < 1000; ++i) {
        assert(splitmix64_mix(i) == splitmix64_mix(i));
    }

    // --- (2) Distinctness: a handful of clearly different strings must not ---
    //         collide under djb2 or FNV-1a (collisions are possible in general,
    //         but not for this tiny, well-separated set).
    const std::vector<std::string> words = {
        "apple", "banana", "cherry", "date", "elderberry",
        "fig", "grape", "honeydew", "kiwi", "lemon"
    };
    for (std::size_t i = 0; i < words.size(); ++i) {
        for (std::size_t j = i + 1; j < words.size(); ++j) {
            assert(djb2(words[i])     != djb2(words[j]));
            assert(fnv1a_32(words[i]) != fnv1a_32(words[j]));
        }
    }

    // Sanity: near-identical keys should avalanche to very different values.
    assert(fnv1a_32("item-1") != fnv1a_32("item-2"));
    assert(djb2("item-1")     != djb2("item-2"));

    // --- (3) Distribution test -----------------------------------------------
    //     Generate N deterministic keys and bucket them into M slots with one
    //     function; a healthy hash keeps every bucket near the average N/M.
    constexpr std::size_t N = 10000; // number of keys
    constexpr std::size_t M = 1024;  // number of buckets
    std::vector<std::size_t> buckets(M, 0);

    for (std::size_t i = 0; i < N; ++i) {
        const std::string key = "item-" + std::to_string(i);
        const std::size_t idx = static_cast<std::size_t>(fnv1a_32(key)) % M;
        ++buckets[idx];
    }

    std::size_t maxLoad = 0;
    std::size_t emptyBuckets = 0;
    for (std::size_t load : buckets) {
        maxLoad = std::max(maxLoad, load);
        if (load == 0) ++emptyBuckets;
    }
    const double avgLoad = static_cast<double>(N) / static_cast<double>(M);

    // Expected average is N/M ~= 9.77. A well-behaved hash stays comfortably
    // below 40 in its worst bucket; a degenerate hash would spike far higher.
    assert(maxLoad < 40);
    // A good hash also should not leave a large fraction of buckets empty.
    assert(emptyBuckets < M / 4);

    // --- Human-readable demo -------------------------------------------------
    std::cout << "Hash function demo\n";
    std::cout << "------------------\n";
    std::cout << "djb2(\"hello\")          = " << djb2("hello")         << '\n';
    std::cout << "fnv1a_32(\"hello\")      = " << fnv1a_32("hello")     << '\n';
    std::cout << "poly_rolling(\"hello\")  = " << poly_rolling("hello") << '\n';
    std::cout << "splitmix64_mix(12345)  = " << splitmix64_mix(12345)  << '\n';
    std::cout << '\n';
    std::cout << "Distribution of " << N << " keys into " << M << " buckets (FNV-1a):\n";
    std::cout << "  average bucket load = " << avgLoad << '\n';
    std::cout << "  maximum bucket load = " << maxLoad << '\n';
    std::cout << "  empty buckets       = " << emptyBuckets << " of " << M << '\n';
    std::cout << "\nAll assertions passed.\n";

    return 0;
}
