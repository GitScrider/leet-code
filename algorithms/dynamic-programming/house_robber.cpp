/*
 * House Robber -- Max Sum of Non-Adjacent Elements
 * (Algorithm - Dynamic Programming)
 *
 * Problem:
 *   Houses in a row each hold some non-negative amount of money. A thief cannot
 *   rob two ADJACENT houses (an alarm links neighbors). Maximize the total loot.
 *   Equivalently: pick a subset of array elements, no two adjacent, of maximum
 *   sum.
 *
 * Idea:
 *   State:
 *     dp[i] = the maximum loot obtainable considering only houses 0..i
 *             (a decision about the prefix ending at house i).
 *   Recurrence:
 *     At house i we either SKIP it (keep the best up to i-1) or ROB it (take a[i]
 *     plus the best up to i-2, since i-1 is now forbidden):
 *         dp[i] = max( dp[i-1], dp[i-2] + a[i] )
 *   Base cases:
 *     dp[0] = a[0]
 *     dp[1] = max(a[0], a[1])
 *   The answer for n houses is dp[n-1].
 *
 * Why it is correct (optimal substructure + overlapping subproblems):
 *   Any optimal plan for houses 0..i makes a binary choice about house i, and
 *   whichever branch it takes, the remaining choice is itself an optimal plan
 *   for a strictly shorter prefix (0..i-1 or 0..i-2). Those shorter prefixes are
 *   solved over and over by the naive recursion (each dp[i] is asked for by both
 *   dp[i+1] and dp[i+2]), which is exactly the overlap that memoization /
 *   tabulation removes.
 *
 * Complexity:
 *   +------------------------+-----------+-----------+
 *   |  Method                |   Time    |   Space   |
 *   +------------------------+-----------+-----------+
 *   |  Bottom-up tabulation  |   O(n)    |   O(n)    |
 *   |  O(1)-space rolling    |   O(n)    |   O(1)    |
 *   +------------------------+-----------+-----------+
 *   The recurrence only ever reads dp[i-1] and dp[i-2], so two rolling variables
 *   suffice and the full dp[] array is optional.
 *
 * Key points:
 *   - Bottom-up here is the clearer formulation; the O(1) version simply keeps
 *     the last two dp values. A top-down memoization would give the same table.
 *   - Careful with std::size_t indices: never compute i-2 when i < 2. We handle
 *     the first two houses as explicit base cases before the loop.
 *   - Amounts are non-negative, so no sentinel/overflow tricks are needed beyond
 *     using long long to sum comfortably.
 */

#include <vector>
#include <cassert>
#include <iostream>
#include <cstddef>
#include <algorithm>

// --- Bottom-up tabulation (explicit dp[] array) -----------------------------
long long robTab(const std::vector<int>& a) {
    const std::size_t n = a.size();
    if (n == 0) return 0;          // no houses -> nothing to steal
    if (n == 1) return a[0];       // only one house -> take it

    std::vector<long long> dp(n, 0);
    dp[0] = a[0];
    dp[1] = std::max<long long>(a[0], a[1]);
    for (std::size_t i = 2; i < n; ++i) {
        // Skip house i, or rob it and add the best from two houses back.
        dp[i] = std::max(dp[i - 1], dp[i - 2] + a[i]);
    }
    return dp[n - 1];
}

// --- O(1)-space rolling version --------------------------------------------
// prev1 == dp[i-1], prev2 == dp[i-2]. We advance them one house at a time.
long long robConstSpace(const std::vector<int>& a) {
    long long prev2 = 0;  // best loot up to house i-2 (dp[-1] treated as 0)
    long long prev1 = 0;  // best loot up to house i-1 (dp[-2] treated as 0)
    for (const int money : a) {
        // Rob this house (prev2 + money) or skip it (prev1).
        const long long cur = std::max(prev1, prev2 + money);
        prev2 = prev1;
        prev1 = cur;
    }
    return prev1;  // dp[n-1], or 0 for an empty array
}

// --- Brute-force reference: try every subset with no two adjacent indices ---
// Exponential; used only to validate the DP on small inputs.
long long robBrute(const std::vector<int>& a) {
    const std::size_t n = a.size();
    long long best = 0;
    // Enumerate all 2^n subsets via bitmask; reject any with adjacent bits set.
    for (unsigned mask = 0; mask < (1u << n); ++mask) {
        if (mask & (mask >> 1)) continue;   // two adjacent bits -> invalid
        long long sum = 0;
        for (std::size_t i = 0; i < n; ++i) {
            if (mask & (1u << i)) sum += a[i];
        }
        best = std::max(best, sum);
    }
    return best;
}

int main() {
    // Cross-check both DP variants against brute force on many small arrays.
    const std::vector<std::vector<int>> tests = {
        {},                         // empty -> 0
        {5},                        // single -> 5
        {2, 1},                     // -> 2
        {1, 2, 3, 1},               // classic -> 4 (houses 0 and 2)
        {2, 7, 9, 3, 1},            // classic -> 12 (houses 0, 2, 4)
        {2, 1, 1, 2},               // -> 4 (houses 0 and 3)
        {0, 0, 0, 0},               // all zeros -> 0
        {5, 5, 10, 100, 10, 5},     // -> 110
        {1, 2, 3, 4, 5, 6, 7, 8},   // increasing
        {10, 1, 1, 10, 1, 1, 10},   // -> 30
    };

    for (const std::vector<int>& a : tests) {
        const long long expected = robBrute(a);
        assert(robTab(a) == expected);
        assert(robConstSpace(a) == expected);
    }

    // Hardcoded known cases, including empty and single-element edge cases.
    assert(robTab({}) == 0);
    assert(robConstSpace({}) == 0);
    assert(robTab({5}) == 5);
    assert(robConstSpace({7}) == 7);
    assert(robTab({1, 2, 3, 1}) == 4);
    assert(robTab({2, 7, 9, 3, 1}) == 12);

    // Short demo output.
    const std::vector<int> demo = {2, 7, 9, 3, 1};
    std::cout << "House Robber demo on {2,7,9,3,1}: max loot = "
              << robConstSpace(demo) << " (rob houses 0, 2, 4)\n";
    std::cout << "All House Robber tests passed.\n";
    return 0;
}
