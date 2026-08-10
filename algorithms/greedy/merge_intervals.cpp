/*
 * Problem: Merge Intervals (+ Insert Interval, Min Meeting Rooms)
 *                                                    Category: Algorithm - Greedy
 * ----------------------------------------------------------------------------
 * Three classic interval sweeps that share one greedy skeleton: process events
 * in sorted order and make an irrevocable local decision.
 *
 *   1) MERGE OVERLAPPING: given intervals, combine every group that overlaps.
 *   2) INSERT INTERVAL:  given a sorted, disjoint list, add one interval and
 *      re-merge.
 *   3) MIN MEETING ROOMS: minimum rooms so no two overlapping meetings share a
 *      room = maximum number of intervals overlapping at any instant.
 *
 * GREEDY CHOICE (and WHY it is safe):
 *   MERGE/INSERT: sort by START ascending. Keep the last output interval; extend
 *   its end whenever the next interval starts <= that end, else start a new one.
 *   Because starts are sorted, once we pass an interval's start we will never see
 *   an earlier-starting one, so a non-overlap with the running block is final --
 *   nothing later can bridge back into it. That makes the local merge decision
 *   provably safe (exchange argument: any optimal partition into maximal blocks
 *   coincides with this left-to-right sweep).
 *
 *   MIN ROOMS: sort arrival and departure times SEPARATELY and two-pointer sweep.
 *   Every "start before the earliest outstanding end" forces +1 concurrent
 *   interval; the peak concurrency is a lower bound (that many pairwise overlap
 *   at one instant, all need distinct rooms) AND achievable, so it is optimal.
 *
 * Complexity:
 *   +----------------------+---------------------+
 *   | Operation            | Cost                |
 *   +----------------------+---------------------+
 *   | Merge / Min-rooms    | O(n log n) sort     |
 *   | Insert (pre-sorted)  | O(n) single pass    |
 *   +----------------------+---------------------+
 *   Everything is dominated by the initial sort.
 *
 * Complexity derivation (sort then linear sweep; per-operation counts):
 *   Let n = number of intervals.
 *   MERGE. Sorting by start is a comparison sort => O(n log n) (log n levels of
 *   n work, as in merge sort). The sweep then does one pass i = 1..n-1, each
 *   iteration O(1) (one compare, then extend-end or push-back):
 *       C_merge = SUM_{i=1}^{n-1} O(1) = (n-1) = O(n).
 *   Total = O(n log n) + O(n) = O(n log n)  (sort dominates).
 *   INSERT (input already sorted & disjoint). Three consecutive while-loops
 *   TOGETHER advance the single index i from 0 to n exactly once -- every
 *   interval is handled by exactly one loop -- so
 *       C_insert = SUM_{i=0}^{n-1} O(1) = n = O(n),  with no sort at all.
 *   MIN ROOMS. Copy starts/ends O(n), sort BOTH arrays 2*O(n log n)=O(n log n).
 *   The two-pointer sweep runs while s < n; each iteration increments exactly
 *   one of s or e, and s reaches n in n steps while e advances at most n times:
 *       C_sweep = SUM_{iterations} O(1) <= (n + n)*O(1) = 2n = O(n).
 *   Total = O(n log n) (the two sorts dominate).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f(n) = O(g)      iff  EXISTS c2, n0 :       f(n) <= c2*g(n)  for n >= n0
 *     f(n) = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f(n) = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   MERGE / MIN-ROOMS: f(n) = c1*n log n + c2*n. With g = n log n:
 *     upper  O:     f(n) <= c*(n log n)   for n >= n0  => O(n log n)
 *     lower  Omega: the sort of the n starts needs Omega(n log n) comparisons
 *                   (decision-tree bound) and runs on every input => Omega(n log n)
 *     tight  Theta: both  => Theta(n log n); data-independent, best=avg=worst.
 *   The comparison-sort Omega(n log n) applies: merging is at least as hard as
 *   sorting (sorted order is required to detect every overlap in one sweep), so
 *   no hashing trick beats it. INSERT works on PRE-SORTED input and never sorts:
 *   f(n) = c*n exactly, every interval visited once => Theta(n) (best=worst).
 *
 * Key points:
 *   - Sort key: START ascending for merging; ties do not affect correctness, but
 *     touching intervals [1,2],[2,3] ARE merged here (change <= to < for
 *     "strictly overlapping only").
 *   - Min-rooms counts max overlap, NOT interval count; the "==" boundary (a room
 *     freed exactly when the next starts) is treated as reusable (dep <= arr).
 *   - These sweeps are optimal for THIS structure; weighted interval scheduling
 *     (maximize value of non-overlapping picks) is NOT solvable by this greedy --
 *     it needs DP.
 */

#include <vector>
#include <algorithm>
#include <utility>
#include <cstddef>
#include <cassert>
#include <iostream>

using Interval = std::pair<int, int>;           // [start, end], inclusive-ish
using Intervals = std::vector<Interval>;

// 1) Merge overlapping intervals. Touching endpoints (a.end == b.start) merge.
static Intervals mergeIntervals(Intervals iv) {
    if (iv.empty()) return {};
    std::sort(iv.begin(), iv.end(),
              [](const Interval& a, const Interval& b) { return a.first < b.first; });

    Intervals out;
    out.push_back(iv.front());
    for (std::size_t i = 1; i < iv.size(); ++i) {
        Interval& last = out.back();
        if (iv[i].first <= last.second)             // overlap or touch -> extend
            last.second = std::max(last.second, iv[i].second);
        else
            out.push_back(iv[i]);                    // gap -> new block
    }
    return out;
}

