/*
 * Unbounded Knapsack - Algorithm - Dynamic Programming
 * ====================================================
 *
 * Idea:
 *   N item TYPES, each with weight wt[i] and value val[i], capacity W. Unlike
 *   0/1 knapsack, every item type may be used an UNLIMITED number of times.
 *   Maximize total value without exceeding capacity.
 *
 *   State:  dp[w] = best value achievable with total weight <= w.
 *   Base:   dp[0] = 0 (empty knapsack).
 *   Recur:  dp[w] = max over all items i with wt[i] <= w of
 *                       val[i] + dp[w - wt[i]]
 *           (and dp[w] >= dp[w-1] implicitly, since not adding is allowed).
 *   Why correct: consider an optimal packing for capacity w. It uses at least
 *   one item i (or is empty); remove ONE copy of i and what remains is an
 *   optimal packing for capacity w - wt[i] -- but crucially item i is still
 *   available afterward, so the subproblem is over the SAME item set. That is
 *   the difference from 0/1, and it is why we may reuse dp[w - wt[i]] from the
 *   CURRENT round.
 *
 *   LOOP DIRECTION -- the one-line contrast with 0/1 knapsack:
 *     Unbounded: iterate w ASCENDING. When we read dp[w - wt[i]], it may already
 *                include item i (updated earlier this same pass), which is
 *                exactly what lets item i be reused.
 *     0/1:       iterate w DESCENDING so dp[w - wt[i]] is the PREVIOUS row and
 *                item i is used at most once.
 *
 * Complexity:
 *   +---------------------+-----------------+---------------------------------+
 *   | Version             | Time            | Space                           |
 *   +---------------------+-----------------+---------------------------------+
 *   | 1D tabulation       | O(N * W)        | O(W)                            |
 *   +---------------------+-----------------+---------------------------------+
 *   Pseudo-polynomial in the capacity magnitude W.
 *
 * Complexity derivation (DP states * work per transition):
 *   1D TABULATION. Two nested loops, body O(1):
 *       C(N, W) = SUM_{w=1}^{W} SUM_{i=0}^{N-1} O(1)
 *               = SUM_{w=1}^{W} N
 *               = N * W
 *               = O(N * W).
 *     Equivalently there are (W+1) DP states dp[w]; computing each takes a max
 *     over all N item types (an O(N) transition) -> (W+1) * N = O(N * W). The
 *     wt[i] <= w guard skips work INSIDE the body but not the loop iteration,
 *     so the W*N body-execution count is exact. Unlike 0/1 knapsack the state is
 *     1D (item reuse is allowed), yet the time is identical because each of the
 *     W amounts still scans all N items.
 *     PSEUDO-POLYNOMIAL: W is a magnitude in ~log W bits, so O(N*W) is
 *     exponential in the bit-length of W, not polynomial in the input SIZE.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants); take g = N * W:
 *     f = O(g)      iff  EXISTS c2, n0 :        f <= c2*g   for N,W >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g <= f          for N,W >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   The inner body runs exactly W*N times for EVERY input (data-independent: the
 *   guard changes the value written, never whether the iteration runs), so
 *   f = W*N and the running time is Theta(N * W): best = average = worst.
 *   Nothing is sorted or compared for order, so the comparison-sort lower bound
 *   Omega(n log n) does not apply; the O(N*W) cost is pseudo-polynomial.
 *
 * Key points:
 *   - Same table shape as 0/1; ONLY the inner-loop direction changes.
 *   - Coin-change "min coins" is unbounded knapsack with val=1 and min instead
 *     of max -- same family of recurrence.
 *   - Verified below against a brute-force recursion for small inputs.
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

// 1D bottom-up tabulation, weight ASCENDING to permit unlimited reuse.
int unboundedKnapsack(const std::vector<int>& wt, const std::vector<int>& val,
                      int W) {
    std::vector<int> dp(static_cast<std::size_t>(W) + 1, 0);
    for (int w = 1; w <= W; ++w) {
        for (std::size_t i = 0; i < wt.size(); ++i) {
            if (wt[i] <= w) {
                // dp[w - wt[i]] may already count item i from earlier in THIS
                // ascending pass -> reuse is allowed (contrast with 0/1).
                const int take =
                    val[i] + dp[static_cast<std::size_t>(w - wt[i])];
                dp[static_cast<std::size_t>(w)] =
                    std::max(dp[static_cast<std::size_t>(w)], take);
            }
        }
    }
    return dp[static_cast<std::size_t>(W)];
}

// Brute-force oracle: at each step either finish or add one more copy of some
// item that still fits. Exponential; only for small-capacity cross-checks.
int unboundedKnapsackBrute(const std::vector<int>& wt,
                           const std::vector<int>& val, int W) {
    int best = 0;  // "add nothing more" is always an option
    for (std::size_t i = 0; i < wt.size(); ++i) {
        if (wt[i] <= W) {
            const int cand = val[i] + unboundedKnapsackBrute(wt, val, W - wt[i]);
            best = std::max(best, cand);
        }
    }
    return best;
}

int main() {
    // --- Known instance: wt {2,3,4}, val {5,6,8}, W=7.
    //     Best is 2+2+3 (values 5+5+6 = 16) or 3+4 (6+8=14); 5*3+? ...
    //     Optimal = 16 (three items: two of weight 2, one of weight 3). ---
    assert(unboundedKnapsack({2, 3, 4}, {5, 6, 8}, 7) == 16);

    // --- Reuse clearly beats 0/1: one item weight 1 value 5, W=4 -> 20 ---
    assert(unboundedKnapsack({1}, {5}, 4) == 20);

    // --- Edge cases ---
    assert(unboundedKnapsack({}, {}, 10) == 0);   // no item types
    assert(unboundedKnapsack({3}, {9}, 0) == 0);  // zero capacity
    assert(unboundedKnapsack({5}, {9}, 3) == 0);  // nothing fits

    // --- Cross-check against brute force on several small instances ---
    {
        const std::vector<std::vector<int>> weights = {
            {2, 3, 4}, {1, 4, 5}, {3, 5}};
        const std::vector<std::vector<int>> values = {
            {5, 6, 8}, {1, 5, 7}, {4, 7}};
        for (std::size_t t = 0; t < weights.size(); ++t) {
            for (int W = 0; W <= 14; ++W) {
                assert(unboundedKnapsack(weights[t], values[t], W) ==
                       unboundedKnapsackBrute(weights[t], values[t], W));
            }
        }
    }

    // --- Demo ---
    std::cout << "unboundedKnapsack({2,3,4} wt, {5,6,8} val, W=7) = "
              << unboundedKnapsack({2, 3, 4}, {5, 6, 8}, 7)
              << "  (expected 16)\n";
    std::cout << "unboundedKnapsack({1} wt, {5} val, W=4) = "
              << unboundedKnapsack({1}, {5}, 4)
              << "  (expected 20, reuse)\n";

    std::cout << "\nAll unbounded knapsack tests passed.\n";
    return 0;
}
