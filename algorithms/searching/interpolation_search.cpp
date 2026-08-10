/*
 * Interpolation Search (Algorithm - Searching)
 *
 * Idea:
 *   A refinement of binary search for a SORTED range of UNIFORMLY distributed
 *   numeric keys. Instead of always probing the midpoint, we *estimate* where
 *   the target should sit by linear interpolation between the current endpoints:
 *
 *       pos = lo + (target - a[lo]) * (hi - lo) / (a[hi] - a[lo])
 *
 *   i.e. "target covers this fraction of the value-range, so look at the same
 *   fraction of the index-range". If the data really is evenly spaced (think a
 *   phone book), the probe lands very close to the target and the range shrinks
 *   dramatically each step. Precondition: the range must be sorted ascending.
 *   Loop invariant: if the target exists it lies within [lo, hi]; every step
 *   either finds it or discards a[pos] plus everything on the wrong side, so the
 *   range strictly shrinks and the loop terminates.
 *
 * Complexity:
 *   +----------+---------------+
 *   |  Case    |     Time      |
 *   +----------+---------------+
 *   |  Best    |     O(1)      |
 *   |  Average | O(log log n)  |  (keys uniformly distributed)
 *   |  Worst   |     O(n)      |  (skewed / clustered keys)
 *   +----------+---------------+
 *   Auxiliary Space: O(1)  (iterative, a few index variables)
 *
 * Complexity derivation (probe-count recurrence, uniform-key model):
 *   Let R(n) be the expected number of PROBES to search n keys drawn uniformly
 *   at random from [a[lo], a[hi]]. A classic result for such data is that after
 *   ONE interpolation probe the expected number of keys still bracketed drops
 *   from n to about sqrt(n) (the probe's positional error has standard deviation
 *   ~sqrt(n)). This yields the recurrence:
 *
 *       R(n) = 1 + R(sqrt(n)) ,      R(2) = O(1)     (base: 1-2 keys left)
 *
 *   Track the surviving bracket size after each probe:
 *
 *       probe d      surviving size
 *       ---------    --------------------
 *       d = 0        n            = n^(1/2^0)
 *       d = 1        sqrt(n)      = n^(1/2^1)
 *       d = 2        n^(1/4)      = n^(1/2^2)
 *       d = k        n^(1/2^k)
 *
 *   The search bottoms out when the size falls to a constant (2):
 *       n^(1/2^k) = 2  <=>  (1/2^k) * log2 n = 1  <=>  2^k = log2 n
 *                      <=>  k = log2(log2 n).
 *   So R(n) = k + O(1) = O(log log n) probes, each O(1) arithmetic + 1 compare.
 *
 *   WORST case (skewed / clustered keys): a probe can eliminate as few as ONE
 *   element, so the bracket shrinks by 1 per step and
 *       C_worst(n) = SUM_{i=0}^{n-1} 1 = n = O(n).
 *   BEST case: the very first probe lands on the target -> C_best(n) = 1 = O(1).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f(n) = O(g)      iff  EXISTS c2, n0 :       f(n) <= c2*g(n)  for n >= n0
 *     f(n) = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f(n) = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   The cost is strongly DATA-DEPENDENT, so bounds are stated per case:
 *     BEST  case  C(n) = 1                   -> Theta(1).
 *     AVG   case  R(n) = log2(log2 n) + O(1) -> Theta(log log n) under the
 *                 uniform model (c1*log log n <= R(n) <= c2*log log n, n >= 4).
 *     WORST case  C(n) = n                   -> Theta(n) on adversarial/skewed data.
 *   Over ALL inputs the running time is therefore O(n) (upper bound, worst case)
 *   and Omega(1) (lower bound, best case); there is no single Theta across all
 *   inputs because best, average and worst differ. Interpolation search is NOT a
 *   comparison sort, so the sorting lower bound Omega(n log n) is irrelevant, and
 *   the log log n average even beats binary search's log n precisely by doing
 *   ARITHMETIC on key VALUES rather than relying on comparisons alone.
 *
 * Key points / when to use:
 *   - Beats binary search's O(log n) ONLY when keys are roughly uniform; on
 *     skewed data every probe barely advances and it degrades to O(n).
 *   - Must guard against division by zero when a[hi] == a[lo] (a flat range).
 *   - Restrict the probe to numeric keys where (b - a) and division make sense.
 *   - Returns std::optional<std::size_t>: an index if found, std::nullopt if not.
 */

#include <vector>
#include <optional>
#include <cassert>
#include <cstddef>
#include <iostream>

