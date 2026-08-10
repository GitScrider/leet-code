/*
 * Climbing Stairs (Algorithm - Dynamic Programming)
 *
 * Problem:
 *   You are climbing a staircase with n steps. Each move you may climb either
 *   1 or 2 stairs. In how many DISTINCT ordered ways can you reach the top?
 *
 * Idea:
 *   State:
 *     dp[i] = number of distinct ways to reach step i (the top of an i-step
 *             staircase), counting order (1+2 differs from 2+1).
 *   Recurrence:
 *     The LAST move into step i is either a single step (from i-1) or a double
 *     step (from i-2). These two possibilities are disjoint and cover every way,
 *     so we simply add them:
 *         dp[i] = dp[i-1] + dp[i-2]      (for i >= 2)
 *   Base cases:
 *     dp[0] = 1   (one way to "stand" at the ground: do nothing)
 *     dp[1] = 1   (a single 1-step)
 *   This is exactly the Fibonacci recurrence, so dp[i] = Fib(i+1). The demo asks
 *   for ways to reach the TOP of n stairs, which equals dp[n]; note the classic
 *   sequence gives dp(1)=1, dp(2)=2, dp(3)=3, dp(4)=5, dp(5)=8, ... .
 *
 * Why DP works here:
 *   OVERLAPPING SUBPROBLEMS: naive recursion recomputes ways(i) exponentially
 *   often (the recursion tree branches like Fibonacci). OPTIMAL SUBSTRUCTURE:
 *   the count for i is built exactly from the counts for i-1 and i-2, so caching
 *   each subproblem once collapses the exponential blow-up to linear work.
 *
 * Complexity:
 *   +------------------------+-----------+-----------+
 *   |  Method                |   Time    |   Space   |
 *   +------------------------+-----------+-----------+
 *   |  Naive recursion       |  O(phi^n) |   O(n)    |  (call stack)
 *   |  Top-down memoization  |   O(n)    |   O(n)    |
 *   |  Bottom-up tabulation  |   O(n)    |   O(n)    |
 *   |  O(1)-space rolling    |   O(n)    |   O(1)    |
 *   +------------------------+-----------+-----------+
 *   The rolling-variable version keeps only the last two results, which is all
 *   the recurrence ever reads.
 *
 * Key points:
 *   - Top-down (memoization) vs bottom-up (tabulation) compute the SAME table;
 *     bottom-up avoids recursion overhead and is the natural home for the O(1)
 *     space trick.
 *   - Results grow like Fibonacci, so they overflow 64-bit integers for n in the
 *     low 90s; we keep n small and use unsigned long long.
 */

#include <vector>
#include <cassert>
#include <iostream>
#include <cstddef>

using ull = unsigned long long;

// --- 1) Naive exponential recursion (reference / teaching only) -------------
// Directly mirrors the recurrence; recomputes shared subproblems, hence O(phi^n).
ull climbNaive(std::size_t n) {
    if (n <= 1) return 1;  // dp[0] = dp[1] = 1
    return climbNaive(n - 1) + climbNaive(n - 2);
}

// --- 2) Top-down memoization ------------------------------------------------
// Same recursion, but each dp[i] is computed once and cached. memo holds the
// answer or a sentinel meaning "not yet computed".
ull climbMemoHelper(std::size_t n, std::vector<ull>& memo) {
    if (n <= 1) return 1;
    if (memo[n] != 0) return memo[n];      // already solved (0 == "empty")
    memo[n] = climbMemoHelper(n - 1, memo) + climbMemoHelper(n - 2, memo);
    return memo[n];
}

ull climbMemo(std::size_t n) {
    std::vector<ull> memo(n + 1, 0);       // index 0..n; 0 marks unfilled
    return climbMemoHelper(n, memo);
}

// --- 3) Bottom-up tabulation ------------------------------------------------
// Fill dp[0..n] in increasing order so every dependency is ready when needed.
ull climbTab(std::size_t n) {
    if (n <= 1) return 1;
    std::vector<ull> dp(n + 1, 0);
    dp[0] = 1;
    dp[1] = 1;
    for (std::size_t i = 2; i <= n; ++i) {
        dp[i] = dp[i - 1] + dp[i - 2];
    }
    return dp[n];
}

// --- 4) O(1)-space rolling version ------------------------------------------
// Only the previous two values are ever read, so keep just those two.
ull climbConstSpace(std::size_t n) {
    if (n <= 1) return 1;
    ull prev2 = 1;  // dp[i-2], starts as dp[0]
    ull prev1 = 1;  // dp[i-1], starts as dp[1]
    for (std::size_t i = 2; i <= n; ++i) {
        const ull cur = prev1 + prev2;
        prev2 = prev1;
        prev1 = cur;
    }
    return prev1;  // dp[n]
}

int main() {
    // All four implementations must agree on every small n.
    for (std::size_t n = 0; n <= 20; ++n) {
        const ull expected = climbNaive(n);
        assert(climbMemo(n) == expected);
        assert(climbTab(n) == expected);
        assert(climbConstSpace(n) == expected);
    }

    // Hardcoded known answers from the guidance (ways to reach the TOP of n).
    assert(climbTab(1) == 1ULL);
    assert(climbTab(2) == 2ULL);
    assert(climbTab(5) == 8ULL);
    assert(climbTab(10) == 89ULL);

    // Edge case: zero stairs -> exactly one way (already at the top).
    assert(climbTab(0) == 1ULL);
    assert(climbConstSpace(0) == 1ULL);

    // Short demo output.
    std::cout << "Climbing Stairs (ways to reach the top):\n";
    for (std::size_t n : {0u, 1u, 2u, 3u, 4u, 5u, 10u}) {
        std::cout << "  n = " << n << " -> " << climbConstSpace(n) << " ways\n";
    }
    std::cout << "All Climbing Stairs tests passed.\n";
    return 0;
}
