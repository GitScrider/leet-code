/*
 * ============================================================================
 * Sudoku Solver
 * Category: Algorithm - Recursion / Backtracking (constraint satisfaction)
 *
 * Idea:
 *   Fill a 9x9 grid so every row, every column, and every 3x3 box contains the
 *   digits 1..9 exactly once. We scan for the FIRST empty cell, try each digit
 *   1..9 that does not violate a constraint, recurse, and undo on failure.
 *
 *   Backtracking template on the first empty cell (r, c):
 *       for d in 1..9:
 *           if d is valid at (r, c):
 *               CHOOSE   : write d into the cell
 *               EXPLORE  : recurse to fill the rest of the grid
 *               UNCHOOSE : if that failed, erase d (backtrack) and try d+1
 *       if no digit worked -> return false (dead end, unwind to the caller)
 *
 *   Validity of digit d at (r, c) -- the three constraint checks:
 *       - row r must not already contain d,
 *       - column c must not already contain d,
 *       - the 3x3 box (r/3, c/3) must not already contain d.
 *
 * Complexity:
 *   +----------+-----------------------------------+
 *   | Aspect   | Cost                              |
 *   +----------+-----------------------------------+
 *   | Time     | O(9^m), m = number of blanks      |
 *   | Space    | O(m) recursion depth (grid O(1))  |
 *   +----------+-----------------------------------+
 *   WHY exponential: each of the m empty cells branches into up to 9 digits, so
 *   the search tree is bounded by 9^m. In practice PRUNING via the row/col/box
 *   checks rejects the vast majority of digits immediately, so a well-formed
 *   puzzle is solved almost instantly despite the frightening upper bound.
 *   The 9x9 grid is fixed size, so the only growing memory is the O(m) stack.
 *
 * Key points / when to use:
 *   - Textbook exact-cover / constraint-satisfaction backtracking.
 *   - Constant-time validity checks keep each node of the search cheap.
 *   - Always recursing on the FIRST blank keeps the code simple; choosing the
 *     MOST-CONSTRAINED blank (fewest candidates) is the classic speed-up.
 *   - Use when a solution is a total assignment subject to "all-different" style
 *     constraints that can be checked incrementally.
 * ============================================================================
 */

#include <array>
#include <vector>
#include <cassert>
#include <iostream>
#include <cstddef>

// A grid is 9 rows of 9 cells; 0 marks an empty cell, 1..9 a filled one.
using Grid = std::array<std::array<int, 9>, 9>;

// Is it legal to write `d` into (row, col)? Checks the three Sudoku regions.
static bool isValid(const Grid& g, std::size_t row, std::size_t col, int d) {
    // Row and column: neither may already hold the digit d.
    for (std::size_t k = 0; k < 9; ++k) {
        if (g[row][k] == d) return false;   // clashes within the row
        if (g[k][col] == d) return false;   // clashes within the column
    }
    // 3x3 box: find its top-left corner, then scan the 9 cells.
    std::size_t boxRow = (row / 3) * 3;
    std::size_t boxCol = (col / 3) * 3;
    for (std::size_t r = boxRow; r < boxRow + 3; ++r)
        for (std::size_t c = boxCol; c < boxCol + 3; ++c)
            if (g[r][c] == d) return false; // clashes within the 3x3 box
    return true;
}

// Solve in place. Returns true if the grid was completed successfully.
static bool solve(Grid& g) {
    // Find the first empty cell. If there is none, the grid is fully solved.
    for (std::size_t row = 0; row < 9; ++row) {
        for (std::size_t col = 0; col < 9; ++col) {
            if (g[row][col] != 0) continue;         // skip already-filled cells

            for (int d = 1; d <= 9; ++d) {
                if (!isValid(g, row, col, d)) continue; // PRUNE invalid digits

                g[row][col] = d;        // CHOOSE:   tentatively write d
                if (solve(g)) return true; // EXPLORE: solved the remainder?
                g[row][col] = 0;        // UNCHOOSE: erase d and try the next
            }
            return false;   // no digit fits here -> dead end, force a backtrack
        }
    }
    return true;            // BASE CASE: no empty cell remained -> solved
}

