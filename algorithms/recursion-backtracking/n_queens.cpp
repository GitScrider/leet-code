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
 * Complexity derivation (state-space tree -> summation over levels):
 *   Model the search as a tree whose level d is board row d (one place() call
 *   per node). At the root (level 0) there is 1 node. A queen already placed in
 *   an earlier row rules out at least one distinct column, so a node at row d
 *   has at most (N - d) safe columns, i.e. branching factor <= N - d. The number
 *   of place() calls (nodes) per level is therefore bounded by:
 *
 *       level d     #nodes (<=)          subproblem     work per node
 *       --------    -----------------    -----------    ----------------
 *       d = 0       1                    rows [0, N)    O(N) column scan
 *       d = 1       N                    rows [1, N)    O(N) column scan
 *       d = 2       N*(N-1)              rows [2, N)    O(N) column scan
 *       ...         ...                  ...            ...
 *       d = k       N!/(N-k)!            rows [k, N)    O(N) column scan
 *       d = N       N!/0! = N! (leaves)  -              O(1)
 *
 *   Summing the node count over all levels (substitute j = N - d):
 *
 *       T(N) = SUM_{d=0}^{N} N!/(N-d)!
 *            = N! * SUM_{j=0}^{N} 1/j!
 *            < N! * SUM_{j=0}^{inf} 1/j!
 *            = N! * e                     (e = 2.718..., the exp series)
 *            = O(N!)
 *
 *   So the pruned tree holds O(N!) nodes. Counting the per-node O(N) column scan
 *   gives O(N * N!) isSafe tests; the file quotes the node/placement count O(N!),
 *   the standard statement. Without pruning the tree would have N^N leaves.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f(n) = O(g)      iff  EXISTS c2, n0 :       f(n) <= c2*g(n)  for n >= n0
 *     f(n) = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f(n) = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   For a fixed N the search is deterministic: it visits an exact number P(N) of
 *   nodes, so the running time is Theta(P(N)). We can only bracket P(N):
 *     upper  O:     P(N) <= N! * e            => time = O(N!)   (worst case)
 *     lower  Omega: the solver descends at least one full depth-N path before it
 *                   reaches a leaf, so P(N) >= N  => time = Omega(N)
 *   No simple closed-form Theta is claimed: heavy diagonal/column pruning makes
 *   P(N) grow far slower than N! yet strictly above the linear floor. The
 *   comparison-sort Omega(N log N) bound is irrelevant -- this is a search, not
 *   a sort.
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
