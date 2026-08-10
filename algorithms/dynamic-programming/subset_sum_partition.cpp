/*
 * Subset Sum & Equal-Subset Partition - Algorithm - Dynamic Programming
 * =====================================================================
 *
 * Idea:
 *   Given non-negative integers `nums`, decide feasibility questions that are
 *   0/1-knapsack in disguise (each number used at most once, value == weight).
 *
 *   (1) SUBSET SUM: is there a subset summing to exactly `target`?
 *       State:  dp[s] = true iff some subset of the numbers seen so far sums
 *               to exactly s.
 *       Base:   dp[0] = true (the empty subset sums to 0).
 *       Recur:  after considering number x, dp[s] |= dp[s - x]  for s >= x.
 *       Why correct: a subset summing to s either excludes x (already
 *       reachable) or includes x (then s - x was reachable without x). This is
 *       boolean 0/1 knapsack: iterate s DESCENDING so each x is used once.
 *
 *   (2) EQUAL PARTITION: can `nums` be split into two subsets of equal sum?
 *       Total must be even; then it reduces to SUBSET SUM with target=sum/2.
 *       If a subset reaches sum/2, its complement also sums to sum/2.
 *
 * Complexity:
 *   +---------------------+-----------------+---------------------------------+
 *   | Problem             | Time            | Space                           |
 *   +---------------------+-----------------+---------------------------------+
 *   | Subset sum          | O(N * target)   | O(target)  (1D boolean row)     |
 *   | Equal partition     | O(N * sum/2)    | O(sum)                          |
 *   +---------------------+-----------------+---------------------------------+
 *   Pseudo-polynomial in the target/sum magnitude.
 *
 * Complexity derivation (nested-loop instruction count / summation):
 *   subsetSum: the outer loop runs once per number (N iterations). For a fixed
 *   number x the inner loop runs s = target, target-1, ..., x, that is
 *   (target - x + 1) iterations, each doing O(1) OR-aggregate work. Summing the
 *   inner cost over all numbers:
 *
 *       C(N, target) = SUM_{x in nums} (target - x + 1)
 *                    <= SUM_{x in nums} (target + 1)          (since x >= 0)
 *                    =  N * (target + 1)
 *                    =  O(N * target)
 *
 *   The one-time dp allocation/initialization adds Theta(target). Equal
 *   partition first computes total = SUM nums in O(N), rejects odd totals, then
 *   calls subsetSum with target = sum/2, giving O(N * sum/2) = O(N * sum).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants), problem SIZE = (N, target):
 *     f = O(g)      iff  EXISTS c2, n0 :        f <= c2*g   for inputs >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g <= f          for inputs >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   WORST case (e.g. all x = 0, or every x <= target): each inner loop runs
 *   ~target times, so C = Theta(N * target). BEST case (every x > target): the
 *   inner-loop body never executes, leaving only the outer scan plus the
 *   Theta(target) init, i.e. Omega(N + target). Overall the running time is thus
 *   O(N * target) (upper bound, from the worst case) and Omega(N + target).
 *   This is PSEUDO-POLYNOMIAL: linear in the VALUE of target but exponential in
 *   its bit-length log(target). No comparison-sort Omega(n log n) bound applies
 *   -- this is a boolean reachability DP, not a comparison sort.
 *
 * Key points:
 *   - Boolean DP: OR-aggregate instead of min/max. Same 0/1 loop discipline
 *     (descending target) to forbid reusing an element.
 *   - Partition is a thin wrapper: even-total check + subset sum to half.
 *   - Cross-checked against 2^N brute-force subset enumeration below.
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <vector>

// (1) Subset-sum feasibility using a 1D boolean table (0/1 discipline).
bool subsetSum(const std::vector<int>& nums, int target) {
    if (target < 0) return false;  // non-negative inputs cannot reach it
    std::vector<char> dp(static_cast<std::size_t>(target) + 1, 0);
    dp[0] = 1;  // empty subset reaches 0
    for (const int x : nums) {
        // DESCENDING so dp[s - x] refers to the state BEFORE using x this pass,
        // i.e. x contributes to any subset at most once.
        for (int s = target; s >= x; --s) {
            if (dp[static_cast<std::size_t>(s - x)]) {
                dp[static_cast<std::size_t>(s)] = 1;
            }
        }
    }
    return dp[static_cast<std::size_t>(target)] != 0;
}

// (2) Can nums be partitioned into two equal-sum halves?
bool canPartition(const std::vector<int>& nums) {
    const int total = std::accumulate(nums.begin(), nums.end(), 0);
    if (total % 2 != 0) return false;  // odd total can never split evenly
    return subsetSum(nums, total / 2);
}

// Brute-force oracle for subset sum: try all 2^N subsets. Exponential.
bool subsetSumBrute(const std::vector<int>& nums, int target) {
    const std::size_t n = nums.size();
    for (unsigned mask = 0; mask < (1u << n); ++mask) {
        int s = 0;
        for (std::size_t i = 0; i < n; ++i) {
            if (mask & (1u << i)) s += nums[i];
        }
        if (s == target) return true;
    }
    return false;
}

int main() {
    // --- Subset sum: known yes/no instances ---
    assert(subsetSum({3, 34, 4, 12, 5, 2}, 9) == true);   // 4 + 5
    assert(subsetSum({3, 34, 4, 12, 5, 2}, 30) == false); // no subset hits 30
    assert(subsetSum({1, 2, 3}, 0) == true);              // empty subset
    assert(subsetSum({}, 0) == true);                     // empty set, target 0
    assert(subsetSum({}, 5) == false);                    // empty set, target>0
    assert(subsetSum({2, 4, 6}, 5) == false);             // parity: all even

    // --- Equal partition: known yes/no instances ---
    assert(canPartition({1, 5, 11, 5}) == true);   // {11},{1,5,5}
    assert(canPartition({1, 2, 3, 5}) == false);   // total 11 is odd
    assert(canPartition({}) == true);              // empty splits into {},{}
    assert(canPartition({7}) == false);            // single odd element

    // --- Cross-check subset sum vs brute force over many targets ---
    {
        const std::vector<std::vector<int>> sets = {
            {3, 34, 4, 12, 5, 2}, {1, 1, 1, 1}, {6, 2, 4}, {5}};
        for (const auto& nums : sets) {
            const int sum = std::accumulate(nums.begin(), nums.end(), 0);
            for (int t = 0; t <= sum + 2; ++t) {
                assert(subsetSum(nums, t) == subsetSumBrute(nums, t));
            }
        }
    }

    // --- Cross-check partition via brute force (subset hitting half) ---
    {
        const std::vector<std::vector<int>> sets = {
            {1, 5, 11, 5}, {1, 2, 3, 5}, {2, 2, 2, 2}, {3, 3}, {8}};
        for (const auto& nums : sets) {
            const int sum = std::accumulate(nums.begin(), nums.end(), 0);
            const bool oracle =
                (sum % 2 == 0) && subsetSumBrute(nums, sum / 2);
            assert(canPartition(nums) == oracle);
        }
    }

    // --- Demo ---
    std::cout << "subsetSum({3,34,4,12,5,2}, 9)  = "
              << (subsetSum({3, 34, 4, 12, 5, 2}, 9) ? "true" : "false")
              << "  (expected true)\n";
    std::cout << "canPartition({1,5,11,5})       = "
              << (canPartition({1, 5, 11, 5}) ? "true" : "false")
              << "  (expected true)\n";
    std::cout << "canPartition({1,2,3,5})        = "
              << (canPartition({1, 2, 3, 5}) ? "true" : "false")
              << "  (expected false)\n";

    std::cout << "\nAll subset-sum / partition tests passed.\n";
    return 0;
}
