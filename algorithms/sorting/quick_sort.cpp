/*
 * ============================================================================
 * Quick Sort
 * Category: Algorithm - Sorting (comparison-based, divide & conquer)
 *
 * Idea:
 *   Pick a PIVOT, then PARTITION the range so every element < pivot lands to
 *   its left and every element >= pivot to its right; the pivot is then in its
 *   final sorted position. Recurse on the two sides. This implementation uses
 *   LOMUTO partitioning with a MEDIAN-OF-THREE pivot (median of first/middle/
 *   last, moved to the end before partitioning). Invariant of the partition
 *   loop: everything in [lo, i) is < pivot and everything in [i, j) is >= pivot.
 *
 * Complexity:
 *   +----------+-------------+
 *   | Case     | Time        |
 *   +----------+-------------+
 *   | Best     | O(n log n)  |
 *   | Average  | O(n log n)  |
 *   | Worst    | O(n^2)      |
 *   +----------+-------------+
 *   Auxiliary Space: O(log n) expected recursion stack (see notes).
 *   WHY: a balanced split gives log n levels * O(n) partition work = n log n.
 *   A consistently extreme pivot (e.g. smallest element every time on already-
 *   sorted input with a naive last-element pivot) yields n levels => O(n^2).
 *   MEDIAN-OF-THREE makes the sorted/reverse-sorted inputs split well, turning
 *   the classic worst case into the good case; randomizing the pivot instead
 *   would make the O(n^2) case astronomically unlikely for adversarial data.
 *
 * Complexity derivation (recurrence -> recursion tree -> summation):
 *   Let T(n) be the number of basic operations to sort n elements. Partitioning
 *   a range of size m scans it once and costs c*m comparisons. What happens next
 *   depends ENTIRELY on where the pivot lands, so quicksort has two regimes.
 *
 *   BEST / AVERAGE case -- a BALANCED split (pivot near the median) cuts the
 *   range into two halves of size ~n/2:
 *
 *       T(n) = 2 * T(n/2) + c*n ,      T(1) = c0        (base case)
 *
 *   This is exactly merge sort's recurrence. Unfold it and count partition work:
 *
 *       level d      #nodes      size each        work on the level
 *       ---------    --------    -------------    --------------------------
 *       d = 0        1           n                c*n
 *       d = 1        2           n/2              2 * c*(n/2)   = c*n
 *       d = 2        4           n/4              4 * c*(n/4)   = c*n
 *       d = k        2^k         n/2^k            2^k * c*(n/2^k) = c*n
 *
 *   Every level costs c*n and there are (log2 n + 1) levels, so:
 *
 *       T(n) = SUM_{d=0}^{log2 n} (c*n) = c*n * (log2 n + 1) = Theta(n log n)
 *
 *   Master Theorem (a=2, b=2, f(n)=c*n): n^(log_b a) = n^1 = n matches f(n)
 *   -> case 2 -> T(n) = Theta(n^(log_b a) * log n) = Theta(n log n).
 *
 *   WORST case -- a maximally UNBALANCED split (e.g. the pivot is the smallest
 *   or largest element every time, as with a naive pivot on sorted input): one
 *   side gets n-1 elements and the other gets 0. The recursion degenerates into
 *   a chain:
 *
 *       T(n) = T(n-1) + c*n ,          T(0) = c0        (base case)
 *
 *       level d      #nodes      size            work on the level
 *       ---------    --------    ------------    -------------------
 *       d = 0        1           n               c*n
 *       d = 1        1           n-1             c*(n-1)
 *       d = k        1           n-k             c*(n-k)
 *
 *   There are n levels and NO branching, so summing the work:
 *
 *       T(n) = SUM_{k=1}^{n} c*k = c * (1 + 2 + ... + n)
 *            = c * n(n+1)/2                       (arithmetic series, Gauss)
 *            = Theta(n^2)
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 are positive constants):
 *     f(n) = O(g(n))      iff  EXISTS c2, n0 :        f(n) <= c2*g(n)   for n >= n0
 *     f(n) = Omega(g(n))  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)         for n >= n0
 *     f(n) = Theta(g(n))  iff  f = O(g) AND f = Omega(g)
 *   The cost is DATA-DEPENDENT (it hinges on pivot quality), so we bound per case:
 *     WORST case  T(n) = c*n(n+1)/2. With g=n^2, (c/2)n^2 <= T(n) <= c*n^2 for
 *                 n >= 1  =>  worst-case time is Theta(n^2)  (tight).
 *     BEST case   T(n) = c*n*(log2 n + 1)  =>  best-case time is Theta(n log n).
 *   Any comparison sort must make >= log2(n!) = Omega(n log n) comparisons, so no
 *   input can do better than n log n -- that is the lower bound and it coincides
 *   with the best case. Over ALL inputs the running time is therefore O(n^2)
 *   (upper, from the worst case) and Omega(n log n) (lower, from the best case);
 *   it is NOT a single Theta because best (n log n) != worst (n^2). The AVERAGE
 *   over random pivots is Theta(n log n) -- most splits are "good enough" -- and
 *   MEDIAN-OF-THREE makes the Theta(n^2) worst case practically unreachable.
 *
 * Properties:
 *   - Stable?    no   (partition swaps reorder equal keys).
 *   - In-place?  yes  (only O(log n) stack; no O(n) data buffer).
 *   - Adaptive?  no   (median-of-three helps presorted input, but it is not a
 *                      true adaptive/"runs-aware" algorithm).
 *
 * When to use / notes:
 *   - Usually the fastest general in-memory comparison sort in practice
 *     (excellent cache locality, tiny constant factors).
 *   - Use when stability is NOT required and O(n) extra memory is undesirable.
 *   - We recurse into the SMALLER partition first and loop on the larger one
 *     (tail-call elimination) to BOUND recursion depth at O(log n) even in the
 *     worst case -- this prevents stack overflow on adversarial inputs.
 * ============================================================================
 */

