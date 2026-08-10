/*
 * ============================================================================
 * Tower of Hanoi
 * Category: Algorithm - Recursion (divide & conquer)
 *
 * Idea:
 *   Move a stack of n disks from the SOURCE peg to the TARGET peg using one
 *   AUXILIARY peg, never placing a larger disk on a smaller one. The recursive
 *   insight: to move n disks source -> target,
 *     1. move the top n-1 disks   source -> auxiliary   (recursive subproblem),
 *     2. move the single largest disk source -> target  (one legal move),
 *     3. move the n-1 disks        auxiliary -> target   (recursive subproblem).
 *   Steps 1 and 3 are the SAME problem on n-1 disks with the roles of the pegs
 *   permuted. BASE CASE: n == 0 disks require zero moves.
 *
 * Recurrence for the move count:
 *     T(0) = 0
 *     T(n) = 2 * T(n-1) + 1          (two recursive moves of n-1, plus one)
 *   Solving it: T(n) = 2^n - 1. This is provably optimal -- no sequence of
 *   legal moves can solve n disks in fewer than 2^n - 1 moves.
 *
 * Complexity:
 *   +-----------+---------------------------------------------------------+
 *   | Time      | O(2^n)  -- exactly 2^n - 1 moves are produced/printed.  |
 *   | Space     | O(n)    -- recursion depth (call stack), plus O(2^n) if |
 *   |           |            the moves are STORED as they are here.       |
 *   +-----------+---------------------------------------------------------+
 *   WHY exponential: T(n) = 2^n - 1 grows doubling with each added disk;
 *   there is no pruning to apply because every one of those moves is required.
 *
 * Key points / when to use:
 *   - The textbook example of a problem whose recursion tree size equals the
 *     answer itself: work is inherently exponential, not an inefficiency.
 *   - Classic model for "move a structure through a constrained intermediate".
 *   - The move sequence is unique and optimal for the standard 3-peg puzzle.
 * ============================================================================
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

// A single legal move: take the top disk off peg `from` and place it on `to`.
struct Move {
    char from;
    char to;
};

// Recursively record the moves that transfer `n` disks from `source` to
// `target` using `aux` as the spare peg. Moves are appended to `out` in the
// exact order they must be performed.
void hanoi(unsigned n, char source, char target, char aux,
           std::vector<Move>& out) {
    if (n == 0) return;                       // BASE CASE: no disks -> no moves

    hanoi(n - 1, source, aux, target, out);   // 1) n-1 disks: source -> aux
    out.push_back(Move{source, target});      // 2) move largest disk directly
    hanoi(n - 1, aux, target, source, out);   // 3) n-1 disks: aux -> target
}

// Validate a move sequence by actually simulating three pegs modeled as stacks.
// Disks are integers 1..n (1 = smallest). A move is legal only if the source
// peg is non-empty and the moved disk is smaller than the current top of the
// destination peg. Returns true iff every move is legal AND the whole tower
// ends up stacked correctly on `target`.
bool simulateValid(unsigned n, char source, char target, char aux,
                   const std::vector<Move>& moves) {
    std::vector<std::vector<int>> peg(128);   // indexed by the peg character
    // Start: disks n (bottom) .. 1 (top) all on the source peg. Use a signed
    // counter so the "d >= 1" bound cannot underflow.
    for (int d = static_cast<int>(n); d >= 1; --d) {
        peg[static_cast<unsigned char>(source)].push_back(d);
    }

    for (const Move& m : moves) {
        auto& src = peg[static_cast<unsigned char>(m.from)];
        auto& dst = peg[static_cast<unsigned char>(m.to)];
        if (src.empty()) return false;                 // nothing to move
        int disk = src.back();
        if (!dst.empty() && disk > dst.back()) {
            return false;                              // larger onto smaller: illegal
        }
        src.pop_back();
        dst.push_back(disk);
    }

    // The auxiliary peg must be empty and the target must hold the full,
    // correctly ordered tower n (bottom) .. 1 (top).
    if (!peg[static_cast<unsigned char>(aux)].empty()) return false;
    const auto& t = peg[static_cast<unsigned char>(target)];
    if (t.size() != n) return false;
    for (std::size_t i = 0; i < t.size(); ++i) {
        // Bottom (i = 0) should be disk n, decreasing to 1 at the top.
        if (t[i] != static_cast<int>(n - i)) return false;
    }
    return true;
}

int main() {
    // ---- move count must equal 2^n - 1 for every n ------------------------
    for (unsigned n = 0; n <= 15; ++n) {
        std::vector<Move> moves;
        hanoi(n, 'A', 'C', 'B', moves);
        const std::size_t expected = (static_cast<std::size_t>(1) << n) - 1; // 2^n - 1
        assert(moves.size() == expected);
        // Every recorded sequence must also be LEGAL and actually solve it.
        assert(simulateValid(n, 'A', 'C', 'B', moves));
    }

    // ---- spot-check a couple of exact counts -------------------------------
    {
        std::vector<Move> m3;
        hanoi(3, 'A', 'C', 'B', m3);
        assert(m3.size() == 7);                 // 2^3 - 1
        // The unique optimal 3-disk solution, verified move-by-move.
        const std::vector<Move> known3 = {
            {'A','C'}, {'A','B'}, {'C','B'}, {'A','C'},
            {'B','A'}, {'B','C'}, {'A','C'}
        };
        assert(m3.size() == known3.size());
        for (std::size_t i = 0; i < m3.size(); ++i) {
            assert(m3[i].from == known3[i].from && m3[i].to == known3[i].to);
        }
    }

    // ---- short demo: print the 3-disk solution ----------------------------
    std::vector<Move> demo;
    hanoi(3, 'A', 'C', 'B', demo);
    std::cout << "Tower of Hanoi demo (n = 3, A -> C using B)\n";
    std::cout << "  total moves = " << demo.size() << "  (2^3 - 1 = 7)\n";
    std::size_t step = 1;
    for (const Move& m : demo) {
        std::cout << "  " << step++ << ": move top disk " << m.from
                  << " -> " << m.to << "\n";
    }
    std::cout << "All assertions passed.\n";
    return 0;
}
