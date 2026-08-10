/*
 * Modular Multiplicative Inverse  --  Algorithm - Math / Number Theory
 *
 * Idea:
 *   The modular inverse of a modulo m is the value x with a*x == 1 (mod m).
 *   It exists if and only if gcd(a, m) == 1. Two classic constructions:
 *     (1) Fermat's Little Theorem (modulus m = p is PRIME): for gcd(a, p) = 1,
 *         a^(p-1) == 1 (mod p)  =>  a^(p-2) == a^{-1} (mod p).
 *         Evaluated with fast binary exponentiation (mod-pow).
 *     (2) Extended Euclidean Algorithm (ANY modulus m with gcd(a, m) = 1):
 *         it finds integers x, y with a*x + m*y = gcd(a, m). When the gcd is 1
 *         this gives a*x == 1 (mod m), so x (normalized) is the inverse.
 *
 * Complexity:
 *   +-----------------------------+------------------+
 *   | Operation                   | Time             |
 *   +-----------------------------+------------------+
 *   | Fermat inverse (mod-pow)    | O(log p)         |
 *   | Extended Euclid inverse     | O(log m)         |
 *   +-----------------------------+------------------+
 *   Space: O(log m) recursion for ext_gcd, O(1) otherwise.
 *
 * Key points / assumptions:
 *   - Fermat's method REQUIRES a prime modulus; Extended Euclid is the general
 *     tool and also reports the non-invertible case (gcd != 1).
 *   - Inverse exists iff gcd(a, m) == 1.
 *   - Overflow safety: we keep the modulus <= ~2^31, so every intermediate
 *     product of two residues (each < m) is < m^2 <= 2^62 < 2^63-1 and fits
 *     safely in std::int64_t. (For a modulus near 2^63 one would switch the
 *     multiplications to __int128; here we deliberately stay in the safe range.)
 */

#include <cstdint>
#include <cassert>
#include <iostream>
#include <initializer_list>

// Fast binary exponentiation: base^exp mod m.
// Each product multiplies two residues < m, so it stays < m^2 <= 2^62 when
// m <= 2^31; this fits in std::int64_t and cannot overflow.
static std::int64_t mod_pow(std::int64_t base, std::int64_t exp, std::int64_t mod) {
    base %= mod;
    if (base < 0) base += mod;           // keep base in [0, mod)
    std::int64_t result = 1 % mod;       // 1 % mod also handles mod == 1
    while (exp > 0) {
        if (exp & 1) result = (result * base) % mod;
        base = (base * base) % mod;
        exp >>= 1;
    }
    return result;
}

// Inverse via Fermat's Little Theorem. Precondition: p is prime and a % p != 0.
static std::int64_t inverse_fermat(std::int64_t a, std::int64_t p) {
    return mod_pow(a, p - 2, p);         // a^(p-2) mod p
}

// Extended Euclid: returns g = gcd(a, b) and sets x, y with a*x + b*y = g.
static std::int64_t ext_gcd(std::int64_t a, std::int64_t b,
                            std::int64_t& x, std::int64_t& y) {
    if (b == 0) {                        // base case: gcd(a, 0) = a, coefficients (1, 0)
        x = 1;
        y = 0;
        return a;
    }
    std::int64_t x1, y1;
    const std::int64_t g = ext_gcd(b, a % b, x1, y1);
    // Back-substitute Bezout coefficients.
    x = y1;
    y = x1 - (a / b) * y1;
    return g;
}

// General inverse via Extended Euclid. Returns false when gcd(a, m) != 1
// (no inverse exists); otherwise writes the inverse normalized to [0, m).
static bool inverse_euclid(std::int64_t a, std::int64_t m, std::int64_t& out) {
    std::int64_t x, y;
    const std::int64_t a0 = ((a % m) + m) % m;     // normalize a into [0, m)
    const std::int64_t g = ext_gcd(a0, m, x, y);
    if (g != 1) return false;            // not coprime -> not invertible
    out = ((x % m) + m) % m;             // normalize the coefficient into [0, m)
    return true;
}

int main() {
    // ---- Fermat's method against several small primes ------------------------
    for (const std::int64_t p : {2, 3, 5, 7, 13, 97, 101}) {
        for (std::int64_t a = 1; a < p; ++a) {       // every a in [1, p-1] is invertible
            const std::int64_t inv = inverse_fermat(a, p);
            assert((inv * a) % p == 1);              // definition of the inverse
            std::int64_t inv2;
            assert(inverse_euclid(a, p, inv2));      // Euclid agrees with Fermat
            assert(inv2 == inv);
        }
    }

    // ---- Known values --------------------------------------------------------
    assert(inverse_fermat(3, 7) == 5);   // 3 * 5 = 15 == 1 (mod 7)
    assert(inverse_fermat(1, 13) == 1);  // inverse of 1 is always 1
    {
        std::int64_t inv;
        assert(inverse_euclid(3, 11, inv) && inv == 4);   // 3 * 4 = 12 == 1 (mod 11)
    }

    // ---- Large prime modulus (< 2^31, so 64-bit products stay safe) ----------
    const std::int64_t P = 1000000007;   // 1e9+7, a prime < 2^31
    for (const std::int64_t a : {2, 3, 123456, 999999999, P - 1}) {
        const std::int64_t inv = inverse_fermat(a, P);
        assert((inv * a) % P == 1);
        std::int64_t inv2;
        assert(inverse_euclid(a, P, inv2) && inv2 == inv);
    }

    // ---- Non-invertible cases must be detected (gcd != 1) --------------------
    {
        std::int64_t inv;
        assert(!inverse_euclid(4, 6, inv));   // gcd(4, 6) = 2
        assert(!inverse_euclid(6, 9, inv));   // gcd(6, 9) = 3
        assert(!inverse_euclid(0, 5, inv));   // 0 has no inverse
    }

    // ---- Short demo ----------------------------------------------------------
    std::cout << "Modular inverse demo\n";
    std::cout << "  3^{-1} mod 7   (Fermat) = " << inverse_fermat(3, 7) << "\n";
    std::cout << "  3^{-1} mod 11  (Euclid) = 4 (checked)\n";
    std::int64_t big;
    inverse_euclid(123456, P, big);
    std::cout << "  123456^{-1} mod 1e9+7   = " << big << "\n";
    std::cout << "  inverse of 4 mod 6      = none (gcd = 2)\n";
    std::cout << "All modular-inverse assertions passed.\n";
    return 0;
}
