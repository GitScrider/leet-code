/*
 * Combinatorics  --  Algorithm - Math / Number Theory
 *
 * Idea:
 *   Three classic tools for counting problems:
 *     1) Exact nPr / nCr for small inputs, computed with an OVERFLOW-SAFE
 *        multiplicative loop instead of dividing two huge factorials.
 *        C(n,r) = prod_{i=1..r} (n-r+i)/i. After step i the running value is
 *        exactly C(n-r+i, i), an integer, so every division is exact and no
 *        intermediate factorial is ever formed.
 *     2) Pascal's triangle by DP using the identity
 *        C(n,k) = C(n-1,k-1) + C(n-1,k). Row sums satisfy sum_k C(n,k) = 2^n.
 *     3) nCr mod p for PRIME p via factorials and modular inverse.
 *        By Fermat's little theorem a^(p-1) = 1 (mod p) for gcd(a,p)=1, hence
 *        a^(-1) = a^(p-2) (mod p). Then C(n,r) = n! * inv(r!) * inv((n-r)!).
 *
 * Complexity:
 *   +---------------------------+-----------------+
 *   | Operation                 | Time            |
 *   +---------------------------+-----------------+
 *   | nPr / nCr (multiplicative)| O(r)            |
 *   | Pascal's triangle (rows n)| O(n^2)          |
 *   | nCr mod p (with factorials| O(n) precompute |
 *   |   precomputed, per query) | O(log p) inverse|
 *   +---------------------------+-----------------+
 *
 * Key points / assumptions:
 *   - Modulus p must be PRIME for the Fermat inverse to exist for r! and (n-r)!.
 *   - p is kept <= ~2^31 so that a product of two residues fits in int64_t
 *     (~9.2e18) without overflow; this is asserted at runtime.
 *   - Exact nCr uses int64_t and only handles values that fit in 63 bits.
 */

#include <vector>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <cstddef>

// ---- Exact small values (overflow-safe multiplicative computation) ----------

// nCr: multiply-then-divide keeping the running value integral at every step.
std::int64_t nCr_exact(int n, int r) {
    if (r < 0 || r > n) return 0;
    if (r > n - r) r = n - r;               // symmetry C(n,r)=C(n,n-r), fewer steps
    std::int64_t result = 1;
    for (int i = 1; i <= r; ++i) {
        // After this line result == C(n-r+i, i), which is always an integer,
        // so the division is exact and we never build a full factorial.
        result = result * static_cast<std::int64_t>(n - r + i) / i;
    }
    return result;
}

// nPr = n! / (n-r)! = n * (n-1) * ... * (n-r+1).
std::int64_t nPr_exact(int n, int r) {
    if (r < 0 || r > n) return 0;
    std::int64_t result = 1;
    for (int i = 0; i < r; ++i) result *= static_cast<std::int64_t>(n - i);
    return result;
}

// ---- Pascal's triangle (DP) -------------------------------------------------

std::vector<std::vector<std::int64_t>> pascal_triangle(int rows) {
    std::vector<std::vector<std::int64_t>> tri(static_cast<std::size_t>(rows));
    for (int n = 0; n < rows; ++n) {
        tri[static_cast<std::size_t>(n)].assign(static_cast<std::size_t>(n) + 1, 1);
        for (int k = 1; k < n; ++k) {
            const std::size_t un = static_cast<std::size_t>(n);
            const std::size_t uk = static_cast<std::size_t>(k);
            tri[un][uk] = tri[un - 1][uk - 1] + tri[un - 1][uk];
        }
    }
    return tri;
}

// ---- nCr mod prime p (Fermat modular inverse) -------------------------------

std::int64_t mod_pow(std::int64_t base, std::int64_t exp, std::int64_t mod) {
    base %= mod;
    std::int64_t result = 1;
    while (exp > 0) {
        // Products below are (residue < mod) * (residue < mod); with mod <= 2^31
        // each product is < 2^62 and safely fits in int64_t.
        if (exp & 1) result = result * base % mod;
        base = base * base % mod;
        exp >>= 1;
    }
    return result;
}

