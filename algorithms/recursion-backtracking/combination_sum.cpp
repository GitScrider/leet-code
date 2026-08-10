/*
 * Combination Sum I (reusable candidates) - Algorithm - Recursion/Backtracking
 * ============================================================================
 *
 * Idea:
 *   Given DISTINCT positive candidates and a target, list every UNIQUE
 *   multiset of candidates that sums to target. Each candidate may be used an
 *   UNLIMITED number of times (e.g. target 7 from {2,3,6,7} -> {7} and {2,2,3}).
 *
 *   Backtracking structure (choose / explore / unchoose):
 *     - State: a partial combination `current`, the running `remaining` amount,
 *       and a `start` index into the sorted candidates.
 *     - At each level we try candidate c = a[j] for j >= start:
 *         choose c  -> push c, subtract from remaining;
 *         explore   -> recurse with start = j (NOT j+1) so c may be reused;
 *         unchoose  -> pop c, restore remaining (backtrack).
 *     - Recursing from `start = j` (never revisiting earlier indices) enforces
 *       non-decreasing pick order, so each unique combination is generated once
 *       -- this is the de-duplication mechanism (candidates are distinct).
 *
 *   Pruning (why we SORT first):
 *     - Sorting lets us STOP the loop as soon as a[j] > remaining: since the
 *       array is ascending, every later candidate is also too big. This turns
 *       a "skip and continue" into an early "break".
 *
 * Complexity:
 *   +--------------------+------------------------+-------------------------+
 *   | Quantity           | Cost                   | Why                     |
 *   +--------------------+------------------------+-------------------------+
 *   | Time               | O(N^(T/M + 1))         | branching over N cands, |
 *   |                    |  (loose upper bound)   | depth up to T/M         |
 *   | Space (recursion)  | O(T / M)               | deepest chain of picks  |
 *   | Space (output)     | O(#combos * avg len)   | storing results         |
 *   +--------------------+------------------------+-------------------------+
 *   where N = #candidates, T = target, M = smallest candidate. The search is
 *   EXPONENTIAL because a single candidate can repeat, so the reachable
 *   combination space grows combinatorially with target/M. Sorting + the
 *   "break when a[j] > remaining" prune cuts away all hopeless branches.
 *
 * Complexity derivation (backtracking state-space tree, pseudo-polynomial):
 *   Let N = #candidates, T = target, M = min candidate. Two structural facts
 *   bound the recursion tree:
 *     DEPTH: every pick subtracts a candidate value >= M from `remaining`, which
 *       starts at T; after d picks remaining <= T - d*M, and the recursion stops
 *       once remaining hits 0, so any path has at most D = floor(T / M) picks
 *       -> tree height D = T/M (this is also the O(T/M) recursion-stack depth).
 *     BRANCHING: at a node the loop tries candidates a[start..N-1], i.e. up to
 *       N children per node -> branching factor b <= N.
 *   A tree with branching <= N and height <= D has node count
 *
 *       #nodes <= SUM_{d=0}^{D} N^d = (N^(D+1) - 1)/(N - 1) <= N^(D+1)
 *              =  N^(T/M + 1)                                (geometric series)
 *
 *   Each node does O(1) choose/unchoose work; each success leaf additionally
 *   copies a combination of length <= T/M. Folding that into the node bound:
 *
 *       T = O(N^(T/M + 1))          (loose UPPER bound)
 *
 *   It is loose because the sort + "break when a[j] > remaining" prune discards
 *   most of this full N-ary tree; T appearing in the EXPONENT is what makes the
 *   cost PSEUDO-POLYNOMIAL (it tracks the numeric magnitude of target, not the
 *   bit-length of the input). The one-time sort adds O(N log N), dominated.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f = O(g)      iff  EXISTS c2, n0 :       f <= c2*g(n)   for n >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f         for n >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   The cost is strongly DATA-DEPENDENT (candidate values, target, how early the
 *   prune fires), so there is no single Theta over all inputs:
 *     UPPER: over all inputs the search is O(N^(T/M + 1)) (weak-prune worst case).
 *     LOWER: a run must at least sort, O(N log N), and emit every valid combo, so
 *            it is Omega(N log N + #combos * avg_len); when M > T the very first
 *            comparison breaks and only the sort remains -> best case Theta(N log
 *            N). Because best << worst, only O(worst) / Omega(best) are stated.
 *   This is not a sorting problem, so the comparison-sort Omega(N log N) bound is
 *   not the governing limit -- the true floor is the output size (all combos).
 *
 * Key points / when to use:
 *   - Use when items are reusable and you need every exact-sum breakdown:
 *     making change with unlimited coins, ways to reach a score, etc.
 *   - `start = j` (reuse) vs. `start = j + 1` (use-once) is the single line
 *     that separates Combination Sum I from II.
 *   - Sort once up front: it powers both the early-break prune and, for the
 *     distinct-candidate case, keeps output in a canonical order.
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <vector>

// Backtracking core over sorted `a`. `remaining` is target minus current sum.
void combinationSumRec(const std::vector<int>& a, int remaining,
                       std::size_t start, std::vector<int>& current,
                       std::vector<std::vector<int>>& out) {
    if (remaining == 0) {          // base case: current picks sum exactly to target
        out.push_back(current);
        return;
    }
    for (std::size_t j = start; j < a.size(); ++j) {
        // Prune: candidates are sorted ascending, so once one exceeds what is
        // left, all later ones do too -- no valid completion remains here.
        if (a[j] > remaining) break;

        current.push_back(a[j]);                              // choose a[j]
        // Explore with start = j (NOT j+1): a[j] may be reused any number
        // of times, which is what "each number reusable" means.
        combinationSumRec(a, remaining - a[j], j, current, out);
        current.pop_back();                                   // UNDO (backtrack)
    }
}

std::vector<std::vector<int>> combinationSum(std::vector<int> candidates,
                                             int target) {
    std::sort(candidates.begin(), candidates.end()); // enables the break-prune
    std::vector<std::vector<int>> out;
    std::vector<int> current;
    if (target > 0) combinationSumRec(candidates, target, 0, current, out);
    return out;
}

int main() {
    // --- Test 1: the canonical instance ---
    // candidates {2,3,6,7}, target 7 => {2,2,3} and {7}.
    {
        const auto result = combinationSum({2, 3, 6, 7}, 7);
        const std::vector<std::vector<int>> expected = {{2, 2, 3}, {7}};
        assert(result == expected); // ascending sort makes the order canonical
    }

    // --- Test 2: candidates {2,3,5}, target 8 => three combinations ---
    // {2,2,2,2}, {2,3,3}, {3,5}.
    {
        const auto result = combinationSum({2, 3, 5}, 8);
        const std::vector<std::vector<int>> expected = {
            {2, 2, 2, 2}, {2, 3, 3}, {3, 5}
        };
        assert(result.size() == 3);
        assert(result == expected);
    }

    // --- Test 3: every reported combination truly sums to the target ---
    {
        const int target = 11;
        const auto result = combinationSum({2, 3, 6, 7}, target);
        for (const auto& comb : result) {
            const int s = std::accumulate(comb.begin(), comb.end(), 0);
            assert(s == target);
        }
        // {2,2,2,2,3}, {2,3,3,3}, {2,2,7}, {3,3,5? no 5}... just check count > 0.
        assert(!result.empty());
    }

    // --- Test 4: no solution / degenerate targets ---
    {
        assert(combinationSum({5, 10}, 3).empty());  // target smaller than min
        assert(combinationSum({2, 4}, 7).empty());   // parity: even sums only
        assert(combinationSum({2, 3}, 0).empty());   // target 0 -> nothing to build
    }

    // --- Test 5: single candidate equal to target ---
    {
        const auto result = combinationSum({7}, 7);
        const std::vector<std::vector<int>> expected = {{7}};
        assert(result == expected);
    }

    // --- Short demo: target 7 from {2,3,6,7} ---
    std::cout << "combinationSum({2,3,6,7}, 7):\n";
    for (const auto& comb : combinationSum({2, 3, 6, 7}, 7)) {
        std::cout << "  { ";
        for (int x : comb) std::cout << x << ' ';
        std::cout << "}\n";
    }

    std::cout << "\nAll combination-sum tests passed.\n";
    return 0;
}
