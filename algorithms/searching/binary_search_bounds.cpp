/*
 * Binary Search Bounds  (Algorithm - Searching)
 * ---------------------------------------------
 * Idea:
 *   On a SORTED range that may contain duplicates, plain binary search is not
 *   enough -- you often want the RANGE of equal keys, not just any one. This
 *   toolkit builds everything from two primitives:
 *     lowerBound(x) = first index i with a[i] >= x   (insertion point of x)
 *     upperBound(x) = first index i with a[i] >  x   (one past the last x)
 *   From those:
 *     firstOccurrence(x) = lowerBound(x) if a[that] == x, else "not found"
 *     lastOccurrence(x)  = upperBound(x) - 1 if that slot holds x, else "none"
 *     countOf(x)         = upperBound(x) - lowerBound(x)
 *   Precondition: the range MUST be sorted in non-decreasing order.
 *
 * Complexity:
 *   +-----------+----------+-----------+-----------+
 *   |           |   Best   |  Average  |   Worst   |
 *   +-----------+----------+-----------+-----------+
 *   | Time      | O(log n) | O(log n)  | O(log n)  |  (each primitive)
 *   | Space     |   O(1)   |   O(1)    |   O(1)    |
 *   +-----------+----------+-----------+-----------+
 *
 * Complexity derivation (loop iteration count / recurrence):
 *   Each primitive keeps a half-open interval [lo, hi) of size s = hi - lo and
 *   does O(1) work per iteration (one midpoint compare, then move lo or hi).
 *   With mid = lo + floor(s/2):
 *     branch lo = mid + 1 : new size = s - floor(s/2) - 1 = ceil(s/2) - 1
 *     branch hi = mid     : new size = floor(s/2)
 *   Either way the size at least HALVES each step, giving the recurrence
 *
 *       T(n) = T(n/2) + c ,      T(0) = c0        (base case: empty interval)
 *
 *   Counting iterations: the size starts at n and halves until it reaches 0,
 *
 *       #iters = SUM_{d=0}^{ceil(log2(n+1)) - 1} 1 = ceil(log2(n+1)) = O(log n)
 *
 *   Unlike plain binary search there is NO equality early-exit: the loop always
 *   runs the full depth to pin down the boundary, so every case costs the same.
 *   countOf calls both primitives: 2 * O(log n) = O(log n). firstOccurrence and
 *   lastOccurrence add one O(1) index check, so they are O(log n) as well.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f(n) = O(g)      iff  EXISTS c2, n0 :       f(n) <= c2*g(n)  for n >= n0
 *     f(n) = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f(n) = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   Because there is NO data-dependent early exit, the iteration count is
 *   ceil(log2(n+1)) for EVERY input. With g = log n, (1/2)log2 n <= f(n) <=
 *   2 log2 n for n >= 2, so best = average = worst = Theta(log n) -- a single
 *   tight bound, no per-case split needed (contrast plain binary search, which
 *   has a Theta(1) best case). The information-theoretic floor for locating a
 *   boundary among the n+1 gaps by comparisons is Omega(log n), so the toolkit
 *   is asymptotically optimal.
 *
 * Key points / when to use:
 *   - The single most useful binary-search toolkit: "how many equal keys",
 *     "where would x be inserted", "range of a duplicate value".
 *   - lowerBound / upperBound mirror std::lower_bound / std::upper_bound; we
 *     reimplement them to expose the invariant and index arithmetic.
 *   - Both primitives return a value in [0, n] (n means "past the end"), so
 *     the caller must range-check before indexing.
 *
 * "Not found" convention: lowerBound/upperBound/countOf always return a valid
 * std::size_t (an insertion point / count). firstOccurrence and lastOccurrence
 * return std::optional<std::size_t>; std::nullopt means "x is absent".
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <optional>
#include <vector>

// First index i in [0, n] with a[i] >= x. Returns n if x is greater than every
// element. This is exactly the position where x could be inserted to keep the
// range sorted while landing before any existing copies of x.
template <typename T>
std::size_t lowerBound(const std::vector<T>& a, const T& x) {
    std::size_t lo = 0;
    std::size_t hi = a.size();  // Half-open search space [lo, hi]; hi==size is valid.

    // Loop invariant: the answer lies in [lo, hi]; every index < lo has
    // a[i] < x, and every index >= hi has a[i] >= x. Because mid < hi always,
    // each branch strictly shrinks (hi - lo), so the loop terminates.
    while (lo < hi) {
        std::size_t mid = lo + (hi - lo) / 2;  // Overflow-safe; mid in [lo, hi).
        if (a[mid] < x) {
            lo = mid + 1;   // a[mid] too small: answer is strictly right of mid.
        } else {
            hi = mid;       // a[mid] >= x: mid is a candidate, keep it in range.
        }
    }
    return lo;  // lo == hi: the unique boundary.
}

// First index i in [0, n] with a[i] > x. Returns n if no element exceeds x.
// Identical to lowerBound but with a "<=" test, so equal keys fall to the left.
template <typename T>
std::size_t upperBound(const std::vector<T>& a, const T& x) {
    std::size_t lo = 0;
    std::size_t hi = a.size();
    while (lo < hi) {
        std::size_t mid = lo + (hi - lo) / 2;
        if (a[mid] <= x) {
            lo = mid + 1;   // a[mid] not yet past x: move right of mid.
        } else {
            hi = mid;       // a[mid] > x: candidate boundary.
        }
    }
    return lo;
}

// Index of the FIRST element equal to x, or nullopt if x is absent.
template <typename T>
std::optional<std::size_t> firstOccurrence(const std::vector<T>& a, const T& x) {
    std::size_t i = lowerBound(a, x);
    if (i < a.size() && a[i] == x) {
        return i;
    }
    return std::nullopt;
}

// Index of the LAST element equal to x, or nullopt if x is absent.
template <typename T>
std::optional<std::size_t> lastOccurrence(const std::vector<T>& a, const T& x) {
    std::size_t u = upperBound(a, x);
    // u is one past the last x. If u == 0 there is nothing to the left, so x
    // cannot be present; otherwise the candidate is u - 1.
    if (u > 0 && a[u - 1] == x) {
        return u - 1;
    }
    return std::nullopt;
}

// Number of elements equal to x. Zero when absent. The subtraction never
// underflows because upperBound(x) >= lowerBound(x) always.
template <typename T>
std::size_t countOf(const std::vector<T>& a, const T& x) {
    return upperBound(a, x) - lowerBound(a, x);
}

// Brute-force references used only to validate the log-n toolkit in tests.
template <typename T>
std::size_t lowerBoundRef(const std::vector<T>& a, const T& x) {
    std::size_t i = 0;
    while (i < a.size() && a[i] < x) ++i;
    return i;
}
template <typename T>
std::size_t upperBoundRef(const std::vector<T>& a, const T& x) {
    std::size_t i = 0;
    while (i < a.size() && a[i] <= x) ++i;
    return i;
}
template <typename T>
std::size_t countRef(const std::vector<T>& a, const T& x) {
    std::size_t c = 0;
    for (const T& v : a) if (v == x) ++c;
    return c;
}

int main() {
    // ---- assert-based tests -------------------------------------------------

    // Empty range: bounds collapse to 0, no occurrences, zero count.
    std::vector<int> empty;
    assert(lowerBound(empty, 5) == 0);
    assert(upperBound(empty, 5) == 0);
    assert(!firstOccurrence(empty, 5).has_value());
    assert(!lastOccurrence(empty, 5).has_value());
    assert(countOf(empty, 5) == 0);

    // Single element.
    std::vector<int> one{4};
    assert(lowerBound(one, 4) == 0 && upperBound(one, 4) == 1);
    assert(firstOccurrence(one, 4).value() == 0);
    assert(lastOccurrence(one, 4).value() == 0);
    assert(countOf(one, 4) == 1);
    assert(countOf(one, 3) == 0 && countOf(one, 5) == 0);

    // Sorted array with MANY duplicates.
    //  idx: 0  1  2  3  4  5  6  7  8  9
    //  val: 1  2  2  2  4  4  6  6  6  6
    std::vector<int> a{1, 2, 2, 2, 4, 4, 6, 6, 6, 6};

    // lowerBound / upperBound spot checks.
    assert(lowerBound(a, 2) == 1 && upperBound(a, 2) == 4);   // three 2's
    assert(lowerBound(a, 4) == 4 && upperBound(a, 4) == 6);   // two 4's
    assert(lowerBound(a, 6) == 6 && upperBound(a, 6) == 10);  // four 6's

    // Missing keys: below, between, and above the data.
    assert(lowerBound(a, 0) == 0  && upperBound(a, 0) == 0);   // before start
    assert(lowerBound(a, 3) == 4  && upperBound(a, 3) == 4);   // gap (empty span)
    assert(lowerBound(a, 9) == 10 && upperBound(a, 9) == 10);  // past the end

    // first / last occurrence.
    assert(firstOccurrence(a, 2).value() == 1);
    assert(lastOccurrence(a, 2).value() == 3);
    assert(firstOccurrence(a, 6).value() == 6);
    assert(lastOccurrence(a, 6).value() == 9);
    assert(firstOccurrence(a, 1).value() == 0);   // at the very first slot
    assert(lastOccurrence(a, 6).value() == 9);    // at the very last slot
    assert(!firstOccurrence(a, 3).has_value());   // absent (gap)
    assert(!lastOccurrence(a, 5).has_value());    // absent

    // countOf and the identity count == upper - lower.
    assert(countOf(a, 2) == 3);
    assert(countOf(a, 4) == 2);
    assert(countOf(a, 6) == 4);
    assert(countOf(a, 3) == 0);

    // Exhaustive cross-check against the brute-force references over a range
    // of queries that spans below, inside, gaps, and above the data.
    for (int q = -1; q <= 8; ++q) {
        assert(lowerBound(a, q) == lowerBoundRef(a, q));
        assert(upperBound(a, q) == upperBoundRef(a, q));
        assert(countOf(a, q) == countRef(a, q));
        assert(countOf(a, q) == upperBound(a, q) - lowerBound(a, q));  // identity
    }

    // All-equal array: bounds span the whole range.
    std::vector<int> allSame{7, 7, 7, 7};
    assert(lowerBound(allSame, 7) == 0);
    assert(upperBound(allSame, 7) == 4);
    assert(countOf(allSame, 7) == 4);
    assert(firstOccurrence(allSame, 7).value() == 0);
    assert(lastOccurrence(allSame, 7).value() == 3);

    // ---- short demo ---------------------------------------------------------
    std::cout << "Binary Search Bounds demo\n";
    std::cout << "  data = [1, 2, 2, 2, 4, 4, 6, 6, 6, 6]\n";
    std::cout << "  lowerBound(4) = " << lowerBound(a, 4)
              << ", upperBound(4) = " << upperBound(a, 4) << "\n";
    std::cout << "  first(6) = " << firstOccurrence(a, 6).value()
              << ", last(6) = " << lastOccurrence(a, 6).value()
              << ", count(6) = " << countOf(a, 6) << "\n";
    std::cout << "  count(3) [absent] = " << countOf(a, 3) << "\n";
    std::cout << "All assertions passed.\n";
    return 0;
}
