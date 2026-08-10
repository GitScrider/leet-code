/*
 * Problem: Minimum Railway Platforms                Category: Algorithm - Greedy
 * ----------------------------------------------------------------------------
 * Given arrival[] and departure[] times of trains at one station, find the
 * minimum number of platforms so that no train waits. A train occupies a
 * platform during [arrival, departure]; if a departure equals another train's
 * arrival, they CANNOT share (the platform is still occupied at that instant),
 * so we require a new platform on a tie.
 *
 * This is exactly "maximum number of intervals overlapping at one moment": that
 * many trains are simultaneously present, each needing its own platform.
 *
 * GREEDY CHOICE (and WHY it is safe):
 *   Sort arrivals and departures INDEPENDENTLY, then two-pointer merge them in
 *   time order. Walking the timeline: each arrival before the next outstanding
 *   departure raises the count of trains present (+1); each departure lowers it
 *   (-1). The running peak is the answer.
 *
 *   Why optimal: at the instant the peak is reached, `peak` trains are all
 *   physically present, so at least `peak` platforms are unavoidable (lower
 *   bound). The sweep also PRODUCES a valid assignment using exactly `peak`
 *   platforms (free a platform on each departure, reuse it on the next arrival),
 *   so `peak` is achievable. Lower bound == achievable => optimal. No exchange is
 *   even needed; it is a direct max-overlap counting argument.
 *
 * Complexity:
 *   +--------------------------+----------------+
 *   | Step                     | Cost           |
 *   +--------------------------+----------------+
 *   | Sort arrivals+departures | O(n log n)     |
 *   | Two-pointer sweep        | O(n)           |
 *   | Extra space              | O(1) beyond in |
 *   +--------------------------+----------------+
 *   Dominated by the two sorts.
 *
 * Complexity derivation (two comparison-sorts + linear two-pointer sweep):
 *   Let n = number of trains (arr.size() == dep.size()). The work has two parts.
 *     (a) Two INDEPENDENT sorts, each of n integer keys by comparison. One
 *         comparison sort costs c_s * n * log2 n key comparisons, so the pair
 *         costs
 *             S(n) = 2 * (c_s * n * log2 n) = 2 c_s * n log2 n.
 *     (b) The two-pointer sweep. The while loop runs while i < n; every iteration
 *         does O(1) work and advances EXACTLY ONE pointer: ++i on an arrival, ++j
 *         on a departure. i climbs 0 -> n (those n increments are what end the
 *         loop) and j increments k times with 0 <= k <= n (j never overtakes i).
 *         So the iteration count is
 *             W(n) = (#++i) + (#++j) = n + k <= 2n,
 *         giving  W(n) = SUM over sweep steps of O(1) = Theta(n).
 *   Adding the parts:
 *       C(n) = S(n) + W(n) = 2 c_s * n log2 n + Theta(n) = O(n log n).
 *   The linear sweep is a lower-order term; the two sorts dominate, matching the
 *   "Dominated by the two sorts" note above.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f(n) = O(g)      iff  EXISTS c2, n0 :       f(n) <= c2*g(n)  for n >= n0
 *     f(n) = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f(n) = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   Exact count f(n) = 2 c_s * n log2 n + c_w * n. Take g(n) = n log n:
 *     upper  O:     f(n) <= (2 c_s + c_w) * (n log2 n)  for n >= 2 => O(n log n)
 *     lower  Omega: f(n) >= 2 c_s * (n log2 n)          for n >= 2 => Omega(n log n)
 *     tight  Theta: both hold                                      => Theta(n log n)
 *   The cost is INPUT-INDEPENDENT: std::sort (introsort) is not adaptive and does
 *   Theta(n log n) work on any ordering, and the sweep is always Theta(n). So
 *   best == average == worst == Theta(n log n); no per-case split is needed.
 *   Because the times are ordered with a COMPARISON sort, the decision-tree lower
 *   bound Omega(n log n) applies and is unavoidable as written (a radix/counting
 *   sort on bounded integer times could drop the sorts to O(n), leaving an O(n)
 *   sweep-only variant -- but that is not what this code does).
 *
 * Key points:
 *   - Sort key: times ascending, arrivals and departures in SEPARATE arrays.
 *   - Tie rule matters: `arrival <= departure` counts as still-occupied, so a
 *     shared instant forces an extra platform (matches railway convention).
 *   - This greedy is optimal because the answer literally IS the max overlap;
 *     it is not a heuristic. (Assigning WHICH train to WHICH specific platform
 *     to satisfy extra constraints, e.g. platform lengths, would need more.)
 */

