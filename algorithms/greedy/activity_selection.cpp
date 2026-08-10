/*
 * Activity Selection Problem  (Algorithm - Greedy)
 * ------------------------------------------------------------------
 * Problem:
 *   Given n activities, each with a start time s[i] and finish time f[i],
 *   select the maximum-size subset of mutually compatible activities
 *   (no two selected activities overlap in time). A single person /
 *   resource can perform only one activity at a time.
 *
 * Idea (the greedy choice and WHY it is safe):
 *   Greedy choice: always pick the compatible activity that FINISHES
 *   EARLIEST. Sorting by finish time and repeatedly taking the next
 *   activity whose start >= the last chosen finish yields an optimum.
 *
 *   Exchange argument (why it is safe):
 *     Let a1 be the activity with the earliest finish time. Consider any
 *     optimal solution O and let its earliest-finishing member be o1.
 *     Because a1 finishes no later than o1 (f[a1] <= f[o1]), swapping o1
 *     for a1 keeps the solution feasible (a1 ends at least as early, so it
 *     cannot conflict with anything O scheduled after o1) and keeps the
 *     same cardinality. Hence there is an optimal solution containing the
 *     earliest-finishing activity. Removing a1 and every activity that
 *     overlaps it leaves an identical subproblem, so induction gives an
 *     optimal greedy solution. (Greedy-choice + optimal-substructure.)
 *
 * Complexity:
 *   +-------------------+------------------+
 *   | Step              | Cost             |
 *   +-------------------+------------------+
 *   | Sort by finish    | O(n log n)       |
 *   | Single scan/pick  | O(n)             |
 *   | Extra space       | O(n) (indices)   |
 *   +-------------------+------------------+
 *   Total: O(n log n), dominated by the sort.
 *
 * Complexity derivation (sort + linear scan):
 *   Two phases whose costs simply add. Let C(n) be the total work.
 *     Phase 1 -- sort by finish time. std::stable_sort is a comparison sort,
 *     so it performs Theta(n log n) key comparisons (see the decision-tree
 *     lower bound below); with enough scratch memory this is the O(n log n)
 *     the table cites.
 *     Phase 2 -- one linear scan. The loop visits each sorted activity once
 *     and does O(1) work apiece (one test start >= lastFinish plus at most one
 *     push_back and a lastFinish update):
 *         S(n) = SUM_{i=0}^{n-1} c1  =  c1 * n  =  O(n).
 *   Adding the phases:
 *         C(n) = (c*n*log2 n) + (c1*n) = O(n log n) + O(n) = O(n log n),
 *   since n log n dominates n for n >= 2. The sort is the bottleneck.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f(n) = O(g)      iff  EXISTS c2, n0 :       f(n) <= c2*g(n)  for n >= n0
 *     f(n) = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f(n) = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   The greedy scan is input-independent Theta(n); the comparison sort fixes
 *   the overall order. A comparison sort must distinguish all n! input
 *   permutations, so its decision tree has >= n! leaves and height
 *   >= log2(n!) = Theta(n log n) (Stirling) -> sorting is Omega(n log n), and
 *   stable_sort matches it at O(n log n) -> Theta(n log n). Hence, with
 *   g(n) = n log n:
 *     upper  O:     C(n) <= c2 * (n log2 n)  for n >= 2  => O(n log n)
 *     lower  Omega: C(n) >= c1 * (n log2 n)  for n >= 2  => Omega(n log n)
 *     tight  Theta: both hold                            => Theta(n log n)
 *   The bound is the SAME for best/average/worst: the O(n) selection pass can
 *   never dominate the mandatory Theta(n log n) sort, so no per-case split is
 *   needed (unlike an adaptive algorithm such as bubble sort).
 *
 * Key points:
 *   - Sort key is FINISH time (ascending); sorting by start time or by
 *     shortest duration does NOT give an optimum in general.
 *   - Compatibility test: start >= last_finish (half-open intervals;
 *     an activity may start exactly when another ends).
 *   - Greedy is optimal here because the problem is a matroid-like
 *     "interval scheduling" instance with the exchange property above.
 *   - This maximizes the COUNT of activities, not total busy time or
 *     weighted value; the weighted variant needs dynamic programming.
 */

