/*
 * Bubble Sort  (Algorithm - Sorting)
 *
 * Idea:
 *   Repeatedly walk the array from left to right, swapping any adjacent pair
 *   that is out of order. After pass k the k largest elements have "bubbled"
 *   up to their final slots at the end. Pass invariant: once pass k finishes,
 *   the suffix of the last k elements is sorted and holds the k global maxima.
 *
 * Complexity:
 *   +-----------+-----------+-----------+---------------+
 *   |   Best    |  Average  |   Worst   |  Aux. Space   |
 *   +-----------+-----------+-----------+---------------+
 *   |   O(n)    |  O(n^2)   |  O(n^2)   |     O(1)       |
 *   +-----------+-----------+-----------+---------------+
 *   Best case O(n) comes from the early-exit flag: a single clean pass over
 *   already-sorted data performs zero swaps and stops. Average/worst are
 *   quadratic because up to ~n passes each scan up to ~n adjacent pairs.
 *
 * Properties:
 *   Stable?    yes  (only STRICTLY out-of-order adjacent pairs are swapped, so
 *                    equal elements never leapfrog one another)
 *   In-place?  yes  (O(1) extra memory; all work is in-array swaps)
 *   Adaptive?  yes  (the early-exit flag yields O(n) on sorted / nearly-sorted
 *                    input)
 *
 * When to use / notes:
 *   - Educational only; essentially never the right production choice.
 *   - Acceptable when n is tiny or the data is already almost sorted.
 *   - Shrinking the scanned range each pass avoids re-touching the sorted tail.
 */

#include <vector>
#include <utility>     // std::swap, std::move
#include <cstddef>     // std::size_t
#include <cassert>
#include <iostream>
#include <string>
#include <algorithm>   // std::sort (tests only)

// Sorts 'v' ascending using operator< only. Stable and in-place.
template<typename T>
void bubbleSort(std::vector<T>& v) {
    const std::size_t n = v.size();
    if (n < 2) return;                         // 0 or 1 element: already sorted
    // Each outer pass floats the largest remaining element to its final slot,
    // so the sorted tail grows by one and we scan one fewer element next time.
    for (std::size_t pass = 0; pass + 1 < n; ++pass) {
        bool swapped = false;                  // adaptive early-exit flag
        // Compare adjacent pairs in the still-unsorted prefix [0, n-1-pass).
        for (std::size_t i = 0; i + 1 < n - pass; ++i) {
            if (v[i + 1] < v[i]) {             // strict '<' keeps equals stable
                std::swap(v[i], v[i + 1]);
                swapped = true;
            }
        }
        if (!swapped) break;                   // a clean pass => fully sorted
    }
}

// ------------------------------- tests -------------------------------------

namespace {

// Sort a copy with bubbleSort and confirm it equals a std::sort'd reference.
template<typename T>
bool matchesStdSort(std::vector<T> v) {
    std::vector<T> expected = v;
    std::sort(expected.begin(), expected.end());
    bubbleSort(v);
    return v == expected;
}

// Adapter for the stability test: comparing by 'key' ONLY makes equal keys
// "equivalent", so a stable sort must preserve their original .seq order.
struct KeyItem {
    int key;   // the sort key (only field used by operator<)
    int seq;   // original input position, i.e. the payload we watch
};
bool operator<(const KeyItem& a, const KeyItem& b) { return a.key < b.key; }

void runTests() {
    // 1) empty
    {
        std::vector<int> v;
        bubbleSort(v);
        assert(v.empty());
    }
    // 2) single element
    {
        std::vector<int> v{42};
        bubbleSort(v);
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

    // Stability: equal keys must keep their original relative order.
    {
        std::vector<KeyItem> items{
            {2, 0}, {1, 1}, {2, 2}, {1, 3}, {3, 4}, {2, 5}, {1, 6}};
        bubbleSort(items);
        for (std::size_t i = 0; i + 1 < items.size(); ++i) {
            assert(!(items[i + 1].key < items[i].key));      // sorted by key
            if (items[i].key == items[i + 1].key)
                assert(items[i].seq < items[i + 1].seq);     // order preserved
        }
    }
}

}  // namespace

int main() {
    runTests();

    std::vector<int> demo{5, 1, 4, 2, 8, 1, -3};
    std::cout << "before:";
    for (int x : demo) std::cout << ' ' << x;
    std::cout << '\n';

    bubbleSort(demo);

    std::cout << "after :";
    for (int x : demo) std::cout << ' ' << x;
    std::cout << '\n';

    std::cout << "all tests passed\n";
    return 0;
}