#include <vector>
#include <algorithm>
#include <cstddef>
#include <cassert>
#include <iostream>

// Two-pointer sweep over independently sorted arrivals/departures.
static int minPlatforms(std::vector<int> arr, std::vector<int> dep) {
    assert(arr.size() == dep.size());
    if (arr.empty()) return 0;
    std::sort(arr.begin(), arr.end());
    std::sort(dep.begin(), dep.end());

    int platforms = 0, peak = 0;
    std::size_t i = 0, j = 0;
    const std::size_t n = arr.size();
    while (i < n) {
        // Tie rule: a train arriving exactly when another departs still needs a
        // platform, hence `<=` keeps the earlier train counted as present.
        if (arr[i] <= dep[j]) {
            ++platforms;
            peak = std::max(peak, platforms);
            ++i;
        } else {
            --platforms;   // a train has left; free its platform
            ++j;
        }
    }
    return peak;
}

// Brute-force reference for small inputs: for each arrival instant, count how
// many trains are present (present iff arr[k] <= t <= dep[k]); take the max.
// Using arrival instants suffices because overlap peaks always occur at some
// arrival time.
static int bruteForcePlatforms(const std::vector<int>& arr, const std::vector<int>& dep) {
    int best = 0;
    for (std::size_t t = 0; t < arr.size(); ++t) {
        int instant = arr[t];
        int count = 0;
        for (std::size_t k = 0; k < arr.size(); ++k)
            if (arr[k] <= instant && instant <= dep[k]) ++count;
        best = std::max(best, count);
    }
    return best;
}

int main() {
    // Classic GfG instance: answer is 3 (times are 24h HHMM integers).
    std::vector<int> arr = {900, 940, 950, 1100, 1500, 1800};
    std::vector<int> dep = {910, 1200, 1120, 1130, 1900, 2000};
    assert(minPlatforms(arr, dep) == 3);
    assert(minPlatforms(arr, dep) == bruteForcePlatforms(arr, dep));

    // Disjoint intervals [900,1000],[1100,1200],[1235,1240] -> 1 platform.
    assert(minPlatforms({900, 1100, 1235}, {1000, 1200, 1240}) == 1);
    // Two trains overlapping ([900,1100] vs [940,1200]) -> 2 platforms.
    assert(minPlatforms({900, 940}, {1100, 1200}) == 2);

    // Shared-instant tie: train B arrives exactly when train A departs -> 2.
    assert(minPlatforms({1000, 1030}, {1030, 1100}) == 2);

    // No overlap at all: sequential trains reuse one platform.
    assert(minPlatforms({100, 200, 300}, {150, 250, 350}) == 1);

    // Everyone present at once -> platforms == number of trains.
    assert(minPlatforms({100, 110, 120, 130}, {900, 900, 900, 900}) == 4);
    assert(minPlatforms({100, 110, 120, 130}, {900, 900, 900, 900}) ==
           bruteForcePlatforms({100, 110, 120, 130}, {900, 900, 900, 900}));

    // Edge cases: empty and single train.
    assert(minPlatforms({}, {}) == 0);
    assert(minPlatforms({500}, {600}) == 1);

    std::cout << "Minimum Platforms demo\n";
    std::cout << "  arrivals : ";
    for (int a : arr) std::cout << a << " ";
    std::cout << "\n  departures: ";
    for (int d : dep) std::cout << d << " ";
    std::cout << "\n  minimum platforms = " << minPlatforms(arr, dep) << "\n";
    std::cout << "All assertions passed.\n";
    return 0;
}