// 2) Insert one interval into an already sorted, disjoint list, then re-merge.
// Single O(n) pass: emit intervals strictly before, absorb overlaps, emit after.
static Intervals insertInterval(const Intervals& sortedDisjoint, Interval add) {
    Intervals out;
    std::size_t i = 0;
    const std::size_t n = sortedDisjoint.size();

    // Intervals entirely before `add` (their end < add.start): copy as-is.
    while (i < n && sortedDisjoint[i].second < add.first)
        out.push_back(sortedDisjoint[i++]);

    // Overlapping region: expand `add` to cover everything it touches.
    while (i < n && sortedDisjoint[i].first <= add.second) {
        add.first = std::min(add.first, sortedDisjoint[i].first);
        add.second = std::max(add.second, sortedDisjoint[i].second);
        ++i;
    }
    out.push_back(add);

    // Intervals entirely after `add`: copy the tail.
    while (i < n) out.push_back(sortedDisjoint[i++]);
    return out;
}

// 3) Minimum meeting rooms = peak concurrent intervals (max overlap sweep).
static int minMeetingRooms(const Intervals& iv) {
    if (iv.empty()) return 0;
    std::vector<int> starts, ends;
    starts.reserve(iv.size());
    ends.reserve(iv.size());
    for (const Interval& x : iv) { starts.push_back(x.first); ends.push_back(x.second); }
    std::sort(starts.begin(), starts.end());
    std::sort(ends.begin(), ends.end());

    const std::size_t n = starts.size();
    int rooms = 0, peak = 0;
    std::size_t s = 0, e = 0;
    // Bounded on `e` so a departure pointer can never read past the array even
    // for zero-length or coincident intervals. A room frees at its end time and
    // is reusable when dep <= next arr (back-to-back meetings share a room).
    while (s < n) {
        if (e < n && ends[e] <= starts[s]) {
            --rooms;                 // a meeting ended at/before this start: reuse its room
            ++e;
        } else {
            ++rooms;                 // this meeting needs a room right now
            peak = std::max(peak, rooms);
            ++s;
        }
    }
    return peak;
}

int main() {
    // ---- Merge ----
    Intervals a = {{1, 3}, {2, 6}, {8, 10}, {15, 18}};
    Intervals mergedA = mergeIntervals(a);
    assert((mergedA == Intervals{{1, 6}, {8, 10}, {15, 18}}));

    // Touching intervals merge into one block.
    Intervals b = {{1, 4}, {4, 5}};
    assert((mergeIntervals(b) == Intervals{{1, 5}}));

    // Fully nested interval is absorbed.
    Intervals c = {{1, 10}, {2, 3}, {4, 8}};
    assert((mergeIntervals(c) == Intervals{{1, 10}}));

    // Unsorted input still merges correctly (sort handles it).
    Intervals d = {{8, 10}, {1, 3}, {2, 6}, {15, 18}};
    assert((mergeIntervals(d) == Intervals{{1, 6}, {8, 10}, {15, 18}}));

    // Edge: empty and single.
    assert(mergeIntervals({}).empty());
    assert((mergeIntervals({{5, 7}}) == Intervals{{5, 7}}));

    // ---- Insert ----
    Intervals base = {{1, 2}, {3, 5}, {6, 7}, {8, 10}, {12, 16}};
    // Inserting [4,8] bridges [3,5],[6,7],[8,10] into [3,10].
    assert((insertInterval(base, {4, 8}) ==
            Intervals{{1, 2}, {3, 10}, {12, 16}}));
    // Insert with no overlap slots in between.
    assert((insertInterval({{1, 3}, {6, 9}}, {2, 5}) ==
            Intervals{{1, 5}, {6, 9}}));
    // Insert into empty list.
    assert((insertInterval({}, {4, 8}) == Intervals{{4, 8}}));
    // Insert entirely after everything.
    assert((insertInterval({{1, 2}}, {5, 6}) == Intervals{{1, 2}, {5, 6}}));

    // ---- Min meeting rooms ----
    // {[0,30],[5,10],[15,20]} -> [0,30] overlaps both others (never simultaneously
    // all three), peak concurrency = 2.
    assert(minMeetingRooms({{0, 30}, {5, 10}, {15, 20}}) == 2);
    // Back-to-back meetings sharing a boundary reuse one room.
    assert(minMeetingRooms({{1, 5}, {5, 9}, {9, 12}}) == 1);
    // Three fully overlapping meetings need three rooms.
    assert(minMeetingRooms({{1, 4}, {2, 5}, {3, 6}}) == 3);
    // Edge: none, and a single meeting.
    assert(minMeetingRooms({}) == 0);
    assert(minMeetingRooms({{2, 8}}) == 1);

    std::cout << "Merge Intervals demo\n";
    std::cout << "  merge {[1,3],[2,6],[8,10],[15,18]} -> ";
    for (const Interval& x : mergedA) std::cout << "[" << x.first << "," << x.second << "] ";
    std::cout << "\n  insert [4,8] -> ";
    for (const Interval& x : insertInterval(base, {4, 8}))
        std::cout << "[" << x.first << "," << x.second << "] ";
    std::cout << "\n  min rooms {[0,30],[5,10],[15,20]} = "
              << minMeetingRooms({{0, 30}, {5, 10}, {15, 20}}) << "\n";
    std::cout << "All assertions passed.\n";
    return 0;
}
