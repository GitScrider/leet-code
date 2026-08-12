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
 * Complexity derivation (binary exponentiation + Euclid step count):
 *   Treat each 64-bit modular multiply / mod as O(1) (word-RAM model; the file
 *   caps m <= 2^31, so every product of two residues < m^2 fits in int64). The
 *   two constructions are analysed independently.
 *
 *   (1) Fermat via mod_pow(a, p-2, p). Let e = p - 2. The while loop runs once
 *       per bit of e: after iteration i the exponent equals floor(e / 2^i), and
 *       it halts when floor(e / 2^i) = 0, i.e. when 2^i > e. So the iteration
 *       count is L = floor(log2 e) + 1 = floor(log2(p-2)) + 1. Each iteration
 *       does one squaring plus (only if bit b_i = 1) one multiply, hence
 *           M(p) = SUM_{i=0}^{L-1} (1 + b_i) = L + popcount(e)
 *                <= 2L = 2*(floor(log2(p-2)) + 1) = O(log p).
 *
 *   (2) Extended Euclid ext_gcd(a0, m). Each call maps (a, b) -> (b, a mod b)
 *       with O(1) local work, giving the recurrence
 *           T(a, b) = T(b, a mod b) + O(1),   T(a, 0) = O(1)   (base case).
 *       By Lame's theorem the k-step worst case is consecutive Fibonacci inputs:
 *       k steps force b >= F_{k+1} ~ phi^(k+1)/sqrt5, phi = (1+sqrt5)/2, so
 *           b >= phi^(k-1)  =>  k <= log_phi(b) + 1 = O(log m).
 *       Total work = SUM over the k = O(log m) steps of O(1) = O(log m), matched
 *       by an O(log m)-deep recursion stack (the stated space).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f = O(g)      iff  EXISTS c2, n0 :        f(x) <= c2*g(x)  for x >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g(x) <= f(x)         for x >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   (1) Fermat: L = floor(log2(p-2)) + 1 is fixed by the bit length of p, not by
 *       the data, so with g = log p, (1/2)*log2 p <= L <= 2*log2 p for p >= 4
 *       =>  time is Theta(log p) (tight).
 *   (2) Extended Euclid is ADAPTIVE (step count depends on the operands):
 *       BEST  case (a0 = 1 or m mod a0 = 0) halts in O(1) steps  => Omega(1);
 *       WORST case (Fibonacci-adjacent operands) needs Theta(log m) steps.
 *       Over all inputs the time is thus O(log m) (upper, worst) and Omega(1)
 *       (lower, best), not a single Theta. Both routines are number-theoretic,
 *       not comparison sorts, so the Omega(n log n) comparison bound is moot.
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
    for (const std::int64_t a : {std::int64_t(2), std::int64_t(3),
                                 std::int64_t(123456), std::int64_t(999999999),
                                 P - 1}) {
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
