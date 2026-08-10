/*
 * Prime Factorization  --  Algorithm - Math / Number Theory
 *
 * Idea:
 *   Every integer n > 1 has a unique multiset of prime factors (Fundamental
 *   Theorem of Arithmetic). We expose two strategies:
 *     (1) Trial division: strip out the factor 2, then test only ODD candidates
 *         d up to sqrt(n). If n still has a prime divisor it must be <= sqrt(n)
 *         (a composite has such a divisor); once no d <= sqrt(n) divides the
 *         remaining value, that leftover value (> 1) is itself prime.
 *     (2) Smallest-Prime-Factor (SPF) sieve: precompute spf[x] for all x <= N
 *         once, then factor any x <= N in O(log x) by repeatedly dividing by
 *         spf[x]. This is the fast option when MANY numbers must be factored.
 *
 * Complexity:
 *   +-----------------------------------+-------------------------+
 *   | Operation                         | Time                    |
 *   +-----------------------------------+-------------------------+
 *   | Trial division of n               | O(sqrt(n))              |
 *   | Build SPF sieve up to N           | O(N log log N)          |
 *   | Factor one x <= N with SPF        | O(log x)                |
 *   +-----------------------------------+-------------------------+
 *   Space: O(1) for trial division, O(N) for the SPF sieve.
 *
 * Complexity derivation (trial-division count / sieve harmonic sum / SPF chain):
 *   Treat each 64-bit division or modulo as O(1) (word-RAM model).
 *
 *   (1) Trial division of n. The worst input is a PRIME n (nothing divides, so n
 *       never shrinks). After the factor-2 loop the odd loop tests d = 3,5,7,...
 *       while d*d <= n, i.e. d <= sqrt(n); the count of such candidates is
 *           C(n) = SUM_{d = 3,5,...,<= sqrt(n)} 1
 *                = #{ odd d : 3 <= d <= floor(sqrt n) }
 *                = floor((floor(sqrt n) - 1) / 2)  ~  sqrt(n)/2  =  O(sqrt n),
 *       each step one O(1) modulo. Any factor that IS found only shrinks n (and
 *       thus sqrt n), so the prime case dominates.
 *
 *   (2) build_spf(N). For each prime i the inner loop stamps the multiples
 *       i, 2i, 3i, ..., <= N (floor(N/i) of them); composite i costs an O(1)
 *       test. Summing the inner work over primes:
 *           T(N) = SUM_{p prime <= N} floor(N/p) <= N * SUM_{p prime <= N} 1/p
 *                = N * (ln ln N + M + o(1))          (Mertens' 2nd theorem)
 *                = O(N log log N),
 *       plus O(N) for the outer scan  ->  O(N log log N) overall.
 *
 *   (3) factorize_spf(x). Each iteration divides out one prime spf[x] >= 2, so x
 *       drops by a factor >= 2 per division. With x = PRODUCT p_i^e_i, p_i >= 2,
 *       the number of divisions is SUM_i e_i <= log2(x) = O(log x).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Definitions: f = O(g) iff f <= c2*g; f = Omega(g) iff f >= c1*g; f = Theta(g)
 *   iff both, for some positive c1, c2 and all n >= n0.
 *   (1) Trial division is ADAPTIVE in n's arithmetic structure:
 *       BEST  case n = 2^k: the factor-2 loop strips k = log2 n twos and the odd
 *                  loop never runs (3*3 > 1)          =>  Theta(log n);
 *       WORST case n prime: the odd loop runs to sqrt(n)  =>  Theta(sqrt n).
 *       So over all n the time is O(sqrt n) (upper, prime) and Omega(log n)
 *       (lower, power of two), not a single Theta.
 *   (2) build_spf: the harmonic-over-primes sum is fixed by N, so with
 *       g = N log log N, c1*g <= T(N) <= c2*g for N >= n0 => Theta(N log log N).
 *   (3) factorize_spf: SUM_i e_i divisions, from Omega(1) (x prime -> 1 division)
 *       up to O(log x) (x = 2^k -> log2 x divisions); the stated O(log x) is the
 *       worst case. These are number-theoretic, not comparison sorts, so the
 *       Omega(n log n) comparison lower bound does not apply.
 *
 * Key points / assumptions:
 *   - Output is a list of (prime, exponent) pairs in increasing prime order.
 *   - n = 0 and n = 1 have no prime factors -> an empty list is returned.
 *   - Overflow safety: the trial loop guard uses d*d <= n instead of sqrt().
 *     For n < 2^62 the largest d is ~2^31 so d*d < 2^63 fits std::int64_t.
 */

#include <vector>
#include <utility>
#include <cstdint>
#include <cassert>
#include <iostream>

