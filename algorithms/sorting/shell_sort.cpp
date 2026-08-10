/*
 * Shell Sort  (Algorithm - Sorting)
 *
 * Idea:
 *   A generalization of insertion sort over diminishing GAPS. For a decreasing
 *   sequence of gaps h, run an insertion sort on the subsequences of elements
 *   that are h apart ("h-sorting"). Large early gaps move elements huge
 *   distances cheaply, so by the time gap == 1 (a plain insertion sort) the
 *   array is nearly sorted. Invariant: after h-sorting, every h-th element is
 *   in order, and h-sortedness is preserved by later smaller-gap passes.
 *
 * Complexity (depends on the gap sequence; Knuth 3h+1 used here):
 *   +-------------+-----------------+-----------------+---------------+
 *   |    Best     |     Average     |      Worst      |  Aux. Space   |
 *   +-------------+-----------------+-----------------+---------------+
 *   | O(n log n)  | ~O(n^1.25)-1.5  |   O(n^1.5)      |     O(1)       |
 *   +-------------+-----------------+-----------------+---------------+
 *   Exact bounds are gap-dependent and still an open problem in general; the
 *   Knuth sequence 1,4,13,40,... gives a proven O(n^(3/2)) worst case. Each
 *   pass is a gapped insertion sort, and shrinking gaps keep the total number
 *   of shifts far below the O(n^2) of plain insertion sort.
 *
 * Properties:
 *   Stable?    NO   (gapped moves jump elements over equal ones far apart, so
 *                    equal keys can be reordered)
 *   In-place?  yes  (O(1) extra memory; only the gap and one key are held)
 *   Adaptive?  partly (already-ordered subsequences skip shifts, so nearly
 *                    sorted input is faster, but not as sharply as insertion)
 *
 * When to use / notes:
 *   - A solid, simple in-place sort for medium arrays with no recursion.
 *   - Much faster than insertion/bubble in practice; no extra memory.
 *   - Not stable; choice of gap sequence materially affects performance.
 */

#include <vector>
#include <utility>     // std::move
#include <cstddef>     // std::size_t
#include <cassert>
#include <iostream>
#include <string>
#include <algorithm>   // std::sort (tests only)

// Sorts 'v' ascending using operator< only. In-place, NOT stable.
template<typename T>
void shellSort(std::vector<T>& v) {
    const std::size_t n = v.size();
    if (n < 2) return;                       // 0 or 1 element: already sorted
    // Build the largest Knuth gap (1, 4, 13, 40, ...) strictly below n/3.
    std::size_t gap = 1;
    while (gap < n / 3) gap = gap * 3 + 1;
    // Iterate gaps downward. The update (gap-1)/3 reverses the 3h+1 rule and
    // yields 0 after gap == 1, ending the loop (gap-1 is safe: gap >= 1 here,
    // so no unsigned underflow). 'gap > 0' means gap != 0 for std::size_t.
    for (; gap > 0; gap = (gap - 1) / 3) {
        // Gapped insertion sort: treat each element as joining the sorted
        // h-subsequence ending at index i.
        for (std::size_t i = gap; i < n; ++i) {
            T key = std::move(v[i]);
            std::size_t j = i;               // gap walks left in steps of 'gap'
            // Guard 'j >= gap' BEFORE reading v[j-gap] so the unsigned index
            // j-gap never wraps below zero.
            while (j >= gap && key < v[j - gap]) {
                v[j] = std::move(v[j - gap]);
                j -= gap;
            }
            v[j] = std::move(key);
        }
    }
}

// ------------------------------- tests -------------------------------------

namespace {

// Sort a copy with shellSort and confirm it equals a std::sort'd reference.
template<typename T>
bool matchesStdSort(std::vector<T> v) {
    std::vector<T> expected = v;
    std::sort(expected.begin(), expected.end());
    shellSort(v);
    return v == expected;
}

void runTests() {
    // 1) empty
    {
        std::vector<int> v;
        shellSort(v);
        assert(v.empty());
    }
    // 2) single element
    {
        std::vector<int> v{42};
        shellSort(v);
        assert((v == std::vector<int>{42}));
    }
    // 3) already sorted    4) reverse    5) all equal
    assert(matchesStdSort(std::vector<int>{1, 2, 3, 4, 5}));
    assert(matchesStdSort(std::vector<int>{5, 4, 3, 2, 1}));
    assert(matchesStdSort(std::vector<int>{7, 7, 7, 7}));
    // 6) duplicates + negatives + large values
    assert(matchesStdSort(std::vector<int>{3, -1, 4, 1, 5, -9, 2, 6, 5, -3,
                                           1000000, -1000000}));
    // a larger case to exercise several gaps
    assert(matchesStdSort(std::vector<int>{9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
                                           15, 13, 11, 12, 14, 10}));
    // generic over another type
    assert(matchesStdSort(std::vector<std::string>{"pear", "apple", "banana",
                                                    "apple"}));

    // No stability test: shell sort is NOT stable by design (see header).
}

}  // namespace

int main() {
    runTests();

    std::vector<int> demo{5, 1, 4, 2, 8, 1, -3, 9, 0};
    std::cout << "before:";
    for (int x : demo) std::cout << ' ' << x;
    std::cout << '\n';

    shellSort(demo);

    std::cout << "after :";
    for (int x : demo) std::cout << ' ' << x;
    std::cout << '\n';

    std::cout << "all tests passed\n";
    return 0;
}
