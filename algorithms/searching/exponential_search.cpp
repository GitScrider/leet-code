/*
 * Exponential Search (Algorithm - Searching)
 *
 * Idea:
 *   Search a SORTED range when the target is expected near the front, or when
 *   the range is unbounded/streamed and we do not know its length up front.
 *   Two phases:
 *     1) Range-finding: start with a bound of 1 and DOUBLE it (1, 2, 4, 8, ...)
 *        until a[bound] passes the target (or we run off the end). This locates
 *        an interval [bound/2, bound] that must contain the target if present,
 *        using only O(log i) comparisons where i is the target's position.
 *     2) Binary search within that interval.
 *   Invariant after phase 1: a[bound/2] <= target (that endpoint was checked
 *   before we doubled past it), so the target, if present, lies in the bracket
 *   we hand to binary search. Doubling guarantees the bound strictly grows, so
 *   phase 1 terminates; binary search halves its range, so phase 2 terminates.
 *
 * Complexity:
 *   +----------+-----------+
 *   |  Case    |   Time    |
 *   +----------+-----------+
 *   |  Best    |   O(1)    |  (target at index 0)
 *   |  Average | O(log i)  |  (i = index of the target)
 *   |  Worst   | O(log n)  |  (target near the end)
 *   +----------+-----------+
 *   Auxiliary Space: O(1)  (iterative)
 *
 * Complexity derivation (two phases: geometric doubling + binary search):
 *   Let i be the index where the target sits (or would sit) in the sorted array.
 *
 *   Phase 1 (range-finding by doubling). The probed bounds form a GEOMETRIC
 *   sequence 1, 2, 4, ..., 2^k, stopping at the first 2^k with a[2^k] >= target,
 *   i.e. the first 2^k >= i. That threshold is k = ceil(log2 i). One comparison
 *   is spent per term of the sequence:
 *
 *       C1(i) = SUM_{d=0}^{k} 1 = k + 1 = ceil(log2 i) + 1 = O(log i).
 *
 *   Phase 2 (binary search on the bracket). The surviving bracket is
 *   [2^(k-1), 2^k], whose width is 2^k - 2^(k-1) = 2^(k-1) elements. Binary
 *   search over w elements costs floor(log2 w) + 1 comparisons, so:
 *
 *       C2(i) = floor(log2( 2^(k-1) )) + 1 = (k - 1) + 1 = k = O(log i).
 *
 *   Adding the two phases:
 *
 *       C(i) = C1(i) + C2(i) = (k + 1) + k = 2k + 1 = 2*ceil(log2 i) + 1
 *            = O(log i).
 *
 *   WORST case: the target is near the end, i ~ n-1 (and the bound is clamped to
 *   n), so log i -> log n and C(n) = O(log n). BEST case: a[0] == target is the
 *   explicit fast path, C = 1 = O(1).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f = O(g)      iff  EXISTS c2, n0 :       f(n) <= c2*g(n)  for n >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   The cost is parameterised by the target position i, so bounds are per case:
 *     BEST  case  C = 1                    -> Theta(1)      (target at index 0).
 *     TYPICAL     C(i) = 2*ceil(log2 i)+1  -> Theta(log i)  (position-dependent).
 *     WORST case  i ~ n => C = O(log n)    -> Theta(log n)  (target near the end).
 *   Taking g(n) = log n and the worst-case f(n) = 2*log2 n + O(1):
 *     1 * log2 n <= f(n) <= 3 * log2 n  for n >= 2  =>  worst = Theta(log n).
 *   Over ALL inputs the time is O(log n) (upper, worst case) and Omega(1) (lower,
 *   best case). This is a comparison search on ALREADY-sorted data, so the
 *   comparison-SORT lower bound Omega(n log n) does not apply; the search lower
 *   bound Omega(log n) is met in the worst case and beaten (Theta(log i)) when
 *   the target lies near the front.
 *
 * Key points / when to use:
 *   - Faster than plain binary search when the target sits near the beginning,
 *     because the cost depends on i, not n.
 *   - The classic technique for searching an "infinite"/unknown-length sorted
 *     stream where you can index but not cheaply ask for the length.
 *   - Reuses an ordinary binary-search helper on the discovered bracket.
 *   - Returns std::optional<std::size_t>: an index if found, std::nullopt if not.
 */

#include <vector>
#include <optional>
#include <cassert>
#include <cstddef>
#include <algorithm>
#include <iostream>

