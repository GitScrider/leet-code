/*
 * ============================================================================
 * Word Search
 * Category: Algorithm - Recursion / Backtracking (grid DFS with state undo)
 *
 * Idea:
 *   Given a grid of characters and a target word, decide whether the word can
 *   be spelled by walking 4-directionally between adjacent cells, using each
 *   cell at most once. We try to start the match at every cell; from a matching
 *   cell we recurse into the four neighbours seeking the next letter.
 *
 *   Backtracking template at cell (r, c) matching word[i]:
 *       if grid[r][c] != word[i]: fail immediately (mismatch prune)
 *       if i is the last index: the whole word matched -> succeed
 *       CHOOSE   : mark (r, c) used (temporarily blank it out)
 *       EXPLORE  : recurse into each neighbour to match word[i+1]
 *       UNCHOOSE : restore grid[r][c] (backtrack) whether or not we succeeded,
 *                  so the cell is free for a different starting point / route
 *
 * Complexity:
 *   +----------+-------------------------------------+
 *   | Aspect   | Cost                                |
 *   +----------+-------------------------------------+
 *   | Time     | O(R * C * 4^L)  (L = word length)   |
 *   | Space    | O(L) recursion depth                |
 *   +----------+-------------------------------------+
 *   WHY exponential in L: each matched letter branches into up to 4 neighbours,
 *   so one starting cell explores up to 4^L paths, and we may start from all
 *   R*C cells. PRUNING keeps it tractable: a branch dies the instant a letter
 *   mismatches or a cell is reused, so most of the 4^L tree is never visited.
 *   Extra space is only the O(L) call stack -- we mutate the grid in place with
 *   a sentinel rather than allocating a separate visited matrix.
 *
 * Key points / when to use:
 *   - Backtracking that undoes state by RESTORING the grid cell, not a side set.
 *   - The "requires backtracking" case: a promising prefix hits a dead end and
 *     the search must unwind and retry from another neighbour or start cell.
 *   - Marking + restoring in place is O(1) per step and avoids O(R*C) copies.
 *   - Use for path-in-grid matching where cells cannot repeat within one match.
 * ============================================================================
 */

#include <vector>
#include <string>
#include <cassert>
#include <iostream>
#include <cstddef>

using Board = std::vector<std::string>;

// The four cardinal neighbours: down, up, right, left.
static const int DR[4] = { 1, -1, 0, 0 };
static const int DC[4] = { 0, 0, 1, -1 };

// Try to match word[idx..] starting AT cell (r, c). The board is mutated in
// place: a matched cell is temporarily set to a sentinel so it cannot be
// reused, then restored on the way out (backtracking).
static bool match(Board& board, const std::string& word,
                  std::size_t idx, std::size_t r, std::size_t c) {
    // Mismatch prune: this cell does not carry the letter we need.
    if (board[r][c] != word[idx]) return false;

    // BASE CASE: the last letter just matched -> the whole word is found.
    if (idx + 1 == word.size()) return true;

    const std::size_t rows = board.size();
    const std::size_t cols = board[0].size();

    // CHOOSE: consume (r, c). '#' cannot appear in a real word, so it doubles
    // as a "visited" marker that the mismatch prune above will reject.
    const char saved = board[r][c];
    board[r][c] = '#';

    // EXPLORE: attempt the next letter in each in-bounds neighbour.
    bool found = false;
    for (int k = 0; k < 4 && !found; ++k) {
        long nr = static_cast<long>(r) + DR[k];
        long nc = static_cast<long>(c) + DC[k];
        if (nr < 0 || nc < 0) continue;
        if (nr >= static_cast<long>(rows)) continue;
        if (nc >= static_cast<long>(cols)) continue;
        found = match(board, word, idx + 1,
                      static_cast<std::size_t>(nr), static_cast<std::size_t>(nc));
    }

    // UNCHOOSE: restore the cell so other paths (and other start cells) may use
    // it. This runs on both success and failure to leave the board pristine.
    board[r][c] = saved;
    return found;
}

// Does `word` exist somewhere in the grid? Empty word is vacuously present.
// Takes the board by value so callers keep their original grid untouched.
static bool exists(Board board, const std::string& word) {
    if (word.empty()) return true;
    if (board.empty() || board[0].empty()) return false;

    // Try every cell as a potential starting point for the first letter.
    for (std::size_t r = 0; r < board.size(); ++r)
        for (std::size_t c = 0; c < board[r].size(); ++c)
            if (match(board, word, 0, r, c)) return true;
    return false;
}

int main() {
    Board board = {
        "ABCE",
        "SFCS",
        "ADEE",
    };

    // --- Words that ARE present. ---
    assert(exists(board, "ABCCED")); // A(0,0) B(0,1) C(0,2) C(1,2) E(2,2) D(2,1)
    assert(exists(board, "ABF"));    // A(0,0) B(0,1) F(1,1)
    assert(exists(board, "A"));      // single-letter word

    // --- A TRUE case that REQUIRES backtracking. Matching "SEE" must start at
    //     S(1,3); its first explored neighbour E(0,3) is a DEAD END (no second
    //     E adjacent), so the search unwinds and retries via E(2,3)->E(2,2). ---
    assert(exists(board, "SEE"));    // S(1,3) [E(0,3) dead end] E(2,3) E(2,2)
    assert(exists(board, "SEED"));   // S(1,3) E(2,3) E(2,2) D(2,1) -- winding

    // --- The classic FALSE case. "ABCB" spells the A-B-C prefix but the final
    //     B is unreachable without reusing the first B, so the search explores
    //     the whole prefix, hits a dead end, and unwinds to report failure. ---
    assert(!exists(board, "ABCB"));

    // --- A FALSE case that also requires deep backtracking: "SEEB" walks the
    //     SEE path (past the E(0,3) dead end) but then finds no B adjacent to
    //     E(2,2), forcing the search to unwind everything and fail. ---
    assert(!exists(board, "SEEB"));

    // --- Other absent words. ---
    assert(!exists(board, "ABCESEEEFS")); // needs a 4th 'E'; board has only 3
    assert(!exists(board, "XYZ"));        // letters not even on the board
    assert(!exists(board, "CADE"));       // letters exist but no C is adjacent to any A

    // --- The board must be left UNMODIFIED after searches (in-place restore). ---
    Board pristine = {"ABCE", "SFCS", "ADEE"};
    exists(board, "ABCCED");
    assert(board == pristine);

    // --- A second grid exercising a longer winding match. ---
    Board snake = {
        "GAT",
        "XXO",
        "DOG",
    };
    // "GATOGOD": G(0,0) A(0,1) T(0,2) O(1,2) G(2,2) O(2,1) D(2,0) -- a spiral.
    assert(exists(snake, "GATOGOD"));
    assert(!exists(snake, "GATG"));   // no G adjacent to T(0,2)

    std::cout << "Word Search demo on board:\n";
    for (const std::string& row : board) std::cout << "  " << row << '\n';
    std::cout << "  exists(\"ABCCED\") = " << std::boolalpha << exists(board, "ABCCED") << '\n';
    std::cout << "  exists(\"ABCB\")   = " << exists(board, "ABCB") << '\n';
    std::cout << "All tests passed.\n";
    return 0;
}
