/*
 * ============================================================================
 * Merge Sort
 * Category: Algorithm - Sorting (comparison-based, divide & conquer)
 *
 * Idea:
 *   Top-down divide and conquer. Split the range in half, recursively sort
 *   each half, then MERGE the two sorted halves into one sorted run using an
 *   auxiliary buffer. The invariant each merge maintains: after merging [lo,mid)
 *   and [mid,hi), the whole range [lo,hi) is sorted; recursion guarantees both
 *   inputs to every merge are already sorted.
 *
 * Complexity:
 *   +----------+-------------+
 *   | Case     | Time        |
 *   +----------+-------------+
 *   | Best     | O(n log n)  |
 *   | Average  | O(n log n)  |
 *   | Worst    | O(n log n)  |
 *   +----------+-------------+
 *   Auxiliary Space: O(n)  (buffer) + O(log n) recursion stack.
 *   WHY n log n always: the recursion tree has log n levels (halving), and each
 *   level touches all n elements exactly once during merging => n * log n. The
 *   bound is data-independent, so best == average == worst.
 *
 * Complexity derivation (recurrence -> recursion tree -> summation):
 *   Let T(n) be the number of basic operations to sort n elements.
 *     - Splitting the range in half is O(1).
 *     - Two recursive calls each sort n/2 elements  -> 2 * T(n/2).
 *     - Merging the two sorted halves scans every element once -> c*n ops.
 *   This gives the recurrence:
 *
 *       T(n) = 2 * T(n/2) + c*n ,      T(1) = c0        (base case)
 *
 *   Unfold it as a recursion tree and count the MERGE work per level:
 *
 *       level d      #nodes      size each        work on the level
 *       ---------    --------    -------------    --------------------------
 *       d = 0        1           n                c*n
 *       d = 1        2           n/2              2 * c*(n/2)   = c*n
 *       d = 2        4           n/4              4 * c*(n/4)   = c*n
 *       ...          ...         ...              ...
 *       d = k        2^k         n/2^k            2^k * c*(n/2^k) = c*n
 *
 *   Every level costs exactly c*n. The tree height is the number of halvings of
 *   n down to 1, i.e. k = log2(n), so there are (log2 n + 1) levels. Summing:
 *
 *       T(n) = SUM_{d=0}^{log2 n} (c * n)
 *            = c*n * (log2 n + 1)
 *            = O(n log n)
 *
 *   Master Theorem check (a=2, b=2, f(n)=c*n): n^(log_b a) = n^(log2 2) = n^1 = n,
 *   which is the same order as f(n) -> case 2 -> T(n) = O(n^(log_b a) * log n)
 *   = O(n log n). No data-dependent branch exists, so best = average = worst.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 are positive constants):
 *     f(n) = O(g(n))      iff  EXISTS c2, n0 :        f(n) <= c2*g(n)   for n >= n0
 *     f(n) = Omega(g(n))  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)         for n >= n0
 *     f(n) = Theta(g(n))  iff  f = O(g) AND f = Omega(g)  (squeezed both sides):
 *                              c1*g(n) <= f(n) <= c2*g(n) for n >= n0
 *   Here the exact count is f(n) = c*n*(log2 n + 1). Take g(n) = n log n:
 *     upper  O:     f(n) <= 2c * (n log2 n)  for n >= 2   => T(n) = O(n log n)
 *     lower  Omega: f(n) >=  c * (n log2 n)  for n >= 2   => T(n) = Omega(n log n)
 *     tight  Theta: both hold (c1 = c, c2 = 2c)           => T(n) = Theta(n log n)
 *   Because the work is input-independent, the SAME Theta(n log n) is the tight
 *   bound for best, average AND worst case.
 *
 * Properties:
 *   - Stable?    yes  (ties in merge() break toward the LEFT half).
 *   - In-place?  no   (needs an O(n) auxiliary buffer).
 *   - Adaptive?  no   (as written it does the same work on sorted input).
 *
 * When to use / notes:
 *   - Preferred when stability matters or worst-case guarantees are required.
 *   - Great for linked lists and external/streaming sorts (sequential access).
 *   - Not in-place: costs O(n) extra memory, unlike quicksort/heapsort.
 *   - The n log n worst case beats quicksort's O(n^2) pathological case.
 * ============================================================================
 */

#include <vector>
#include <cassert>
#include <iostream>
#include <string>
#include <utility>
#include <algorithm>

