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
