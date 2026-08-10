/*
 * Graph Coloring - m-Coloring Decision (Algorithm - Recursion/Backtracking)
 * -------------------------------------------------------------------------
 * Problem: Given an undirected graph and m colors, decide whether every vertex
 *          can be assigned one of the m colors so that NO two adjacent vertices
 *          share the same color (a proper coloring). Optionally recover one.
 *
 * Idea (choose / explore / unchoose backtracking):
 *   Color vertices one at a time in index order. For the current vertex try
 *   each color 1..m; a color is legal only if none of the already-colored
 *   neighbours use it (isSafe check). If legal, CHOOSE it, recurse on the next
 *   vertex (EXPLORE); if that fails, UNCHOOSE (reset to 0) and try the next
 *   color. When every vertex is colored we have a valid m-coloring.
 *
 *   Recurrence: solve(v) = OR over colors c in 1..m of
 *               ( isSafe(v,c) AND solve(v+1) with color[v]=c )
 *
 * Complexity:
 *   +-----------+---------------------------------------------------------+
 *   | Time      | O(m^V) worst case: V vertices, m color choices each.     |
 *   |           | Exponential because m-coloring is NP-complete for m>=3 - |
 *   |           | there is no known polynomial algorithm. The isSafe()     |
 *   |           | check PRUNES: a color conflicting with a colored         |
 *   |           | neighbour is rejected before recursing, cutting large    |
 *   |           | portions of the m^V tree.                               |
 *   | Space     | O(V) recursion depth + O(V) color array (adjacency       |
 *   |           | matrix is O(V^2) input).                                 |
 *   +-----------+---------------------------------------------------------+
 *
 * Complexity derivation (state-space tree: branching^depth * work/node):
 *   The recursion fixes exactly one vertex per level, and at each vertex the
 *   for-loop branches on the m possible colors. So the search space is an
 *   m-ary tree of height V (levels d = 0, 1, ..., V), with a complete color
 *   assignment sitting at each depth-V leaf. Count the NODES visited when no
 *   pruning fires (the worst case) as a finite geometric series:
 *
 *       N = SUM_{d=0}^{V} m^d
 *         = 1 + m + m^2 + ... + m^V
 *         = (m^(V+1) - 1) / (m - 1)        (geometric series, ratio m >= 2)
 *
 *   For m >= 2 this is squeezed as  m^V <= N <= 2 * m^V, hence N = Theta(m^V):
 *   the m^V leaves (the m^V candidate colorings) dominate the node count.
 *   Local work per node: the loop tries up to m colors and each isSafe() scans
 *   the length-V adjacency row -> O(m * V) per node. Multiplying gives the exact
 *   worst-case operation count
 *
 *       C(V) = Theta(m^V) * O(m * V) = O(V * m^(V+1)),
 *
 *   and the reported O(m^V) keeps only the dominant EXPONENTIAL factor, absorbing
 *   the polynomial m*V (standard for exponential / NP-complete bounds). The
 *   isSafe() test can only DELETE subtrees, never add them, so pruning lowers the
 *   real cost on most inputs but never raises this m^V ceiling.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f = O(g)      iff  EXISTS c2, n0 :        f(n) <= c2*g(n)  for n >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   This search is INPUT-DEPENDENT (adaptive via isSafe pruning), so the bound is
 *   stated PER CASE, with node visits as the basic operation:
 *     WORST case  a graph with NO valid m-coloring (e.g. K4 with m=3 in the
 *                 tests) forces every one of the m^V assignments to be refuted
 *                 before returning false => Theta(m^V) node visits (equivalently
 *                 Theta(V * m^V) once the O(V) isSafe scan per node is counted).
 *     BEST case   when color 1 is always legal (e.g. the edgeless graph, or the
 *                 leftmost root-to-leaf path just works) the recursion descends V
 *                 levels with no backtracking, visiting V+1 nodes => Theta(V)
 *                 (Theta(V^2) counting the isSafe scans).
 *   Over ALL inputs the running time is therefore O(m^V) (upper bound, from the
 *   worst case) and Omega(V) (lower bound, from the best case); it is NOT a single
 *   Theta because best != worst -- that gap is exactly why pruning helps in
 *   practice. Note m-coloring is NP-complete for m >= 3, so no polynomial
 *   algorithm is known and the exponential ceiling is intrinsic, not an artifact
 *   of this code. The comparison-sort Omega(n log n) lower bound is irrelevant
 *   here: this is a decision/search over colorings, not a comparison sort.
 *
 * Key points / when to use:
 *   - Canonical constraint-satisfaction backtracking (register allocation,
 *     timetabling, map coloring, frequency assignment all reduce to this).
 *   - The chromatic number is the smallest m for which a coloring exists; find
 *     it by trying m = 1, 2, 3, ... until canColor(m) succeeds.
 *   - isSafe is the pruning heart: reject a color the moment it clashes.
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <utility> // std::pair
#include <vector>

using Graph = std::vector<std::vector<bool>>; // adjacency matrix, symmetric

// Can we give vertex v the color c without clashing with a colored neighbour?
static bool isSafe(const Graph &adj, const std::vector<int> &color,
                   std::size_t v, int c) {
    const std::size_t n = adj.size();
    for (std::size_t u = 0; u < n; ++u) {
        // u is adjacent to v AND already carries the same color -> not safe.
        if (adj[v][u] && color[u] == c) return false;
    }
    return true;
}

// Backtracking core: try to color vertices v, v+1, ... using colors 1..m.
static bool colorFrom(const Graph &adj, int m, std::size_t v,
                      std::vector<int> &color) {
    const std::size_t n = adj.size();

    // Base case: all vertices colored successfully.
    if (v == n) return true;

    // Try each of the m colors for vertex v.
    for (int c = 1; c <= m; ++c) {
        if (isSafe(adj, color, v, c)) {
            color[v] = c;                              // CHOOSE color c
            if (colorFrom(adj, m, v + 1, color))       // EXPLORE next vertex
                return true;                           // propagate success
            color[v] = 0;                              // UNCHOOSE (backtrack)
        }
    }
    return false; // no color worked for v -> signal caller to backtrack
}

// Decision entry point: is the graph m-colorable? Fills `color` on success.
static bool canColor(const Graph &adj, int m, std::vector<int> &color) {
    color.assign(adj.size(), 0); // 0 == uncolored
    return colorFrom(adj, m, 0, color);
}

// Convenience overload when the caller does not need the assignment back.
static bool canColor(const Graph &adj, int m) {
    std::vector<int> color;
    return canColor(adj, m, color);
}

// Build a symmetric adjacency matrix from an undirected edge list.
static Graph makeGraph(std::size_t n,
                       const std::vector<std::pair<std::size_t, std::size_t>> &edges) {
    Graph adj(n, std::vector<bool>(n, false));
    for (const auto &e : edges) {
        adj[e.first][e.second] = true;
        adj[e.second][e.first] = true; // undirected
    }
    return adj;
}

// Verify a coloring is proper: adjacent vertices differ, all vertices colored.
static bool isProperColoring(const Graph &adj, const std::vector<int> &color) {
    const std::size_t n = adj.size();
    for (std::size_t v = 0; v < n; ++v) {
        if (color[v] == 0) return false; // uncolored vertex
        for (std::size_t u = v + 1; u < n; ++u)
            if (adj[v][u] && color[v] == color[u]) return false; // clash
    }
    return true;
}

int main() {
    // --- Triangle K3: needs 3 colors, cannot be done with 2 ---
    // Vertices 0-1-2 all mutually connected (odd cycle).
    {
        Graph tri = makeGraph(3, {{0, 1}, {1, 2}, {0, 2}});
        assert(!canColor(tri, 2)); // 2 colors impossible for a triangle
        std::vector<int> color;
        assert(canColor(tri, 3, color)); // 3 colors succeed
        assert(isProperColoring(tri, color));
    }

    // --- Bipartite graph (even cycle C4): 2 colors suffice, 1 does not ---
    // Square 0-1-2-3-0; two color classes {0,2} and {1,3}.
    {
        Graph square = makeGraph(4, {{0, 1}, {1, 2}, {2, 3}, {3, 0}});
        assert(!canColor(square, 1)); // 1 color impossible (edges exist)
        std::vector<int> color;
        assert(canColor(square, 2, color)); // 2 colors is the chromatic number
        assert(isProperColoring(square, color));
    }

    // --- Complete graph K4: chromatic number is 4 ---
    {
        Graph k4 = makeGraph(
            4, {{0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}});
        assert(!canColor(k4, 3)); // K_n needs exactly n colors
        assert(canColor(k4, 4));
    }

    // --- Graph with no edges: 1 color is always enough ---
    {
        Graph empty = makeGraph(5, {});
        assert(canColor(empty, 1));
    }

    // --- Find the chromatic number of the triangle by increasing m ---
    {
        Graph tri = makeGraph(3, {{0, 1}, {1, 2}, {0, 2}});
        int chromatic = 0;
        for (int m = 1; m <= 3; ++m) {
            if (canColor(tri, m)) {
                chromatic = m;
                break;
            }
        }
        assert(chromatic == 3);
    }

    // --- Short std::cout demo ---
    Graph square = makeGraph(4, {{0, 1}, {1, 2}, {2, 3}, {3, 0}});
    std::vector<int> color;
    canColor(square, 2, color);
    std::cout << "2-coloring of the 4-cycle (bipartite):\n";
    for (std::size_t v = 0; v < color.size(); ++v)
        std::cout << "  vertex " << v << " -> color " << color[v] << '\n';

    std::cout << "All assertions passed.\n";
    return 0;
}
