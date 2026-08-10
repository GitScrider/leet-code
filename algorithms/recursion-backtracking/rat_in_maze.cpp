/*
 * ============================================================================
 * Rat in a Maze
 * Category: Algorithm - Recursion / Backtracking (path finding on a grid)
 *
 * Idea:
 *   Given an R x C grid of 0 (open) and 1 (blocked), find a path for a rat from
 *   the top-left (0,0) to the bottom-right (R-1, C-1) moving in the four
 *   cardinal directions. We walk depth-first: from the current cell we try each
 *   neighbour; a cell may be used at most once, so we MARK it visited on the way
 *   in and UNMARK it on the way out to allow other paths to reuse it.
 *
 *   Backtracking template at cell (r, c):
 *       if (r, c) is the goal: record the path, succeed.
 *       CHOOSE   : mark (r, c) visited and push it onto the path
 *       EXPLORE  : for each of the 4 neighbours that is in-bounds, open, and
 *                  unvisited, recurse; if any recursion succeeds, propagate it
 *       UNCHOOSE : pop (r, c) and clear its visited mark (backtrack) so the cell
 *                  is available to alternative routes explored later
 *
 * Complexity:
 *   +----------+-------------------------------+
 *   | Aspect   | Cost                          |
 *   +----------+-------------------------------+
 *   | Time     | O(4^(R*C)) worst case         |
 *   | Space    | O(R*C) visited + recursion    |
 *   +----------+-------------------------------+
 *   WHY exponential: from every cell there are up to 4 choices and a path can
 *   in principle wander across all R*C cells, giving a 4^(R*C) envelope. The
 *   visited marks are the PRUNING: they forbid revisiting a cell, so no branch
 *   ever exceeds R*C steps and cycles are impossible. Space is O(R*C) for the
 *   visited grid plus a recursion stack no deeper than the number of cells.
 *
 * Complexity derivation (state-space tree -> geometric summation):
 *   Let n = R*C be the number of cells. Model the DFS as a tree: each dfs() node
 *   loops over the 4 cardinal directions, so the branching factor is b <= 4. The
 *   visited marks let any cell appear at most once on the current path, so every
 *   root-to-leaf path has length <= n and the tree depth is <= n. The number of
 *   dfs() nodes is bounded by the geometric series:
 *
 *       level d     #nodes (<=)     work per node
 *       --------    -----------     -------------------------------------------
 *       d = 0       1               4-direction loop, O(1) checks + push/pop
 *       d = 1       4               same
 *       d = 2       4^2             ...
 *       ...         ...             ...
 *       d = n       4^n (leaves)    O(1)
 *
 *       T(n) = SUM_{d=0}^{n} 4^d
 *            = (4^(n+1) - 1) / (4 - 1)      (geometric series, ratio 4)
 *            = (4^(n+1) - 1) / 3
 *            = O(4^n) = O(4^(R*C))
 *
 *   Per-node work is O(1): the fixed 4-iteration loop with O(1) bounds/blocked/
 *   visited tests plus O(1) push_back/pop_back. Hence total = O(4^(R*C)). Note
 *   the parent cell is always already visited, so the effective branching is
 *   <= 3, a tighter O(3^(R*C)); 4^(R*C) is the loose bound the header states.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f(n) = O(g)      iff  EXISTS c2, n0 :       f(n) <= c2*g(n)  for n >= n0
 *     f(n) = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f(n) = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   The cost is DATA-DEPENDENT (it depends on the maze), so bounds are per-case:
 *     WORST case  a large open grid riddled with dead ends forces the DFS to
 *                 explore near the whole tree before it finds a route or fails:
 *                 O(4^(R*C)).
 *     BEST case   a blocked start/goal returns at once (Theta(1)); an open maze
 *                 whose first-tried directions march straight to the goal costs
 *                 one path of length R*C with no backtracking: Theta(R*C).
 *   Over all inputs the running time is thus O(4^(R*C)) (worst) and Omega(1)
 *   (best, immediate rejection); no single Theta since best != worst. This is a
 *   reachability search, not a sort, so the Omega(n log n) sort bound is moot.
 *
 * Key points / when to use:
 *   - Classic "explore, mark, unmark" DFS backtracking on a grid.
 *   - Marking visited is what guarantees termination (no infinite loops).
 *   - Finds SOME path (the first the DFS order encounters), not necessarily the
 *     shortest -- use BFS if the shortest path is required.
 *   - Use for reachability / one-path problems where any valid route suffices.
 * ============================================================================
 */

#include <vector>
#include <utility>
#include <cassert>
#include <iostream>
#include <cstddef>

using Maze = std::vector<std::vector<int>>;      // 0 = open, 1 = blocked
using Cell = std::pair<std::size_t, std::size_t>; // (row, col)
using Path = std::vector<Cell>;

// The four cardinal moves: down, up, right, left. We use signed deltas and do
// bounds arithmetic carefully to avoid unsigned underflow at the edges.
static const int DR[4] = { 1, -1, 0, 0 };
static const int DC[4] = { 0, 0, 1, -1 };

