/*
 * Selection Sort  (Algorithm - Sorting)
 *
 * Idea:
 *   For each position i, scan the unsorted suffix v[i..n) to find its minimum
 *   and swap that minimum into position i. Pass invariant: after step i the
 *   prefix v[0..i] holds the i+1 smallest elements in final sorted order.
 *
 * Complexity:
 *   +-----------+-----------+-----------+---------------+
 *   |   Best    |  Average  |   Worst   |  Aux. Space   |
 *   +-----------+-----------+-----------+---------------+
 *   |  O(n^2)   |  O(n^2)   |  O(n^2)   |     O(1)       |
 *   +-----------+-----------+-----------+---------------+
 *   Quadratic in ALL cases: the min-scan always inspects the entire remaining
 *   suffix regardless of order, so there is no cheap best case. Its one virtue
 *   is exactly n-1 swaps (O(n) writes) -- handy when writes are expensive.
 *
 * Properties:
 *   Stable?    NO   (see note below)
 *   In-place?  yes  (O(1) extra memory)
 *   Adaptive?  no   (comparison count is independent of the input order)
 *
 *   Why not stable: the long-distance swap that pulls the suffix minimum into
 *   place can hop an element over an EQUAL one. Example on keys, subscripts
 *   marking original order:  [ 2a, 2b, 1 ]. Step 0 finds min '1' and swaps it
 *   with index 0, giving [ 1, 2b, 2a ] -- the two 2's are now reversed.
 *
 * When to use / notes:
 *   - When the number of writes must be minimized (only n-1 swaps).
 *   - Simple and predictable, but insertion sort beats it on nearly all data.
 *   - Not stable; if stability matters, prefer insertion or merge sort.
 */

#include <vector>
#include <utility>     // std::swap
#include <cstddef>     // std::size_t
#include <cassert>
#include <iostream>
#include <string>
#include <algorithm>   // std::sort (tests only)

// Sorts 'v' ascending using operator< only. In-place, NOT stable.
template<typename T>
void selectionSort(std::vector<T>& v) {
    const std::size_t n = v.size();
    if (n < 2) return;                         // 0 or 1 element: already sorted
    // Each pass extends the sorted prefix by placing the next-smallest element.
    for (std::size_t i = 0; i + 1 < n; ++i) {
        std::size_t minIdx = i;                // index of the suffix minimum
        for (std::size_t j = i + 1; j < n; ++j) {
            if (v[j] < v[minIdx]) minIdx = j;  // track the smallest so far
        }
        if (minIdx != i) std::swap(v[i], v[minIdx]);  // at most one swap/pass
    }
}

// ------------------------------- tests -------------------------------------

namespace {

// Sort a copy with selectionSort and confirm it equals a std::sort'd reference.
template<typename T>
bool matchesStdSort(std::vector<T> v) {
    std::vector<T> expected = v;
    std::sort(expected.begin(), expected.end());
    selectionSort(v);
    return v == expected;
}

void runTests() {
    // 1) empty
    {
        std::vector<int> v;
        selectionSort(v);
        assert(v.empty());
    }
    // 2) single element
    {
        std::vector<int> v{42};
        selectionSort(v);
        assert((v == std::vector<int>{42}));
    }
    // 3) already sorted    4) reverse    5) all equal
    assert(matchesStdSort(std::vector<int>{1, 2, 3, 4, 5}));
    assert(matchesStdSort(std::vector<int>{5, 4, 3, 2, 1}));
    assert(matchesStdSort(std::vector<int>{7, 7, 7, 7}));
    // 6) duplicates + negatives + large values
    assert(matchesStdSort(std::vector<int>{3, -1, 4, 1, 5, -9, 2, 6, 5, -3,
                                           1000000, -1000000}));
    // generic over another type
    assert(matchesStdSort(std::vector<std::string>{"pear", "apple", "banana",
                                                    "apple"}));

    // No stability test: selection sort is NOT stable by design (see header).
    // We still verify the multiset of values is correct for a duplicate-heavy
    // case, which matchesStdSort covers above.
}

}  // namespace

int main() {
    runTests();

    std::vector<int> demo{5, 1, 4, 2, 8, 1, -3};
    std::cout << "before:";
    for (int x : demo) std::cout << ' ' << x;
    std::cout << '\n';

    selectionSort(demo);

    std::cout << "after :";
    for (int x : demo) std::cout << ' ' << x;
    std::cout << '\n';

    std::cout << "all tests passed\n";
    return 0;
}
