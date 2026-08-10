/*
 * Deterministic Miller-Rabin Primality Test  --  Algorithm - Math / Number Theory
 *
 * Idea:
 *   Write n - 1 = d * 2^s with d odd. For a prime n and any base a coprime to n,
 *   Fermat says a^(n-1) == 1 (mod n). Miller-Rabin sharpens this: the only square
 *   roots of 1 modulo a prime are +-1, so the sequence
 *       a^d, a^(2d), a^(4d), ..., a^(2^(s-1) d)  (mod n)
 *   must either start at 1 or hit n-1 at some step. If it does neither, n is
 *   composite and a is a "witness". For 64-bit n the FIXED base set
 *   {2,3,5,7,11,13,17,19,23,29,31,37} is proven to give ZERO false positives
 *   (it is deterministic for every n < 3.3 * 10^24, covering all of uint64).
 *
 * Complexity:
 *   +-----------------------------+---------------------------+
 *   | Operation                   | Time                      |
 *   +-----------------------------+---------------------------+
 *   | is_prime(n)                 | O(k log^2 n), k=12 bases  |
 *   +-----------------------------+---------------------------+
 *   Space: O(1).
 *
 * Complexity derivation (bases * exponent length * multiply cost):
 *   Fix the witness-set size k = 12. Factor n - 1 = d * 2^s; since 2^s <= n - 1
 *   we have s <= log2 n and d < n. For EACH base a the work is:
 *     - pow_mod(a, d, n): binary exponentiation runs floor(log2 d) + 1 iterations,
 *       each a squaring plus at most one multiply, i.e. SUM_{i} O(1) mul_mod
 *       = O(log d) = O(log n) modular multiplications;
 *     - witness loop: at most s - 1 < log2 n further squarings = O(log n) mul_mod.
 *   So each base issues O(log n) + O(log n) = O(log n) modular multiplications.
 *   Cost of ONE mul_mod:
 *     - portable fallback: a Russian-peasant loop over the bits of b (b < n),
 *       i.e. floor(log2 b) + 1 = O(log n) add/shift steps  ->  O(log n);
 *     - __int128 fast path: one 128-bit multiply plus one mod  ->  O(1).
 *   Multiplying the three factors in the portable model (the stated bound):
 *       C(n) = k * (O(log n) mul_mod per base) * (O(log n) per mul_mod)
 *            = O(k log^2 n).
 *   On the __int128 path each mul_mod is O(1), dropping one log to O(k log n).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Definitions: f = O(g) iff f <= c2*g; f = Omega(g) iff f >= c1*g; f = Theta(g)
 *   iff both, for positive c1, c2 and all n >= n0.
 *   The base count k = 12 is a CONSTANT, and the exponent length floor(log2 d)+1
 *   and round count s are pinned by n's bit pattern, so the operation count is
 *   fixed to within constant factors. Every call past the O(1) small-prime screen
 *   runs at least the base-2 pow_mod (a full O(log n)-length exponentiation), so
 *   C(n) = Omega(log^2 n); at most k bases with their witness loops run, so
 *   C(n) = O(k log^2 n). As k is constant (1 <= bases executed <= 12) these meet:
 *       C(n) = Theta(log^2 n)   (portable model; Theta(log n) with __int128).
 *   Miller-Rabin is an exponentiation/divisibility test, not a comparison sort,
 *   so the Omega(n log n) comparison lower bound is irrelevant.
 *
 * Key points / assumptions:
 *   - Deterministic for the entire 64-bit range with the fixed 12-base set.
 *   - Safe modular multiplication is essential: a*b with a,b < n can overflow
 *     64 bits. We use __int128 when the compiler provides it (guarded below);
 *     otherwise a portable Russian-peasant fallback that only ever ADDS two
 *     residues, which is exact for any modulus < 2^63.
 *   - Small n and small prime multiples are handled before the main loop.
 */

#include <cstdint>
#include <cassert>
#include <iostream>
#include <initializer_list>

// Safe (a * b) mod mod without 64-bit overflow.
static std::uint64_t mul_mod(std::uint64_t a, std::uint64_t b, std::uint64_t mod) {
#if defined(__SIZEOF_INT128__)
    // Fast path: the 128-bit intermediate cannot overflow for any 64-bit mod.
    return static_cast<std::uint64_t>((static_cast<__uint128_t>(a) * b) % mod);
#else
    // Portable fallback: binary (Russian-peasant) multiplication modulo mod.
    // Every step only doubles a or adds a to result, each kept in [0, mod);
    // since two residues sum to < 2*mod, this is exact for any mod < 2^63.
    a %= mod;
    b %= mod;
    std::uint64_t result = 0;
    while (b > 0) {
        if (b & 1) {
            result += a;
            if (result >= mod) result -= mod;
        }
        a <<= 1;
        if (a >= mod) a -= mod;
        b >>= 1;
    }
    return result;
#endif
}

