/*
 * Grid Paths
 * Category: Algorithm - Dynamic Programming (Grid / Path DP)
 *
 * Two classic grid DP problems are solved here:
 *
 *   (1) Unique Paths (plus an obstacles variant)
 *       Count the distinct paths from the top-left to the bottom-right corner
 *       of an m x n grid, moving only RIGHT or DOWN.
 *       State:  dp[i][j] = number of paths reaching cell (i, j).
 *       Recurrence: dp[i][j] = dp[i-1][j] + dp[i][j-1]
 *                   (a cell is entered either from above or from the left).
 *       Base:   dp[0][0] = 1.
 *       Obstacles: a blocked cell has dp[i][j] = 0 (no path passes through it).
 *
 *   (2) Minimum Path Sum
 *       Each cell holds a non-negative cost; find the minimum total cost of a
 *       top-left -> bottom-right path moving only right or down.
 *       State:  dp[i][j] = minimum cost to reach (i, j).
 *       Recurrence: dp[i][j] = grid[i][j] + min(dp[i-1][j], dp[i][j-1]).
 *       Base:   dp[0][0] = grid[0][0].
 *
 *   Why correct: the LAST step into any cell comes from exactly one of two
 *   neighbours (up or left). For counting, the two ways are disjoint so counts
 *   ADD; for cost, we take the cheaper predecessor -- optimal substructure with
 *   overlapping subproblems (each cell is a subproblem reused by its neighbours).
 *
 * Complexity (both, for an m x n grid):
 *   +-----------+-------------------+
 *   | Time      | O(m * n)          |
 *   | Space     | O(m * n)          |
 *   +-----------+-------------------+
 *   Space optimization: each row depends only on the previous row, so a single
 *   rolling 1-D array of length n suffices -> O(n) space. A rolling version of
 *   uniquePaths is implemented below (uniquePathsRolling).
 *
 * Key points:
 *   - Pure bottom-up tabulation; the first row and first column are the
 *     boundary base cases (only one straight-line way to reach them).
 *   - Obstacles are handled by forcing the count to 0 at blocked cells.
 *   - Careful std::size_t indexing: we read dp[i-1] / dp[j-1] only when
 *     i > 0 / j > 0 to avoid unsigned underflow.
 */

#include <vector>
#include <algorithm>
#include <climits>
#include <cstddef>
#include <cassert>
#include <iostream>

using std::size_t;

// (1) Count unique paths in an m x n grid (right/down moves only).
long long uniquePaths(size_t m, size_t n) {
    if (m == 0 || n == 0) return 0;
    std::vector<std::vector<long long>> dp(m, std::vector<long long>(n, 0));
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (i == 0 && j == 0) {
                dp[i][j] = 1; // starting cell: one (empty) path
            } else {
                const long long fromUp   = (i > 0) ? dp[i - 1][j] : 0;
                const long long fromLeft = (j > 0) ? dp[i][j - 1] : 0;
                dp[i][j] = fromUp + fromLeft;
            }
        }
    }
    return dp[m - 1][n - 1];
}

// (1b) Same count using an O(n) rolling array (space-optimized).
long long uniquePathsRolling(size_t m, size_t n) {
    if (m == 0 || n == 0) return 0;
    // First row: exactly one path to each cell (keep moving right).
    std::vector<long long> row(n, 1);
    for (size_t i = 1; i < m; ++i) {
        // row[0] stays 1: only one way straight down the first column.
        for (size_t j = 1; j < n; ++j) {
            // row[j] (old) = paths from above; row[j-1] (new) = paths from left.
            row[j] += row[j - 1];
        }
    }
    return row[n - 1];
}

// (1c) Unique paths with obstacles (1 = blocked, 0 = free).
long long uniquePathsWithObstacles(const std::vector<std::vector<int>>& grid) {
    if (grid.empty() || grid[0].empty()) return 0;
    const size_t m = grid.size();
    const size_t n = grid[0].size();
    std::vector<std::vector<long long>> dp(m, std::vector<long long>(n, 0));
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (grid[i][j] == 1) {
                dp[i][j] = 0; // blocked: no path may stand here
            } else if (i == 0 && j == 0) {
                dp[i][j] = 1;
            } else {
                const long long fromUp   = (i > 0) ? dp[i - 1][j] : 0;
                const long long fromLeft = (j > 0) ? dp[i][j - 1] : 0;
                dp[i][j] = fromUp + fromLeft;
            }
        }
    }
    return dp[m - 1][n - 1];
}

