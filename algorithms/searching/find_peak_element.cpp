/*
 * Find Peak Element (Algorithm - Searching)
 *
 * Idea:
 *   A "peak" is an index i with a[i] >= its neighbours, where any out-of-range
 *   neighbour is treated as -infinity (so the array ends always look "downhill"
 *   to the outside). At least one peak therefore always exists in a non-empty
 *   array. We do NOT need the input to be sorted: we binary-search on the SLOPE.
 *
 *   At mid, compare a[mid] with a[mid+1]:
 *     - a[mid] < a[mid+1]  -> the slope rises to the right, so a peak must exist
 *                             somewhere in (mid, hi]; move lo = mid + 1.
 *     - otherwise          -> a[mid] is >= its right neighbour, so a peak exists
 *                             in [lo, mid]; move hi = mid.
 *   Invariant: the surviving range [lo, hi] always contains a peak, because we
 *   only ever discard a side that is guaranteed to keep climbing back toward the
 *   part we keep. Since mid < hi whenever lo < hi, the range strictly shrinks and
 *   the loop converges to lo == hi, which is a peak.
 *
 * Complexity:
 *   +----------+-----------+
 *   |  Case    |   Time    |
 *   +----------+-----------+
 *   |  Best    |   O(1)    |
 *   |  Average | O(log n)  |
 *   |  Worst   | O(log n)  |
 *   +----------+-----------+
 *   Auxiliary Space: O(1)  (iterative)
 *
 * Key points / when to use:
 *   - Any peak is acceptable; this is NOT about the global maximum.
 *   - Works on unsorted data because only the local slope is inspected.
 *   - Edges count as peaks (outside neighbours are -infinity); plateaus are
 *     handled by treating a[mid] >= a[mid+1] as "go left".
 *   - Returns std::optional<std::size_t>: an index for non-empty input, else
 *     std::nullopt (an empty array has no peak).
 */

#include <vector>
#include <optional>
#include <cassert>
#include <cstddef>
#include <iostream>

// Returns the index of A peak in 'a', or std::nullopt if 'a' is empty.
template <typename T>
std::optional<std::size_t> findPeakElement(const std::vector<T>& a) {
    if (a.empty()) {
        return std::nullopt;
    }

    std::size_t lo = 0;
    std::size_t hi = a.size() - 1;
    while (lo < hi) {
        // 'lo + (hi - lo) / 2' avoids overflow; because lo < hi we always get
        // mid < hi, so mid + 1 is a valid index and hi = mid strictly shrinks.
        const std::size_t mid = lo + (hi - lo) / 2;
        if (a[mid] < a[mid + 1]) {
            lo = mid + 1;                   // uphill to the right: peak is right
        } else {
            hi = mid;                       // downhill or flat: peak is here/left
        }
    }
    return lo;                              // lo == hi: guaranteed a peak
}

// ------------------------------- Tests & demo -------------------------------

// Reference peak check with out-of-range neighbours treated as -infinity.
template <typename T>
bool isPeak(const std::vector<T>& a, std::size_t i) {
    const bool okLeft  = (i == 0)              || (a[i] >= a[i - 1]);
    const bool okRight = (i + 1 == a.size())   || (a[i] >= a[i + 1]);
    return okLeft && okRight;
}

int main() {
    // Empty range: no peak.
    assert(findPeakElement<int>({}) == std::nullopt);

    // Single element: it is trivially a peak (both neighbours are -infinity).
    {
        auto r = findPeakElement<int>({99});
        assert(r == std::optional<std::size_t>(0));
    }

    // Peak at the left end (strictly descending array).
    {
        std::vector<int> a = {5, 4, 3, 2, 1};
        auto r = findPeakElement(a);
        assert(r && isPeak(a, *r));
        assert(*r == 0);
    }

    // Peak at the right end (strictly ascending array).
    {
        std::vector<int> a = {1, 2, 3, 4, 5};
        auto r = findPeakElement(a);
        assert(r && isPeak(a, *r));
        assert(*r == 4);
    }

    // Peak in the middle.
    {
        std::vector<int> a = {1, 3, 7, 4, 2};
        auto r = findPeakElement(a);
        assert(r && isPeak(a, *r));
        assert(*r == 2);
    }

    // Two-element arrays: whichever endpoint is larger (or either if equal).
    {
        std::vector<int> a = {1, 2};
        auto r = findPeakElement(a);
        assert(r && isPeak(a, *r));
    }
    {
        std::vector<int> a = {2, 1};
        auto r = findPeakElement(a);
        assert(r && isPeak(a, *r));
    }

    // Plateaus: equal runs. Whatever index is returned must satisfy the peak
    // predicate (a[i] >= both neighbours).
    {
        std::vector<int> a = {2, 2, 2};
        auto r = findPeakElement(a);
        assert(r && isPeak(a, *r));
    }
    {
        std::vector<int> a = {1, 2, 2, 1};
        auto r = findPeakElement(a);
        assert(r && isPeak(a, *r));
    }
    {
        std::vector<int> a = {3, 1, 1, 3};
        auto r = findPeakElement(a);
        assert(r && isPeak(a, *r));
    }

    // Exhaustive: for many hand-picked arrays the result is always a valid peak.
    {
        std::vector<std::vector<int>> cases = {
            {1}, {1, 2, 1}, {1, 2, 3, 1}, {3, 2, 1, 2, 3},
            {1, 1, 1, 1, 2, 1}, {10, 20, 15, 2, 23, 90, 67},
            {1, 2, 3, 4, 5, 6}, {6, 5, 4, 3, 2, 1}, {5, 5, 4, 3},
        };
        for (const auto& a : cases) {
            auto r = findPeakElement(a);
            assert(r && isPeak(a, *r));
        }
    }

    // Short demo.
    std::vector<int> demo = {1, 3, 20, 4, 1, 0};
    auto r = findPeakElement(demo);
    std::cout << "array:";
    for (int x : demo) std::cout << ' ' << x;
    std::cout << "\npeak at index " << *r << " with value " << demo[*r] << '\n';

    std::cout << "All find-peak tests passed.\n";
    return 0;
}