std::int64_t mod_inverse(std::int64_t a, std::int64_t p) {
    return mod_pow(a, p - 2, p);            // Fermat: a^(p-2) = a^(-1) (mod p)
}

std::int64_t nCr_mod_p(int n, int r, std::int64_t p) {
    if (r < 0 || r > n) return 0;
    std::vector<std::int64_t> fact(static_cast<std::size_t>(n) + 1);
    fact[0] = 1 % p;
    for (int i = 1; i <= n; ++i)
        fact[static_cast<std::size_t>(i)] =
            fact[static_cast<std::size_t>(i - 1)] * i % p;
    const std::int64_t num = fact[static_cast<std::size_t>(n)];
    const std::int64_t den = fact[static_cast<std::size_t>(r)] *
                             fact[static_cast<std::size_t>(n - r)] % p;
    return num * mod_inverse(den, p) % p;
}

int main() {
    // ---- Exact nCr / nPr: known values and edge cases -----------------------
    assert(nCr_exact(5, 2) == 10);
    assert(nCr_exact(10, 3) == 120);
    assert(nCr_exact(0, 0) == 1);
    assert(nCr_exact(6, 0) == 1);
    assert(nCr_exact(6, 6) == 1);
    assert(nCr_exact(6, 7) == 0);           // r > n
    assert(nCr_exact(52, 5) == 2598960);    // poker hands, overflow-prone size
    assert(nPr_exact(5, 2) == 20);
    assert(nPr_exact(10, 3) == 720);
    assert(nPr_exact(6, 0) == 1);

    // ---- Pascal's triangle: recurrence and row-sum identity -----------------
    const int rows = 15;
    const auto tri = pascal_triangle(rows);
    for (int n = 0; n < rows; ++n) {
        std::int64_t row_sum = 0;
        for (std::size_t k = 0; k < tri[static_cast<std::size_t>(n)].size(); ++k) {
            row_sum += tri[static_cast<std::size_t>(n)][k];
            // Triangle entries must match the exact multiplicative computation.
            assert(tri[static_cast<std::size_t>(n)][k] ==
                   nCr_exact(n, static_cast<int>(k)));
        }
        assert(row_sum == (static_cast<std::int64_t>(1) << n));   // sum = 2^n
    }

    // ---- nCr mod p vs a DP reference ---------------------------------------
    const std::int64_t p = 1000000007;      // prime, < 2^31, so products are safe
    assert(p <= (static_cast<std::int64_t>(1) << 31));
    // DP reference table of C(n,k) mod p built purely by additions.
    const int N = 60;
    std::vector<std::vector<std::int64_t>> dp(
        static_cast<std::size_t>(N) + 1,
        std::vector<std::int64_t>(static_cast<std::size_t>(N) + 1, 0));
    for (int n = 0; n <= N; ++n) {
        dp[static_cast<std::size_t>(n)][0] = 1;
        for (int k = 1; k <= n; ++k) {
            const std::size_t un = static_cast<std::size_t>(n);
            const std::size_t uk = static_cast<std::size_t>(k);
            dp[un][uk] = (dp[un - 1][uk - 1] + dp[un - 1][uk]) % p;
        }
    }
    for (int n = 0; n <= N; ++n)
        for (int k = 0; k <= n; ++k)
            assert(nCr_mod_p(n, k, p) == dp[static_cast<std::size_t>(n)][static_cast<std::size_t>(k)]);

    // ---- Short demo ---------------------------------------------------------
    std::cout << "C(5,2)   = " << nCr_exact(5, 2)   << '\n';
    std::cout << "P(10,3)  = " << nPr_exact(10, 3)  << '\n';
    std::cout << "C(52,5)  = " << nCr_exact(52, 5)  << " (poker hands)\n";
    std::cout << "C(60,30) mod 1e9+7 = " << nCr_mod_p(60, 30, p) << '\n';
    std::cout << "All combinatorics assertions passed.\n";
    return 0;
}
