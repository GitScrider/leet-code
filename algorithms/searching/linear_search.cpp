/*
 * Linear Search  (Algorithm - Searching)
 * ---------------------------------------
 * Idea:
 *   Scan the range from left to right, comparing each element with the target.
 *   Return the index of the FIRST element that equals the target; if the scan
 *   finishes without a match, report "not found".
 *   Precondition: NONE. Linear search works on ANY range -- the data does not
 *   need to be sorted, which is its main advantage over binary search.
 *
 * Complexity:
 *   +-----------+----------+----------+----------+
 *   |           |   Best   |  Average |  Worst   |
 *   +-----------+----------+----------+----------+
 *   | Time      |   O(1)   |   O(n)   |   O(n)   |
 *   | Space     |   O(1)   |   O(1)   |   O(1)   |
 *   +-----------+----------+----------+----------+
 *   Best case: target sits at index 0. Worst case: target absent or last.
 *
 * Key points / when to use:
 *   - Use when the data is unsorted, tiny, or searched only once (sorting first
 *     would not pay off).
 *   - The only search that also works on singly-linked / forward-only ranges.
 *   - Sentinel optimization (see note below) removes the per-iteration bounds
 *     check, roughly halving the comparisons in the hot loop.
 *   - Easy to adapt to "find ALL matches" by collecting every hit instead of
 *     returning on the first one.
 *
 * "Not found" convention: this file returns std::optional<std::size_t>.
 * std::nullopt means "not present"; a value means "found at this index".
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

// Generic linear search: returns the index of the first element equal to
// `target`, or std::nullopt if none matches. Works on any std::vector<T>
// whose element type supports operator==.
template <typename T>
std::optional<std::size_t> linearSearch(const std::vector<T>& data, const T& target) {
    for (std::size_t i = 0; i < data.size(); ++i) {
        // Loop invariant: every element in data[0 .. i-1] has already been
        // checked and did NOT equal target. Each step shrinks the unchecked
        // suffix by one, so the loop always terminates after at most n steps.
        if (data[i] == target) {
            return i;  // First match wins.
        }
    }
    return std::nullopt;  // Fell off the end: target is absent.
}

// Variant: find ALL matching indices (useful when duplicates matter).
template <typename T>
std::vector<std::size_t> linearSearchAll(const std::vector<T>& data, const T& target) {
    std::vector<std::size_t> hits;
    for (std::size_t i = 0; i < data.size(); ++i) {
        if (data[i] == target) {
            hits.push_back(i);
        }
    }
    return hits;
}

/*
 * Sentinel-optimization note
 * --------------------------
 * A plain scan tests TWO conditions per element: "i < size" (bounds) and
 * "data[i] == target" (match). By temporarily appending the target as a
 * "sentinel" at the end, the match is GUARANTEED to occur, so the bounds test
 * can be dropped from the inner loop -- the sentinel stops the scan. After the
 * loop, if the stop index is the sentinel position, the real range had no
 * match. This halves the comparison count in the hot path. Below we copy into
 * a scratch buffer to keep the caller's data const; in practice one reserves a
 * spare slot to avoid the copy.
 */
template <typename T>
std::optional<std::size_t> linearSearchSentinel(const std::vector<T>& data, const T& target) {
    const std::size_t n = data.size();
    if (n == 0) {
        return std::nullopt;  // Empty range: nothing to find.
    }
    std::vector<T> buf(data);   // Scratch copy so we may append a sentinel.
    buf.push_back(target);      // Sentinel guarantees the scan will stop.
    std::size_t i = 0;
    while (!(buf[i] == target)) {  // No bounds check: the sentinel ends it.
        ++i;
    }
    if (i == n) {
        return std::nullopt;  // Stopped on the sentinel => target not in [0, n).
    }
    return i;  // Real match inside the original range.
}

int main() {
    // ---- assert-based tests -------------------------------------------------

    // Empty range: nothing can be found.
    std::vector<int> empty;
    assert(!linearSearch(empty, 42).has_value());
    assert(!linearSearchSentinel(empty, 42).has_value());
    assert(linearSearchAll(empty, 42).empty());

    // Single element: found and not-found.
    std::vector<int> one{7};
    assert(linearSearch(one, 7).value() == 0);
    assert(!linearSearch(one, 8).has_value());

    std::vector<int> v{4, 8, 15, 16, 23, 42};
    // Found at first, middle, last.
    assert(linearSearch(v, 4).value() == 0);
    assert(linearSearch(v, 16).value() == 3);
    assert(linearSearch(v, 42).value() == 5);
    // Not found.
    assert(!linearSearch(v, 99).has_value());

    // Sentinel version must agree with the plain version on every query.
    for (int q : {4, 8, 15, 16, 23, 42, 99, -1}) {
        assert(linearSearch(v, q) == linearSearchSentinel(v, q));
    }

    // Duplicates: first match index, and full match list.
    std::vector<int> dups{5, 1, 5, 2, 5, 3};
    assert(linearSearch(dups, 5).value() == 0);  // First occurrence.
    std::vector<std::size_t> all = linearSearchAll(dups, 5);
    assert((all == std::vector<std::size_t>{0, 2, 4}));
    assert(linearSearchAll(dups, 9).empty());

    // Works on a non-arithmetic type too (std::string).
    std::vector<std::string> words{"alpha", "beta", "gamma"};
    assert(linearSearch(words, std::string("gamma")).value() == 2);
    assert(!linearSearch(words, std::string("delta")).has_value());

    // ---- short demo ---------------------------------------------------------
    std::cout << "Linear Search demo\n";
    std::cout << "  data = [4, 8, 15, 16, 23, 42]\n";
    auto r = linearSearch(v, 16);
    std::cout << "  index of 16 = " << (r ? static_cast<long>(*r) : -1) << "\n";
    auto positions = linearSearchAll(dups, 5);
    std::cout << "  all indices of 5 in [5,1,5,2,5,3] =";
    for (std::size_t p : positions) std::cout << " " << p;
    std::cout << "\n";
    std::cout << "All assertions passed.\n";
    return 0;
}
