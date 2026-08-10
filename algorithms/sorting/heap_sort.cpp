/*
 * Heap Sort (Algorithm - Sorting)
 *
 * Idea:
 *   Interpret the array as a complete binary tree stored implicitly. First
 *   build a MAX-heap in place (largest element sits at index 0). Then, on each
 *   pass, swap the root (the current maximum) with the last element of the
 *   unsorted region and shrink that region by one; siftDown restores the heap
 *   property on the smaller heap. Invariant: after k passes the k largest
 *   elements occupy the k rightmost slots in sorted order, and elements [0, n-k)
 *   still form a valid max-heap.
 *
 * Complexity:
 *   +----------+-------------+
 *   |  Case    |    Time     |
 *   +----------+-------------+
 *   |  Best    |  O(n log n) |
 *   |  Average |  O(n log n) |
 *   |  Worst   |  O(n log n) |
 *   +----------+-------------+
 *   Auxiliary Space: O(1)  (sorts in place; recursion avoided via a loop)
 *
 *   Why O(n log n): building the heap costs O(n) (tighter than the naive
 *   n log n bound, because most nodes are shallow). The extraction phase does
 *   n-1 siftDown calls, each O(log n) because a node can fall at most the tree
 *   height. n * log n dominates, and this holds for every input -- heap sort is
 *   not sensitive to initial order, so best = average = worst.
 *
 * Properties:
 *   Stable?    no  (long-range swaps of the root with the tail reorder equal keys)
 *   In-place?  yes (only O(1) extra memory; the heap lives in the same array)
 *   Adaptive?  no  (already-sorted input is not any faster)
 *
 * When to use / notes:
 *   - Guaranteed O(n log n) worst case with O(1) space -- good when worst-case
 *     bounds matter and extra memory (as merge sort needs) is unacceptable.
 *   - Not stable and has poor cache locality, so std::sort (introsort) usually
 *     beats it in practice despite the same asymptotics.
 *   - The heap structure is the basis of std::priority_queue.
 */

#include <vector>
#include <cassert>
#include <iostream>
#include <utility>
#include <algorithm>
#include <cstddef>
#include <string>

// Restore the max-heap property for the subtree rooted at 'root', assuming both
// child subtrees are already valid heaps. 'heapSize' is the number of elements
// currently considered part of the heap (region [0, heapSize)).
//
// Child index formulas for a node at index i in a 0-based implicit heap:
//   left  child = 2*i + 1
//   right child = 2*i + 2
// We repeatedly push the offending value down toward the leaves until it is
// >= both children (or becomes a leaf). Iterative form keeps space at O(1).
template <typename T>
void siftDown(std::vector<T>& v, std::size_t root, std::size_t heapSize) {
    while (true) {
        std::size_t largest = root;          // assume the root is the largest
        const std::size_t left  = 2 * root + 1;
        const std::size_t right = 2 * root + 2;

        // If the left child exists and is larger, it becomes the new candidate.
        if (left < heapSize && v[largest] < v[left]) {
            largest = left;
        }
        // If the right child exists and is larger still, prefer it.
        if (right < heapSize && v[largest] < v[right]) {
            largest = right;
        }
        // Root already dominates its children: heap property holds, stop.
        if (largest == root) {
            break;
        }
        std::swap(v[root], v[largest]);
        root = largest;                       // follow the value down one level
    }
}

// Primary entry point: sorts 'v' ascending using operator< only.
template <typename T>
void heapSort(std::vector<T>& v) {
    const std::size_t n = v.size();
    if (n < 2) {
        return;                               // 0 or 1 element is already sorted
    }

    // Phase 1 -- build a max-heap in O(n).
    // Leaves (indices >= n/2) are trivially valid heaps, so we only siftDown the
    // internal nodes, from the last internal node up to the root. We use the
    // 'while (i-- > 0)' idiom to walk i = n/2-1 ... 0 safely: with std::size_t a
    // plain 'for (i = n/2-1; i >= 0; --i)' would underflow and loop forever.
    std::size_t i = n / 2;
    while (i-- > 0) {
        siftDown(v, i, n);
    }

    // Phase 2 -- repeatedly extract the maximum.
    // Swap the root (max) into the last slot of the current heap, then shrink
    // the heap and siftDown the new root. After each step the tail grows into a
    // sorted suffix; the loop ends when only one element remains in the heap.
    std::size_t end = n;
    while (end-- > 1) {
        std::swap(v[0], v[end]);              // move current max to its final place
        siftDown(v, 0, end);                  // restore heap on the shrunk region
    }
}

// -------------------------- Tests & demo --------------------------

// Check heapSort against std::sort on a copy of the same input.
template <typename T>
void checkAgainstStdSort(std::vector<T> input) {
    std::vector<T> expected = input;
    std::sort(expected.begin(), expected.end());
    heapSort(input);
    assert(input == expected);
}

int main() {
    // Required edge cases.
    checkAgainstStdSort<int>({});                          // empty
    checkAgainstStdSort<int>({42});                        // single element
    checkAgainstStdSort<int>({1, 2, 3, 4, 5});             // already sorted
    checkAgainstStdSort<int>({5, 4, 3, 2, 1});             // reverse sorted
    checkAgainstStdSort<int>({7, 7, 7, 7});                // all equal
    checkAgainstStdSort<int>({3, 1, 4, 1, 5, 9, 2, 6, 5}); // duplicates
    checkAgainstStdSort<int>({-3, 10, -100, 0, 55, -7, 8}); // negatives / large

    // Works for other comparable types too (generic on operator<).
    checkAgainstStdSort<std::string>({"pear", "apple", "fig", "apple"});

    // Heap sort is NOT stable. We do not assert stability; instead we document
    // and demonstrate it: sorting pairs by the full pair still yields a globally
    // correct order, but equal first-keys may have their .second reordered.
    // (No stability test is required for an unstable algorithm.)
    {
        std::vector<std::pair<int, int>> p = {{1, 0}, {1, 1}, {0, 0}, {1, 2}};
        std::vector<std::pair<int, int>> expected = p;
        std::sort(expected.begin(), expected.end());
        heapSort(p);
        assert(p == expected);  // fully-ordered on the pair; not a stability claim
    }

    // Before/after demo.
    std::vector<int> demo = {9, 3, 7, 1, 8, 2, 5};
    std::cout << "before:";
    for (const int x : demo) std::cout << ' ' << x;
    std::cout << '\n';

    heapSort(demo);

    std::cout << "after :";
    for (const int x : demo) std::cout << ' ' << x;
    std::cout << '\n';

    std::cout << "All heap sort tests passed.\n";
    return 0;
}
