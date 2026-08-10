/*
 * Algorithm: Sieve of Eratosthenes (+ Linear / Smallest-Prime-Factor sieve)
 *            (Math / Number Theory)
 *
 * Idea:
 *   - Classic sieve: every composite has a prime factor <= sqrt(n). Starting
 *     from the smallest prime, mark all its multiples as composite; whatever
 *     remains unmarked is prime. We start crossing out at i*i (smaller
 *     multiples of i already carry a smaller prime factor) and step by i.
 *   - Linear sieve: iterate i from 2..n and, for each prime p in increasing
 *     order, mark p*i as composite recording p as its smallest prime factor
 *     (spf). Breaking when p divides i guarantees every composite is crossed
 *     out exactly once by its smallest prime factor, giving O(n) time.
 *
 * Complexity:
 *   +---------------------+----------------------+
 *   | Sieve               | Time                 |
 *   +---------------------+----------------------+
 *   | Eratosthenes        | O(n log log n)       |
 *   | Linear (SPF)        | O(n)                 |
 *   +---------------------+----------------------+
 *   Space: O(n) for the boolean / spf array.
 *
 * Complexity derivation (Eratosthenes: harmonic-over-primes / Mertens):
 *   Initialization and the outer scan touch each index once: O(n). The marking
 *   cost dominates. The inner loop runs ONLY for a prime p, crossing out
 *   p*p, p*p+p, ..., <= n, i.e. about (n - p*p)/p + 1 ~= n/p iterations. Only
 *   primes p with p*p <= n (p <= sqrt(n)) ever enter the inner loop. Summing:
 *
 *       C(n) = SUM_{p prime, p <= sqrt(n)} (n / p)
 *            = n * SUM_{p prime, p <= sqrt(n)} (1 / p).
 *
 *   Mertens' second theorem gives the sum of reciprocals of the primes:
 *
 *       SUM_{p prime, p <= x} (1 / p) = ln ln x + M + o(1)     (M ~= 0.2615)
 *
 *   With x = sqrt(n),  ln ln sqrt(n) = ln((ln n)/2) = ln ln n - ln 2, hence
 *
 *       C(n) = n * (ln ln n + O(1)) = O(n log log n).
 *
 *   Linear (SPF) sieve: the assignment spf[p*i] = p writes each COMPOSITE k in
 *   [2, n] exactly once (the "if (p == spf[i]) break" stops i as soon as its own
 *   smallest prime factor is reached), so the inner body runs once per composite:
 *
 *       C_lin(n) = (outer loop: n-1 iters) + (each composite <= n marked once)
 *                = O(n) + O(n) = O(n).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f = O(g)      iff  EXISTS c2, n0 :        f(n) <= c2*g(n)  for n >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)         for n >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   Both sieves are OBLIVIOUS: the work depends only on n, never on the values,
 *   so each case coincides and the bound is tight:
 *     Eratosthenes:  C(n) = n*(ln ln n + O(1))  => Theta(n log log n).
 *     Linear sieve:  C_lin(n) = Theta(n).
 *   These are marking algorithms, not comparison sorts, so the Omega(n log n)
 *   comparison lower bound does not apply -- indeed both n log log n and n beat
 *   it, which is why sieving primes is faster than sorting could ever be.
 *
 * Key points / assumptions:
 *   - "primes below N" here means primes strictly less than N.
 *   - Inner loop uses i*i <= n and steps by i; i*i is guarded with a 64-bit
 *     cast so the bound check cannot overflow for large n.
 *   - The linear sieve additionally yields spf[k] = smallest prime factor of k,
 *     useful for O(log k) factorization.
 */

#include <vector>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <iostream>

