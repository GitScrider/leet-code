/*
 * Insertion Sort  (Algorithm - Sorting)
 *
 * Idea:
 *   Grow a sorted prefix one element at a time. Take the next element (the
 *   "key"), then shift every prefix element greater than it one slot to the
 *   right until the key's gap is found, and drop it in. Pass invariant: after
 *   step i the subarray v[0..i] is a sorted permutation of the original first
 *   i+1 elements.
 *
 * Complexity:
 *   +-----------+-----------+-----------+---------------+
 *   |   Best    |  Average  |   Worst   |  Aux. Space   |
 *   +-----------+-----------+-----------+---------------+
 *   |   O(n)    |  O(n^2)   |  O(n^2)   |     O(1)       |
 *   +-----------+-----------+-----------+---------------+
 *   Best case O(n) occurs on sorted input: each key needs a single comparison
 *   and no shift. Worst case (reverse sorted) shifts the whole prefix every
 *   step, giving 1+2+...+(n-1) = O(n^2) moves.
 *
 * Properties:
 *   Stable?    yes  (we shift only elements STRICTLY greater than the key, so
 *                    an equal element is never moved past its equal peer)
 *   In-place?  yes  (O(1) extra memory; one key held aside at a time)
 *   Adaptive?  yes  (work is proportional to the number of inversions, hence
 *                    O(n) on nearly-sorted data)
 *
 * When to use / notes:
 *   - Excellent for small or almost-sorted inputs; low constant factors.
 *   - Often used as the base case that quicksort/mergesort fall back to.
 *   - Online: can absorb new elements as they arrive.
 */

#include <vector>
#include <utility>     // std::move
#include <cstddef>     // std::size_t
#include <cassert>
#include <iostream>
#include <string>
#include <algorithm>   // std::sort (tests only)

// Sorts 'v' ascending using operator< only. Stable and in-place.
template<typename T>
void insertionSort(std::vector<T>& v) {
    const std::size_t n = v.size();
    for (std::size_t i = 1; i < n; ++i) {
        T key = std::move(v[i]);            // element to insert into the prefix
        std::size_t j = i;                  // gap that travels left as we shift
        // Shift prefix elements greater than key one slot right. The guard
        // 'j > 0' is checked BEFORE touching v[j-1]: with unsigned std::size_t
        // a bare 'j-1' at j==0 would wrap to a huge value and read out of range.
        while (j > 0 && key < v[j - 1]) {
            v[j] = std::move(v[j - 1]);
            --j;
        }
        v[j] = std::move(key);              // key drops into its sorted place
    }
}

// ------------------------------- tests -------------------------------------

namespace {

// Sort a copy with insertionSort and confirm it equals a std::sort'd reference.
template<typename T>
bool matchesStdSort(std::vector<T> v) {
    std::vector<T> expected = v;
    std::sort(expected.begin(), expected.end());
    insertionSort(v);
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
        insertionSort(v);
        assert(v.empty());
    }
    // 2) single element
    {
        std::vector<int> v{42};
        insertionSort(v);
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
        insertionSort(items);
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

    insertionSort(demo);

    std::cout << "after :";
    for (int x : demo) std::cout << ' ' << x;
    std::cout << '\n';

    std::cout << "all tests passed\n";
    return 0;
}
