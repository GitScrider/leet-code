/*
 * ============================================================================
 * N-Queens
 * Category: Algorithm - Recursion / Backtracking (constraint satisfaction)
 *
 * Idea:
 *   Place N queens on an N x N board so that no two attack each other (no two
 *   share a row, a column, or a diagonal). We fix EXACTLY ONE queen per row and
 *   recurse row by row -- this collapses the "same row" constraint for free.
 *   State space: at each row we try every column, so the naive tree has N^N
 *   leaves; pruning discards branches the instant a placement is illegal.
 *
 *   Backtracking template applied to each row:
 *       for each column c in [0, N):
 *           if placing (row, c) is safe:
 *               CHOOSE   : mark column c and both diagonals as occupied
 *               EXPLORE  : recurse into the next row
 *               UNCHOOSE : unmark them (backtrack) before trying the next c
 *
 *   O(1) safety test via three boolean occupancy arrays instead of scanning:
 *       - cols[c]        : is column c already taken?
 *       - diagMain[r-c]  : is the "\" diagonal taken?  (r - c is constant on it)
 *       - diagAnti[r+c]  : is the "/" diagonal taken?  (r + c is constant on it)
 *     r - c ranges over [-(N-1), N-1]; we shift by +(N-1) to index 0..2N-2.
 *
 * Complexity:
 *   +----------+--------------------------------+
 *   | Aspect   | Cost                           |
 *   +----------+--------------------------------+
 *   | Time     | O(N!) upper bound (worst case) |
 *   | Space    | O(N) recursion + occupancy     |
 *   +----------+--------------------------------+
 *   WHY exponential: row 0 has N legal columns, row 1 has at most N-1 that avoid
 *   column/diagonal clashes, then N-2, ... so the explored tree is bounded by
 *   ~N! rather than N^N. PRUNING (the O(1) safety check) is what removes the
 *   illegal sub-trees early and turns N^N into the far smaller N! envelope.
 *   Space is O(N): the recursion depth is N and each occupancy array is O(N).
 *
 * Key points / when to use:
 *   - Canonical example of backtracking with constant-time constraint checks.
 *   - One-queen-per-row modeling eliminates an entire class of conflicts.
 *   - Diagonal indexing by (r - c) and (r + c) is the reusable trick.
 *   - Use for placement/constraint puzzles where partial solutions are cheap to
 *     validate and invalid prefixes can be abandoned immediately.
 * ============================================================================
 */

#include <vector>
#include <string>
#include <cassert>
#include <iostream>
#include <cstddef>

// Solver holding the mutable board state and the occupancy sets. We keep the
// arrays as members so recursion mutates shared state in place (no copying).
class NQueens {
public:
    explicit NQueens(std::size_t n)
        : n_(n),
          queenCol_(n, 0),                 // queenCol_[r] = column of the queen in row r
          cols_(n, false),                 // column occupancy
          diagMain_(2 * n - 1, false),     // "\" diagonals, indexed by (r - c + n - 1)
          diagAnti_(2 * n - 1, false) {}   // "/" diagonals, indexed by (r + c)

    // Count every distinct valid placement.
    std::size_t countSolutions() {
        solutions_ = 0;
        collectFirst_ = false;
        place(0);
        return solutions_;
    }

    // Return ONE valid board (each string a row: '.' empty, 'Q' queen), or an
    // empty vector if none exists. Uses the same search but stops at the first.
    std::vector<std::string> firstSolution() {
        solutions_ = 0;
        collectFirst_ = true;
        found_.clear();
        place(0);
        return found_;
    }

private:
    // Can a queen sit at (row, col) given what is already placed?
    // O(1): consult the column and the two diagonal occupancy arrays.
    bool isSafe(std::size_t row, std::size_t col) const {
        return !cols_[col]
            && !diagMain_[row - col + (n_ - 1)] // r - c shifted into [0, 2n-2]
            && !diagAnti_[row + col];           // r + c already in [0, 2n-2]
    }

