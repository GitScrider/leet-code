/*
 * Binary Search  (Algorithm - Searching)
 * --------------------------------------
 * Idea:
 *   Repeatedly halve a SORTED range. Compare the target with the middle
 *   element: if equal, done; if the target is smaller, the answer (if any)
 *   lies strictly in the left half; otherwise in the right half. Each step
 *   discards half of the remaining candidates.
 *   Precondition: the range MUST be sorted in non-decreasing order. On an
 *   unsorted range the result is undefined.
 *
 * Complexity:
 *   +-----------+----------+-----------+-----------+
 *   |           |   Best   |  Average  |   Worst   |
 *   +-----------+----------+-----------+-----------+
 *   | Time      |   O(1)   | O(log n)  | O(log n)  |
 *   | Space     |   O(1)   |   O(1)    |   O(1)    |  (iterative)
 *   | Space     |   O(1)   | O(log n)  | O(log n)  |  (recursive: call stack)
 *   +-----------+----------+-----------+-----------+
 *
 * Complexity derivation (recurrence -> recursion tree -> summation):
 *   Let T(n) be the number of comparisons to search a range of n elements. Each
 *   call does O(1) work (one midpoint compare) and then recurses on ONE half --
 *   unlike merge sort it makes a single recursive call, not two:
 *
 *       T(n) = T(n/2) + c ,      T(0) = c0        (base case: empty range)
 *
 *   The "recursion tree" is a single CHAIN (one node per level, not branching),
 *   because only one side of mid survives each step:
 *
 *       level d      #nodes      size each        work on the level
 *       ---------    --------    -------------    ------------------
 *       d = 0        1           n                c
 *       d = 1        1           n/2              c
 *       d = 2        1           n/4              c
 *       ...          ...         ...              ...
 *       d = k        1           n/2^k            c
 *
 *   The chain ends when the size drops from 1 to 0, i.e. at k = floor(log2 n),
 *   so there are (floor(log2 n) + 1) levels. Summing the per-level work:
 *
 *       T(n) = SUM_{d=0}^{floor(log2 n)} c = c * (floor(log2 n) + 1) = O(log n)
 *
 *   Master Theorem check (a=1, b=2, f(n)=c): n^(log_b a) = n^(log2 1) = n^0 = 1,
 *   the same order as f(n) -> case 2 -> T(n) = O(n^0 * log n) = O(log n). The
 *   recursive and iterative versions run the identical compare sequence; only
 *   the O(log n) call stack vs O(1) space differs.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f(n) = O(g)      iff  EXISTS c2, n0 :       f(n) <= c2*g(n)  for n >= n0
 *     f(n) = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f(n) = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   Binary search returns early when mid hits the target, so it is data-
 *   dependent:
 *     BEST case   T(n) = 1                => Theta(1)   (target is the 1st mid)
 *     WORST case  T(n) = floor(log2 n)+1  => Theta(log n)  (absent, or found
 *                 only at the last probe); with g = log n, (1/2)log2 n <= T(n)
 *                 <= 2 log2 n for n >= 2, so log n is tight.
 *   Over ALL inputs the time is O(log n) (upper, worst case) and Omega(1)
 *   (lower, best case). The information-theoretic floor for ANY comparison-based
 *   search of a sorted range is Omega(log n): a decision tree separating the
 *   n+1 possible outcomes needs height >= log2(n+1), so O(log n) is optimal.
 *
 * Key points / when to use:
 *   - Use on sorted, random-access data that is searched many times; the one-
 *     time sort cost amortizes away.
 *   - If duplicates exist and you need boundaries (first/last/count), use the
 *     lower_bound / upper_bound toolkit instead (see binary_search_bounds.cpp).
 *   - Two classic bugs, both avoided here: integer overflow in the midpoint
 *     and non-shrinking ranges that loop forever.
 *
 * "Not found" convention: both versions return a signed long long index, and
 * use -1 as the "not present" sentinel (indices are always >= 0).
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

// Iterative binary search over a sorted vector. Returns an index in [0, n),
// or -1 if `target` is absent. Uses a half-open search space via signed
// bounds [lo, hi] that always shrink.
template <typename T>
long long binarySearchIter(const std::vector<T>& a, const T& target) {
    long long lo = 0;
    long long hi = static_cast<long long>(a.size()) - 1;  // Signed: hi can go to -1 safely.

    // Loop invariant: if target is present, its index lies within [lo, hi].
    // Everything left of lo is known < target; everything right of hi is
    // known > target. The range strictly shrinks each iteration (mid is
    // always excluded from the surviving half), so the loop terminates.
    while (lo <= hi) {
        // Overflow-safe midpoint. Writing (lo + hi) / 2 can overflow when both
        // bounds are large; lo + (hi - lo) / 2 computes the same value without
        // ever forming a sum that may exceed the type's range.
        long long mid = lo + (hi - lo) / 2;

        const T& m = a[static_cast<std::size_t>(mid)];
        if (m == target) {
            return mid;              // Hit.
        } else if (m < target) {
            lo = mid + 1;            // Discard mid and everything to its left.
        } else {
            hi = mid - 1;            // Discard mid and everything to its right.
        }
    }
    return -1;  // lo > hi: the range is empty, target is absent.
}

// Recursive helper working on the inclusive slice [lo, hi] of a sorted vector.
template <typename T>
long long binarySearchRecImpl(const std::vector<T>& a, const T& target, long long lo, long long hi) {
    if (lo > hi) {
        return -1;  // Base case: empty range => not found.
    }
    long long mid = lo + (hi - lo) / 2;  // Same overflow-safe midpoint.
    const T& m = a[static_cast<std::size_t>(mid)];
    if (m == target) {
        return mid;
    } else if (m < target) {
        // Recurse strictly right of mid: the sub-range is strictly smaller,
        // guaranteeing progress toward the base case.
        return binarySearchRecImpl(a, target, mid + 1, hi);
    } else {
        return binarySearchRecImpl(a, target, lo, mid - 1);  // Strictly left.
    }
}

// Recursive binary search: same contract as the iterative version.
template <typename T>
long long binarySearchRec(const std::vector<T>& a, const T& target) {
    return binarySearchRecImpl(a, target, 0, static_cast<long long>(a.size()) - 1);
}

// Linear reference used only to validate the fast searches in tests.
template <typename T>
long long linearRef(const std::vector<T>& a, const T& target) {
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i] == target) return static_cast<long long>(i);
    }
    return -1;
}

int main() {
    // ---- assert-based tests -------------------------------------------------

    // Empty range: nothing found by either version.
    std::vector<int> empty;
    assert(binarySearchIter(empty, 1) == -1);
    assert(binarySearchRec(empty, 1) == -1);

    // Single element: found and not-found.
    std::vector<int> one{10};
    assert(binarySearchIter(one, 10) == 0);
    assert(binarySearchRec(one, 10) == 0);
    assert(binarySearchIter(one, 11) == -1);
    assert(binarySearchRec(one, 11) == -1);

    // A sorted range: probe first, middle, last, and absent values.
    std::vector<int> a{1, 3, 5, 7, 9, 11, 13};
    assert(binarySearchIter(a, 1) == 0);    // First.
    assert(binarySearchIter(a, 7) == 3);    // Middle.
    assert(binarySearchIter(a, 13) == 6);   // Last.
    assert(binarySearchIter(a, 8) == -1);   // Between existing values.
    assert(binarySearchIter(a, 0) == -1);   // Below range.
    assert(binarySearchIter(a, 99) == -1);  // Above range.

    // Iterative and recursive must agree, and both must match the linear
    // reference for a sweep of queries that includes present and absent keys.
    for (int q = -2; q <= 15; ++q) {
        long long it = binarySearchIter(a, q);
        long long re = binarySearchRec(a, q);
        assert(it == re);
        assert(it == linearRef(a, q));
    }

    // Larger sorted array (even size) to exercise midpoint rounding.
    std::vector<int> big;
    for (int i = 0; i < 1000; ++i) big.push_back(i * 2);  // 0,2,4,...,1998
    for (int i = 0; i < 2000; ++i) {
        long long it = binarySearchIter(big, i);
        long long re = binarySearchRec(big, i);
        assert(it == re);
        assert(it == linearRef(big, i));   // Odd i => -1; even i => i/2.
    }

    // Note on duplicates: with equal keys binary search returns SOME matching
    // index, not necessarily the first. Here every element equals 5, so any
    // index in [0, 4] is a valid answer; we assert the returned index is a hit.
    std::vector<int> dup{5, 5, 5, 5, 5};
    long long d = binarySearchIter(dup, 5);
    assert(d >= 0 && dup[static_cast<std::size_t>(d)] == 5);

    // ---- short demo ---------------------------------------------------------
    std::cout << "Binary Search demo\n";
    std::cout << "  data = [1, 3, 5, 7, 9, 11, 13]\n";
    std::cout << "  iterative index of 9  = " << binarySearchIter(a, 9) << "\n";
    std::cout << "  recursive index of 9  = " << binarySearchRec(a, 9) << "\n";
    std::cout << "  index of 8 (absent)   = " << binarySearchIter(a, 8) << "\n";
    std::cout << "All assertions passed.\n";
    return 0;
}