// Standard binary search over the INCLUSIVE index range [lo, hi] of a sorted
// vector. Precondition: hi < a.size() (the caller clamps it). Returns the index
// of 'target' or std::nullopt.
template <typename T>
std::optional<std::size_t> binarySearchRange(const std::vector<T>& a,
                                             std::size_t lo, std::size_t hi,
                                             const T& target) {
    while (lo <= hi) {
        // 'lo + (hi - lo) / 2' instead of '(lo + hi) / 2' to avoid overflow.
        const std::size_t mid = lo + (hi - lo) / 2;
        if (a[mid] == target) {
            return mid;
        }
        if (a[mid] < target) {
            lo = mid + 1;                   // target is in the right half
        } else {
            if (mid == 0) break;            // guard: mid-1 would wrap std::size_t
            hi = mid - 1;                   // target is in the left half
        }
    }
    return std::nullopt;
}

// Exponential search entry point over a sorted vector.
template <typename T>
std::optional<std::size_t> exponentialSearch(const std::vector<T>& a, const T& target) {
    const std::size_t n = a.size();
    if (n == 0) {
        return std::nullopt;
    }
    // Check index 0 explicitly. This handles the "target at the front" fast path
    // and lets phase 1 safely start its bound at 1.
    if (a[0] == target) {
        return static_cast<std::size_t>(0);
    }

    // Phase 1 -- find a bracket by doubling. Stop as soon as a[bound] reaches or
    // exceeds the target, or the bound would leave the array.
    std::size_t bound = 1;
    while (bound < n && a[bound] < target) {
        bound *= 2;
    }

    // Phase 2 -- binary search in [bound/2, min(bound, n-1)]. The lower endpoint
    // was the last position known to be < target; the upper endpoint is clamped
    // inside the array.
    const std::size_t lo = bound / 2;
    const std::size_t hi = std::min(bound, n - 1);
    return binarySearchRange(a, lo, hi, target);
}

// ------------------------------- Tests & demo -------------------------------

// Linear reference: first index holding 'target', or std::nullopt.
template <typename T>
std::optional<std::size_t> linearFind(const std::vector<T>& a, const T& target) {
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i] == target) return i;
    }
    return std::nullopt;
}

int main() {
    // Empty range.
    assert(exponentialSearch<int>({}, 1) == std::nullopt);

    // Single element: found and not-found.
    assert(exponentialSearch<int>({42}, 42) == std::optional<std::size_t>(0));
    assert(exponentialSearch<int>({42}, 7)  == std::nullopt);

    // General sorted array: first, last, middle, and misses.
    {
        std::vector<int> a = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19};
        assert(exponentialSearch(a, 1)  == std::optional<std::size_t>(0));   // first
        assert(exponentialSearch(a, 19) == std::optional<std::size_t>(9));   // last
        assert(exponentialSearch(a, 9)  == std::optional<std::size_t>(4));   // middle
        assert(exponentialSearch(a, 8)  == std::nullopt);                    // gap
        assert(exponentialSearch(a, 0)  == std::nullopt);                    // below
        assert(exponentialSearch(a, 20) == std::nullopt);                    // above
        // Exhaustive cross-check against the linear reference.
        for (int v = -1; v <= 21; ++v) {
            assert(exponentialSearch(a, v) == linearFind(a, v));
        }
    }

    // Duplicates: the array stays sorted; any correct matching index is fine, so
    // we only assert that a value that exists is located and one that does not is
    // rejected (exponential search does not promise the first duplicate).
    {
        std::vector<int> dup = {2, 2, 2, 4, 4, 6, 6, 6, 6};
        auto r2 = exponentialSearch(dup, 2);
        assert(r2 && dup[*r2] == 2);
        auto r6 = exponentialSearch(dup, 6);
        assert(r6 && dup[*r6] == 6);
        assert(exponentialSearch(dup, 5) == std::nullopt);
    }

    // Sizes around powers of two to exercise the doubling / clamping boundaries.
    for (std::size_t n = 1; n <= 33; ++n) {
        std::vector<int> a(n);
        for (std::size_t i = 0; i < n; ++i) a[i] = static_cast<int>(i) * 2;  // 0,2,4,...
        for (int v = -2; v <= static_cast<int>(2 * n); ++v) {
            assert(exponentialSearch(a, v) == linearFind(a, v));
        }
    }

    // Short demo.
    std::vector<int> demo = {1, 2, 4, 8, 16, 32, 64};
    const int key = 16;
    auto r = exponentialSearch(demo, key);
    std::cout << "searching for " << key << " -> ";
    if (r) std::cout << "found at index " << *r << '\n';
    else   std::cout << "not found\n";

    std::cout << "All exponential search tests passed.\n";
    return 0;
}