// Generic over a numeric, comparable key type T (int, long, double, ...).
// Returns the index of 'target' in the sorted vector 'a', or std::nullopt.
template <typename T>
std::optional<std::size_t> interpolationSearch(const std::vector<T>& a, const T& target) {
    if (a.empty()) {
        return std::nullopt;
    }

    // Signed bounds so we can safely write hi = pos - 1 even when pos == 0
    // without the std::size_t wrap-around that would make the loop run forever.
    long long lo = 0;
    long long hi = static_cast<long long>(a.size()) - 1;

    // Continue only while the target is still bracketed by the endpoint values.
    // Once target < a[lo] or target > a[hi], it cannot be in the range at all.
    while (lo <= hi && target >= a[lo] && target <= a[hi]) {
        // Guard: if the endpoints hold equal values the denominator is zero.
        // Because target is within [a[lo], a[hi]] and those are equal, either the
        // whole flat range equals target (report lo) or the target is absent.
        if (a[hi] == a[lo]) {
            return (a[lo] == target) ? std::optional<std::size_t>(static_cast<std::size_t>(lo))
                                     : std::nullopt;
        }

        // Interpolate the probe position. The fraction is kept in double so that
        // the multiplication cannot overflow the index type; a[hi] > a[lo] here,
        // so the denominator is strictly positive and the fraction is in [0, 1].
        const double frac = static_cast<double>(target - a[lo]) /
                            static_cast<double>(a[hi] - a[lo]);
        long long pos = lo + static_cast<long long>(frac * static_cast<double>(hi - lo));

        // Floating rounding could nudge pos just outside [lo, hi]; clamp it so the
        // subsequent index accesses stay in bounds.
        if (pos < lo) pos = lo;
        if (pos > hi) pos = hi;

        const std::size_t p = static_cast<std::size_t>(pos);
        if (a[p] == target) {
            return p;                       // exact hit
        }
        if (a[p] < target) {
            lo = pos + 1;                   // discard a[p] and everything left of it
        } else {
            hi = pos - 1;                   // discard a[p] and everything right of it
        }
    }
    return std::nullopt;
}

// ------------------------------- Tests & demo -------------------------------

// Linear reference: the first index holding 'target', or std::nullopt.
template <typename T>
std::optional<std::size_t> linearFind(const std::vector<T>& a, const T& target) {
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i] == target) return i;
    }
    return std::nullopt;
}

int main() {
    // Empty range: nothing can be found.
    assert(interpolationSearch<int>({}, 5) == std::nullopt);

    // Single element: found and not-found.
    assert(interpolationSearch<int>({7}, 7) == std::optional<std::size_t>(0));
    assert(interpolationSearch<int>({7}, 9) == std::nullopt);

    // Uniformly spaced data -- the ideal case for interpolation search.
    {
        std::vector<int> uni = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
        assert(interpolationSearch(uni, 0)  == std::optional<std::size_t>(0));   // first
        assert(interpolationSearch(uni, 90) == std::optional<std::size_t>(9));   // last
        assert(interpolationSearch(uni, 40) == std::optional<std::size_t>(4));   // middle
        assert(interpolationSearch(uni, 25) == std::nullopt);                    // gap
        assert(interpolationSearch(uni, -5) == std::nullopt);                    // below
        assert(interpolationSearch(uni, 95) == std::nullopt);                    // above
        // Cross-check every position against the linear reference.
        for (int v = -5; v <= 95; ++v) {
            assert(interpolationSearch(uni, v) == linearFind(uni, v));
        }
    }

    // Clustered / skewed data -- correctness must still hold (just slower).
    {
        std::vector<int> skew = {1, 2, 3, 4, 5, 6, 1000};
        for (int v = 0; v <= 1001; ++v) {
            assert(interpolationSearch(skew, v) == linearFind(skew, v));
        }
    }

    // Flat range (all equal) exercises the division-by-zero guard.
    {
        std::vector<int> flat = {5, 5, 5, 5};
        assert(interpolationSearch(flat, 5).has_value());   // some index of a 5
        assert(interpolationSearch(flat, 4) == std::nullopt);
        assert(interpolationSearch(flat, 6) == std::nullopt);
    }

    // Works for floating-point keys as well.
    {
        std::vector<double> d = {1.5, 2.5, 3.5, 4.5};
        assert(interpolationSearch(d, 3.5) == std::optional<std::size_t>(2));
        assert(interpolationSearch(d, 3.0) == std::nullopt);
    }

    // Short demo.
    std::vector<int> demo = {2, 4, 6, 8, 10, 12, 14, 16};
    const int key = 12;
    auto r = interpolationSearch(demo, key);
    std::cout << "searching for " << key << " -> ";
    if (r) std::cout << "found at index " << *r << '\n';
    else   std::cout << "not found\n";

    std::cout << "All interpolation search tests passed.\n";
    return 0;
}
