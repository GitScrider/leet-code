/*
 * Matrix Exponentiation  --  Algorithm - Math / Number Theory
 *
 * Idea:
 *   Any linear recurrence can be advanced by a matrix-vector product, so
 *   iterating it n times is one matrix power. For Fibonacci:
 *        | F(n+1) |   | 1 1 | ^n   | F(1) |
 *        | F(n)   | = | 1 0 |    * | F(0) |
 *   hence [[1,1],[1,0]]^n has F(n+1), F(n), F(n), F(n-1) in its entries; the
 *   top-right entry equals F(n). We raise the matrix by BINARY EXPONENTIATION
 *   (exponent by squaring): M^n is built from the M^(2^k) whose product of the
 *   selected powers matches the set bits of n, giving O(log n) multiplies.
 *
 * Complexity (K = matrix dimension):
 *   +--------------------------+---------------------+
 *   | Operation                | Time                |
 *   +--------------------------+---------------------+
 *   | one KxK matrix multiply  | O(K^3)              |
 *   | matrix power M^n         | O(K^3 * log n)      |
 *   | Fibonacci(n) (K = 2)     | O(log n)            |
 *   +--------------------------+---------------------+
 *
 * Key points / assumptions:
 *   - Entries are kept modulo MOD to prevent overflow: without a modulus F(n)
 *     grows exponentially and blows past 64 bits within ~90 terms.
 *   - MOD is chosen <= ~2^31 (1e9+7). Then each entry is < MOD < 2^31, a single
 *     product is < 2^62, and a 2x2 dot product (two such products summed) stays
 *     below ~2^63, so std::int64_t never overflows. This bound is asserted.
 *   - fib(0)=0, fib(1)=1. The tested values fit in the modulus, so the modular
 *     result equals the true integer value for the ranges checked.
 */

#include <vector>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <cstddef>

using Matrix = std::vector<std::vector<std::int64_t>>;

// Square NxN matrix multiply with modular entries.
// Each term a*b has a,b < MOD <= 2^31, so a*b < 2^62; summing K of them and
// taking % MOD each step keeps the accumulator < 2^63 (safe for int64_t).
Matrix mat_mul(const Matrix& A, const Matrix& B, std::int64_t mod) {
    const std::size_t n = A.size();
    Matrix C(n, std::vector<std::int64_t>(n, 0));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t k = 0; k < n; ++k) {
            if (A[i][k] == 0) continue;                 // small speedup on sparse rows
            const std::int64_t a = A[i][k];
            for (std::size_t j = 0; j < n; ++j)
                C[i][j] = (C[i][j] + a * B[k][j]) % mod;
        }
    return C;
}

// NxN identity matrix (the neutral element for matrix multiplication).
Matrix identity(std::size_t n) {
    Matrix I(n, std::vector<std::int64_t>(n, 0));
    for (std::size_t i = 0; i < n; ++i) I[i][i] = 1;
    return I;
}

// Binary exponentiation (exponent by squaring) on matrices.
Matrix mat_pow(Matrix base, std::int64_t exp, std::int64_t mod) {
    Matrix result = identity(base.size());
    while (exp > 0) {
        if (exp & 1) result = mat_mul(result, base, mod);
        base = mat_mul(base, base, mod);
        exp >>= 1;
    }
    return result;
}

// Fibonacci via matrix power: top-right entry of [[1,1],[1,0]]^n is F(n).
std::int64_t fib_matrix(std::int64_t n, std::int64_t mod) {
    if (n == 0) return 0;
    const Matrix M = {{1, 1}, {1, 0}};
    const Matrix P = mat_pow(M, n, mod);
    return P[0][1];                                     // == F(n)
}

// Linear-DP reference (mod) for cross-checking.
std::int64_t fib_linear(std::int64_t n, std::int64_t mod) {
    std::int64_t a = 0, b = 1;                          // F(0), F(1)
    for (std::int64_t i = 0; i < n; ++i) {
        const std::int64_t next = (a + b) % mod;
        a = b;
        b = next;
    }
    return a;                                           // F(n)
}

int main() {
    const std::int64_t mod = 1000000007;                // prime, < 2^31
    assert(mod <= (static_cast<std::int64_t>(1) << 31)); // guarantees no overflow

    // ---- Known values (fit in the modulus, so equal the true integers) ------
    assert(fib_matrix(0, mod) == 0);
    assert(fib_matrix(1, mod) == 1);
    assert(fib_matrix(2, mod) == 1);
    assert(fib_matrix(10, mod) == 55);
    assert(fib_matrix(20, mod) == 6765);

    // ---- Match the linear-DP reference for many n ---------------------------
    for (std::int64_t n = 0; n <= 2000; ++n)
        assert(fib_matrix(n, mod) == fib_linear(n, mod));

    // ---- Large n via Cassini's identity (O(log n), no linear loop) ----------
    // Cassini: F(n-1)*F(n+1) - F(n)^2 = (-1)^n. We verify this modulo `mod` at
    // large indices; a linear reference here would need up to 1e9 iterations,
    // so this exercises overflow safety of mod-mul without that cost.
    for (std::int64_t n : {12345LL, 100000LL, 1000000LL, 1000000000LL}) {
        const std::int64_t a = fib_matrix(n - 1, mod);
        const std::int64_t b = fib_matrix(n,     mod);
        const std::int64_t c = fib_matrix(n + 1, mod);
        // a*c and b*b are each < mod^2 < 2^62; their difference fits in int64_t.
        const std::int64_t lhs = ((a * c - b * b) % mod + mod) % mod;
        const std::int64_t expected = (n % 2 == 0) ? 1 : (mod - 1);  // (-1)^n mod p
        assert(lhs == expected);
    }

    // ---- Matrix power sanity: M^0 is identity, M^1 is M ---------------------
    const Matrix M = {{1, 1}, {1, 0}};
    const Matrix I = mat_pow(M, 0, mod);
    assert(I[0][0] == 1 && I[0][1] == 0 && I[1][0] == 0 && I[1][1] == 1);
    const Matrix M1 = mat_pow(M, 1, mod);
    assert(M1[0][0] == 1 && M1[0][1] == 1 && M1[1][0] == 1 && M1[1][1] == 0);

    // ---- Short demo ---------------------------------------------------------
    std::cout << "fib(10)   = " << fib_matrix(10, mod)   << '\n';
    std::cout << "fib(20)   = " << fib_matrix(20, mod)   << '\n';
    std::cout << "fib(1e9) mod 1e9+7 = " << fib_matrix(1000000000LL, mod) << '\n';
    std::cout << "All matrix exponentiation assertions passed.\n";
    return 0;
}
