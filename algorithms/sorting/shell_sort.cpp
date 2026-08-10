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
 * Complexity derivation (gap-sequence dependent -- no single closed form):
 *   Let the gap sequence be h_t > ... > h_1 = 1. For a FIXED gap h the pass is a
 *   gapped insertion sort over the h interleaved subsequences; across all of
 *   them it touches ~n elements, so one h-pass costs
 *
 *       work(h) = O(n * s_h),   s_h = avg shifts per element at gap h
 *
 *   and the total is a SUM over the gaps actually used:
 *
 *       C(n) = SUM_{gap h in sequence} O(n * s_h)
 *
 *   The subtlety is that s_h depends on how "h-sorted" the earlier larger gaps
 *   already left the array, which is why the sum resists a clean closed form and
 *   is sequence-dependent:
 *     - Halving gaps (n/2, n/4, ..., 1): a known-bad family keeps s_h ~ n on
 *       adversarial input, so C(n) = SUM ~ Theta(n^2) in the worst case.
 *     - Knuth 3h+1 (1, 4, 13, 40, ...), used here: elements travel O(sqrt n) per
 *       pass over O(log n) passes, giving a PROVEN O(n^(3/2)) = O(n^1.5) worst.
 *     - Sedgewick gaps: O(n^(4/3)) = O(n^1.33) worst case.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f(n) = O(g)      iff  EXISTS c2, n0 :       f(n) <= c2*g(n)  for n >= n0
 *     f(n) = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f(n) = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   Because cost is governed by the gap sequence, the bounds are stated PER
 *   sequence, not as one universal Theta:
 *     upper  O:     O(n^2) worst for halving gaps; O(n^1.5) for Knuth 3h+1 here.
 *     lower  Omega: Omega(n log n) for EVERY gap sequence -- the information-
 *                   theoretic floor any comparison sort must pay.
 *     tight  Theta: gap-sequence-dependent; for several practical sequences the
 *                   exact worst-case Theta is STILL AN OPEN PROBLEM, so no single
 *                   honest Theta can be quoted for Shell sort in general.
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
