/*
 * ============================================================================
 * Maximum Clique
 * Category: Algorithm - NP-Hard (optimization) / NP-Complete (k-clique decision)
 *
 * DECISION PROBLEM (k-Clique):
 *   Given an undirected graph G = (V, E) and an integer k, is there a set of k
 *   vertices that are PAIRWISE adjacent (a complete subgraph K_k)?  The
 *   optimization version asks for the LARGEST such set (the maximum clique).
 *
 * Complexity class:
 *   k-Clique (decision) is NP-COMPLETE; Maximum Clique (optimization) is
 *   NP-HARD.  WHY: classic reduction from 3-SAT (Karp).  Also dual to
 *   Independent Set:  a clique in G is exactly an INDEPENDENT SET in the
 *   complement graph G-bar, so  omega(G) = alpha(G-bar).
 *
 * ALGORITHMS IMPLEMENTED:
 *   +--------------------------+---------------------------+-----------------+
 *   | Method                   | Time                      | Result          |
 *   +--------------------------+---------------------------+-----------------+
 *   | Brute force (reference)  | O(2^n * n)                | EXACT maximum   |
 *   | Bron-Kerbosch + pivoting | O(3^(n/3)) worst case     | EXACT maximum   |
 *   +--------------------------+---------------------------+-----------------+
 *   3^(n/3) is optimal in the worst case: it matches the maximum possible
 *   number of maximal cliques in an n-vertex graph (Moon-Moser bound).
 *
 * Key points:
 *   - Bron-Kerbosch enumerates all MAXIMAL cliques via sets R (current clique),
 *     P (candidates that extend R), X (already-explored vertices).
 *   - PIVOTING: choose pivot u in P U X maximizing |P & N(u)|; we only branch on
 *     P \ N(u). Any maximal clique either contains u or a non-neighbour of u, so
 *     skipping N(u) loses nothing while pruning huge redundant subtrees.
 *   - Adjacency is stored as a bitmask per vertex (neighbour set), so set
 *     intersection / difference are single AND / ANDNOT machine instructions.
 * ============================================================================
 */

#include <vector>
#include <utility>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <iostream>

// popcount via Kernighan's trick (one iteration per set bit).
static int popcount32(std::uint32_t x) {
    int c = 0;
    while (x) { x &= (x - 1); ++c; }
    return c;
}

// Index of the lowest set bit (x must be non-zero). x & -x isolates it; here a
// simple scan keeps the code standard-library-only and easy to read.
static int lowestBitIndex(std::uint32_t x) {
    int i = 0;
    while (((x >> i) & 1u) == 0u) ++i;
    return i;
}

// Build neighbour bitmasks: adj[i] has bit j set iff edge (i,j) exists.
// Note adj[i] never has bit i set (no self-loops).
static std::vector<std::uint32_t> buildAdjacency(
        int n, const std::vector<std::pair<int, int>>& edges) {
    std::vector<std::uint32_t> adj(static_cast<std::size_t>(n), 0u);
    for (const auto& e : edges) {
        adj[static_cast<std::size_t>(e.first)]  |= (std::uint32_t{1} << e.second);
        adj[static_cast<std::size_t>(e.second)] |= (std::uint32_t{1} << e.first);
    }
    return adj;
}

// ---------------------------------------------------------------------------
// Bron-Kerbosch with pivoting. Tracks the single LARGEST clique seen.
// ---------------------------------------------------------------------------
static void bronKerbosch(std::uint32_t R, std::uint32_t P, std::uint32_t X,
                         const std::vector<std::uint32_t>& adj,
                         int& bestSize, std::uint32_t& bestClique) {
    if (P == 0u && X == 0u) {                    // R is a maximal clique
        const int sz = popcount32(R);
        if (sz > bestSize) { bestSize = sz; bestClique = R; }
        return;
    }

    // Choose pivot u in (P U X) maximizing |P & N(u)|.
    std::uint32_t PX = P | X;
    int pivot = lowestBitIndex(PX);
    int bestCnt = -1;
    for (std::uint32_t t = PX; t != 0u; t &= (t - 1)) {
        const int u = lowestBitIndex(t);
        const int cnt = popcount32(P & adj[static_cast<std::size_t>(u)]);
        if (cnt > bestCnt) { bestCnt = cnt; pivot = u; }
    }

    // Branch only on candidates that are NOT neighbours of the pivot.
    std::uint32_t candidates = P & ~adj[static_cast<std::size_t>(pivot)];
    while (candidates != 0u) {
        const int v = lowestBitIndex(candidates);
        const std::uint32_t bit = std::uint32_t{1} << v;
        const std::uint32_t Nv = adj[static_cast<std::size_t>(v)];
        bronKerbosch(R | bit, P & Nv, X & Nv, adj, bestSize, bestClique);
        P &= ~bit;                               // move v from P ...
        X |= bit;                                // ... into X (explored)
        candidates &= ~bit;
    }
}

