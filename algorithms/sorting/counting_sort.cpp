/*
 * Counting Sort  -- Algorithm - Sorting
 *
 * Idea:
 *   For integer keys drawn from a small, bounded range [0, k], we never compare
 *   elements at all. Instead we COUNT how many times each key occurs, turn those
 *   counts into PREFIX SUMS (so count[v] becomes "the number of elements <= v",
 *   i.e. the exclusive end position of key v's block in the output), then scan the
 *   input RIGHT-TO-LEFT placing each element at position --count[key]. Scanning in
 *   reverse and pre-decrementing is what makes the sort STABLE: the last equal key
 *   seen lands in the last slot of its block, preserving original relative order.
 *
 * Complexity:
 *   Case    | Time      | Aux Space
 *   --------+-----------+-----------
 *   Best    | O(n + k)  | O(n + k)
 *   Average | O(n + k)  | O(n + k)
 *   Worst   | O(n + k)  | O(n + k)
 *   (n = element count, k = size of the key range. If k dominates n, e.g. k ~ n^2,
 *    counting sort loses its edge -- the k term is why the range must stay bounded.)
 *
 * Complexity derivation (instruction count / summation, non-comparison):
 *   Three sequential linear passes; no element is ever compared against another.
 *   Let n = element count and k = size of the key range (keys in [0, k]).
 *     Pass 1 -- count occurrences: one increment per input element
 *         P1 = SUM_{i=0}^{n-1} 1 = n
 *     Pass 2 -- prefix sums over the (k+1)-slot count array: one add per slot
 *         P2 = SUM_{v=1}^{k} 1 = k
 *     Pass 3 -- place each element at its key index (right-to-left scan):
 *         P3 = SUM_{i=0}^{n-1} 1 = n
 *   Total operations:
 *
 *       C(n,k) = P1 + P2 + P3 = n + k + n = 2n + k = Theta(n + k)
 *
 *   No data-dependent branch exists: the three sums are identical for every input
 *   of size n over range k, so best = average = worst = Theta(n + k). Auxiliary
 *   space is the count array (k+1) plus the output buffer (n) = Theta(n + k).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants), with g = n + k:
 *     f = O(g)      iff  EXISTS c2, n0 :        f <= c2*(n+k)   for n+k >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*(n+k) <= f          for n+k >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   Here the exact count is f = 2n + k. Take g = n + k:
 *     lower  Omega:  1*(n+k) <= 2n + k           => Omega(n + k)
 *     upper  O:      2n + k <= 2*(n+k)            => O(n + k)
 *     tight  Theta:  both hold (c1 = 1, c2 = 2)   => Theta(n + k)
 *   Input-independent, so this Theta(n + k) is tight for best = average = worst.
 *   The comparison-sort lower bound Omega(n log n) does NOT apply here: that bound
 *   counts decisions in a comparison decision tree, but counting sort makes ZERO
 *   key comparisons -- it uses each key directly as an array INDEX. Escaping the
 *   decision-tree model is exactly how it beats n log n when k = O(n).
 *
 * Properties:
 *   Stable?    yes  (reverse fill with pre-decremented prefix positions)
 *   In-place?  no   (needs a count array of size k+1 and an output buffer of size n)
 *   Adaptive?  no   (cost depends on k, not on how sorted the input already is)
 *
 * When to use / notes:
 *   - Integer (or mappable-to-integer) keys in a known, small range.
 *   - The stable building block that makes LSD radix sort work.
 *   - Negatives / arbitrary ranges: find the minimum and OFFSET every key by -min
 *     so the smallest maps to index 0 (shown in shiftedCountingSort below).
 *   - Avoid when the range k is huge relative to n -- memory and time blow up.
 */

#include <vector>
#include <algorithm>   // std::sort, std::max_element, std::minmax_element, std::is_sorted
#include <utility>     // std::pair, std::move
#include <cstddef>     // std::size_t
#include <cassert>
#include <iostream>

