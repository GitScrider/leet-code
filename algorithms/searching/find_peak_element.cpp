/*
 * Find Peak Element (Algorithm - Searching)
 *
 * Idea:
 *   A "peak" is an index i with a[i] >= its neighbours, where any out-of-range
 *   neighbour is treated as -infinity (so the array ends always look "downhill"
 *   to the outside). At least one peak therefore always exists in a non-empty
 *   array. We do NOT need the input to be sorted: we binary-search on the SLOPE.
 *
 *   At mid, compare a[mid] with a[mid+1]:
 *     - a[mid] < a[mid+1]  -> the slope rises to the right, so a peak must exist
 *                             somewhere in (mid, hi]; move lo = mid + 1.
 *     - otherwise          -> a[mid] is >= its right neighbour, so a peak exists
 *                             in [lo, mid]; move hi = mid.
 *   Invariant: the surviving range [lo, hi] always contains a peak, because we
 *   only ever discard a side that is guaranteed to keep climbing back toward the
 *   part we keep. Since mid < hi whenever lo < hi, the range strictly shrinks and
 *   the loop converges to lo == hi, which is a peak.
 *
 * Complexity:
 *   +----------+-----------+
 *   |  Case    |   Time    |
 *   +----------+-----------+
 *   |  Best    |   O(1)    |
 *   |  Average | O(log n)  |
 *   |  Worst   | O(log n)  |
 *   +----------+-----------+
 *   Auxiliary Space: O(1)  (iterative)
 *
 * Complexity derivation (slope binary search: recurrence -> recursion tree):
 *   Let T(n) count the basic operations for a candidate range holding n indices.
 *   Each loop turn does O(1) work (one midpoint, one comparison a[mid] vs
 *   a[mid+1]) and then keeps exactly one half of the range: either lo = mid+1
 *   (drops the closed left part [lo, mid]) or hi = mid (drops the open right part
 *   (mid, hi]). Because mid = lo + (hi-lo)/2, each surviving half holds at most
 *   ceil(n/2) indices. This gives the recurrence:
 *
 *       T(n) = T(n/2) + c ,      T(1) = c0        (base: lo == hi, loop ends)
 *
 *   Unfold it as a recursion tree -- a single chain, no branching:
 *
 *       level d      #nodes      size each        work on the level
 *       ---------    --------    -------------    -----------------
 *       d = 0        1           n                c
 *       d = 1        1           n/2              c
 *       d = 2        1           n/4              c
 *       d = k        1           n/2^k            c
 *
 *   The chain ends when n/2^k = 1, i.e. k = log2 n, so there are (log2 n + 1)
 *   levels each costing c:
 *
 *       T(n) = SUM_{d=0}^{log2 n} c = c*(log2 n + 1) = O(log n).
 *
 *   Master Theorem check (a=1, b=2, f(n)=c): n^(log_b a) = n^(log2 1) = n^0 = 1,
 *   same order as f(n)=c -> case 2 -> T(n) = Theta(n^0 * log n) = Theta(log n).
 *   There is NO data-dependent early exit -- the loop always runs down to
 *   lo == hi. But the two branches do NOT shrink the range equally: from a range
 *   of w indices, hi = mid keeps ceil(w/2) indices while lo = mid+1 keeps only
 *   floor(w/2), and these differ by 1 whenever w is odd. So the EXACT iteration
 *   count is data-dependent, ranging from floor(log2 n) (whenever the faster
 *   floor(w/2) shrink is taken every step) up to ceil(log2 n) (the slower
 *   ceil(w/2) every step); the two coincide only when n is a power of 2. Example:
 *   n = 3 takes 1 iteration if a[1] < a[2] (lo jumps straight to hi) but 2
 *   iterations otherwise -- floor(log2 3) = 1, ceil(log2 3) = 2. Either way the
 *   count is log2 n +/- O(1), so T(n) = Theta(log n) is unaffected. The O(1)
 *   "best" row is only the degenerate n = 1 case, where lo == hi and the loop body
 *   never runs.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f = O(g)      iff  EXISTS c2, n0 :       f(n) <= c2*g(n)  for n >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   Here the loop runs I(n) times with floor(log2 n) <= I(n) <= ceil(log2 n) (see
 *   above: only a +/- 1 wobble set by which branches are taken, never a change of
 *   order), so f(n) = c*I(n) + c0. Using floor(log2 n) >= (1/2)*log2 n and
 *   ceil(log2 n) <= log2 n + 1 for n >= 2, with g(n) = log n:
 *     upper  O:     f(n) <= (2c + c0) * log2 n  for n >= 2  => O(log n)
 *     lower  Omega: f(n) >=  (c/2)    * log2 n  for n >= 2  => Omega(log n)
 *     tight  Theta: both hold (c1 = c/2, c2 = 2c + c0)      => Theta(log n)
 *   The SAME Theta(log n) is thus the tight bound for best, average AND worst on
 *   any array of size n >= 2 (the +/- 1 iteration wobble stays well inside this
 *   Theta; the O(1) row is only the n = 1 input). This is a
 *   comparison search, not a sort, so the sorting bound Omega(n log n) does not
 *   apply; it even beats the Omega(n) that locating a specific VALUE in UNSORTED
 *   data would need, because inspecting the local SLOPE (not the values) lets
 *   each comparison prune half of the remaining indices.
 *
 * Key points / when to use:
 *   - Any peak is acceptable; this is NOT about the global maximum.
 *   - Works on unsorted data because only the local slope is inspected.
 *   - Edges count as peaks (outside neighbours are -infinity); plateaus are
 *     handled by treating a[mid] >= a[mid+1] as "go left".
 *   - Returns std::optional<std::size_t>: an index for non-empty input, else
 *     std::nullopt (an empty array has no peak).
 */