// Returns a maximum clique as a sorted vertex list; writes its size to `size`.
static std::vector<int> maxClique(int n,
                                  const std::vector<std::pair<int, int>>& edges,
                                  int& size) {
    const std::vector<std::uint32_t> adj = buildAdjacency(n, edges);
    int bestSize = 0;
    std::uint32_t bestClique = 0u;
    const std::uint32_t allVertices =
        (n == 0) ? 0u : ((std::uint32_t{1} << n) - 1u);  // bits 0..n-1
    bronKerbosch(0u, allVertices, 0u, adj, bestSize, bestClique);
    size = bestSize;
    std::vector<int> res;
    for (int i = 0; i < n; ++i)
        if (((bestClique >> i) & 1u) != 0) res.push_back(i);
    return res;
}

// ---------------------------------------------------------------------------
// Helpers: verify a set is a clique; reference brute-force maximum clique.
// ---------------------------------------------------------------------------
static bool isClique(std::uint32_t mask, const std::vector<std::uint32_t>& adj) {
    std::uint32_t m = mask;
    while (m != 0u) {
        const int v = lowestBitIndex(m);
        m &= (m - 1);
        const std::uint32_t others = mask & ~(std::uint32_t{1} << v);
        // every OTHER selected vertex must be a neighbour of v
        if ((others & adj[static_cast<std::size_t>(v)]) != others) return false;
    }
    return true;
}

static int maxCliqueBrute(int n,
                          const std::vector<std::pair<int, int>>& edges) {
    assert(n <= 20 && "brute force limited to small graphs");
    const std::vector<std::uint32_t> adj = buildAdjacency(n, edges);
    int best = 0;
    const std::uint32_t total = (n == 0) ? 1u : (std::uint32_t{1} << n);
    for (std::uint32_t mask = 0; mask < total; ++mask) {
        if (isClique(mask, adj)) {
            const int c = popcount32(mask);
            if (c > best) best = c;
        }
    }
    return best;
}

int main() {
    // ----- Graph 1: triangle K3 -> max clique = 3 ----------------------------
    {
        const std::vector<std::pair<int, int>> edges = {{0, 1}, {1, 2}, {0, 2}};
        int sz = 0;
        const std::vector<int> cl = maxClique(3, edges, sz);
        assert(sz == 3);
        assert(maxCliqueBrute(3, edges) == 3);
        const std::vector<std::uint32_t> adj = buildAdjacency(3, edges);
        std::uint32_t m = 0u;
        for (int v : cl) m |= (std::uint32_t{1} << v);
        assert(isClique(m, adj));                // witness really is a clique
    }

    // ----- Graph 2: K4 on {0,1,2,3} plus a pendant vertex 4 -> max clique = 4
    {
        const std::vector<std::pair<int, int>> edges = {
            {0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3},  // the 4-clique
            {0, 4}                                            // vertex 4 hangs off 0
        };
        int sz = 0;
        const std::vector<int> cl = maxClique(5, edges, sz);
        assert(sz == 4);
        assert(maxCliqueBrute(5, edges) == 4);
        const std::vector<std::uint32_t> adj = buildAdjacency(5, edges);
        std::uint32_t m = 0u;
        for (int v : cl) m |= (std::uint32_t{1} << v);
        assert(isClique(m, adj));
        assert(((m >> 4) & 1u) == 0u);           // vertex 4 cannot be in the clique
    }

    // ----- Graph 3: two disjoint edges (matching) -> max clique = 2 ----------
    {
        const std::vector<std::pair<int, int>> edges = {{0, 1}, {2, 3}};
        int sz = 0;
        const std::vector<int> cl = maxClique(4, edges, sz);
        assert(sz == 2);
        assert(maxCliqueBrute(4, edges) == 2);
        assert(cl.size() == 2);
    }

    // ----- Graph 4: edgeless graph -> max clique = 1 (a single vertex) -------
    {
        const std::vector<std::pair<int, int>> edges = {};
        int sz = 0;
        const std::vector<int> cl = maxClique(4, edges, sz);
        assert(sz == 1);
        assert(maxCliqueBrute(4, edges) == 1);
        assert(cl.size() == 1);
    }

    // ----- Demo output -------------------------------------------------------
    const std::vector<std::pair<int, int>> demo = {
        {0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}, {0, 4}};
    int sz = 0;
    const std::vector<int> cl = maxClique(5, demo, sz);
    std::cout << "Maximum Clique demo (K4 on {0,1,2,3} + pendant 4)\n";
    std::cout << "  clique size = " << sz << ", vertices = {";
    for (std::size_t i = 0; i < cl.size(); ++i)
        std::cout << (i ? ", " : "") << cl[i];
    std::cout << "}\n";
    std::cout << "All assertions passed.\n";
    return 0;
}
