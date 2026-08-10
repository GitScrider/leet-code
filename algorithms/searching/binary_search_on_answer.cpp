/*
 * Binary Search on the Answer (Algorithm - Searching)
 *
 * Idea:
 *   Many optimisation problems are not searches over an array but over a RANGE
 *   OF CANDIDATE ANSWERS on which some predicate is MONOTONE. If a boolean
 *   predicate pred(x) is false for all small x and true for all large x (or vice
 *   versa), the values look like:
 *
 *       x:      lo ............................ hi
 *       pred:   F  F  F  F  F  T  T  T  T  T  T        (monotone false -> true)
 *                             ^ the boundary we want
 *
 *   Binary search finds the exact flip point in O(log(range)) predicate calls:
 *     - firstTrue:  smallest x with pred(x) == true  (needs pred(hi) true).
 *     - lastTrue :  largest  x with pred(x) == true  (needs pred(lo) true).
 *   The trick is to *design a monotone predicate* for your problem; the search
 *   itself is the same skeleton every time.
 *
 *   Two worked examples below:
 *     (1) integer square root  floor(sqrt(n))  -- largest x with x*x <= n.
 *     (2) Koko / minimum ship capacity style   -- smallest speed k such that the
 *         total time to finish is within the allowed limit H.
 *
 * Complexity (per problem, R = size of the answer range):
 *   +----------+---------------------------+
 *   |  Case    |          Time             |
 *   +----------+---------------------------+
 *   |  Best    | O(log R) * cost(pred)     |
 *   |  Average | O(log R) * cost(pred)     |
 *   |  Worst   | O(log R) * cost(pred)     |
 *   +----------+---------------------------+
 *   Auxiliary Space: O(1)  (iterative; predicate may use its own O(1) scratch)
 *
 * Complexity derivation (interval halving / summation):
 *   Let R = hi - lo + 1 be the number of candidate answers in the closed range
 *   [lo, hi]. Both skeletons run the SAME loop: while (lo < hi) shrink [lo,hi].
 *   Let w_d = (hi - lo + 1) be the count of still-live candidates at the START
 *   of iteration d. Each iteration does O(1) index arithmetic + EXACTLY ONE
 *   predicate call of cost P = cost(pred), and discards at least floor(w_d/2)
 *   candidates, so w_{d+1} <= ceil(w_d / 2). The loop stops when w = 1 (lo==hi):
 *
 *       iteration d      candidates w_d      pred calls on the level
 *       -----------      ----------------    -----------------------
 *       d = 0            R                   1
 *       d = 1            <= ceil(R/2)        1
 *       d = 2            <= ceil(R/4)        1
 *       ...              ...                 ...
 *       d = t-1          2                   1
 *       (stop)           1                   -
 *
 *   The width halves each step: R -> ceil(R/2) -> ceil(R/4) -> ... -> 1, so the
 *   number of iterations is t = ceil(log2 R). Summing the per-iteration cost:
 *
 *       C(R) = SUM_{d=0}^{t-1} (P + c) = t * (P + c)
 *            = ceil(log2 R) * (cost(pred) + O(1))
 *            = O(log R) * cost(pred)
 *
 *   Which branch is taken sets the new width to ceil(w/2) (pred true, hi = mid)
 *   or floor(w/2) (pred false, lo = mid+1); these differ only when w is odd, so
 *   the EXACT iteration count can vary by +/-1 across inputs of the same R,
 *   lying between floor(log2 R) and ceil(log2 R) (t = ceil(log2 R) is the upper
 *   count used above). That +/-1 wobble is a lower-order effect and does not
 *   change the asymptotics: best = average = worst = Theta(log R) predicate
 *   calls -> hence the three identical table rows.
 *   Worked costs of the two examples:
 *     (1) isqrt(n):        range [0, n] => R = n+1; pred = one division+compare
 *                          = O(1). Total O(log n) * O(1) = O(log n).
 *     (2) minEatingSpeed:  range [1, max(pile)] => R = max(pile); pred =
 *                          hoursToFinish scans all m piles = O(m).
 *                          Total O(m * log(max pile)).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f = O(g)      iff  EXISTS c2, n0 :       f(R) <= c2*g(R)  for R >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g(R) <= f(R)        for R >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   Here f(R) = ceil(log2 R) predicate calls; take g(R) = log R = log2 R:
 *     upper  O:     f(R) <= log2 R + 1 <= 2*log2 R  for R >= 2 => O(log R)
 *     lower  Omega: f(R) >= log2 R                   for R >= 1 => Omega(log R)
 *     tight  Theta: both hold (c1 = 1, c2 = 2)       => Theta(log R)
 *   The loop has NO data-dependent early exit (it always contracts to one point);
 *   the exact iteration count still varies by at most +/-1 with R -- between
 *   floor(log2 R) and ceil(log2 R), depending on the branch sequence -- but this
 *   lower-order wobble leaves best = average = worst = Theta(log R), a SINGLE
 *   tight bound -- unlike an adaptive algorithm. Multiply by cost(pred)
 *   for the running time. Lower-bound note: this is a search, not a comparison
 *   sort, so the Omega(n log n) sorting bound does NOT apply. The relevant limit
 *   is information-theoretic: distinguishing R possible answers needs log2 R
 *   yes/no outcomes and each predicate call yields exactly 1 bit, so ANY correct
 *   method needs >= ceil(log2 R) calls -- binary search meets it and is optimal.
 *
 * Key points / when to use:
 *   - Applicable whenever "is an answer of x feasible?" is monotone in x.
 *   - Use lo + (hi - lo)/2 to avoid overflow; bias the mid UP for lastTrue so the
 *     range still shrinks when lo = mid is taken.
 *   - Comparisons are kept overflow-free (e.g. x <= n/x instead of x*x <= n).
 *   - Values (not array indices) are returned; validate against brute force.
 */

