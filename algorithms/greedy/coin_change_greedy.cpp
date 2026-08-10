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
 * Complexity derivation (comparison-sort + single linear scan; D denominations):
 *   Greedy path. Let D = number of denominations.
 *     (a) Sort the D coins descending: a comparison sort costs
 *             S(D) = c_s * D * log2 D   key comparisons.
 *     (b) Greedy scan: ONE for loop over the D sorted coins. Each iteration does
 *         O(1) work -- an integer division (count += remaining / coin) and a
 *         modulo (remaining %= coin) -- so
 *             G(D) = SUM_{k=0}^{D-1} O(1) = c_g * D = O(D).
 *         Note the answer VALUE (number of coins used) does NOT enter the cost:
 *         taking all copies of a coin at once via integer division keeps each of
 *         the D denominations O(1), instead of an amount-many, one-coin-at-a-time
 *         loop.
 *   Total:
 *       C(D) = S(D) + G(D) = c_s * D log2 D + c_g * D = O(D log D),
 *   sort-dominated, matching the table.
 *
 *   DP verifier (coinChangeDP). Two nested loops: outer a = 1..amount (amount
 *   iterations) times inner over D coins, each transition O(1):
 *       C_dp = SUM_{a=1}^{amount} SUM_{coin} O(1) = amount * D * O(1)
 *            = O(amount * D).
 *   This is PSEUDO-POLYNOMIAL: polynomial in the numeric VALUE `amount`, not in
 *   the input size log2(amount) bits. Space O(amount) for the dp[] table.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, D0 positive constants):
 *     f(D) = O(g)      iff  EXISTS c2, D0 :       f(D) <= c2*g(D)  for D >= D0
 *     f(D) = Omega(g)  iff  EXISTS c1, D0 :  c1*g(D) <= f(D)        for D >= D0
 *     f(D) = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   Greedy exact count f(D) = c_s * D log2 D + c_g * D. Take g(D) = D log D:
 *     upper  O:     f(D) <= (c_s + c_g) * (D log2 D)  for D >= 2 => O(D log D)
 *     lower  Omega: f(D) >= c_s * (D log2 D)          for D >= 2 => Omega(D log D)
 *     tight  Theta: both hold                                    => Theta(D log D)
 *   The COMPARISON-sort decision-tree lower bound Omega(D log D) applies to
 *   sorting the D denominations, so it dominates the O(D) scan and the greedy is
 *   Theta(D log D) regardless of input order (introsort is not adaptive; the
 *   scan's early break on remaining == 0 only trims a lower-order O(D) term). The
 *   DP verifier does NO comparison sort, so that Omega(D log D) bound is
 *   irrelevant to it; its cost is the tight, pseudo-polynomial Theta(amount * D).
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