// Depth-first search from (r, c) toward the bottom-right cell. `visited`
// tracks cells already on the current path. On success `path` holds the route.
static bool dfs(const Maze& maze,
                std::vector<std::vector<bool>>& visited,
                std::size_t r, std::size_t c,
                Path& path) {
    const std::size_t rows = maze.size();
    const std::size_t cols = maze[0].size();

    // CHOOSE: step onto (r, c) -- record it and mark it as used on this path.
    visited[r][c] = true;
    path.push_back({r, c});

    // BASE CASE: reached the goal cell -> the accumulated path is a solution.
    if (r == rows - 1 && c == cols - 1) return true;

    // EXPLORE: try every neighbour that is in-bounds, open, and unvisited.
    for (int k = 0; k < 4; ++k) {
        // Compute the neighbour with signed math, then range-check.
        long nr = static_cast<long>(r) + DR[k];
        long nc = static_cast<long>(c) + DC[k];
        if (nr < 0 || nc < 0) continue;                    // off the top/left
        if (nr >= static_cast<long>(rows)) continue;       // off the bottom
        if (nc >= static_cast<long>(cols)) continue;       // off the right

        std::size_t ur = static_cast<std::size_t>(nr);
        std::size_t uc = static_cast<std::size_t>(nc);
        if (maze[ur][uc] == 1) continue;                   // PRUNE: blocked wall
        if (visited[ur][uc]) continue;                     // PRUNE: already used

        if (dfs(maze, visited, ur, uc, path)) return true; // success bubbles up
    }

    // UNCHOOSE: no neighbour led to the goal -> backtrack. Remove (r, c) from
    // the path and clear its mark so OTHER routes may pass through it later.
    path.pop_back();
    visited[r][c] = false;
    return false;
}

// Public entry point. Returns a path (list of cells) or an empty path if the
// maze has no route. Also returns empty if the start or goal is itself blocked.
static Path solveMaze(const Maze& maze) {
    if (maze.empty() || maze[0].empty()) return {};
    const std::size_t rows = maze.size();
    const std::size_t cols = maze[0].size();
    if (maze[0][0] == 1 || maze[rows - 1][cols - 1] == 1) return {};

    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
    Path path;
    if (dfs(maze, visited, 0, 0, path)) return path;
    return {};
}

// Validate that `path` is a legitimate route through `maze`: it starts at the
// top-left, ends at the bottom-right, every cell is open, and consecutive cells
// are 4-directionally adjacent with no repeats.
static bool isValidPath(const Maze& maze, const Path& path) {
    if (path.empty()) return false;
    const std::size_t rows = maze.size();
    const std::size_t cols = maze[0].size();

    if (path.front() != Cell{0, 0}) return false;
    if (path.back() != Cell{rows - 1, cols - 1}) return false;

    std::vector<std::vector<bool>> seen(rows, std::vector<bool>(cols, false));
    for (std::size_t i = 0; i < path.size(); ++i) {
        std::size_t r = path[i].first, c = path[i].second;
        if (r >= rows || c >= cols) return false;   // in bounds
        if (maze[r][c] == 1) return false;          // on an open cell
        if (seen[r][c]) return false;               // no cell reused
        seen[r][c] = true;
        if (i > 0) {                                 // adjacent to predecessor
            std::size_t pr = path[i - 1].first, pc = path[i - 1].second;
            std::size_t dr = r > pr ? r - pr : pr - r;
            std::size_t dc = c > pc ? c - pc : pc - c;
            if (dr + dc != 1) return false;          // exactly one step apart
        }
    }
    return true;
}

int main() {
    // --- A solvable maze. The only route must snake around the 1s. ---
    Maze solvable = {
        {0, 0, 1, 0},
        {1, 0, 1, 0},
        {0, 0, 0, 0},
        {0, 1, 1, 0},
    };
    Path p = solveMaze(solvable);
    assert(!p.empty());              // a path exists
    assert(isValidPath(solvable, p)); // and it is a legal, contiguous route

    // --- A maze with NO route: the goal is walled off from the start. ---
    Maze unsolvable = {
        {0, 0, 0},
        {1, 1, 1},   // a solid wall separates the top row from the bottom
        {0, 0, 0},
    };
    Path none = solveMaze(unsolvable);
    assert(none.empty());            // no path can exist

    // --- Trivial 1x1 open maze: start IS the goal. ---
    Maze single = {{0}};
    Path trivial = solveMaze(single);
    assert(trivial.size() == 1);
    assert(isValidPath(single, trivial));

    // --- Blocked start cell yields no path. ---
    Maze blockedStart = {{1, 0}, {0, 0}};
    assert(solveMaze(blockedStart).empty());

    // --- Demo: print the discovered path through the solvable maze. ---
    std::cout << "Rat in a Maze -- path found (row,col):\n  ";
    for (std::size_t i = 0; i < p.size(); ++i) {
        std::cout << '(' << p[i].first << ',' << p[i].second << ')';
        if (i + 1 < p.size()) std::cout << " -> ";
    }
    std::cout << "\nMaze with the path marked ('*' = step, '#' = wall):\n";
    for (std::size_t r = 0; r < solvable.size(); ++r) {
        std::cout << "  ";
        for (std::size_t c = 0; c < solvable[r].size(); ++c) {
            bool onPath = false;
            for (const Cell& cell : p)
                if (cell.first == r && cell.second == c) { onPath = true; break; }
            if (solvable[r][c] == 1) std::cout << '#';
            else std::cout << (onPath ? '*' : '.');
        }
        std::cout << '\n';
    }
    std::cout << "All tests passed.\n";
    return 0;
}