#include <vector>
#include <cassert>
#include <cstdint>
#include <algorithm>
#include <iostream>

// -------------------- Generic monotone-boundary skeletons --------------------

// Smallest x in [lo, hi] with pred(x) == true.
// Precondition: pred is monotone false...false,true...true AND pred(hi) is true.
template <typename Pred>
long long firstTrue(long long lo, long long hi, Pred pred) {
    while (lo < hi) {
        // Lower mid (floor). Since mid < hi, the branch hi = mid strictly shrinks
        // the range, guaranteeing termination.
        const long long mid = lo + (hi - lo) / 2;
        if (pred(mid)) {
            hi = mid;                       // mid works; the boundary is mid or left
        } else {
            lo = mid + 1;                   // mid fails; the boundary is strictly right
        }
    }
    return lo;                              // lo == hi == first true
}

// Largest x in [lo, hi] with pred(x) == true.
// Precondition: pred is monotone true...true,false...false AND pred(lo) is true.
template <typename Pred>
long long lastTrue(long long lo, long long hi, Pred pred) {
    while (lo < hi) {
        // Upper mid (ceil). Biasing UP guarantees mid > lo, so the branch lo = mid
        // strictly shrinks the range and the loop terminates.
        const long long mid = lo + (hi - lo + 1) / 2;
        if (pred(mid)) {
            lo = mid;                       // mid works; the boundary is mid or right
        } else {
            hi = mid - 1;                   // mid fails; the boundary is strictly left
        }
    }
    return lo;                              // lo == hi == last true
}

// ------------------------- Example 1: integer sqrt --------------------------

// floor(sqrt(n)) for n >= 0, via "largest x with x*x <= n".
// Overflow-free feasibility: for x >= 1, (x*x <= n) is EXACTLY (x <= n / x)
// using integer division, so we never compute x*x.
long long isqrt(long long n) {
    assert(n >= 0);
    // Search x in [0, n]. The predicate is true at x = 0 (0 <= n) as required by
    // lastTrue; inside the loop mid >= 1, so the division n / mid is always safe.
    return lastTrue(0, n, [n](long long x) {
        return x == 0 || x <= n / x;
    });
}

// -------------- Example 2: minimum eating speed (Koko bananas) --------------

// Hours needed to finish all piles at speed k (>= 1): sum of ceil(pile / k).
long long hoursToFinish(const std::vector<long long>& piles, long long k) {
    assert(k >= 1);
    long long total = 0;
    for (long long p : piles) {
        total += (p + k - 1) / k;           // ceil division, overflow-safe for our sizes
    }
    return total;
}

