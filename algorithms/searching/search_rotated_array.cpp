/*
 * Search in Rotated Sorted Array (Algorithm - Searching)
 *
 * Idea:
 *   We are given an array that was originally sorted in strictly ascending
 *   order (NO duplicates) and then rotated at some unknown pivot. For example
 *   [0,1,2,4,5,6,7] rotated by 3 becomes [4,5,6,7,0,1,2]. We want the index of
 *   a target value in O(log n) using a single modified binary search.
 *
 *   Invariant / case analysis at each step (lo <= hi):
 *     Let mid = lo + (hi - lo)/2. Split [lo..hi] into [lo..mid] and [mid..hi].
 *     Because there is exactly one rotation point, AT LEAST ONE of the two
 *     halves is fully sorted (contiguous ascending). We detect which:
 *       - If a[lo] <= a[mid], the LEFT half [lo..mid] is sorted.
 *           * If target lies inside that sorted range (a[lo] <= target < a[mid])
 *             search left  -> hi = mid - 1; otherwise search right -> lo = mid+1.
 *       - Else the RIGHT half [mid..hi] is sorted.
 *           * If target lies inside it (a[mid] < target <= a[hi])
 *             search right -> lo = mid + 1; otherwise search left -> hi = mid-1.
 *     Each branch discards at least one element, so the range strictly shrinks
 *     and the loop terminates. Correctness: we only recurse into a side that
 *     provably can contain the target, so we never miss it.
 *
 * Complexity:
 *   +-----------+----------+----------+----------+
 *   | Case      | Best     | Average  | Worst    |
 *   +-----------+----------+----------+----------+
 *   | Time      | O(1)     | O(log n) | O(log n) |
 *   | Space     | O(1)     | O(1)     | O(1)     |
 *   +-----------+----------+----------+----------+
 *   (Best case: the target happens to be at the first probed mid.)
 *
 * Complexity derivation (recurrence -> iteration table -> summation):
 *   Each while-iteration computes one mid, does a constant number of key
 *   comparisons, then DISCARDS one side of the split. Whichever branch runs,
 *   the surviving range [lo..hi] loses either [lo..mid] or [mid..hi], so a range
 *   of n = hi-lo+1 elements shrinks to at most floor(n/2). Let T(n) count the
 *   probes on a range of size n:
 *
 *       T(n) = T(n/2) + c ,        T(1) = c0        (base case: one probe)
 *
 *   Unfold this single-branch recursion as an iteration table (range size only):
 *
 *       iteration k    surviving size      work on the step
 *       -----------    ----------------    ----------------
 *       k = 0          n                   c
 *       k = 1          <= n/2              c
 *       k = 2          <= n/4              c
 *       ...            ...                 ...
 *       k = K          <= n/2^K            c
 *
 *   The loop stops once the range holds a single element, i.e. n/2^K <= 1, giving
 *   K = floor(log2 n). Summing the constant work over all steps:
 *
 *       T(n) = SUM_{k=0}^{log2 n} c = c*(log2 n + 1) = O(log n)
 *
 *   Master Theorem (a=1, b=2, f(n)=c=Theta(1)): n^(log_b a) = n^(log2 1) = n^0
 *   = 1, the same order as f(n) -> case 2 -> T(n) = Theta(n^0 * log n)
 *   = Theta(log n). Best case: target equals a[mid] on the FIRST probe -> exactly
 *   1 step -> O(1).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f(n) = O(g)      iff  EXISTS c2, n0 :       f(n) <= c2*g(n)  for n >= n0
 *     f(n) = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f(n) = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   The probe count is data-dependent, so the bound is PER CASE:
 *     WORST/AVERAGE  T(n) = c*(log2 n + 1) with g = log n: (c/2)*log2 n <= T(n)
 *                    <= 2c*log2 n for n >= 2  =>  Theta(log n) (tight).
 *     BEST case      target hits the first mid -> T(n) = c  =>  Theta(1) (tight).
 *   Over ALL inputs the running time is therefore O(log n) (upper, from the worst
 *   case) and Omega(1) (lower, from the best case), not a single Theta -- which is
 *   exactly why the table splits Best from Average/Worst.
 *
 * Key points / when to use:
 *   - Requires a range that is sorted ascending then rotated, with NO duplicates
 *     (duplicates break the "one half is sorted" test and degrade to O(n)).
 *   - One binary search; no need to first find the pivot separately.
 *   - Great for lookups in a rotated log/ring buffer that stays sorted-ish.
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <optional>
#include <vector>

// Returns the index of `target` in a rotated ascending array with no
// duplicates, or std::nullopt if absent. We use std::optional<std::size_t>
// as the explicit "not found" sentinel.
template <typename T>
std::optional<std::size_t> search_rotated(const std::vector<T>& a, const T& target) {
    if (a.empty()) return std::nullopt;

    // Signed bounds are unnecessary here because we keep the classic
    // "lo <= hi" form and only ever move lo up or hi down by using mid+1 /
    // mid-1 guarded by the comparison, so hi never underflows past lo.
    std::size_t lo = 0;
    std::size_t hi = a.size() - 1;

    while (lo <= hi) {
        std::size_t mid = lo + (hi - lo) / 2;  // overflow-safe midpoint
        if (a[mid] == target) return mid;

        if (a[lo] <= a[mid]) {
            // Left half [lo..mid] is sorted.
            if (a[lo] <= target && target < a[mid]) {
                if (mid == 0) break;  // guard: target would be left of index 0
                hi = mid - 1;
            } else {
                lo = mid + 1;
            }
        } else {
            // Right half [mid..hi] is sorted.
            if (a[mid] < target && target <= a[hi]) {
                lo = mid + 1;
            } else {
                if (mid == 0) break;  // range already at the bottom
                hi = mid - 1;
            }
        }
    }
    return std::nullopt;
}

// Linear reference used to validate the binary search in tests.
template <typename T>
std::optional<std::size_t> linear_find(const std::vector<T>& a, const T& target) {
    for (std::size_t i = 0; i < a.size(); ++i)
        if (a[i] == target) return i;
    return std::nullopt;
}

int main() {
    // Base sorted array with no duplicates.
    const std::vector<int> base = {0, 1, 2, 4, 5, 6, 7};

    // Try every rotation amount, including 0 (no rotation).
    for (std::size_t r = 0; r <= base.size(); ++r) {
        std::vector<int> arr;
        arr.reserve(base.size());
        for (std::size_t i = 0; i < base.size(); ++i)
            arr.push_back(base[(i + r) % base.size()]);

        // Every present value must be found at the same spot linear_find reports.
        for (int v : base) {
            auto got = search_rotated(arr, v);
            auto ref = linear_find(arr, v);
            assert(got.has_value() && got == ref);
        }
        // A value that is not present must report "not found".
        assert(!search_rotated(arr, 3).has_value());
        assert(!search_rotated(arr, 99).has_value());
    }

    // found at first / last / middle (explicit rotation [4,5,6,7,0,1,2]).
    std::vector<int> rot = {4, 5, 6, 7, 0, 1, 2};
    assert(search_rotated(rot, 4) == std::optional<std::size_t>(0));  // first
    assert(search_rotated(rot, 2) == std::optional<std::size_t>(6));  // last
    assert(search_rotated(rot, 7) == std::optional<std::size_t>(3));  // middle

    // empty range.
    std::vector<int> empty;
    assert(!search_rotated(empty, 1).has_value());

    // single element: found and not-found.
    std::vector<int> one = {42};
    assert(search_rotated(one, 42) == std::optional<std::size_t>(0));
    assert(!search_rotated(one, 7).has_value());

    // Short demo.
    std::cout << "search_rotated demo on [4,5,6,7,0,1,2]\n";
    for (int v : {4, 0, 7, 3}) {
        auto idx = search_rotated(rot, v);
        std::cout << "  target " << v << " -> ";
        if (idx) std::cout << "index " << *idx << "\n";
        else std::cout << "not found\n";
    }

    std::cout << "All assertions passed.\n";
    return 0;
}