// (1) Trial division. Returns prime->exponent pairs in ascending prime order.
static std::vector<std::pair<std::int64_t, int>> factorize_trial(std::int64_t n) {
    std::vector<std::pair<std::int64_t, int>> factors;
    if (n <= 1) return factors;                     // 0 and 1: no prime factors

    int e2 = 0;
    while (n % 2 == 0) { n /= 2; ++e2; }            // pull out all factors of 2
    if (e2 > 0) factors.emplace_back(2, e2);

    // Only odd divisors remain. d*d <= n avoids a costly / imprecise sqrt call.
    for (std::int64_t d = 3; d * d <= n; d += 2) {
        if (n % d == 0) {
            int e = 0;
            while (n % d == 0) { n /= d; ++e; }
            factors.emplace_back(d, e);
        }
    }
    if (n > 1) factors.emplace_back(n, 1);          // leftover value is prime
    return factors;
}

// (2) Build the smallest-prime-factor table for [0, N]. spf[x] = smallest prime
// dividing x (spf[0] = spf[1] = 0). Sieve of Eratosthenes variant: when i is
// found prime, stamp it onto every still-unstamped multiple.
static std::vector<int> build_spf(int N) {
    std::vector<int> spf(static_cast<std::size_t>(N) + 1, 0);
    for (int i = 2; i <= N; ++i) {
        if (spf[static_cast<std::size_t>(i)] == 0) {     // i is prime
            for (int j = i; j <= N; j += i) {
                if (spf[static_cast<std::size_t>(j)] == 0)
                    spf[static_cast<std::size_t>(j)] = i;
            }
        }
    }
    return spf;
}

// (2) Factor n <= N in O(log n) using a prebuilt SPF table (fast for many queries).
static std::vector<std::pair<int, int>> factorize_spf(int n, const std::vector<int>& spf) {
    std::vector<std::pair<int, int>> factors;
    while (n > 1) {
        const int p = spf[static_cast<std::size_t>(n)];
        int e = 0;
        while (n % p == 0) { n /= p; ++e; }
        factors.emplace_back(p, e);
    }
    return factors;
}

// Brute-force reference: multiply the factors back to reconstruct the number.
static std::int64_t product_of(const std::vector<std::pair<std::int64_t, int>>& f) {
    std::int64_t prod = 1;
    for (const auto& pe : f)
        for (int k = 0; k < pe.second; ++k) prod *= pe.first;
    return prod;
}

int main() {
    // ---- Known factorizations ------------------------------------------------
    {
        const auto f = factorize_trial(360);         // 360 = 2^3 * 3^2 * 5
        const std::vector<std::pair<std::int64_t, int>> expected{{2, 3}, {3, 2}, {5, 1}};
        assert(f == expected);
    }
    assert(factorize_trial(1).empty());               // edge: 1
    assert(factorize_trial(0).empty());               // edge: 0
    assert((factorize_trial(97) == std::vector<std::pair<std::int64_t, int>>{{97, 1}}));  // prime
    assert((factorize_trial(1000000) ==               // 10^6 = 2^6 * 5^6
            std::vector<std::pair<std::int64_t, int>>{{2, 6}, {5, 6}}));
    assert((factorize_trial(10403) ==                 // 101 * 103, exercises the
            std::vector<std::pair<std::int64_t, int>>{{101, 1}, {103, 1}}));  // leftover-prime path

    // ---- Product of factors must reconstruct n; primes must be increasing ----
    for (std::int64_t n = 2; n <= 2000; ++n) {
        const auto f = factorize_trial(n);
        assert(product_of(f) == n);                   // reconstruction check
        for (std::size_t i = 1; i < f.size(); ++i)
            assert(f[i - 1].first < f[i].first);       // strictly increasing primes
        for (const auto& pe : f) assert(pe.second >= 1);
    }

    // ---- SPF sieve must agree with trial division for all x in [2, N] --------
    const int N = 5000;
    const std::vector<int> spf = build_spf(N);
    for (int x = 2; x <= N; ++x) {
        const auto a = factorize_spf(x, spf);
        const auto b = factorize_trial(x);
        assert(a.size() == b.size());
        for (std::size_t i = 0; i < a.size(); ++i) {
            assert(static_cast<std::int64_t>(a[i].first) == b[i].first);
            assert(a[i].second == b[i].second);
        }
    }

    // ---- Short demo ----------------------------------------------------------
    std::cout << "Prime factorization demo\n  360 = ";
    const auto demo = factorize_trial(360);
    for (std::size_t i = 0; i < demo.size(); ++i) {
        std::cout << demo[i].first << "^" << demo[i].second
                  << (i + 1 < demo.size() ? " * " : "\n");
    }
    std::cout << "  SPF-based factorization of 84 = ";
    const auto d84 = factorize_spf(84, spf);          // 84 = 2^2 * 3 * 7
    for (std::size_t i = 0; i < d84.size(); ++i) {
        std::cout << d84[i].first << "^" << d84[i].second
                  << (i + 1 < d84.size() ? " * " : "\n");
    }
    std::cout << "All prime-factorization assertions passed.\n";
    return 0;
}