// Stable counting sort for NON-NEGATIVE integers assumed to lie in [0, k].
// If k < 0 is passed we derive it from the data. Operates on std::vector<int>.
void countingSort(std::vector<int>& a, int k = -1) {
    if (a.empty()) return;  // nothing to do; also guards max_element below

    // Determine the upper bound k of the key range if the caller did not supply it.
    if (k < 0) {
        k = *std::max_element(a.begin(), a.end());
    }

    // count[v] will first hold the number of occurrences of key v.
    // Size is k+1 because valid keys are 0..k inclusive.
    std::vector<int> count(static_cast<std::size_t>(k) + 1, 0);
    for (const int v : a) {
        ++count[static_cast<std::size_t>(v)];
    }

    // Prefix sums: after this loop count[v] = number of elements with key <= v.
    // That value is exactly the exclusive upper index of key v's block in output.
    for (std::size_t v = 1; v < count.size(); ++v) {
        count[v] += count[v - 1];
    }

    // Stable placement: scan input RIGHT-TO-LEFT. Pre-decrementing count[key] turns
    // the exclusive end index into the correct slot and reserves it for the next
    // equal key to its left -- preserving original order among equal keys.
    std::vector<int> output(a.size());
    for (std::size_t i = a.size(); i-- > 0; ) {  // safe reverse loop for std::size_t
        const int v = a[i];
        output[static_cast<std::size_t>(--count[static_cast<std::size_t>(v)])] = v;
    }

    a = std::move(output);
}

// Domain-shift wrapper: handles ANY integer range, including negatives, by mapping
// every key to key - min so the smallest becomes 0, sorting, then mapping back.
// Time/space stay O(n + range) where range = max - min.
void shiftedCountingSort(std::vector<int>& a) {
    if (a.empty()) return;
    const auto mm = std::minmax_element(a.begin(), a.end());
    const int lo = *mm.first;
    const int hi = *mm.second;

    std::vector<int> shifted(a.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        shifted[i] = a[i] - lo;            // now in [0, hi - lo]
    }
    countingSort(shifted, hi - lo);
    for (std::size_t i = 0; i < a.size(); ++i) {
        a[i] = shifted[i] + lo;            // undo the offset
    }
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

static void print(const char* label, const std::vector<int>& v) {
    std::cout << label;
    for (const int x : v) std::cout << x << ' ';
    std::cout << '\n';
}

static void expectSorted(std::vector<int> in) {
    std::vector<int> reference = in;
    std::sort(reference.begin(), reference.end());
    shiftedCountingSort(in);          // exercises the general (negatives-safe) path
    assert(in == reference);
}

int main() {
    // Required edge cases -----------------------------------------------------
    expectSorted({});                              // empty
    expectSorted({42});                            // single element
    expectSorted({0, 1, 2, 3, 4, 5});              // already sorted (includes 0)
    expectSorted({9, 7, 5, 3, 1, 0});              // reverse sorted (includes 0)
    expectSorted({4, 4, 4, 4});                    // all equal
    expectSorted({5, 3, 3, 0, 5, 2, 0, 5});        // duplicates with 0
    expectSorted({-3, -1, -7, 0, 2, -1, 5, 5});    // negatives (domain shift)

    // Non-negative bounded path with an explicit k -----------------------------
    {
        std::vector<int> v = {3, 0, 2, 3, 1, 0, 2};
        std::vector<int> ref = v;
        std::sort(ref.begin(), ref.end());
        countingSort(v, 3);
        assert(v == ref);
    }

    // Stability check: keys are .first; .second is a tie-break tag we must NOT
    // reorder among equal keys. We drive countingSort via a key array but place
    // the pairs using the same reverse-fill discipline to prove stability holds.
    {
        std::vector<std::pair<int, int>> in = {
            {2, 0}, {1, 1}, {2, 2}, {1, 3}, {2, 4}, {0, 5}
        };
        const int k = 2;
        std::vector<int> count(static_cast<std::size_t>(k) + 1, 0);
        for (const auto& p : in) ++count[static_cast<std::size_t>(p.first)];
        for (std::size_t v = 1; v < count.size(); ++v) count[v] += count[v - 1];

        std::vector<std::pair<int, int>> out(in.size());
        for (std::size_t i = in.size(); i-- > 0; ) {
            const int key = in[i].first;
            out[static_cast<std::size_t>(--count[static_cast<std::size_t>(key)])] = in[i];
        }

        // Keys must be sorted, and within each key the .second tags must stay
        // in their original ascending order (0<2<4 for key 2; 1<3 for key 1).
        assert((out == std::vector<std::pair<int, int>>{
            {0, 5}, {1, 1}, {1, 3}, {2, 0}, {2, 2}, {2, 4}}));
    }

    // Before/after demo -------------------------------------------------------
    std::vector<int> demo = {5, 3, 3, 0, 5, 2, 0, 5};
    print("before: ", demo);
    shiftedCountingSort(demo);
    print("after:  ", demo);
    assert(std::is_sorted(demo.begin(), demo.end()));

    std::cout << "All counting sort tests passed.\n";
    return 0;
}
