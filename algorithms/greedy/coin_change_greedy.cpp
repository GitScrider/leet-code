/*
 * Coin Change -- Greedy (Algorithm - Greedy)
 *
 * Problem:
 *   Given coin denominations and a target amount, make the amount using the
 *   FEWEST coins (unlimited supply of each denomination). The greedy strategy:
 *   repeatedly take the LARGEST coin that does not exceed the remaining amount.
 *
 * Idea (the greedy choice and when it is safe):
 *   Greedy choice: at each step subtract the biggest usable coin. Intuition for
 *   canonical systems like {1, 5, 10, 25}: using a smaller coin where a larger
 *   one fits would force you to cover the same value with MORE pieces, because in
 *   a canonical system each denomination "absorbs" as much value as any legal
 *   combination of smaller ones could, without overshoot. An EXCHANGE ARGUMENT
 *   makes this precise for US-style coins: in an optimal solution the count of
 *   any smaller coin can never be large enough to be swapped for one bigger coin
 *   (e.g. at most one 5 is "unmerged", never enough pennies to form a nickel,
 *   etc.), so the optimal multiset must match the greedy pick at the top and, by
 *   induction, all the way down.
 *
 *   *** WHEN GREEDY FAILS ***
 *   Greedy is guaranteed optimal ONLY for CANONICAL coin systems. For an
 *   arbitrary system it can be strictly worse. Classic counterexample: coins
 *   {1, 3, 4}, amount 6. Greedy takes 4, then 1, then 1  => 3 coins. The true
 *   optimum is 3 + 3 => 2 coins. The greedy choice of the big "4" is a trap: it
 *   leaves a remainder (2) that only pennies can fill. General optimal coin
 *   change needs DYNAMIC PROGRAMMING, not greed.
 *
 * Complexity (greedy, D denominations):
 *   +-----------------------+------------------------------+
 *   |  Step                 |  Time                        |
 *   +-----------------------+------------------------------+
 *   |  Sort denominations   |  O(D log D)                  |
 *   |  Greedy scan          |  O(D) once sorted            |
 *   |  Total                |  O(D log D)  (sort-dominated)|
 *   +-----------------------+------------------------------+
 *   Space: O(1) beyond the (sorted) input. The DP verifier below is O(amount*D)
 *   time and O(amount) space -- used only to expose the counterexample.
 *
 * Key points:
 *   - Sort denominations DESCENDING so "largest coin <= remaining" is the first
 *     that fits. We sort explicitly rather than assume input order.
 *   - Greedy == DP optimum for canonical systems ({1,5,10,25}); we assert this.
 *   - Greedy != optimum for {1,3,4}, amount 6; we assert greedy gives the WORSE
 *     count (3) while DP gives 2, making the lesson explicit.
 *   - Assumes a unit coin (1) exists so every amount is representable; otherwise
 *     greedy (and any method) may fail to form the amount at all.
 */

#include <vector>
#include <cassert>
#include <iostream>
#include <algorithm>   // std::sort
#include <functional>  // std::greater
#include <cstddef>

// Greedy coin count: repeatedly take the largest coin <= remaining.
// Returns the number of coins used, or -1 if the greedy walk gets stuck
// (remainder cannot be reduced with any available denomination).
int coinChangeGreedy(std::vector<int> coins, int amount) {
    assert(amount >= 0);
    // Sort DESCENDING so the first coin that fits is the largest usable one.
    std::sort(coins.begin(), coins.end(), std::greater<int>());

    int count = 0;
    int remaining = amount;
    for (const int coin : coins) {
        if (coin <= 0) continue;              // ignore non-positive denominations
        if (remaining == 0) break;
        // Take as many of this coin as fit in one shot (integer division).
        count += remaining / coin;
        remaining %= coin;
    }
    return (remaining == 0) ? count : -1;     // -1 => greedy could not finish
}

