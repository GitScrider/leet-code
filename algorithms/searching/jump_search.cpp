/*
 * Jump Search (Algorithm - Searching)
 *
 * Idea:
 *   Jump search works on a SORTED (ascending) array. Instead of stepping one
 *   element at a time (linear) or halving repeatedly (binary), it jumps ahead
 *   in fixed blocks of size `step`. We advance block by block while the value
 *   at the block boundary is still < target. Once a boundary value is >= target
 *   (we "overshoot"), the target, if present, must lie in the block we just
 *   skipped, so we linear-scan backward/forward through that single block.
 *
 *   Why block size sqrt(n) is optimal:
 *     With block size s we do at most n/s jumps plus at most s-1 comparisons in
 *     the final linear scan, i.e. cost ~ n/s + s. Minimizing n/s + s over s
 *     (derivative 1 - n/s^2 = 0) gives s = sqrt(n), yielding O(sqrt(n)) total.
 *
 *   Termination: the jump index strictly increases by `step` each jump and is
 *   clamped at n, and the final linear scan runs over a bounded block, so the
 *   procedure always finishes.
 *
 * Complexity:
 *   +-----------+-------------+-------------+-------------+
 *   | Case      | Best        | Average     | Worst       |
 *   +-----------+-------------+-------------+-------------+
 *   | Time      | O(1)        | O(sqrt n)   | O(sqrt n)   |
 *   | Space     | O(1)        | O(1)        | O(1)        |
 *   +-----------+-------------+-------------+-------------+
 *   (Best case: target sits in the very first block near the front.)
 *
 * Key points / when to use:
 *   - Requires the range to be sorted ascending.
 *   - Faster than linear search, slower than binary search, but only ever steps
 *     FORWARD in the jump phase -- handy when jumping back is cheap but random
 *     access far away is costly (e.g. certain sequential / tape-like media).
 *   - Optimal block size is floor(sqrt(n)).
 */

#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <optional>
#include <vector>

// Returns the index of `target` in a sorted ascending array, or std::nullopt
// if absent. Sentinel choice: std::optional<std::size_t>.
template <typename T>
std::optional<std::size_t> jump_search(const std::vector<T>& a, const T& target) {
    const std::size_t n = a.size();
    if (n == 0) return std::nullopt;

    // Optimal block size = floor(sqrt(n)), clamped to at least 1.
    std::size_t step = static_cast<std::size_t>(std::sqrt(static_cast<double>(n)));
    if (step == 0) step = 1;

    // Jump phase: `prev` is the start of the current block, `curr` its end
    // boundary (exclusive index we test against). Advance while the last element
    // of the current block is still strictly less than target.
    std::size_t prev = 0;
    std::size_t curr = step;
    // a[min(curr, n) - 1] is the last element of the block we are about to keep.
    while (curr < n && a[curr - 1] < target) {
        prev = curr;
        curr += step;
    }

    // Linear scan phase: the target, if present, is in [prev, min(curr, n)).
    const std::size_t end = (curr < n) ? curr : n;
    for (std::size_t i = prev; i < end; ++i) {
        if (a[i] == target) return i;
        if (a[i] > target) break;  // sorted: no point scanning further
    }
    return std::nullopt;
}

// Linear reference for validation.
template <typename T>
std::optional<std::size_t> linear_find(const std::vector<T>& a, const T& target) {
    for (std::size_t i = 0; i < a.size(); ++i)
        if (a[i] == target) return i;
    return std::nullopt;
}

int main() {
    // Sorted array with duplicates to exercise "first matching index" behaviour.
    // linear_find returns the FIRST occurrence; jump_search scans the block
    // left-to-right, so it also returns the first occurrence it meets.
    std::vector<int> a = {1, 1, 3, 5, 5, 5, 8, 13, 21, 34, 55, 89};

    // Every present value must match the linear reference's index.
    for (std::size_t i = 0; i < a.size(); ++i) {
        auto got = jump_search(a, a[i]);
        auto ref = linear_find(a, a[i]);
        assert(got.has_value() && got == ref);
    }

    // duplicates: first occurrence of 5 is index 3; of 1 is index 0.
    assert(jump_search(a, 5) == std::optional<std::size_t>(3));
    assert(jump_search(a, 1) == std::optional<std::size_t>(0));

    // found at first / last / middle.
    assert(jump_search(a, 1) == std::optional<std::size_t>(0));    // first
    assert(jump_search(a, 89) == std::optional<std::size_t>(11));  // last
    assert(jump_search(a, 8) == std::optional<std::size_t>(6));    // middle

    // not-found: below range, above range, and a gap value.
    assert(!jump_search(a, 0).has_value());
    assert(!jump_search(a, 100).has_value());
    assert(!jump_search(a, 4).has_value());

    // empty range.
    std::vector<int> empty;
    assert(!jump_search(empty, 5).has_value());

    // single element: found and not-found.
    std::vector<int> one = {7};
    assert(jump_search(one, 7) == std::optional<std::size_t>(0));
    assert(!jump_search(one, 3).has_value());

    // Cross-check every integer in a wide window against the linear reference.
    for (int v = -5; v <= 100; ++v)
        assert(jump_search(a, v) == linear_find(a, v));

    // Short demo.
    std::cout << "jump_search demo on a 12-element sorted array\n";
    for (int v : {1, 8, 89, 4}) {
        auto idx = jump_search(a, v);
        std::cout << "  target " << v << " -> ";
        if (idx) std::cout << "index " << *idx << "\n";
        else std::cout << "not found\n";
    }

    std::cout << "All assertions passed.\n";
    return 0;
}