#include <vector>
#include <algorithm>
#include <cstddef>
#include <limits>
#include <iostream>
#include <cassert>

struct Activity {
    int start;
    int finish;
    int id;  // original index, kept for reporting
};

// Returns the indices (ids) of a maximum set of non-overlapping activities.
// Half-open intervals: an activity may start exactly when the previous ends.
std::vector<int> selectActivities(std::vector<Activity> acts) {
    // Greedy choice: earliest finish first. Stable sort so that ties in
    // finish time keep a deterministic order (does not affect the count).
    std::stable_sort(acts.begin(), acts.end(),
                     [](const Activity& a, const Activity& b) {
                         return a.finish < b.finish;
                     });

    std::vector<int> chosen;
    int lastFinish = std::numeric_limits<int>::min();
    for (const Activity& a : acts) {
        if (a.start >= lastFinish) {   // compatible with the last pick
            chosen.push_back(a.id);
            lastFinish = a.finish;     // advance the frontier
        }
    }
    return chosen;
}

// Verify a chosen set is pairwise non-overlapping (sorts a local copy first).
bool isPairwiseCompatible(const std::vector<Activity>& all,
                          const std::vector<int>& chosenIds) {
    std::vector<Activity> picked;
    picked.reserve(chosenIds.size());
    for (int id : chosenIds) picked.push_back(all[static_cast<std::size_t>(id)]);
    std::sort(picked.begin(), picked.end(),
              [](const Activity& a, const Activity& b) { return a.start < b.start; });
    for (std::size_t i = 1; i < picked.size(); ++i) {
        if (picked[i].start < picked[i - 1].finish) return false;  // overlap
    }
    return true;
}

int main() {
    // Classic CLRS-style instance. Activities indexed by id 0..10.
    // (start, finish): the known maximum compatible subset has size 4,
    // e.g. {1,4},{5,7},{8,11},{12,16}.
    std::vector<Activity> acts = {
        {1, 4, 0},  {3, 5, 1},  {0, 6, 2},  {5, 7, 3},
        {3, 9, 4},  {5, 9, 5},  {6, 10, 6}, {8, 11, 7},
        {8, 12, 8}, {2, 14, 9}, {12, 16, 10}
    };

    std::vector<int> chosen = selectActivities(acts);

    // The greedy result must equal the known optimum count (4)...
    assert(chosen.size() == 4);
    // ...and every selected pair must be non-overlapping.
    assert(isPairwiseCompatible(acts, chosen));

    // Edge cases.
    {
        std::vector<Activity> empty;
        assert(selectActivities(empty).empty());

        std::vector<Activity> one = {{2, 5, 0}};
        assert(selectActivities(one).size() == 1);

        // All mutually overlapping -> only one can be chosen.
        std::vector<Activity> overlap = {{0, 10, 0}, {1, 9, 1}, {2, 8, 2}};
        assert(selectActivities(overlap).size() == 1);

        // Touching intervals are compatible (start == previous finish).
        std::vector<Activity> touch = {{0, 2, 0}, {2, 4, 1}, {4, 6, 2}};
        assert(selectActivities(touch).size() == 3);
    }

    // Short demo.
    std::cout << "Activity Selection (greedy by earliest finish)\n";
    std::cout << "Chosen " << chosen.size() << " activities (ids): ";
    for (int id : chosen)
        std::cout << "[" << acts[static_cast<std::size_t>(id)].start
                  << "," << acts[static_cast<std::size_t>(id)].finish << "] ";
    std::cout << "\nAll assertions passed.\n";
    return 0;
}
