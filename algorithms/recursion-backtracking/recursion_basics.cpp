/*
 * ============================================================================
 * Recursion Basics
 * Category: Algorithm - Recursion (fundamentals)
 *
 * Idea:
 *   A recursive function solves a problem by (a) handling a trivial BASE CASE
 *   directly, and (b) reducing every other input toward that base case and
 *   combining the result of the smaller subproblem. Each routine below is
 *   written as a plain recurrence:
 *     factorial(n)   = n * factorial(n-1),          base factorial(0) = 1
 *     fib(n)         = fib(n-1) + fib(n-2),          base fib(0)=0, fib(1)=1
 *     gcd(a,b)       = gcd(b, a mod b),              base gcd(a,0) = a   (Euclid)
 *     ipow(b,e)      = (e even) ? ipow(b,e/2)^2                          (fast
 *                    : b * ipow(b,e-1),              base ipow(b,0) = 1   power)
 *     sum(a,i)       = a[i] + sum(a,i+1),            base sum(a,n) = 0
 *     reverse(a,l,r) = swap(a[l],a[r]); reverse(l+1,r-1), base l >= r
 *
 * Complexity:
 *   +----------------------+---------------+---------------------------------+
 *   | Routine              | Time          | Space (recursion stack)         |
 *   +----------------------+---------------+---------------------------------+
 *   | factorial(n)         | O(n)          | O(n)                            |
 *   | fibNaive(n)          | O(phi^n) ~2^n | O(n)   (max depth)              |
 *   | fibMemo(n)           | O(n)          | O(n)   (memo table + stack)     |
 *   | gcd(a,b)             | O(log min)    | O(log min)                      |
 *   | ipow(b,e)            | O(log e)      | O(log e)                        |
 *   | sumArray / reverse   | O(n)          | O(n)                            |
 *   +----------------------+---------------+---------------------------------+
 *   WHY fibNaive is EXPONENTIAL: the call tree branches twice per node and
 *   RECOMPUTES the same fib(k) over and over -- the number of leaves grows like
 *   the Fibonacci numbers themselves (~phi^n). MEMOIZATION caches each fib(k)
 *   the first time it is computed, collapsing the tree to O(n) distinct calls.
 *
 * Complexity derivation (per routine: recurrence -> unfolded count):
 *   factorial(n): a single non-branching chain of calls.
 *       T(n) = T(n-1) + c ,   T(0) = c0        (base case)
 *            = c0 + SUM_{k=1}^{n} c  =  c0 + c*n  =  O(n).
 *     The recursion tree is one PATH of n+1 nodes, O(1) work each => O(n) time
 *     and O(n) stack depth. sumArray is identical; reverse peels TWO elements
 *     per call, T(n) = T(n-2) + c => n/2 levels => still O(n).
 *
 *   fibNaive(n): a binary but UNBALANCED tree that recomputes subproblems.
 *       T(n) = T(n-1) + T(n-2) + c ,   T(0) = T(1) = c0
 *     The node count obeys the Fibonacci recurrence itself: #calls(n) =
 *     2*F(n+1) - 1, and F(n) = (phi^n - psi^n)/sqrt5, phi = (1+sqrt5)/2. Squeeze
 *     it between two clean geometric recurrences:
 *       T(n) >= 2*T(n-2)  => T(n) >= 2^(n/2) = (sqrt2)^n     (lower)
 *       T(n) <= 2*T(n-1)  => T(n) <= 2^n                     (upper)
 *     so the true rate phi^n (~1.618^n) sits between => O(2^n), exponential.
 *
 *   fibMemo(n): there are exactly n+1 distinct states fib(0..n). Each state's
 *     body runs ONCE (first visit fills memo[k]); every later reference is a
 *     cache hit returning in O(1). Work = (n+1) states * O(1) = O(n).
 *
 *   gcd(a,b) [Euclid]: T(a,b) = T(b, a mod b) + c. Key fact: a mod b < a/2
 *     whenever b <= a, so TWO successive steps at least halve the first
 *     argument => #steps <= 2*log2(a) = O(log a). The worst input is a pair of
 *     consecutive Fibonacci numbers (F(k+1), F(k)), taking k steps with
 *     F(k) ~ phi^k, i.e. k ~ log_phi(min) = O(log min(a,b)).
 *
 *   ipow(b,e) [exponentiation by squaring]: an even exponent halves e in one
 *     call; an odd exponent peels one factor (e -> e-1) so the NEXT call is
 *     even -- at most 2 calls per halving:
 *       T(e) = T(e/2) + O(1)      (amortizing the odd peel)
 *     Master Theorem a=1, b=2, f=O(1)=Theta(e^(log_2 1))=Theta(1): case 2 =>
 *     T(e) = Theta(log e). Equivalently e has floor(log2 e)+1 bits, O(1) each.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Definitions (c1, c2, n0 positive constants):
 *     f = O(g)     iff EXISTS c2, n0 :        f(n) <= c2*g(n)  for n >= n0
 *     f = Omega(g) iff EXISTS c1, n0 :  c1*g(n) <= f(n)         for n >= n0
 *     f = Theta(g) iff f = O(g) AND f = Omega(g):
 *                      c1*g(n) <= f(n) <= c2*g(n)  for n >= n0
 *   Each routine's cost is INPUT-INDEPENDENT (given its size parameter), so
 *   best = worst and every bound below is a tight Theta:
 *     factorial, sumArray, reverse : Theta(n)       (n, n, n/2 levels)
 *     fibMemo                      : Theta(n)       (n+1 states, O(1) each)
 *     gcd(a,b)                     : O(log min); worst Theta(log min) on
 *                                    consecutive-Fibonacci inputs
 *     ipow(b,e)                    : Theta(log e)
 *   fibNaive is the exception worth stating with constants: with g = 2^n,
 *     (sqrt2)^n <= T(n) <= 2^n  for n >= 1, i.e. Omega(2^(n/2)) and O(2^n);
 *     the exact tight rate is Theta(phi^n), phi = (1+sqrt5)/2 ~ 1.618. These
 *     are EXPONENTIAL lower bounds -- no input makes naive Fibonacci cheap,
 *     which is precisely why memoization (Theta(n)) is the fix.
 *
 * Key points / when to use:
 *   - Every correct recursion needs a base case that is actually reachable, or
 *     it recurses forever (stack overflow).
 *   - Overlapping subproblems (fib) => memoize; a shrinking single subproblem
 *     (factorial, gcd) => plain recursion is already optimal.
 *   - "Exponentiation by squaring" halves the exponent, turning O(e) into
 *     O(log e); the same trick powers fast modular exponentiation.
 * ============================================================================
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <utility>   // std::swap (used in reverseImpl; not guaranteed via <vector>)
#include <vector>

// factorial(n) = n * factorial(n-1). BASE CASE: 0! = 1 (empty product).
unsigned long long factorial(unsigned n) {
    if (n == 0) return 1ULL;          // base case
    return static_cast<unsigned long long>(n) * factorial(n - 1);
}

// Naive Fibonacci. BASE CASES: fib(0)=0, fib(1)=1. Branches twice => O(2^n)
// because it re-derives the same subvalues repeatedly.
long long fibNaive(unsigned n) {
    if (n < 2) return static_cast<long long>(n);   // base cases fib(0)=0, fib(1)=1
    return fibNaive(n - 1) + fibNaive(n - 2);      // two overlapping subproblems
}

// Memoized Fibonacci. memo[k] == -1 means "not computed yet". Each fib(k) is
// filled exactly once, so total work is O(n). BASE CASE: fib(0)=0, fib(1)=1.
long long fibMemoImpl(unsigned n, std::vector<long long>& memo) {
    if (n < 2) return static_cast<long long>(n);   // base cases
    if (memo[n] != -1) return memo[n];             // cache hit: reuse, do not recompute
    memo[n] = fibMemoImpl(n - 1, memo) + fibMemoImpl(n - 2, memo);
    return memo[n];
}
long long fibMemo(unsigned n) {
    std::vector<long long> memo(n + 1, -1);        // table sized for indices 0..n
    return fibMemoImpl(n, memo);
}

// Euclid's algorithm. BASE CASE: gcd(a, 0) = a. Each step replaces (a,b) with
// (b, a mod b); the second argument strictly shrinks, guaranteeing termination.
unsigned long long gcd(unsigned long long a, unsigned long long b) {
    if (b == 0) return a;             // base case
    return gcd(b, a % b);             // remainder shrinks -> reaches 0
}

// Fast power (exponentiation by squaring). BASE CASE: b^0 = 1.
// Even exponent: b^e = (b^(e/2))^2  -> one recursive call, halving e.
// Odd  exponent: b^e = b * b^(e-1)  -> peel one factor, then it is even.
long long ipow(long long base, unsigned exp) {
    if (exp == 0) return 1;                     // base case
    if (exp % 2 == 0) {                         // even: square the half-power
        long long half = ipow(base, exp / 2);
        return half * half;
    }
    return base * ipow(base, exp - 1);          // odd: peel one factor
}

// Recursive array sum over the suffix a[i .. n). BASE CASE: i == n -> 0.
long long sumArray(const std::vector<int>& a, std::size_t i) {
    if (i == a.size()) return 0;                // base case: empty suffix sums to 0
    return static_cast<long long>(a[i]) + sumArray(a, i + 1);
}

// Recursive in-place reverse of a[lo .. hi]. BASE CASE: lo >= hi (0/1 element).
// Because we only recurse while lo < hi, hi >= 1 there, so hi-1 never underflows.
void reverseImpl(std::vector<int>& a, std::size_t lo, std::size_t hi) {
    if (lo >= hi) return;                       // base case: nothing left to swap
    std::swap(a[lo], a[hi]);                    // fix the outer pair
    reverseImpl(a, lo + 1, hi - 1);             // shrink inward
}
void reverseArray(std::vector<int>& a) {
    if (a.empty()) return;                       // guard so hi = size-1 is valid
    reverseImpl(a, 0, a.size() - 1);
}

int main() {
    // ---- factorial: check against closed-form / known values ---------------
    assert(factorial(0) == 1ULL);
    assert(factorial(1) == 1ULL);
    assert(factorial(5) == 120ULL);
    assert(factorial(10) == 3628800ULL);
    // Iterative reference agrees for a range of n.
    unsigned long long facRef = 1;
    for (unsigned n = 0; n <= 15; ++n) {
        assert(factorial(n) == facRef);
        facRef *= (n + 1);
    }

    // ---- Fibonacci: naive == memoized == known sequence --------------------
    const long long fibKnown[] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55};
    for (unsigned n = 0; n <= 10; ++n) {
        assert(fibNaive(n) == fibKnown[n]);
        assert(fibMemo(n) == fibKnown[n]);
    }
    // The two implementations must agree; memo reaches much larger n cheaply.
    for (unsigned n = 0; n <= 25; ++n) assert(fibNaive(n) == fibMemo(n));
    assert(fibMemo(50) == 12586269025LL);   // exact known value F(50)

    // ---- gcd (Euclid) ------------------------------------------------------
    assert(gcd(48, 18) == 6);
    assert(gcd(18, 48) == 6);   // order-independent
    assert(gcd(17, 5) == 1);    // coprime
    assert(gcd(0, 5) == 5);     // gcd(0, b) = b
    assert(gcd(5, 0) == 5);     // base case directly
    assert(gcd(1071, 462) == 21);

    // ---- fast power vs. a simple iterative reference -----------------------
    assert(ipow(2, 0) == 1);
    assert(ipow(2, 10) == 1024);
    assert(ipow(5, 3) == 125);
    assert(ipow(3, 7) == 2187);
    for (long long b = -3; b <= 3; ++b) {
        for (unsigned e = 0; e <= 10; ++e) {
            long long ref = 1;
            for (unsigned k = 0; k < e; ++k) ref *= b;
            assert(ipow(b, e) == ref);
        }
    }

    // ---- recursive sum and reverse -----------------------------------------
    std::vector<int> nums = {3, 1, 4, 1, 5, 9, 2, 6};
    assert(sumArray(nums, 0) == 31);
    assert(sumArray(std::vector<int>{}, 0) == 0);      // empty -> 0
    assert(sumArray(std::vector<int>{-5, 5}, 0) == 0); // cancellation

    std::vector<int> rev = {1, 2, 3, 4, 5};
    reverseArray(rev);
    assert((rev == std::vector<int>{5, 4, 3, 2, 1}));
    std::vector<int> one = {42};
    reverseArray(one);
    assert((one == std::vector<int>{42}));             // single element unchanged
    std::vector<int> none;
    reverseArray(none);
    assert(none.empty());                              // empty stays empty

    // ---- short demo --------------------------------------------------------
    std::cout << "Recursion Basics demo\n";
    std::cout << "  10!            = " << factorial(10) << "\n";
    std::cout << "  fib(10)        = " << fibMemo(10) << "  (naive agrees: "
              << fibNaive(10) << ")\n";
    std::cout << "  gcd(1071,462)  = " << gcd(1071, 462) << "\n";
    std::cout << "  2^10           = " << ipow(2, 10) << "\n";
    std::cout << "  sum([3,1,4,1,5,9,2,6]) = " << sumArray(nums, 0) << "\n";
    std::cout << "  reverse([1..5])        = ";
    for (int x : rev) std::cout << x << ' ';
    std::cout << "\nAll assertions passed.\n";
    return 0;
}
