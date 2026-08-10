/*
 * Chromatic Number / Graph k-Coloring (Algorithm - NP-Complete / NP-Hard)
 * =======================================================================
 *
 * Decision problem (k-COLORABILITY):
 *   Given an undirected graph G = (V, E) and an integer k, does there exist a
 *   proper k-coloring -- an assignment color : V -> {1, ..., k} such that no
 *   edge {u, v} in E has color[u] == color[v]?
 *
 * Optimization problem (CHROMATIC NUMBER chi(G)):
 *   The SMALLEST k for which G is k-colorable. We compute it by testing
 *   k = 1, 2, 3, ... and returning the first k that is feasible.
 *
 * Complexity class:
 *   - k-COLORABILITY is NP-complete for every fixed k >= 3 (3-COLORING is the
 *     classic member, reducible from 3-SAT / NAE-3-SAT). The witness (a
 *     coloring) is checkable in polynomial time, so it lies in NP; the
 *     reduction shows it is NP-hard, hence NP-complete.
 *   - 2-COLORABILITY is exactly "is G bipartite?", solvable in O(V + E) by
 *     BFS/DFS -- so it is in P. The hardness jump happens at k = 3.
 *   - CHROMATIC NUMBER (find chi) is NP-hard: an efficient chi solver would
 *     answer every k-colorability question, including the NP-complete ones.
 *
 * Exact-algorithm complexity:
 *   +--------------------------+------------------------------------------------+
 *   | k-coloring feasibility   | O(k^V) worst case: V vertices, k choices each. |
 *   |   (backtracking)         | isSafe() pruning discards partial colorings    |
 *   |                          | the moment a neighbour conflicts, so the       |
 *   |                          | explored tree is far smaller in practice.      |
 *   | chi(G) via k = 1,2,3,... | sum over k of O(k^V); dominated by the last     |
 *   |                          | (successful) k. chi <= 1 + maxDegree always.   |
 *   | Space                    | O(V) recursion depth + O(V) color array.       |
 *   +--------------------------+------------------------------------------------+
 *   No polynomial exact algorithm is known (and none exists unless P = NP).
 *
 * Complexity derivation (backtracking state-space tree):
 *   Fix k. colorFrom() extends a partial coloring one vertex at a time, so the
 *   search is a tree of depth V with branching factor k (up to k color choices
 *   per vertex). Count the nodes level by level:
 *
 *       level d      #nodes (<=)     meaning
 *       ---------    -----------     ------------------------------------------
 *       d = 0        1               root: nothing colored yet
 *       d = 1        k               vertex 0 assigned one of k colors
 *       d = 2        k^2             vertices 0,1 assigned
 *       ...          ...             ...
 *       d = V        k^V             all V vertices assigned (a leaf coloring)
 *
 *   Total nodes explored, worst case (no pruning), is a geometric series:
 *
 *       N(V) = SUM_{d=0}^{V} k^d = (k^(V+1) - 1) / (k - 1) = Theta(k^V)   (k >= 2)
 *
 *   Work per node: the color loop tries up to k colors, each calling isSafe(),
 *   which scans all V vertices -> O(k*V) per node. Hence
 *
 *       T(V) = Theta(k^V) * O(k*V) = O(k^V * poly(V))  ->  O(k^V)
 *
 *   dropping polynomial factors, which matches the table. The symmetry break
 *   (fix vertex 0 to color 1) removes the top branching, saving a factor of k
 *   (effectively k^(V-1) leaves); isSafe() pruning discards conflicting prefixes
 *   so the explored tree is far smaller in practice, but the worst case stays
 *   Theta(k^V). chromaticNumber() tries k = 1, 2, ..., chi in turn:
 *
 *       SUM_{k=1}^{chi} O(k^V) = O(chi * chi^V),
 *
 *   whose exponential growth is governed by the final term chi^V (with
 *   chi <= 1 + maxDegree <= V) -- i.e. dominated by the last, successful k.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f = O(g)      iff  EXISTS c2, n0 :        f(V) <= c2*g(V)  for V >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g(V) <= f(V)        for V >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   The cost is DATA-DEPENDENT (pruning), so bounds are per case for fixed k:
 *     WORST case  no proper coloring / heavy backtracking: the tree fills out to
 *                 Theta(k^V) nodes  =>  time Theta(k^V) (times poly(V)).
 *     BEST case   the first color tried works for every vertex (e.g. an edgeless
 *                 graph): V nodes, one isSafe() of O(V) each  =>  Theta(V^2).
 *   Over all inputs the running time is therefore O(k^V) (upper, from the worst
 *   case) and Omega(V^2) (lower, from the best case) -- NOT a single Theta,
 *   which is exactly why pruning helps yet cannot beat the exponential worst
 *   case unless P = NP. The Omega(n log n) comparison-sort bound is irrelevant
 *   here: no sorting happens; the hardness is combinatorial search, not ordering.
 *
 * Key points:
 *   - The pruning heart is isSafe: a color is legal only if no already-colored
 *     neighbour uses it -- reject early instead of coloring the whole graph.
 *   - Symmetry break: fixing vertex 0 to color 1 (colors are interchangeable)
 *     removes a factor-k of redundant search without losing any solution.
 *   - 2-colorability is in P (bipartite test); k >= 3 is NP-complete -- the
 *     reason a "just try one more color" jump is computationally dramatic.
 *   - chi(G) is sandwiched: clique number <= chi(G) <= 1 + max degree.
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <utility> // std::pair
#include <vector>

using Graph = std::vector<std::vector<bool>>; // symmetric adjacency matrix

// Build a symmetric adjacency matrix from an undirected edge list.
static Graph makeGraph(
    std::size_t n,
    const std::vector<std::pair<std::size_t, std::size_t>> &edges) {
    Graph adj(n, std::vector<bool>(n, false));
    for (const auto &e : edges) {
        adj[e.first][e.second] = true;
        adj[e.second][e.first] = true; // undirected: mirror the entry
    }
    return adj;
}

// Is it legal to give vertex v the color c? Legal iff no colored neighbour of
// v already carries c. This is the single pruning check that makes the
// exponential search tractable on small graphs.
static bool isSafe(const Graph &adj, const std::vector<int> &color,
                   std::size_t v, int c) {
    const std::size_t n = adj.size();
    for (std::size_t u = 0; u < n; ++u) {
        if (adj[v][u] && color[u] == c) return false; // adjacent + same color
    }
    return true;
}

// Backtracking core: try to color vertices v, v+1, ... using colors 1..k.
static bool colorFrom(const Graph &adj, int k, std::size_t v,
                      std::vector<int> &color) {
    if (v == adj.size()) return true; // every vertex colored -> success

    for (int c = 1; c <= k; ++c) {
        if (isSafe(adj, color, v, c)) {
            color[v] = c;                            // CHOOSE color c
            if (colorFrom(adj, k, v + 1, color))     // EXPLORE the rest
                return true;                         // propagate success up
            color[v] = 0;                            // UNCHOOSE (backtrack)
        }
    }
    return false; // no color worked for v; signal the caller to backtrack
}

// Decision entry point: is G k-colorable? Fills `color` (1..k) on success.
static bool canColor(const Graph &adj, int k, std::vector<int> &color) {
    const std::size_t n = adj.size();
    color.assign(n, 0); // 0 == uncolored
    if (k <= 0) return n == 0; // 0 colors only suffice for the empty vertex set
    if (n == 0) return true;   // no vertices are trivially colorable (guards color[0])

    // Symmetry break: colors are interchangeable, so we may fix vertex 0 to
    // color 1 (any valid coloring can be relabeled to match). This prunes a
    // factor of k from the search without discarding any feasible solution.
    color[0] = 1;
    return colorFrom(adj, k, 1, color);
}

// Convenience overload when the caller does not need the witness back.
static bool canColor(const Graph &adj, int k) {
    std::vector<int> color;
    return canColor(adj, k, color);
}

// Chromatic number: smallest k with a proper k-coloring. Also returns an
// optimal coloring via `color`. Upper bound 1 + maxDegree guarantees success.
static int chromaticNumber(const Graph &adj, std::vector<int> &color) {
    const std::size_t n = adj.size();
    if (n == 0) { color.clear(); return 0; }        // no vertices -> 0 colors

    // An edgeless graph needs exactly 1 color; detect "any edge present?".
    bool hasEdge = false;
    for (std::size_t v = 0; v < n && !hasEdge; ++v)
        for (std::size_t u = v + 1; u < n; ++u)
            if (adj[v][u]) { hasEdge = true; break; }
    if (!hasEdge) { color.assign(n, 1); return 1; }

    // Try increasing k; the first feasible k IS the chromatic number.
    for (int k = 2; k <= static_cast<int>(n); ++k) {
        if (canColor(adj, k, color)) return k;
    }
    // Unreachable: k = n (assign each vertex its own color) is always proper.
    color.assign(n, 1);
    return static_cast<int>(n);
}

// Verify a coloring is proper AND uses at most k colors: all vertices colored,
// adjacent vertices differ. Used to validate returned witnesses in tests.
static bool isProperColoring(const Graph &adj, const std::vector<int> &color,
                             int k) {
    const std::size_t n = adj.size();
    if (color.size() != n) return false;
    for (std::size_t v = 0; v < n; ++v) {
        if (color[v] < 1 || color[v] > k) return false; // out of palette range
        for (std::size_t u = v + 1; u < n; ++u)
            if (adj[v][u] && color[v] == color[u]) return false; // clash
    }
    return true;
}

// Build the complete graph K_n (every pair adjacent) -- chi(K_n) = n.
static Graph completeGraph(std::size_t n) {
    Graph adj(n, std::vector<bool>(n, false));
    for (std::size_t v = 0; v < n; ++v)
        for (std::size_t u = v + 1; u < n; ++u)
            adj[v][u] = adj[u][v] = true;
    return adj;
}

// Build the cycle C_n: 0-1-2-...-(n-1)-0. Even n is bipartite (chi=2),
// odd n needs 3 colors (chi=3).
static Graph cycleGraph(std::size_t n) {
    Graph adj(n, std::vector<bool>(n, false));
    for (std::size_t v = 0; v < n; ++v) {
        const std::size_t w = (v + 1) % n;
        adj[v][w] = adj[w][v] = true;
    }
    return adj;
}

int main() {
    std::vector<int> color;

    // --- Triangle K_3 (odd cycle C_3): chromatic number 3 ---
    {
        Graph tri = completeGraph(3);
        assert(!canColor(tri, 2));                 // 2 colors impossible
        int chi = chromaticNumber(tri, color);
        assert(chi == 3);
        assert(isProperColoring(tri, color, chi)); // witness is valid
    }

    // --- Even cycle C_4 (bipartite): chromatic number 2 ---
    {
        Graph c4 = cycleGraph(4);
        assert(!canColor(c4, 1));                  // 1 color impossible (edges)
        int chi = chromaticNumber(c4, color);
        assert(chi == 2);
        assert(isProperColoring(c4, color, chi));
    }

    // --- Odd cycle C_5: chromatic number 3 (odd cycles are not bipartite) ---
    {
        Graph c5 = cycleGraph(5);
        assert(!canColor(c5, 2));                  // odd cycle is not 2-colorable
        int chi = chromaticNumber(c5, color);
        assert(chi == 3);
        assert(isProperColoring(c5, color, chi));
    }

    // --- Complete graphs K_n: chromatic number is exactly n ---
    for (std::size_t n = 1; n <= 6; ++n) {
        Graph kn = completeGraph(n);
        int chi = chromaticNumber(kn, color);
        assert(chi == static_cast<int>(n));
        assert(isProperColoring(kn, color, chi));
        if (n >= 2) assert(!canColor(kn, static_cast<int>(n) - 1)); // n-1 fails
    }

    // --- Bipartite graph (a "star" K_{1,4}): chromatic number 2 ---
    // Center 0 joined to leaves 1,2,3,4; leaves are mutually non-adjacent.
    {
        Graph star = makeGraph(5, {{0, 1}, {0, 2}, {0, 3}, {0, 4}});
        int chi = chromaticNumber(star, color);
        assert(chi == 2);
        assert(isProperColoring(star, color, chi));
    }

    // --- Edgeless graph on 5 vertices: chromatic number 1 ---
    {
        Graph empty = makeGraph(5, {});
        int chi = chromaticNumber(empty, color);
        assert(chi == 1);
        assert(isProperColoring(empty, color, chi));
    }

    // --- Edge cases: no vertices at all ---
    {
        Graph none = makeGraph(0, {});
        int chi = chromaticNumber(none, color);
        assert(chi == 0);
        assert(color.empty());
    }

    // --- Short std::cout demo: chromatic number of C_5 with a witness ---
    Graph c5 = cycleGraph(5);
    int chi = chromaticNumber(c5, color);
    std::cout << "5-cycle C_5 chromatic number = " << chi
              << " (expected 3, since odd cycles are non-bipartite)\n";
    for (std::size_t v = 0; v < color.size(); ++v)
        std::cout << "  vertex " << v << " -> color " << color[v] << '\n';

    std::cout << "All assertions passed.\n";
    return 0;
}