// Classic Sieve of Eratosthenes. Returns is_prime[0..n].
static std::vector<bool> sieve_eratosthenes(int n) {
    std::vector<bool> is_prime(static_cast<std::size_t>(n) + 1, true);
    if (n >= 0) is_prime[0] = false;
    if (n >= 1) is_prime[1] = false;
    for (int i = 2; static_cast<std::int64_t>(i) * i <= n; ++i) {
        if (is_prime[static_cast<std::size_t>(i)]) {
            // Start at i*i: smaller multiples were already marked by smaller primes.
            for (int j = i * i; j <= n; j += i)
                is_prime[static_cast<std::size_t>(j)] = false;
        }
    }
    return is_prime;
}

// Linear sieve. Fills spf[k] with the smallest prime factor of k (spf[0]=spf[1]=0)
// and returns the list of primes <= n in increasing order.
static std::vector<int> linear_sieve(int n, std::vector<int>& spf) {
    spf.assign(static_cast<std::size_t>(n) + 1, 0);
    std::vector<int> primes;
    for (int i = 2; i <= n; ++i) {
        if (spf[static_cast<std::size_t>(i)] == 0) { // i has no smaller factor -> prime
            spf[static_cast<std::size_t>(i)] = i;
            primes.push_back(i);
        }
        for (const int p : primes) {
            if (static_cast<std::int64_t>(p) * i > n) break; // stay in range
            spf[static_cast<std::size_t>(p) * static_cast<std::size_t>(i)] = p;
            if (p == spf[static_cast<std::size_t>(i)]) break; // each composite marked once
        }
    }
    return primes;
}

// Count primes strictly below 'limit' using the classic sieve.
static int count_primes_below(int limit) {
    if (limit <= 2) return 0;
    const std::vector<bool> is_prime = sieve_eratosthenes(limit - 1);
    int count = 0;
    for (int i = 2; i < limit; ++i)
        if (is_prime[static_cast<std::size_t>(i)]) ++count;
    return count;
}

int main() {
    // --- Known prime counts ---
    assert(count_primes_below(100) == 25);   // there are 25 primes below 100
    assert(count_primes_below(1000) == 168);  // and 168 primes below 1000

    // --- Individual classification (prime vs composite) ---
    const std::vector<bool> ip = sieve_eratosthenes(1000);
    assert(ip[2] && ip[3] && ip[5] && ip[7]);
    assert(ip[97]);            // 97 is prime
    assert(!ip[1]);            // 1 is not prime (edge case)
    assert(!ip[0]);            // 0 is not prime (edge case)
    assert(!ip[91]);           // 91 = 7 * 13 composite
    assert(!ip[561]);          // 561 = 3 * 11 * 17 composite (a Carmichael number)
    assert(ip[997]);           // largest prime below 1000

    // --- Linear sieve agrees with classic sieve on the prime set ---
    std::vector<int> spf;
    const std::vector<int> primes = linear_sieve(1000, spf);
    {
        int idx = 0;
        for (int i = 2; i <= 1000; ++i) {
            if (ip[static_cast<std::size_t>(i)]) {
                assert(idx < static_cast<int>(primes.size()));
                assert(primes[static_cast<std::size_t>(idx)] == i);
                ++idx;
            }
        }
        assert(idx == static_cast<int>(primes.size()));
    }

    // --- Smallest prime factor spot checks ---
    assert(spf[12] == 2);      // 12 = 2 * 6
    assert(spf[15] == 3);      // 15 = 3 * 5
    assert(spf[97] == 97);     // prime is its own smallest factor
    assert(spf[49] == 7);      // 49 = 7 * 7

    // --- Short demo ---
    std::cout << "Primes below 100  : " << count_primes_below(100) << '\n';
    std::cout << "Primes below 1000 : " << count_primes_below(1000) << '\n';
    std::cout << "First 10 primes   :";
    for (std::size_t k = 0; k < 10 && k < primes.size(); ++k)
        std::cout << ' ' << primes[k];
    std::cout << '\n';
    std::cout << "spf(84) = " << spf[84] << " (84 = 2*2*3*7)\n";
    std::cout << "All sieve tests passed.\n";
    return 0;
}