    // Try to fill rows [row, n_). Returns true once a solution is recorded AND
    // we only want the first (so the search can unwind early).
    bool place(std::size_t row) {
        if (row == n_) {                 // BASE CASE: all rows filled -> a solution
            ++solutions_;
            if (collectFirst_) {
                found_ = render();
                return true;             // signal "stop searching"
            }
            return false;                // keep counting the rest
        }

        for (std::size_t col = 0; col < n_; ++col) {
            if (!isSafe(row, col)) continue;     // PRUNE illegal squares

            // CHOOSE: put the queen down and claim its lines of attack.
            queenCol_[row] = col;
            cols_[col] = true;
            diagMain_[row - col + (n_ - 1)] = true;
            diagAnti_[row + col] = true;

            // EXPLORE: solve the remaining rows.
            bool stop = place(row + 1);

            // UNCHOOSE: lift the queen and release its lines (backtrack).
            cols_[col] = false;
            diagMain_[row - col + (n_ - 1)] = false;
            diagAnti_[row + col] = false;

            if (stop) return true;               // propagate early exit
        }
        return false;
    }

    // Turn the queenCol_ assignment into a printable board.
    std::vector<std::string> render() const {
        std::vector<std::string> board(n_, std::string(n_, '.'));
        for (std::size_t r = 0; r < n_; ++r) board[r][queenCol_[r]] = 'Q';
        return board;
    }

    std::size_t n_;
    std::vector<std::size_t> queenCol_;
    std::vector<bool> cols_, diagMain_, diagAnti_;
    std::size_t solutions_ = 0;
    bool collectFirst_ = false;
    std::vector<std::string> found_;
};

// Convenience wrapper for the tests.
static std::size_t countNQueens(std::size_t n) {
    return NQueens(n).countSolutions();
}

int main() {
    // --- Known solution counts (OEIS A000170). These pin down correctness. ---
    assert(countNQueens(1) == 1);
    assert(countNQueens(2) == 0);   // no placement exists for n = 2
    assert(countNQueens(3) == 0);   // ... nor for n = 3
    assert(countNQueens(4) == 2);
    assert(countNQueens(5) == 10);
    assert(countNQueens(6) == 4);
    assert(countNQueens(7) == 40);
    assert(countNQueens(8) == 92);

    // --- The returned board must itself be a legal, complete placement. ---
    NQueens solver(8);
    std::vector<std::string> board = solver.firstSolution();
    assert(board.size() == 8);
    std::size_t queens = 0;
    for (std::size_t r = 0; r < board.size(); ++r)
        for (std::size_t c = 0; c < board[r].size(); ++c)
            if (board[r][c] == 'Q') {
                ++queens;
                // No other queen may share this row, column, or diagonal.
                for (std::size_t r2 = 0; r2 < board.size(); ++r2)
                    for (std::size_t c2 = 0; c2 < board[r2].size(); ++c2) {
                        if (r == r2 && c == c2) continue;
                        if (board[r2][c2] != 'Q') continue;
                        bool sameRow  = (r == r2);
                        bool sameCol  = (c == c2);
                        std::ptrdiff_t dr = static_cast<std::ptrdiff_t>(r) - static_cast<std::ptrdiff_t>(r2);
                        std::ptrdiff_t dc = static_cast<std::ptrdiff_t>(c) - static_cast<std::ptrdiff_t>(c2);
                        bool sameDiag = (dr == dc) || (dr == -dc);
                        assert(!sameRow && !sameCol && !sameDiag);
                    }
            }
    assert(queens == 8);

    // --- Demo: print one 8-queens board. ---
    std::cout << "One solution to the 8-Queens problem:\n";
    for (const std::string& row : board) std::cout << "  " << row << '\n';
    std::cout << "Total 8-Queens solutions: " << countNQueens(8) << '\n';
    std::cout << "All tests passed.\n";
    return 0;
}
