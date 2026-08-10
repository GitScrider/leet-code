/*
 * 0/1 Knapsack - Algorithm - Dynamic Programming
 * ==============================================
 *
 * Idea:
 *   N items, each with weight wt[i] and value val[i], and a capacity W. Each
 *   item may be taken AT MOST ONCE (the "0/1" = leave it or take it). Maximize
 *   total value without exceeding capacity.
 *
 *   State:  dp[i][w] = best value using a prefix of the first i items with a
 *           knapsack capacity of exactly-at-most w.
 *   Base:   dp[0][w] = 0 for all w (no items -> no value).
 *   Recur (for item i, 1-indexed, weight wt[i-1], value val[i-1]):
 *           dp[i][w] = dp[i-1][w]                                 // skip item i
 *           if (wt[i-1] <= w):
 *               dp[i][w] = max(dp[i][w],
 *                              val[i-1] + dp[i-1][w - wt[i-1]])   // take item i
 *   Why correct: an optimal packing either excludes item i (reduces to the same
 *   capacity over the first i-1 items) or includes it exactly once (the rest is
 *   an optimal packing of the first i-1 items into the remaining capacity). We
 *   take the better of the two -> optimal substructure. Subproblems (i,w) recur
 *   across many item orders -> overlap, so tabulation pays off.
 *
 *   1D SPACE OPTIMIZATION (rolling array):
 *     dp[w] depends only on the PREVIOUS row's dp[w] and dp[w - wt]. Collapse to
 *     a single row and iterate w DESCENDING. Descending order guarantees that
 *     dp[w - wt] still holds the PREVIOUS row's value (item not yet reused this
 *     round) -- this is exactly what enforces "at most once". (Unbounded
 *     knapsack instead iterates ASCENDING to allow reuse.)
 *
 * Complexity:
 *   +---------------------+-----------------+---------------------------------+
 *   | Version             | Time            | Space                           |
 *   +---------------------+-----------------+---------------------------------+
 *   | 2D tabulation       | O(N * W)        | O(N * W)                        |
 *   | 1D rolling array    | O(N * W)        | O(W)                            |
 *   +---------------------+-----------------+---------------------------------+
 *   Pseudo-polynomial: W is a magnitude, not an input length.
 *
 * Key points:
 *   - Loop direction is the whole trick: DESCENDING w => each item used once.
 *   - Bottom-up tabulation shown as the primary method; the 1D form is the
 *     production version. A top-down memo would key on (i, w) identically.
 *   - Verified below against brute-force subset enumeration.
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

// 2D bottom-up tabulation. Returns the maximum achievable value.
int knapsack2D(const std::vector<int>& wt, const std::vector<int>& val, int W) {
    const std::size_t n = wt.size();
    // (n+1) rows so row 0 is the empty-prefix base case; (W+1) columns.
    std::vector<std::vector<int>> dp(
        n + 1, std::vector<int>(static_cast<std::size_t>(W) + 1, 0));

    for (std::size_t i = 1; i <= n; ++i) {
        const int wi = wt[i - 1];
        const int vi = val[i - 1];
        for (int w = 0; w <= W; ++w) {
            // Option A: skip item i-1 -> inherit the row above.
            int best = dp[i - 1][static_cast<std::size_t>(w)];
            // Option B: take item i-1 if it fits.
            if (wi <= w) {
                const int take =
                    vi + dp[i - 1][static_cast<std::size_t>(w - wi)];
                best = std::max(best, take);
            }
            dp[i][static_cast<std::size_t>(w)] = best;
        }
    }
    return dp[n][static_cast<std::size_t>(W)];
}

// 1D rolling-array version. Iterating w DESCENDING keeps dp[w-wt] pointing at
// the previous "row", so each item contributes at most once.
int knapsack1D(const std::vector<int>& wt, const std::vector<int>& val, int W) {
    std::vector<int> dp(static_cast<std::size_t>(W) + 1, 0);
    for (std::size_t i = 0; i < wt.size(); ++i) {
        const int wi = wt[i];
        const int vi = val[i];
        // DESCENDING: once we overwrite dp[w], we never read it again this round
        // for a smaller w, so item i cannot be picked twice.
        for (int w = W; w >= wi; --w) {
            dp[static_cast<std::size_t>(w)] =
                std::max(dp[static_cast<std::size_t>(w)],
                         vi + dp[static_cast<std::size_t>(w - wi)]);
        }
    }
    return dp[static_cast<std::size_t>(W)];
}

// Brute force: enumerate all 2^n subsets, keep the best that fits. Exponential;
// used only as an independent oracle on small instances.
int knapsackBrute(const std::vector<int>& wt, const std::vector<int>& val,
                  int W) {
    const std::size_t n = wt.size();
    int best = 0;
    for (unsigned mask = 0; mask < (1u << n); ++mask) {
        int totW = 0, totV = 0;
        for (std::size_t i = 0; i < n; ++i) {
            if (mask & (1u << i)) {
                totW += wt[i];
                totV += val[i];
            }
        }
        if (totW <= W && totV > best) best = totV;
    }
    return best;
}

int main() {
    // --- Known instance: capacity 50, classic 3-item example -> 220 ---
    {
        const std::vector<int> wt = {10, 20, 30};
        const std::vector<int> val = {60, 100, 120};
        assert(knapsack2D(wt, val, 50) == 220);  // take items {20,30}
        assert(knapsack1D(wt, val, 50) == 220);
    }

    // --- Edge cases ---
    assert(knapsack2D({}, {}, 10) == 0);         // no items
    assert(knapsack1D({}, {}, 10) == 0);
    assert(knapsack2D({5}, {9}, 0) == 0);        // zero capacity
    assert(knapsack1D({5}, {9}, 0) == 0);
    assert(knapsack2D({7}, {9}, 3) == 0);        // single item does not fit
    assert(knapsack1D({7}, {9}, 3) == 0);

    // --- Cross-check both DP forms against brute force on random-ish sets ---
    {
        const std::vector<std::vector<int>> weights = {
            {1, 3, 4, 5}, {2, 2, 2}, {1, 2, 3, 8, 7, 4}};
        const std::vector<std::vector<int>> values = {
            {1, 4, 5, 7}, {3, 3, 3}, {20, 5, 10, 40, 15, 25}};
        for (std::size_t t = 0; t < weights.size(); ++t) {
            for (int W = 0; W <= 15; ++W) {
                const int oracle = knapsackBrute(weights[t], values[t], W);
                assert(knapsack2D(weights[t], values[t], W) == oracle);
                assert(knapsack1D(weights[t], values[t], W) == oracle);
            }
        }
    }

    // --- Demo ---
    std::cout << "knapsack01({10,20,30} wt, {60,100,120} val, W=50) = "
              << knapsack1D({10, 20, 30}, {60, 100, 120}, 50)
              << "  (expected 220)\n";

    std::cout << "\nAll 0/1 knapsack tests passed.\n";
    return 0;
}
