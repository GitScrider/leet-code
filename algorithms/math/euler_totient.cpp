/*
 * Euler's Totient (phi)  --  Algorithm - Math / Number Theory
 *
 * Idea:
 *   phi(n) counts the integers in [1, n] that are coprime to n.
 *   From the Euler product formula, if n = prod p_i^a_i over its distinct
 *   primes p_i, then
 *        phi(n) = n * prod (1 - 1/p_i).
 *   Single-value computation factorizes n by trial division up to sqrt(n) and
 *   applies each factor (1 - 1/p) as "phi -= phi / p". A linear/sieve variant
 *   computes phi(1..n) at once using the same multiplicative structure while
 *   crossing off multiples, in the spirit of the Sieve of Eratosthenes.
 *
 * Divisor-sum identity (used as a test):
 *        sum_{d | n} phi(d) = n.
 *   This holds because grouping the fractions k/n (k in [1,n]) by their reduced
 *   denominator d partitions {1..n} into blocks of size phi(d) over divisors d.
 *
 * Complexity:
 *   +----------------------------+------------------+
 *   | Operation                  | Time             |
 *   +----------------------------+------------------+
 *   | phi(n) single (trial div.) | O(sqrt(n))       |
 *   | phi(1..n) sieve            | O(n log log n)   |
 *   +----------------------------+------------------+
 *
 * Complexity derivation (operation counts):
 *   (1) euler_phi(n) -- trial division to sqrt(n), O(sqrt(n)):
 *       The outer loop tests candidate factors p = 2, 3, ... while p*p <= n, i.e.
 *       at most floor(sqrt(n)) iterations of O(1) work:
 *           C(n) = SUM_{p=2}^{floor(sqrt(n))} c  +  (inner divisions).
 *       Each inner "while (n % p == 0) n /= p" shrinks n by a factor >= p >= 2,
 *       so across the WHOLE run the divisions number at most log2(n) (a geometric
 *       collapse) -- a lower-order term. Hence
 *           C(n) = c*(sqrt(n) - 1) + O(log n) = O(sqrt(n)).
 *   (2) euler_phi_sieve(n) -- Eratosthenes structure, O(n log log n):
 *       For each prime p <= n the inner loop steps m = p, 2p, 3p, ... <= n, i.e.
 *       floor(n/p) updates of O(1); composites are skipped (phi[p] != p). Summing
 *       over primes only:
 *           T(n) = SUM_{p prime <= n} n/p = n * SUM_{p prime <= n} (1/p).
 *       By Mertens' theorem SUM_{p<=n} 1/p = ln ln n + M + o(1), so
 *           T(n) = n * (ln ln n + O(1)) = O(n log log n)
 *       (the iota init and the O(n) prime tests add only a lower-order O(n)).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f = O(g)      iff  EXISTS c2, n0 :        f(n) <= c2*g(n)   for n >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)          for n >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   euler_phi is ADAPTIVE (the loop bound uses the shrinking n):
 *     WORST case  n prime: no factor is ever stripped, so the loop runs the full
 *                 floor(sqrt(n)) steps         => Theta(sqrt(n)).
 *     BEST case   n = 2^k: p = 2 strips everything, but the inner
 *                 "while (n % p == 0) n /= p" runs k = log2(n) times to drop
 *                 n from 2^k down to 1; the p*p <= n test then fails at p = 3,
 *                 yet only AFTER those log n divisions  => Theta(log n).
 *     Over all inputs the running time is thus O(sqrt(n)) (from the worst case)
 *     and Omega(log n) (from the best case), not a single Theta.
 *   euler_phi_sieve does the SAME work for a given n (data-independent), so it is
 *   tight: c1 * n log log n <= T(n) <= c2 * n log log n => Theta(n log log n).
 *   Both are number-theoretic, not comparison sorts, so the comparison-sort lower
 *   bound Omega(n log n) does NOT apply.
 *
 * Key points / assumptions:
 *   - n >= 1; phi(1) = 1 by convention (the empty product / gcd(1,1)=1).
 *   - Trial division divides out each prime factor fully before moving on, so
 *     the leftover after the sqrt loop is either 1 or a single large prime.
 *   - All arithmetic fits in std::int64_t for the tested ranges; phi(n) <= n.
 */