#include <vector>
#include <cassert>
#include <iostream>
#include <string>
#include <utility>
#include <algorithm>

// Choose the pivot as the median of a[lo], a[mid], a[hi] and move it to a[hi].
// Sorting these three in place also nudges the endpoints into decent spots.
// Returns nothing; the chosen pivot ends up at index hi ready for Lomuto.
template <typename T>
void medianOfThreeToEnd(std::vector<T>& a,
                        std::size_t lo, std::size_t hi) {
    std::size_t mid = lo + (hi - lo) / 2;
    // Sort a[lo] <= a[mid] <= a[hi] with three compare-swaps.
    if (a[mid] < a[lo]) std::swap(a[mid], a[lo]);
    if (a[hi]  < a[lo]) std::swap(a[hi],  a[lo]);
    if (a[hi]  < a[mid]) std::swap(a[hi], a[mid]);
    // The median now sits at a[mid]; park it at a[hi] as the pivot.
    std::swap(a[mid], a[hi]);
}

// Lomuto partition over the inclusive range [lo, hi]. Pivot is a[hi].
// Loop invariant: a[lo..i-1] < pivot, a[i..j-1] >= pivot. Returns the final
// pivot index p, with a[lo..p-1] < a[p] <= a[p+1..hi].
template <typename T>
std::size_t partition(std::vector<T>& a, std::size_t lo, std::size_t hi) {
    medianOfThreeToEnd(a, lo, hi);
    const T pivot = a[hi];
    std::size_t i = lo; // next slot for an element < pivot
    for (std::size_t j = lo; j < hi; ++j) {
        if (a[j] < pivot) {
            std::swap(a[i], a[j]);
            ++i;
        }
    }
    std::swap(a[i], a[hi]); // drop the pivot into its final resting index i
    return i;
}

// Sort the inclusive range [lo, hi]. `hi` is a signed-safe boundary passed as
// the index of the last element; callers guard against empty ranges.
template <typename T>
void quickSortRange(std::vector<T>& a, std::size_t lo, std::size_t hi) {
    // Iterative tail-recursion: after handling one side, loop on the other.
    while (lo < hi) {
        std::size_t p = partition(a, lo, hi);

        // Recurse into the SMALLER side first, then iterate on the larger side.
        // This caps stack depth at O(log n). Note p may be 0, so we compare
        // sizes carefully to stay clear of unsigned underflow.
        std::size_t leftSize  = p - lo;          // count in [lo, p-1]
        std::size_t rightSize = hi - p;          // count in [p+1, hi]

        if (leftSize < rightSize) {
            if (p > lo) quickSortRange(a, lo, p - 1); // safe: p > lo here
            lo = p + 1;                               // loop on right side
        } else {
            if (hi > p) quickSortRange(a, p + 1, hi); // safe: hi > p here
            // Loop on left side. Guard against p == 0 to avoid p-1 underflow.
            if (p == lo) return;                      // nothing left of pivot
            hi = p - 1;
        }
    }
}

// Primary entry point: sort the whole vector ascending using operator<.
template <typename T>
void quickSort(std::vector<T>& a) {
    if (a.size() <= 1) return;
    quickSortRange(a, 0, a.size() - 1);
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

// Generic check: quickSort must match std::sort on the same input.
template <typename T>
void checkAgainstStdSort(std::vector<T> input) {
    std::vector<T> expected = input;
    std::sort(expected.begin(), expected.end());
    quickSort(input);
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

    // A larger stress case, including the historically pathological
    // already-sorted input that median-of-three handles gracefully.
    std::vector<int> big;
    for (int i = 0; i < 1000; ++i) big.push_back(i);       // sorted ascending
    checkAgainstStdSort<int>(big);
    for (int i = 0; i < 1000; ++i) big[static_cast<std::size_t>(i)] = 999 - i;
    checkAgainstStdSort<int>(big);                          // sorted descending

    // Works on any comparable type (genericity check).
    checkAgainstStdSort<std::string>({"pear", "apple", "fig", "apple"});
    checkAgainstStdSort<double>({3.5, -1.25, 3.5, 0.0, 2.75});

    // NOTE: no stability test here -- quicksort is NOT stable by design.

    // --- Before/after demo ---
    std::vector<int> demo = {9, 3, 7, 1, 8, 2, 5};
    std::cout << "Quick Sort demo\n  before:";
    for (int x : demo) std::cout << ' ' << x;
    quickSort(demo);
    std::cout << "\n  after :";
    for (int x : demo) std::cout << ' ' << x;
    std::cout << "\nAll tests passed.\n";
    return 0;
}
