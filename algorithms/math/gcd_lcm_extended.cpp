/*
 * Algorithm: GCD, LCM & Extended Euclidean Algorithm  (Math / Number Theory)
 *
 * Idea:
 *   - Euclidean algorithm: gcd(a, b) = gcd(b, a mod b). Any common divisor of
 *     a and b divides the remainder r = a - floor(a/b)*b, and any divisor of
 *     b and r divides a; hence the pairs share the same divisors. Recursion
 *     ends at gcd(a, 0) = a.
 *   - LCM via the identity  lcm(a, b) * gcd(a, b) = |a * b|.  We evaluate it as
 *     (a / gcd) * b, dividing BEFORE multiplying so the intermediate value
 *     stays bounded by lcm and never overflows the way a * b could.
 *   - Extended Euclidean algorithm returns (g, x, y) with  a*x + b*y = g
 *     (Bezout's identity). This is the engine behind modular inverses and the
 *     solution of linear Diophantine equations.
 *
 * Complexity:
 *   +-------------------+---------------------+
 *   | Operation         | Time                |
 *   +-------------------+---------------------+
 *   | gcd / ext_gcd     | O(log min(a, b))    |
 *   | lcm               | O(log min(a, b))    |
 *   +-------------------+---------------------+
 *   Space: O(1) for the iterative gcd, O(log) recursion stack otherwise.
 *
 * Key points / assumptions:
 *   - Inputs are treated as non-negative for clarity; gcd only depends on
 *     magnitudes since gcd(a, b) = gcd(|a|, |b|).
 *   - lcm computed as (a / gcd) * b to avoid overflow of the product a * b.
 *   - Bezout coefficients x, y may be negative, so signed std::int64_t is used.
 */

#include <cstdint>
#include <cassert>
#include <iostream>

// Iterative Euclidean gcd. O(1) space, no recursion depth concerns.
static std::int64_t gcd_iter(std::int64_t a, std::int64_t b) {
    while (b != 0) {
        const std::int64_t r = a % b;
        a = b;
        b = r;
    }
    return a; // gcd(a, 0) = a
}

// Recursive Euclidean gcd (same result, tail-recursive form).
static std::int64_t gcd_rec(std::int64_t a, std::int64_t b) {
    return b == 0 ? a : gcd_rec(b, a % b);
}

// lcm using divide-before-multiply. lcm(0, x) is defined here as 0.
static std::int64_t lcm(std::int64_t a, std::int64_t b) {
    if (a == 0 || b == 0) return 0;
    return (a / gcd_iter(a, b)) * b; // divide first -> intermediate == lcm, no a*b overflow
}

// Result of the extended Euclidean algorithm: a*x + b*y = g, with g = gcd(a, b).
struct ExtGcd {
    std::int64_t g;
    std::int64_t x;
    std::int64_t y;
};

// Extended Euclidean algorithm.
// Base case: gcd(a, 0) = a with a*1 + 0*0 = a, so (g, x, y) = (a, 1, 0).
// Inductive step: from b*x1 + (a mod b)*y1 = g and (a mod b) = a - (a/b)*b,
// substitute to get a*y1 + b*(x1 - (a/b)*y1) = g.
static ExtGcd ext_gcd(std::int64_t a, std::int64_t b) {
    if (b == 0) return {a, 1, 0};
    const ExtGcd sub = ext_gcd(b, a % b);
    return {sub.g, sub.y, sub.x - (a / b) * sub.y};
}

// Brute-force reference gcd for tiny inputs: largest d dividing both a and b.
static std::int64_t gcd_brute(std::int64_t a, std::int64_t b) {
    std::int64_t best = 1;
    const std::int64_t hi = (a < b ? a : b);
    for (std::int64_t d = 1; d <= hi; ++d)
        if (a % d == 0 && b % d == 0) best = d;
    return best;
}

int main() {
    // --- Known gcd values ---
    assert(gcd_iter(48, 18) == 6);
    assert(gcd_rec(48, 18) == 6);
    assert(gcd_iter(17, 5) == 1);   // coprime
    assert(gcd_iter(0, 7) == 7);    // gcd(0, n) = n
    assert(gcd_iter(7, 0) == 7);
    assert(gcd_iter(13, 13) == 13); // equal inputs
    assert(gcd_iter(1071, 462) == 21);

    // --- Iterative vs recursive vs brute force agree on small pairs ---
    for (std::int64_t a = 0; a <= 30; ++a) {
        for (std::int64_t b = 0; b <= 30; ++b) {
            const std::int64_t gi = gcd_iter(a, b);
            assert(gi == gcd_rec(a, b));
            // Brute force is only defined for strictly positive pairs; the
            // zero cases are covered by the known-value asserts above.
            if (a > 0 && b > 0) assert(gi == gcd_brute(a, b));
        }
    }

    // --- Known lcm values, with divide-before-multiply overflow safety ---
    assert(lcm(4, 6) == 12);
    assert(lcm(21, 6) == 42);
    assert(lcm(0, 5) == 0);
    // Consecutive integers are coprime (gcd == 1), so their lcm is the full
    // product. Here a * b = 999999999000000000 overflows 32-bit arithmetic but
    // fits in int64; divide-before-multiply keeps every intermediate in range.
    assert(lcm(1000000000LL, 999999999LL) == 999999999000000000LL);

    // --- Extended Euclidean identity a*x + b*y == g for many pairs ---
    for (std::int64_t a = 0; a <= 40; ++a) {
        for (std::int64_t b = 0; b <= 40; ++b) {
            const ExtGcd e = ext_gcd(a, b);
            assert(e.g == gcd_iter(a, b));
            assert(a * e.x + b * e.y == e.g); // Bezout identity holds exactly
        }
    }

    // Spot check on a larger pair.
    {
        const ExtGcd e = ext_gcd(240, 46);
        assert(e.g == 2);
        assert(240 * e.x + 46 * e.y == 2);
    }

    // --- Short demo ---
    const ExtGcd demo = ext_gcd(240, 46);
    std::cout << "gcd(48, 18)      = " << gcd_iter(48, 18) << '\n';
    std::cout << "lcm(21, 6)       = " << lcm(21, 6) << '\n';
    std::cout << "ext_gcd(240, 46) = (g=" << demo.g
              << ", x=" << demo.x << ", y=" << demo.y << ")\n";
    std::cout << "check: 240*x + 46*y = " << (240 * demo.x + 46 * demo.y) << '\n';
    std::cout << "All GCD/LCM/Extended-Euclid tests passed.\n";
    return 0;
}
