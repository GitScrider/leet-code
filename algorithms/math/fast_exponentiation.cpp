/*
 * Algorithm: Fast (Binary) Exponentiation  (Math / Number Theory)
 *
 * Idea:
 *   Any exponent b can be written in binary. Squaring the base repeatedly
 *   produces base^(2^k), and we multiply into the result exactly for the bits
 *   of b that are set:  base^b = prod over set bits k of base^(2^k).
 *   This turns O(b) multiplications into O(log b).
 *
 *   Modular power a^b mod m applies the same recurrence but reduces after every
 *   multiplication. The dangerous step is the product of two values up to m-1,
 *   which can reach ~m^2; see mulmod below for the overflow-safe handling.
 *
 * Complexity:
 *   +----------------+-----------------+
 *   | Operation      | Time            |
 *   +----------------+-----------------+
 *   | ipow / modpow  | O(log exp)      |
 *   +----------------+-----------------+
 *   Space: O(1).
 *
 * Key points / assumptions:
 *   - Integer ipow: the caller keeps base/exp small enough that base^exp fits
 *     in int64; we avoid a final useless squaring so we don't overflow past the
 *     last needed value.
 *   - mulmod: with __int128 available we compute a*b in 128 bits, so any
 *     modulus up to 2^63-1 is safe. Without it, the fallback (a%m)*(b%m) is
 *     only safe when m <= ~2^32 so the 64-bit product cannot overflow.
 *   - modpow handles exp == 0 (result 1 mod m) and m == 1 (result 0).
 */

#include <cstdint>
#include <cassert>
#include <iostream>

// Integer binary exponentiation. base^exp in int64.
// Overflow: the result must fit in int64; we skip the last squaring so 'base'
// is never squared beyond what the remaining bits actually use.
static std::int64_t ipow(std::int64_t base, unsigned exp) {
    std::int64_t result = 1;
    while (exp > 0) {
        if (exp & 1u) result *= base;
        exp >>= 1;
        if (exp > 0) base *= base; // avoid a final unused (possibly overflowing) square
    }
    return result;
}

// Overflow-safe modular multiplication: (a * b) mod m.
static std::uint64_t mulmod(std::uint64_t a, std::uint64_t b, std::uint64_t m) {
#ifdef __SIZEOF_INT128__
    // 128-bit intermediate: a*b cannot overflow, so any m < 2^63 is safe.
    return static_cast<std::uint64_t>(
        (static_cast<__uint128_t>(a) * static_cast<__uint128_t>(b)) % m);
#else
    // Portable fallback. Safe only when m <= ~2^32 so that (a%m)*(b%m) < 2^64.
    return (a % m) * (b % m) % m;
#endif
}

// Modular binary exponentiation: a^b mod m.
static std::uint64_t modpow(std::uint64_t base, std::uint64_t exp, std::uint64_t mod) {
    if (mod == 1) return 0;              // everything is 0 mod 1
    std::uint64_t result = 1 % mod;      // handles mod == 1 already returned; general 1
    base %= mod;
    while (exp > 0) {
        if (exp & 1u) result = mulmod(result, base, mod);
        exp >>= 1;
        if (exp > 0) base = mulmod(base, base, mod);
    }
    return result;
}

// Naive reference: multiply 'exp' times, reducing each step. O(exp).
static std::uint64_t modpow_naive(std::uint64_t base, std::uint64_t exp, std::uint64_t mod) {
    std::uint64_t r = 1 % mod;
    base %= mod;
    for (std::uint64_t i = 0; i < exp; ++i) r = mulmod(r, base, mod);
    return r;
}

int main() {
    // --- Known integer powers ---
    assert(ipow(2, 10) == 1024);
    assert(ipow(3, 13) == 1594323);
    assert(ipow(5, 0) == 1);      // anything^0 = 1
    assert(ipow(1, 1000000) == 1);
    assert(ipow(2, 62) == (std::int64_t)1 << 62); // large but fits in int64

    // --- Known modular powers ---
    assert(modpow(2, 10, 1000) == 24);   // 1024 mod 1000
    assert(modpow(3, 13, 1) == 0);       // mod 1 -> 0
    assert(modpow(7, 0, 13) == 1);       // exp 0 -> 1
    assert(modpow(2, 0, 1) == 0);        // exp 0 but mod 1 -> 0

    // --- Fermat sanity: for prime p, a^(p-1) == 1 (mod p) when gcd(a,p)=1 ---
    assert(modpow(2, 12, 13) == 1);
    assert(modpow(3, 16, 17) == 1);

    // --- modpow matches the naive loop across many small cases ---
    for (std::uint64_t a = 0; a <= 20; ++a) {
        for (std::uint64_t b = 0; b <= 20; ++b) {
            for (std::uint64_t m = 1; m <= 20; ++m) {
                assert(modpow(a, b, m) == modpow_naive(a, b, m));
            }
        }
    }

    // --- Large modulus exercises mulmod safety (products near m^2) ---
    const std::uint64_t P = 1000000007ULL; // fits the portable fallback (< 2^32)
    assert(modpow(2, 1000000, P) == modpow_naive(2 % P, 1000000, P));
    assert(modpow(P - 1, 2, P) == 1); // (-1)^2 == 1 (mod P)

    // --- Short demo ---
    std::cout << "2^10            = " << ipow(2, 10) << '\n';
    std::cout << "3^13            = " << ipow(3, 13) << '\n';
    std::cout << "modpow(2,10,1000) = " << modpow(2, 10, 1000) << '\n';
    std::cout << "modpow(2,1000000," << P << ") = " << modpow(2, 1000000, P) << '\n';
    std::cout << "All fast-exponentiation tests passed.\n";
    return 0;
}