// Merge the two adjacent sorted runs [lo, mid) and [mid, hi) of `a` into a
// single sorted run, using `buffer` as scratch space. Ties are resolved in
// favor of the LEFT element (a[i] first when a[i] == a[j]) which is exactly
// what makes the overall sort STABLE.
template <typename T>
void merge(std::vector<T>& a, std::vector<T>& buffer,
           std::size_t lo, std::size_t mid, std::size_t hi) {
    std::size_t i = lo;   // cursor into the left half  [lo, mid)
    std::size_t j = mid;  // cursor into the right half [mid, hi)
    std::size_t k = lo;   // write cursor into the buffer

    // Standard two-pointer merge. "!(a[j] < a[i])" means a[i] <= a[j], so on
    // equal keys we take the LEFT element first -> stability.
    while (i < mid && j < hi) {
        if (!(a[j] < a[i])) {
            buffer[k++] = a[i++];
        } else {
            buffer[k++] = a[j++];
        }
    }
    // Drain whichever half still has elements (at most one of these runs).
    while (i < mid) buffer[k++] = a[i++];
    while (j < hi)  buffer[k++] = a[j++];

    // Copy the merged run back into the original range.
    for (std::size_t t = lo; t < hi; ++t) {
        a[t] = buffer[t];
    }
}

// Recursively sort the half-open range [lo, hi) of `a`.
template <typename T>
void mergeSortRange(std::vector<T>& a, std::vector<T>& buffer,
                    std::size_t lo, std::size_t hi) {
    if (hi - lo <= 1) return;             // 0 or 1 element is already sorted.
    std::size_t mid = lo + (hi - lo) / 2; // overflow-safe midpoint.
    mergeSortRange(a, buffer, lo, mid);   // sort left half
    mergeSortRange(a, buffer, mid, hi);   // sort right half
    merge(a, buffer, lo, mid, hi);        // combine
}

// Primary entry point: sort the whole vector ascending using operator<.
template <typename T>
void mergeSort(std::vector<T>& a) {
    if (a.size() <= 1) return;
    std::vector<T> buffer(a.size());      // single shared O(n) scratch buffer.
    mergeSortRange(a, buffer, 0, a.size());
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

// Generic check: mergeSort must match std::sort on the same input.
template <typename T>
void checkAgainstStdSort(std::vector<T> input) {
    std::vector<T> expected = input;
    std::sort(expected.begin(), expected.end());
    mergeSort(input);
    assert(input == expected);
}

int main() {
    // --- Required coverage on std::vector<int> ---
    checkAgainstStdSort<int>({});                          // empty
    checkAgainstStdSort<int>({42});                        // single element
    checkAgainstStdSort<int>({1, 2, 3, 4, 5});             // already sorted
    checkAgainstStdSort<int>({5, 4, 3, 2, 1});             // reverse sorted
    checkAgainstStdSort<int>({7, 7, 7, 7});                // all equal
    checkAgainstStdSort<int>({3, 1, 4, 1, 5, 9, 2, 6, 5}); // duplicates
    checkAgainstStdSort<int>({-3, 10, -7, 0, -7, 100, 2}); // negatives + large

    // Works on any comparable type (genericity check).
    checkAgainstStdSort<std::string>({"pear", "apple", "fig", "apple"});
    checkAgainstStdSort<double>({3.5, -1.25, 3.5, 0.0, 2.75});

    // --- Stability test ---
    // Sort pairs by .first only. Because we only compare .first, equal keys
    // must retain their original relative order of .second for a STABLE sort.
    struct ByFirst {
        bool operator<(const ByFirst& o) const { return first < o.first; }
        int first;
        int second;
    };
    std::vector<ByFirst> pairs = {
        {2, 0}, {1, 1}, {2, 2}, {1, 3}, {3, 4}, {2, 5}, {1, 6}
    };
    mergeSort(pairs);
    std::vector<std::pair<int, int>> got;
    for (const auto& p : pairs) got.emplace_back(p.first, p.second);
    // Keys ascending; within each key, .second stays in original order.
    std::vector<std::pair<int, int>> expectedStable = {
        {1, 1}, {1, 3}, {1, 6}, {2, 0}, {2, 2}, {2, 5}, {3, 4}
    };
    assert(got == expectedStable);

    // --- Before/after demo ---
    std::vector<int> demo = {9, 3, 7, 1, 8, 2, 5};
    std::cout << "Merge Sort demo\n  before:";
    for (int x : demo) std::cout << ' ' << x;
    mergeSort(demo);
    std::cout << "\n  after :";
    for (int x : demo) std::cout << ' ' << x;
    std::cout << "\nAll tests passed.\n";
    return 0;
}