#include <vector>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <numeric>
#include <cstddef>

// ---- Single value via prime factorization -----------------------------------

// phi(n) = n * prod (1 - 1/p). We apply each distinct prime factor once as
// phi = phi / p * (p - 1), equivalently phi -= phi / p.
std::int64_t euler_phi(std::int64_t n) {
    assert(n >= 1);
    std::int64_t result = n;
    for (std::int64_t p = 2; p * p <= n; ++p) {
        if (n % p == 0) {
            while (n % p == 0) n /= p;      // strip the whole prime power p^a
            result -= result / p;           // apply factor (1 - 1/p)
        }
    }
    if (n > 1) result -= result / n;        // leftover n is a single large prime
    return result;
}

// ---- Sieve computing phi for 1..n -------------------------------------------

// Initialize phi[i] = i, then for each prime p multiply every multiple by
// (1 - 1/p) exactly once via phi[m] -= phi[m] / p.
std::vector<std::int64_t> euler_phi_sieve(int n) {
    assert(n >= 1);
    std::vector<std::int64_t> phi(static_cast<std::size_t>(n) + 1);
    std::iota(phi.begin(), phi.end(), static_cast<std::int64_t>(0)); // phi[i]=i
    for (int p = 2; p <= n; ++p) {
        if (phi[static_cast<std::size_t>(p)] == static_cast<std::int64_t>(p)) {
            // phi[p] still equals p  =>  p is prime (untouched so far).
            for (int m = p; m <= n; m += p) {
                const std::size_t um = static_cast<std::size_t>(m);
                phi[um] -= phi[um] / p;
            }
        }
    }
    return phi;
}

// Brute-force reference: count k in [1, n] with gcd(k, n) == 1.
std::int64_t phi_bruteforce(std::int64_t n) {
    std::int64_t count = 0;
    for (std::int64_t k = 1; k <= n; ++k)
        if (std::gcd(k, n) == 1) ++count;
    return count;
}

int main() {
    // ---- Known values and edge cases ---------------------------------------
    assert(euler_phi(1) == 1);
    assert(euler_phi(9) == 6);              // coprime to 9: 1,2,4,5,7,8
    assert(euler_phi(2) == 1);
    assert(euler_phi(10) == 4);             // 1,3,7,9

    // phi(prime p) = p - 1.
    const int primes[] = {2, 3, 5, 7, 13, 97, 101, 7919};
    for (const int pr : primes)
        assert(euler_phi(pr) == pr - 1);

    // phi(p^k) = p^k - p^(k-1); check on 2^k and 3^k.
    assert(euler_phi(8) == 4);              // 2^3 - 2^2
    assert(euler_phi(27) == 18);            // 3^3 - 3^2

    // ---- Single value matches brute force ----------------------------------
    for (std::int64_t n = 1; n <= 200; ++n)
        assert(euler_phi(n) == phi_bruteforce(n));

    // ---- Sieve matches single-value computation ----------------------------
    const int LIMIT = 1000;
    const auto phi = euler_phi_sieve(LIMIT);
    for (int n = 1; n <= LIMIT; ++n)
        assert(phi[static_cast<std::size_t>(n)] == euler_phi(n));

    // ---- Divisor-sum identity: sum_{d | n} phi(d) == n ----------------------
    for (int n = 1; n <= 300; ++n) {
        std::int64_t divisor_sum = 0;
        for (int d = 1; d <= n; ++d)
            if (n % d == 0) divisor_sum += euler_phi(d);
        assert(divisor_sum == n);
    }

    // ---- Short demo ---------------------------------------------------------
    std::cout << "phi(1)   = " << euler_phi(1)   << '\n';
    std::cout << "phi(9)   = " << euler_phi(9)   << '\n';
    std::cout << "phi(97)  = " << euler_phi(97)  << " (prime -> p-1)\n";
    std::cout << "phi(36)  = " << euler_phi(36)  << '\n';
    std::cout << "sum_{d|36} phi(d) = 36 (verified)\n";
    std::cout << "All Euler totient assertions passed.\n";
    return 0;
}