// Verify a completed grid: every row, column, and 3x3 box is a permutation of
// 1..9. Returns true only if all 27 regions are valid.
static bool isSolved(const Grid& g) {
    auto isPermutation = [](const std::array<int, 9>& nine) {
        std::array<bool, 10> seen{}; // index 0 unused; 1..9 tracked
        for (int v : nine) {
            if (v < 1 || v > 9 || seen[static_cast<std::size_t>(v)])
                return false;        // out of range or duplicated
            seen[static_cast<std::size_t>(v)] = true;
        }
        return true;
    };

    for (std::size_t i = 0; i < 9; ++i) {
        std::array<int, 9> rowVals{}, colVals{};
        for (std::size_t j = 0; j < 9; ++j) {
            rowVals[j] = g[i][j];
            colVals[j] = g[j][i];
        }
        if (!isPermutation(rowVals)) return false;
        if (!isPermutation(colVals)) return false;
    }
    for (std::size_t br = 0; br < 9; br += 3) {
        for (std::size_t bc = 0; bc < 9; bc += 3) {
            std::array<int, 9> boxVals{};
            std::size_t idx = 0;
            for (std::size_t r = br; r < br + 3; ++r)
                for (std::size_t c = bc; c < bc + 3; ++c)
                    boxVals[idx++] = g[r][c];
            if (!isPermutation(boxVals)) return false;
        }
    }
    return true;
}

static void print(const Grid& g) {
    for (std::size_t r = 0; r < 9; ++r) {
        if (r % 3 == 0) std::cout << "  +-------+-------+-------+\n";
        std::cout << "  ";
        for (std::size_t c = 0; c < 9; ++c) {
            if (c % 3 == 0) std::cout << "| ";
            std::cout << g[r][c] << ' ';
        }
        std::cout << "|\n";
    }
    std::cout << "  +-------+-------+-------+\n";
}

int main() {
    // A known, uniquely-solvable puzzle (0 = blank).
    Grid puzzle = {{
        {{5, 3, 0, 0, 7, 0, 0, 0, 0}},
        {{6, 0, 0, 1, 9, 5, 0, 0, 0}},
        {{0, 9, 8, 0, 0, 0, 0, 6, 0}},
        {{8, 0, 0, 0, 6, 0, 0, 0, 3}},
        {{4, 0, 0, 8, 0, 3, 0, 0, 1}},
        {{7, 0, 0, 0, 2, 0, 0, 0, 6}},
        {{0, 6, 0, 0, 0, 0, 2, 8, 0}},
        {{0, 0, 0, 4, 1, 9, 0, 0, 5}},
        {{0, 0, 0, 0, 8, 0, 0, 7, 9}},
    }};

    // The given clues must survive unchanged after solving; remember them.
    Grid given = puzzle;

    bool ok = solve(puzzle);
    assert(ok);                     // the puzzle is solvable
    assert(isSolved(puzzle));       // every row/col/box is a permutation of 1..9

    // The solver must not overwrite any of the original clues.
    for (std::size_t r = 0; r < 9; ++r)
        for (std::size_t c = 0; c < 9; ++c)
            if (given[r][c] != 0)
                assert(puzzle[r][c] == given[r][c]);

    // Spot-check the well-known solution's first row.
    std::array<int, 9> expectedFirstRow = {5, 3, 4, 6, 7, 8, 9, 1, 2};
    for (std::size_t c = 0; c < 9; ++c)
        assert(puzzle[0][c] == expectedFirstRow[c]);

    // An unsolvable grid must be detected. We construct one where the single
    // empty cell (0,8) can hold no digit: its row already contains 1..8 and its
    // column already contains 9, so all nine candidates are blocked.
    Grid broken{};                  // all zeros
    for (int d = 1; d <= 8; ++d)
        broken[0][static_cast<std::size_t>(d - 1)] = d; // row 0 = 1..8, then 0
    broken[1][8] = 9;               // put a 9 in column 8
    // Now (0,8) is blank but 1..8 are taken by row 0 and 9 by column 8.
    assert(!solve(broken));

    std::cout << "Solved Sudoku:\n";
    print(puzzle);
    std::cout << "All tests passed.\n";
    return 0;
}
