/*
 * Coin Change (Min Coins + Count Ways) - Algorithm - Dynamic Programming
 * ======================================================================
 *
 * Idea:
 *   Two classic unbounded (coins reusable unlimited times) DP problems over the
 *   same coin set. Both have OPTIMAL SUBSTRUCTURE / OVERLAPPING SUBPROBLEMS:
 *   the answer for amount `a` is built from answers for smaller amounts, and the
 *   same sub-amount is reached through many coin orders, so we memoize once.
 *
 *   (1) MINIMUM number of coins to make `amount`.
 *       State:  dp[a] = fewest coins summing exactly to a.
 *       Base:   dp[0] = 0 (empty selection makes amount 0).
 *       Recur:  dp[a] = min over coins c (c <= a) of dp[a - c] + 1.
 *               If no coin yields a reachable a-c, dp[a] stays INF (impossible).
 *       Why correct: any optimal solution for `a` ends by adding SOME last coin
 *       c; removing it leaves an optimal solution for a-c (cut-and-paste), so
 *       taking the best over all possible last coins is optimal.
 *
 *   (2) NUMBER OF WAYS to make `amount` (order does NOT matter -> combinations).
 *       State:  ways[a] = number of coin multisets summing to a.
 *       Base:   ways[0] = 1 (exactly one way: take nothing).
 *       Recur:  process coins in an OUTER loop, amounts inner:
 *               ways[a] += ways[a - c].
 *       Why the coin-outer loop: iterating coins on the outside fixes a coin
 *       ORDER, so each combination (multiset) is counted once. Swapping the
 *       loops would count permutations (e.g. 1+2 and 2+1 separately).
 *
 * Complexity:
 *   +----------------------+------------------+----------------------------+
 *   | Problem              | Time             | Space                      |
 *   +----------------------+------------------+----------------------------+
 *   | Min coins            | O(amount * N)    | O(amount)                  |
 *   | Count ways           | O(amount * N)    | O(amount)                  |
 *   +----------------------+------------------+----------------------------+
 *   N = number of coin denominations. Both use a single 1D rolling table.
 *
 * Key points:
 *   - Min coins is a MIN aggregation (+1 per coin); ways is a SUM (+= counts).
 *   - Coins are UNBOUNDED, so the amount loop goes ASCENDING (a smaller amount
 *     already reflects reusing the current coin) -- contrast with 0/1 knapsack.
 *   - Impossible min-coins amounts return the sentinel -1 (INF internally).
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

// Sentinel for "unreachable". We keep it well below INT_MAX so that the
// `dp[a - c] + 1` update can never overflow even if it were (wrongly) applied.
constexpr int kUnreachable = 1'000'000'000;

// (1) Minimum number of coins to form `amount`; returns -1 if impossible.
int coinChangeMin(const std::vector<int>& coins, int amount) {
    // dp[a] = fewest coins for amount a. Size amount+1 so index == amount.
    std::vector<int> dp(static_cast<std::size_t>(amount) + 1, kUnreachable);
    dp[0] = 0;  // base case: zero coins make amount 0

    for (int a = 1; a <= amount; ++a) {
        for (const int c : coins) {
            // Only a coin no larger than a can be the "last coin" for amount a.
            if (c <= a && dp[static_cast<std::size_t>(a - c)] != kUnreachable) {
                const int candidate = dp[static_cast<std::size_t>(a - c)] + 1;
                if (candidate < dp[static_cast<std::size_t>(a)]) {
                    dp[static_cast<std::size_t>(a)] = candidate;
                }
            }
        }
    }
    return dp[static_cast<std::size_t>(amount)] == kUnreachable
               ? -1
               : dp[static_cast<std::size_t>(amount)];
}

// (2) Number of distinct combinations (order-insensitive) forming `amount`.
long long coinChangeWays(const std::vector<int>& coins, int amount) {
    std::vector<long long> ways(static_cast<std::size_t>(amount) + 1, 0);
    ways[0] = 1;  // base case: one way to make 0 -- pick nothing

    // Coin-OUTER loop enforces a fixed pick order => counts multisets, not
    // permutations. This is the crucial ordering for the "ways" variant.
    for (const int c : coins) {
        for (int a = c; a <= amount; ++a) {
            ways[static_cast<std::size_t>(a)] +=
                ways[static_cast<std::size_t>(a - c)];
        }
    }
    return ways[static_cast<std::size_t>(amount)];
}

// Brute-force reference for min coins: exhaustive recursion (exponential),
// used only to cross-check the DP on small inputs.
int coinChangeMinBrute(const std::vector<int>& coins, int amount) {
    if (amount == 0) return 0;
    if (amount < 0) return kUnreachable;
    int best = kUnreachable;
    for (const int c : coins) {
        const int sub = coinChangeMinBrute(coins, amount - c);
        if (sub != kUnreachable && sub + 1 < best) best = sub + 1;
    }
    return best;
}

int main() {
    // --- Min coins: canonical {1,2,5}, 11 -> 3 (5+5+1) ---
    assert(coinChangeMin({1, 2, 5}, 11) == 3);

    // --- Min coins edge cases ---
    assert(coinChangeMin({1, 2, 5}, 0) == 0);   // zero target -> zero coins
    assert(coinChangeMin({2}, 3) == -1);        // parity makes 3 impossible
    assert(coinChangeMin({}, 7) == -1);         // no coins, positive target
    assert(coinChangeMin({}, 0) == 0);          // no coins but target already 0

    // --- Min coins cross-checked against brute force on small amounts ---
    {
        const std::vector<int> coins = {1, 3, 4};
        for (int a = 0; a <= 20; ++a) {
            const int fast = coinChangeMin(coins, a);
            const int slow = coinChangeMinBrute(coins, a);
            const int slowNorm = (slow == kUnreachable) ? -1 : slow;
            assert(fast == slowNorm);
        }
    }

    // --- Count ways: {1,2,5}, 5 -> 4  ({5},{2,2,1},{2,1,1,1},{1x5}) ---
    assert(coinChangeWays({1, 2, 5}, 5) == 4);

    // --- Count ways edge cases ---
    assert(coinChangeWays({1, 2, 5}, 0) == 1);  // one way: empty selection
    assert(coinChangeWays({2}, 3) == 0);        // impossible -> zero ways
    assert(coinChangeWays({1, 2, 5}, 11) == 11);

    // --- Demo ---
    std::cout << "coinChangeMin({1,2,5}, 11) = " << coinChangeMin({1, 2, 5}, 11)
              << "  (expected 3)\n";
    std::cout << "coinChangeWays({1,2,5}, 5) = " << coinChangeWays({1, 2, 5}, 5)
              << "  (expected 4)\n";
    std::cout << "coinChangeMin({2}, 3)      = " << coinChangeMin({2}, 3)
              << "  (expected -1, impossible)\n";

    std::cout << "\nAll coin-change tests passed.\n";
    return 0;
}
