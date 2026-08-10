/*
 * Bucket Sort  -- Algorithm - Sorting
 *
 * Idea:
 *   Scatter the values across an ordered array of "buckets" by mapping each value
 *   to a bucket index via a monotonic (non-decreasing) function of the value, sort
 *   each bucket independently, then gather the buckets back in index order. Because
 *   the mapping is monotonic, every element in bucket i is <= every element in
 *   bucket i+1, so concatenating sorted buckets yields a fully sorted array. The
 *   speedup relies on the ASSUMPTION that values are roughly UNIFORMLY distributed,
 *   so each of the ~n buckets holds O(1) elements and the per-bucket sorts are cheap.
 *
 * Complexity:
 *   Case    | Time      | Aux Space
 *   --------+-----------+-----------
 *   Best    | O(n + k)  | O(n + k)
 *   Average | O(n + k)  | O(n + k)   (k = number of buckets; uniform input)
 *   Worst   | O(n^2)    | O(n + k)   (all values collide into ONE bucket, whose
 *                                     internal sort then degrades to O(n^2)/O(n log n))
 *
 * Properties:
 *   Stable?    yes  (as written) -- distribution preserves input order within a
 *                    bucket, and each bucket is sorted with std::stable_sort. Equal
 *                    values always map to the SAME bucket, so relative order of
 *                    equal keys is preserved end to end. (Swap to std::sort and the
 *                    guarantee is lost.)
 *   In-place?  no   (allocates the bucket arrays and gathers into the output)
 *   Adaptive?  no   (structure of the work does not shrink on nearly-sorted input)
 *
 * When to use / notes:
 *   - Values known to be uniformly spread over a range (classic for floats in [0,1);
 *     here shown for ints via a min/max range map).
 *   - Degrades to quadratic when the distribution is skewed -- pick the bucket count
 *     and mapping to match the data.
 *   - Edge cases handled below WITHOUT dividing by zero: empty input returns
 *     immediately; when min == max (empty range, includes the all-equal case) the
 *     data is already sorted and we return before computing any ratio.
 */

#include <vector>
#include <algorithm>   // std::sort, std::stable_sort, std::minmax_element, std::is_sorted
#include <utility>     // std::pair
#include <cstddef>     // std::size_t
#include <cassert>
#include <iostream>

// Stable bucket sort for integers over the data's own [min, max] range.
void bucketSort(std::vector<int>& a) {
    const std::size_t n = a.size();
    if (n <= 1) return;                       // empty or single element: already sorted

    const auto mm = std::minmax_element(a.begin(), a.end());
    const int lo = *mm.first;
    const int hi = *mm.second;
    if (lo == hi) return;                     // all elements equal -> sorted; avoids /0

    // Use n buckets. Map value v -> index in [0, n-1] with a monotonic ratio.
    // long long guards against overflow in (v - lo) * (n - 1).
    const std::size_t numBuckets = n;
    const long long range = static_cast<long long>(hi) - static_cast<long long>(lo);
    std::vector<std::vector<int>> buckets(numBuckets);

    // Distribution: scanning left-to-right and appending keeps equal keys in their
    // original relative order inside a bucket -- the first half of the stability story.
    for (const int v : a) {
        const long long offset = static_cast<long long>(v) - static_cast<long long>(lo);
        std::size_t idx = static_cast<std::size_t>(offset * static_cast<long long>(numBuckets - 1) / range);
        if (idx >= numBuckets) idx = numBuckets - 1;   // clamp guard (defensive)
        buckets[idx].push_back(v);
    }

    // Sort each bucket stably, then gather in bucket-index order. Because the mapping
    // is non-decreasing, bucket i's values are all <= bucket i+1's values, so simple
    // concatenation produces the globally sorted sequence.
    std::size_t pos = 0;
    for (auto& bucket : buckets) {
        std::stable_sort(bucket.begin(), bucket.end());
        for (const int v : bucket) {
            a[pos++] = v;
        }
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
    bucketSort(in);
    assert(in == reference);
}

int main() {
    // Required edge cases -----------------------------------------------------
    expectSorted({});                                  // empty
    expectSorted({42});                                // single element
    expectSorted({1, 2, 3, 4, 5, 6});                  // already sorted
    expectSorted({9, 7, 5, 3, 1, 0});                  // reverse sorted
    expectSorted({4, 4, 4, 4});                        // all equal (min == max path)
    expectSorted({29, 25, 3, 49, 9, 37, 21, 43, 3});   // duplicates
    expectSorted({-8, -3, -8, 0, 5, -1, 12, 5, -20});  // negatives and large spread

    // Stability check: bucket sort as written is stable. Sort pairs by .first only
    // (equal keys must keep original .second order). We mirror bucketSort's logic on
    // pairs, keying the bucket map on .first and using std::stable_sort per bucket.
    {
        std::vector<std::pair<int, int>> in = {
            {3, 0}, {1, 1}, {3, 2}, {1, 3}, {2, 4}, {3, 5}, {1, 6}
        };
        const std::size_t n = in.size();
        const auto keyLess = [](const std::pair<int, int>& x,
                                const std::pair<int, int>& y) { return x.first < y.first; };
        const auto mm = std::minmax_element(in.begin(), in.end(), keyLess);
        const int lo = mm.first->first;
        const int hi = mm.second->first;
        const long long range = static_cast<long long>(hi) - static_cast<long long>(lo);

        std::vector<std::vector<std::pair<int, int>>> buckets(n);
        for (const auto& p : in) {
            const long long offset = static_cast<long long>(p.first) - lo;
            std::size_t idx = static_cast<std::size_t>(offset * static_cast<long long>(n - 1) / range);
            if (idx >= n) idx = n - 1;
            buckets[idx].push_back(p);
        }
        std::vector<std::pair<int, int>> out;
        out.reserve(n);
        for (auto& b : buckets) {
            std::stable_sort(b.begin(), b.end(), keyLess);  // compare .first only
            for (const auto& p : b) out.push_back(p);
        }
        // Keys ascending; within each key the .second tags keep their input order.
        assert((out == std::vector<std::pair<int, int>>{
            {1, 1}, {1, 3}, {1, 6}, {2, 4}, {3, 0}, {3, 2}, {3, 5}}));
    }

    // Before/after demo -------------------------------------------------------
    std::vector<int> demo = {29, 25, 3, 49, 9, 37, 21, 43, 3};
    print("before: ", demo);
    bucketSort(demo);
    print("after:  ", demo);
    assert(std::is_sorted(demo.begin(), demo.end()));

    std::cout << "All bucket sort tests passed.\n";
    return 0;
}
