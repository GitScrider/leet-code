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
