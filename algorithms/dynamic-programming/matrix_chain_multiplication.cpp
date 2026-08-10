/*
 * Matrix Chain Multiplication
 * Category: Algorithm - Dynamic Programming (Interval DP)
 *
 * Problem:
 *   Given a chain of matrices A_1, A_2, ..., A_n where matrix A_i has
 *   dimensions p[i-1] x p[i] (the whole chain is described by the dimension
 *   array p[0..n]), find the minimum number of scalar multiplications needed
 *   to compute the product A_1 * A_2 * ... * A_n. Matrix multiplication is
 *   associative, so we may choose any order (parenthesization); the order
 *   does not change the RESULT but dramatically changes the COST. Multiplying
 *   an (a x b) matrix by a (b x c) matrix costs a*b*c scalar multiplications.
 *
 * Idea (State / Recurrence / Base case):
 *   State:  dp[i][j] = minimum cost to multiply the sub-chain A_i..A_j.
 *   Split:  Any parenthesization makes some LAST multiplication that combines
 *           a left block (A_i..A_k) of size (p[i-1] x p[k]) with a right block
 *           (A_k+1..A_j) of size (p[k] x p[j]), for some k with i <= k < j.
 *   Recurrence:
 *           dp[i][j] = min over k in [i, j-1] of
 *                      dp[i][k] + dp[k+1][j] + p[i-1]*p[k]*p[j]
 *   Base:   dp[i][i] = 0  (a single matrix needs no multiplication).
 *
 *   Why correct: the optimal solution must make SOME last multiplication;
 *   whatever the optimal split k is, both sub-blocks must themselves be solved
 *   optimally (optimal substructure). Subproblems overlap because the same
 *   sub-chain A_i..A_j is reached through many different outer splits.
 *
 * Complexity:
 *   +-----------+-------------------+
 *   | Time      | O(n^3)            |
 *   | Space     | O(n^2)            |
 *   +-----------+-------------------+
 *   n = number of matrices. No simple 1-D space optimization exists: dp[i][j]
 *   depends on a whole row and column of smaller intervals.
 *
 * Key points:
 *   - Classic interval DP: iterate by INCREASING chain length so every
 *     smaller interval is finalized before a larger one uses it.
 *   - Bottom-up tabulation shown here; a top-down memoized version would store
 *     the same dp table and recurse on (i, j).
 *   - A parallel split table s[i][j] records the best k, enabling us to
 *     reconstruct the optimal parenthesization.
 */

#include <vector>
#include <string>
#include <climits>
#include <cstddef>
#include <cassert>
#include <iostream>

using std::size_t;

// Bottom-up tabulation. p has n+1 entries describing n matrices.
// Returns the minimum scalar multiplications and fills the split table.
long long matrixChainOrder(const std::vector<long long>& p,
                           std::vector<std::vector<size_t>>& split) {
    const size_t n = p.size() - 1; // number of matrices
    if (n == 0) return 0;          // empty chain: nothing to multiply

    // We index matrices 1..n. Using a 1-based dp of size (n+1)x(n+1) lets us
    // write p[i-1] safely because i is always >= 1 here (no unsigned underflow).
    std::vector<std::vector<long long>> dp(n + 1, std::vector<long long>(n + 1, 0));
    split.assign(n + 1, std::vector<size_t>(n + 1, 0));

    // len = number of matrices in the current sub-chain.
    for (size_t len = 2; len <= n; ++len) {
        for (size_t i = 1; i + len - 1 <= n; ++i) {
            const size_t j = i + len - 1;
            dp[i][j] = LLONG_MAX; // sentinel; always replaced (len >= 2 => k exists)
            for (size_t k = i; k < j; ++k) {
                const long long cost =
                    dp[i][k] + dp[k + 1][j] + p[i - 1] * p[k] * p[j];
                if (cost < dp[i][j]) {
                    dp[i][j] = cost;
                    split[i][j] = k; // remember where the optimal cut was
                }
            }
        }
    }
    return dp[1][n];
}

// Reconstruct the optimal parenthesization from the split table.
void buildParenthesization(const std::vector<std::vector<size_t>>& split,
                           size_t i, size_t j, std::string& out) {
    if (i == j) {
        out += "A" + std::to_string(i); // a single matrix
        return;
    }
    const size_t k = split[i][j];
    out += "(";
    buildParenthesization(split, i, k, out);
    buildParenthesization(split, k + 1, j, out);
    out += ")";
}

// Brute-force reference (exponential): try every split for the chain A_i..A_j.
long long mcmBrute(const std::vector<long long>& p, size_t i, size_t j) {
    if (i == j) return 0;
    long long best = LLONG_MAX;
    for (size_t k = i; k < j; ++k) {
        const long long cost =
            mcmBrute(p, i, k) + mcmBrute(p, k + 1, j) + p[i - 1] * p[k] * p[j];
        if (cost < best) best = cost;
    }
    return best;
}

int main() {
    // Classic textbook example: dims {40,20,30,10,30} -> 26000.
    {
        const std::vector<long long> p = {40, 20, 30, 10, 30};
        std::vector<std::vector<size_t>> split;
        const long long best = matrixChainOrder(p, split);
        assert(best == 26000);
        assert(best == mcmBrute(p, 1, p.size() - 1));

        std::string paren;
        buildParenthesization(split, 1, p.size() - 1, paren);
        std::cout << "Chain {40,20,30,10,30}: min mults = " << best
                  << ", order = " << paren << "\n";
    }
    // Single matrix (n = 1): no multiplication needed.
    {
        const std::vector<long long> p = {10, 20};
        std::vector<std::vector<size_t>> split;
        assert(matrixChainOrder(p, split) == 0);
    }
    // Two matrices (10x20)(20x30): exactly 10*20*30 = 6000.
    {
        const std::vector<long long> p = {10, 20, 30};
        std::vector<std::vector<size_t>> split;
        assert(matrixChainOrder(p, split) == 6000);
    }
    // Edge case: empty chain (no matrices) -> 0.
    {
        const std::vector<long long> p = {5}; // n = 0
        std::vector<std::vector<size_t>> split;
        assert(matrixChainOrder(p, split) == 0);
    }
    // Cross-check bottom-up against brute force on several small chains.
    {
        const std::vector<std::vector<long long>> chains = {
            {1, 2, 3, 4},
            {5, 4, 6, 2, 7},
            {2, 3, 1, 4, 5, 2}
        };
        for (const auto& p : chains) {
            std::vector<std::vector<size_t>> split;
            assert(matrixChainOrder(p, split) == mcmBrute(p, 1, p.size() - 1));
        }
    }
    std::cout << "All matrix-chain tests passed.\n";
    return 0;
}