// Smallest speed k in [1, max(pile)] so that hoursToFinish <= H.
// feasible(k) = (hoursToFinish(k) <= H) is monotone: raising k never increases
// the time, so once feasible it stays feasible -> use firstTrue.
long long minEatingSpeed(const std::vector<long long>& piles, long long H) {
    assert(!piles.empty());
    assert(H >= static_cast<long long>(piles.size()));   // at k=max, hours == #piles
    const long long hi = *std::max_element(piles.begin(), piles.end());
    return firstTrue(1, hi, [&piles, H](long long k) {
        return hoursToFinish(piles, k) <= H;
    });
}

// ------------------------------- Tests & demo -------------------------------

// Brute-force references for small inputs.
long long isqrtBrute(long long n) {
    long long x = 0;
    while ((x + 1) * (x + 1) <= n) ++x;
    return x;
}

long long minEatingSpeedBrute(const std::vector<long long>& piles, long long H) {
    for (long long k = 1; ; ++k) {
        if (hoursToFinish(piles, k) <= H) return k;
    }
}

int main() {
    // ---- integer sqrt ----
    assert(isqrt(0) == 0);                  // smallest
    assert(isqrt(1) == 1);
    assert(isqrt(2) == 1);
    assert(isqrt(3) == 1);
    assert(isqrt(4) == 2);                  // perfect square
    assert(isqrt(8) == 2);
    assert(isqrt(9) == 3);
    assert(isqrt(15) == 3);
    assert(isqrt(16) == 4);
    // Exhaustive brute-force cross-check for small n.
    for (long long n = 0; n <= 2000; ++n) {
        assert(isqrt(n) == isqrtBrute(n));
    }
    // A couple of larger values (still overflow-free thanks to x <= n/x).
    assert(isqrt(1000000) == 1000);         // 1000^2 exactly
    assert(isqrt(1000000000000LL) == 1000000);

    // ---- minimum eating speed ----
    {
        // Classic case: piles {3,6,7,11}, H = 8  ->  k = 4.
        std::vector<long long> piles = {3, 6, 7, 11};
        assert(minEatingSpeed(piles, 8) == 4);
    }
    {
        std::vector<long long> piles = {30, 11, 23, 4, 20};
        assert(minEatingSpeed(piles, 5)  == 30);   // one pile per hour, need the max
        assert(minEatingSpeed(piles, 6)  == 23);
    }
    {
        // Single pile: split into H chunks -> ceil(pile / H).
        std::vector<long long> piles = {12};
        assert(minEatingSpeed(piles, 1) == 12);
        assert(minEatingSpeed(piles, 3) == 4);
        assert(minEatingSpeed(piles, 5) == 3);     // ceil(12/3)=4 hours <= 5, ceil(12/4)=3<=5, ceil(12/5)=3>? -> k=3 gives 4h
    }
    // Brute-force cross-check across many piles and time budgets.
    {
        std::vector<std::vector<long long>> cases = {
            {1}, {5}, {1, 1, 1}, {3, 6, 7, 11}, {30, 11, 23, 4, 20},
            {8, 8, 8}, {2, 4, 6, 8, 10},
        };
        for (const auto& piles : cases) {
            const long long minH = static_cast<long long>(piles.size());
            long long sum = 0;
            for (long long p : piles) sum += p;     // H = sum -> speed 1 is feasible
            for (long long H = minH; H <= sum; ++H) {
                assert(minEatingSpeed(piles, H) == minEatingSpeedBrute(piles, H));
            }
        }
    }

    // ---- direct skeleton sanity check ----
    {
        // Boundary of "x >= 5" over [0, 10] via firstTrue is 5; lastTrue of
        // "x <= 5" is 5. Confirms both idioms pin the exact flip point.
        assert(firstTrue(0, 10, [](long long x){ return x >= 5; }) == 5);
        assert(lastTrue (0, 10, [](long long x){ return x <= 5; }) == 5);
    }

    // Short demo.
    std::cout << "isqrt(50) = " << isqrt(50) << '\n';                 // 7
    std::vector<long long> demo = {3, 6, 7, 11};
    std::cout << "min eating speed for {3,6,7,11}, H=8 = "
              << minEatingSpeed(demo, 8) << '\n';                     // 4

    std::cout << "All binary-search-on-answer tests passed.\n";
    return 0;
}