// (2) Minimum path sum (right/down moves), non-negative costs.
long long minPathSum(const std::vector<std::vector<int>>& grid) {
    if (grid.empty() || grid[0].empty()) return 0;
    const size_t m = grid.size();
    const size_t n = grid[0].size();
    std::vector<std::vector<long long>> dp(m, std::vector<long long>(n, 0));
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < n; ++j) {
            const long long cell = grid[i][j];
            if (i == 0 && j == 0) {
                dp[i][j] = cell;                         // start
            } else if (i == 0) {
                dp[i][j] = dp[i][j - 1] + cell;          // top row: only from left
            } else if (j == 0) {
                dp[i][j] = dp[i - 1][j] + cell;          // left col: only from up
            } else {
                dp[i][j] = std::min(dp[i - 1][j], dp[i][j - 1]) + cell;
            }
        }
    }
    return dp[m - 1][n - 1];
}

// ---- Brute-force references (exponential) for small-grid cross-checks ----

long long uniquePathsBrute(size_t m, size_t n) {
    if (m == 0 || n == 0) return 0;
    if (m == 1 || n == 1) return 1; // a single row/column: one straight path
    return uniquePathsBrute(m - 1, n) + uniquePathsBrute(m, n - 1);
}

long long minPathSumBrute(const std::vector<std::vector<int>>& grid,
                          size_t i, size_t j) {
    const size_t m = grid.size();
    const size_t n = grid[0].size();
    if (i >= m || j >= n) return LLONG_MAX;              // fell off the grid
    if (i == m - 1 && j == n - 1) return grid[i][j];     // reached the target
    const long long down  = minPathSumBrute(grid, i + 1, j);
    const long long right = minPathSumBrute(grid, i, j + 1);
    // At least one of down/right is finite for any non-target cell, so the
    // std::min picks a valid path and no overflow occurs on the addition.
    return grid[i][j] + std::min(down, right);
}

int main() {
    // (1) uniquePaths(3, 7) = 28, matched by the rolling and brute versions.
    {
        assert(uniquePaths(3, 7) == 28);
        assert(uniquePathsRolling(3, 7) == 28);
        assert(uniquePathsBrute(3, 7) == 28);
        std::cout << "uniquePaths(3,7) = " << uniquePaths(3, 7) << "\n";
    }
    // Cross-check tabulation vs rolling vs brute on several small grids.
    {
        for (size_t m = 1; m <= 5; ++m) {
            for (size_t n = 1; n <= 5; ++n) {
                const long long expected = uniquePathsBrute(m, n);
                assert(uniquePaths(m, n) == expected);
                assert(uniquePathsRolling(m, n) == expected);
            }
        }
    }
    // Edge cases for counting.
    {
        assert(uniquePaths(1, 1) == 1); // already at the target
        assert(uniquePaths(0, 5) == 0); // degenerate empty grid
    }
    // (1c) Obstacle case: the classic 3x3 with one middle block -> 2 paths.
    {
        const std::vector<std::vector<int>> grid = {
            {0, 0, 0},
            {0, 1, 0},
            {0, 0, 0}
        };
        assert(uniquePathsWithObstacles(grid) == 2);
        std::cout << "uniquePathsWithObstacles(3x3, center blocked) = "
                  << uniquePathsWithObstacles(grid) << "\n";
    }
    // No-solution obstacle case: start itself blocked -> 0 paths.
    {
        const std::vector<std::vector<int>> blocked = {{1, 0}, {0, 0}};
        assert(uniquePathsWithObstacles(blocked) == 0);
    }
    // (2) Minimum path sum: classic grid -> 7 (1->3->1->1->1).
    {
        const std::vector<std::vector<int>> grid = {
            {1, 3, 1},
            {1, 5, 1},
            {4, 2, 1}
        };
        assert(minPathSum(grid) == 7);
        assert(minPathSum(grid) == minPathSumBrute(grid, 0, 0));
        std::cout << "minPathSum(classic 3x3) = " << minPathSum(grid) << "\n";
    }
    // Min path sum edge cases: single cell and a single row.
    {
        const std::vector<std::vector<int>> one = {{42}};
        assert(minPathSum(one) == 42);
        const std::vector<std::vector<int>> rowGrid = {{1, 2, 3, 4}};
        assert(minPathSum(rowGrid) == 10);
    }
    std::cout << "All grid-path tests passed.\n";
    return 0;
}