// ------------------------- DP optimum (verifier) -------------------------
// Bottom-up minimum-coins DP: dp[a] = fewest coins to form amount a, or a large
// sentinel if impossible. This is the CORRECT method for arbitrary systems and
// is used to check greedy (matches on canonical, beats it on {1,3,4}).
int coinChangeDP(const std::vector<int>& coins, int amount) {
    const int INF = amount + 1;               // more coins than any real solution
    std::vector<int> dp(static_cast<std::size_t>(amount) + 1, INF);
    dp[0] = 0;                                 // zero coins make amount 0
    for (int a = 1; a <= amount; ++a) {
        for (const int coin : coins) {
            if (coin > 0 && coin <= a && dp[a - coin] + 1 < dp[a]) {
                dp[a] = dp[a - coin] + 1;
            }
        }
    }
    return (dp[amount] == INF) ? -1 : dp[amount];
}

int main() {
    // --- Canonical US system {1,5,10,25}: greedy IS optimal. ---
    {
        const std::vector<int> us = {1, 5, 10, 25};

        // 63 = 25+25+10+1+1+1 -> 6 coins.
        assert(coinChangeGreedy(us, 63) == 6);
        assert(coinChangeGreedy(us, 63) == coinChangeDP(us, 63));

        // 30 = 25 + 5 -> 2 coins.
        assert(coinChangeGreedy(us, 30) == 2);
        assert(coinChangeGreedy(us, 30) == coinChangeDP(us, 30));

        // Exhaustively confirm greedy == DP for every amount 0..99 (canonical).
        for (int amt = 0; amt <= 99; ++amt) {
            assert(coinChangeGreedy(us, amt) == coinChangeDP(us, amt));
        }
    }

    // --- COUNTEREXAMPLE: {1,3,4}, amount 6. Greedy is SUBOPTIMAL. ---
    // Greedy: 4, then 1, then 1  => 3 coins.
    // Optimal (DP): 3 + 3        => 2 coins.
    {
        const std::vector<int> tricky = {1, 3, 4};
        const int greedy = coinChangeGreedy(tricky, 6);
        const int optimal = coinChangeDP(tricky, 6);
        assert(greedy == 3);                 // greedy's WORSE answer, made explicit
        assert(optimal == 2);                // the true optimum
        assert(greedy > optimal);            // greed strictly loses here
    }

    // --- More failures of greedy on the same non-canonical system ---
    // amount 8: greedy 4+4 = 2 (here greedy happens to be optimal, DP agrees).
    // amount 9: greedy 4+4+1 = 3; optimal 3+3+3 = 3 (tie). Use these to show
    // greedy is not ALWAYS wrong on a non-canonical system -- just not GUARANTEED.
    {
        const std::vector<int> tricky = {1, 3, 4};
        assert(coinChangeGreedy(tricky, 8) == 2);
        assert(coinChangeDP(tricky, 8) == 2);
        assert(coinChangeGreedy(tricky, 9) == 3);
        assert(coinChangeDP(tricky, 9) == 3);
    }

    // --- Edge cases ---
    {
        const std::vector<int> us = {1, 5, 10, 25};
        assert(coinChangeGreedy(us, 0) == 0);   // amount 0 needs no coins
        assert(coinChangeDP(us, 0) == 0);
        assert(coinChangeGreedy(us, 1) == 1);   // a single penny
    }
    {
        // No unit coin: some amounts become unrepresentable. Greedy reports -1
        // when it gets stuck; DP confirms the amount truly cannot be formed.
        const std::vector<int> noUnit = {3, 7};
        assert(coinChangeGreedy(noUnit, 5) == -1);  // 5 not reachable with {3,7}
        assert(coinChangeDP(noUnit, 5) == -1);
        assert(coinChangeGreedy(noUnit, 6) == 2);   // 3 + 3
        assert(coinChangeDP(noUnit, 6) == 2);
    }

    // ------------------------------ Demo ------------------------------
    {
        const std::vector<int> us = {1, 5, 10, 25};
        std::cout << "Coin Change (canonical {1,5,10,25}):\n";
        std::cout << "  amount 63 -> greedy " << coinChangeGreedy(us, 63)
                  << " coins, DP " << coinChangeDP(us, 63) << " coins (match)\n";

        const std::vector<int> tricky = {1, 3, 4};
        std::cout << "Coin Change (non-canonical {1,3,4}):\n";
        std::cout << "  amount 6  -> greedy " << coinChangeGreedy(tricky, 6)
                  << " coins, DP " << coinChangeDP(tricky, 6)
                  << " coins  <-- greedy is SUBOPTIMAL here\n";
    }

    std::cout << "All Coin Change (greedy) tests passed.\n";
    return 0;
}