// (base^exp) mod mod using the safe multiplication above.
static std::uint64_t pow_mod(std::uint64_t base, std::uint64_t exp, std::uint64_t mod) {
    std::uint64_t result = 1 % mod;      // handles mod == 1 defensively
    base %= mod;
    while (exp > 0) {
        if (exp & 1) result = mul_mod(result, base, mod);
        base = mul_mod(base, base, mod);
        exp >>= 1;
    }
    return result;
}

// Deterministic Miller-Rabin for all 64-bit n.
static bool is_prime(std::uint64_t n) {
    if (n < 2) return false;
    // The witness set doubles as a small-prime screen: a multiple of one of
    // these primes is prime only if it equals that prime.
    for (const std::uint64_t p : {2ull, 3ull, 5ull, 7ull, 11ull, 13ull,
                                  17ull, 19ull, 23ull, 29ull, 31ull, 37ull}) {
        if (n % p == 0) return n == p;
    }
    // Here n > 37 and is coprime to every witness, so each base a satisfies a < n.

    // Factor n - 1 = d * 2^s with d odd.
    std::uint64_t d = n - 1;
    int s = 0;
    while ((d & 1) == 0) { d >>= 1; ++s; }

    for (const std::uint64_t a : {2ull, 3ull, 5ull, 7ull, 11ull, 13ull,
                                  17ull, 19ull, 23ull, 29ull, 31ull, 37ull}) {
        std::uint64_t x = pow_mod(a, d, n);
        if (x == 1 || x == n - 1) continue;          // this base is inconclusive (probably prime)
        bool composite = true;
        for (int r = 1; r < s; ++r) {                // repeatedly square, looking for -1
            x = mul_mod(x, x, n);
            if (x == n - 1) { composite = false; break; }
        }
        if (composite) return false;                 // a is a witness: n is composite
    }
    return true;
}

// Brute-force reference used to validate the fast test on small inputs.
static bool is_prime_trial(std::uint64_t n) {
    if (n < 2) return false;
    for (std::uint64_t d = 2; d * d <= n; ++d)
        if (n % d == 0) return false;
    return true;
}

int main() {
    // ---- Agreement with trial division for every n < 10000 -------------------
    for (std::uint64_t n = 0; n < 10000; ++n)
        assert(is_prime(n) == is_prime_trial(n));

    // ---- Tiny edge cases -----------------------------------------------------
    assert(!is_prime(0));
    assert(!is_prime(1));
    assert(is_prime(2));
    assert(is_prime(3));
    assert(!is_prime(4));

    // ---- Carmichael numbers: composite yet Fermat-pseudoprime to many bases --
    assert(!is_prime(561));    // 3 * 11 * 17
    assert(!is_prime(1105));   // 5 * 13 * 17
    assert(!is_prime(1729));   // 7 * 13 * 19

    // ---- Known large primes (all < 2^63, safe for both mul_mod paths) --------
    for (const std::uint64_t p : {1000000007ull,          // 1e9+7
                                  1000000009ull,          // 1e9+9
                                  998244353ull,           // common NTT prime
                                  2147483647ull,          // 2^31 - 1 (Mersenne)
                                  4294967291ull,          // largest prime < 2^32
                                  67280421310721ull,       // prime factor of 2^64 + 1
                                  2305843009213693951ull}) // 2^61 - 1 (Mersenne)
        assert(is_prime(p));

    // ---- Known large composites ----------------------------------------------
    const std::uint64_t comp = 1000000007ull * 1000000009ull;  // product of two primes
    assert(!is_prime(comp));
    assert(!is_prime(2147483647ull * 3ull));                   // odd composite
    assert(!is_prime(4294967291ull + 2ull));                   // 4294967293 = 9241 * 464773

    // ---- Short demo ----------------------------------------------------------
    std::cout << "Deterministic Miller-Rabin demo\n";
    std::cout << "  is_prime(2^61 - 1)          = "
              << (is_prime(2305843009213693951ull) ? "true" : "false") << "\n";
    std::cout << "  is_prime(1e9+7 * 1e9+9)     = "
              << (is_prime(comp) ? "true" : "false") << "\n";
    std::cout << "  is_prime(561) [Carmichael]  = "
              << (is_prime(561) ? "true" : "false") << "\n";
    std::cout << "All Miller-Rabin assertions passed.\n";
    return 0;
}