#include <vector>
#include <optional>
#include <cassert>
#include <cstddef>
#include <iostream>

// Returns the index of A peak in 'a', or std::nullopt if 'a' is empty.
template <typename T>
std::optional<std::size_t> findPeakElement(const std::vector<T>& a) {
    if (a.empty()) {
        return std::nullopt;
    }

    std::size_t lo = 0;
    std::size_t hi = a.size() - 1;
    while (lo < hi) {
        // 'lo + (hi - lo) / 2' avoids overflow; because lo < hi we always get
        // mid < hi, so mid + 1 is a valid index and hi = mid strictly shrinks.
        const std::size_t mid = lo + (hi - lo) / 2;
        if (a[mid] < a[mid + 1]) {
            lo = mid + 1;                   // uphill to the right: peak is right
        } else {
            hi = mid;                       // downhill or flat: peak is here/left
        }
    }
    return lo;                              // lo == hi: guaranteed a peak
}

// ------------------------------- Tests & demo -------------------------------

// Reference peak check with out-of-range neighbours treated as -infinity.
template <typename T>
bool isPeak(const std::vector<T>& a, std::size_t i) {
    const bool okLeft  = (i == 0)              || (a[i] >= a[i - 1]);
    const bool okRight = (i + 1 == a.size())   || (a[i] >= a[i + 1]);
    return okLeft && okRight;
}

int main() {
    // Empty range: no peak.
    assert(findPeakElement<int>({}) == std::nullopt);

    // Single element: it is trivially a peak (both neighbours are -infinity).
    {
        auto r = findPeakElement<int>({99});
        assert(r == std::optional<std::size_t>(0));
    }

    // Peak at the left end (strictly descending array).
    {
        std::vector<int> a = {5, 4, 3, 2, 1};
        auto r = findPeakElement(a);
        assert(r && isPeak(a, *r));
        assert(*r == 0);
    }

    // Peak at the right end (strictly ascending array).
    {
        std::vector<int> a = {1, 2, 3, 4, 5};
        auto r = findPeakElement(a);
        assert(r && isPeak(a, *r));
        assert(*r == 4);
    }

    // Peak in the middle.
    {
        std::vector<int> a = {1, 3, 7, 4, 2};
        auto r = findPeakElement(a);
        assert(r && isPeak(a, *r));
        assert(*r == 2);
    }

    // Two-element arrays: whichever endpoint is larger (or either if equal).
    {
        std::vector<int> a = {1, 2};
        auto r = findPeakElement(a);
        assert(r && isPeak(a, *r));
    }
    {
        std::vector<int> a = {2, 1};
        auto r = findPeakElement(a);
        assert(r && isPeak(a, *r));
    }

    // Plateaus: equal runs. Whatever index is returned must satisfy the peak
    // predicate (a[i] >= both neighbours).
    {
        std::vector<int> a = {2, 2, 2};
        auto r = findPeakElement(a);
        assert(r && isPeak(a, *r));
    }
    {
        std::vector<int> a = {1, 2, 2, 1};
        auto r = findPeakElement(a);
        assert(r && isPeak(a, *r));
    }
    {
        std::vector<int> a = {3, 1, 1, 3};
        auto r = findPeakElement(a);
        assert(r && isPeak(a, *r));
    }

    // Exhaustive: for many hand-picked arrays the result is always a valid peak.
    {
        std::vector<std::vector<int>> cases = {
            {1}, {1, 2, 1}, {1, 2, 3, 1}, {3, 2, 1, 2, 3},
            {1, 1, 1, 1, 2, 1}, {10, 20, 15, 2, 23, 90, 67},
            {1, 2, 3, 4, 5, 6}, {6, 5, 4, 3, 2, 1}, {5, 5, 4, 3},
        };
        for (const auto& a : cases) {
            auto r = findPeakElement(a);
            assert(r && isPeak(a, *r));
        }
    }

    // Short demo.
    std::vector<int> demo = {1, 3, 20, 4, 1, 0};
    auto r = findPeakElement(demo);
    std::cout << "array:";
    for (int x : demo) std::cout << ' ' << x;
    std::cout << "\npeak at index " << *r << " with value " << demo[*r] << '\n';

    std::cout << "All find-peak tests passed.\n";
    return 0;
}
