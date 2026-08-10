/*
 * ============================================================================
 * Maximum Independent Set (MIS)
 * Category: Algorithm - NP-Hard (optimization) / NP-Complete (decision)
 *
 * DECISION PROBLEM:
 *   Given an undirected graph G = (V, E) and an integer k, is there a set S of
 *   k vertices that are PAIRWISE NON-adjacent (no edge lies inside S)?  The
 *   optimization version asks for the LARGEST such set; its size is alpha(G).
 *
 * Complexity class:
 *   Independent Set (decision) is NP-COMPLETE; Maximum Independent Set
 *   (optimization) is NP-HARD.  WHY: Karp reduction from 3-SAT / Clique.
 *   Two exact dualities make this the same problem as the other two files here:
 *        alpha(G) = omega(G-bar)          (MIS of G = max clique of complement)
 *        alpha(G) = |V| - tau(G)          (MIS = n - minimum vertex cover)
 *   because S is independent  <=>  V \ S is a vertex cover.
 *
 * ALGORITHMS IMPLEMENTED:
 *   +-------------------------+----------------------------+-----------------+
 *   | Method                  | Time                       | Result          |
 *   +-------------------------+----------------------------+-----------------+
 *   | Brute force (reference) | O(2^n * n)                 | EXACT maximum   |
 *   | Branch & bound          | O(1.38^n) worst, faster    | EXACT maximum   |
 *   +-------------------------+----------------------------+-----------------+
 *
 * Key points:
 *   - Branching rule on a vertex v: either INCLUDE v (then delete v and ALL its
 *     neighbours, since they can no longer join S) or EXCLUDE v (delete just v).
 *     Every independent set falls into exactly one branch, so this is exact.
 *   - Bound / pruning: if |chosen| + |available| <= best, no completion of this
 *     branch can beat the incumbent -> cut it.
 *   - The available vertex set and adjacency are std::uint32_t bitmasks; "delete
 *     v and its neighbours" is a single ANDNOT with (bit_v | N(v)).
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

// Index of the lowest set bit (x must be non-zero).
static int lowestBitIndex(std::uint32_t x) {
    int i = 0;
    while (((x >> i) & 1u) == 0u) ++i;
    return i;
}

// adj[i] has bit j set iff edge (i,j) exists (no self-loops => bit i is 0).
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
// EXACT solver: branch & bound.
//   available = bitmask of vertices still eligible to be picked
//   chosen    = bitmask of vertices already placed in the independent set
// ---------------------------------------------------------------------------
static void misBranch(std::uint32_t available, std::uint32_t chosen,
                      const std::vector<std::uint32_t>& adj,
                      int& best, std::uint32_t& bestSet) {
    const int chosenCnt = popcount32(chosen);
    if (available == 0u) {                       // leaf: nothing left to add
        if (chosenCnt > best) { best = chosenCnt; bestSet = chosen; }
        return;
    }
    // Optimistic bound: even taking every remaining vertex cannot beat `best`.
    if (chosenCnt + popcount32(available) <= best) return;

    const int v = lowestBitIndex(available);
    const std::uint32_t bit = std::uint32_t{1} << v;

    // Branch 1: INCLUDE v -> remove v and all its neighbours from `available`.
    misBranch(available & ~bit & ~adj[static_cast<std::size_t>(v)],
              chosen | bit, adj, best, bestSet);
    // Branch 2: EXCLUDE v -> remove only v.
    misBranch(available & ~bit, chosen, adj, best, bestSet);
}

// Returns a maximum independent set as a sorted vertex list; size -> `size`.
static std::vector<int> maxIndependentSet(
        int n, const std::vector<std::pair<int, int>>& edges, int& size) {
    const std::vector<std::uint32_t> adj = buildAdjacency(n, edges);
    int best = 0;
    std::uint32_t bestSet = 0u;
    const std::uint32_t allVertices =
        (n == 0) ? 0u : ((std::uint32_t{1} << n) - 1u);
    misBranch(allVertices, 0u, adj, best, bestSet);
    size = best;
    std::vector<int> res;
    for (int i = 0; i < n; ++i)
        if (((bestSet >> i) & 1u) != 0) res.push_back(i);
    return res;
}

// ---------------------------------------------------------------------------
// Helpers: verify independence; reference brute-force MIS.
// ---------------------------------------------------------------------------
static bool isIndependent(std::uint32_t mask,
                          const std::vector<std::uint32_t>& adj) {
    std::uint32_t m = mask;
    while (m != 0u) {
        const int v = lowestBitIndex(m);
        m &= (m - 1);
        // v must have NO neighbour inside the selected set
        if ((adj[static_cast<std::size_t>(v)] & mask) != 0u) return false;
    }
    return true;
}

static int maxIndependentSetBrute(
        int n, const std::vector<std::pair<int, int>>& edges) {
    assert(n <= 20 && "brute force limited to small graphs");
    const std::vector<std::uint32_t> adj = buildAdjacency(n, edges);
    int best = 0;
    const std::uint32_t total = (n == 0) ? 1u : (std::uint32_t{1} << n);
    for (std::uint32_t mask = 0; mask < total; ++mask) {
        if (isIndependent(mask, adj)) {
            const int c = popcount32(mask);
            if (c > best) best = c;
        }
    }
    return best;
}

int main() {
    // ----- Graph 1: path P4  0-1-2-3 -> MIS = 2 (e.g. {0,2}) -----------------
    {
        const std::vector<std::pair<int, int>> edges = {{0, 1}, {1, 2}, {2, 3}};
        int sz = 0;
        const std::vector<int> s = maxIndependentSet(4, edges, sz);
        assert(sz == 2);
        assert(maxIndependentSetBrute(4, edges) == 2);
        const std::vector<std::uint32_t> adj = buildAdjacency(4, edges);
        std::uint32_t m = 0u;
        for (int v : s) m |= (std::uint32_t{1} << v);
        assert(isIndependent(m, adj));           // witness really is independent
    }

    // ----- Graph 2: star (center 0, leaves 1..4) -> MIS = 4 (all leaves) -----
    {
        const std::vector<std::pair<int, int>> edges = {
            {0, 1}, {0, 2}, {0, 3}, {0, 4}};
        int sz = 0;
        const std::vector<int> s = maxIndependentSet(5, edges, sz);
        assert(sz == 4);
        assert(maxIndependentSetBrute(5, edges) == 4);
        const std::vector<std::uint32_t> adj = buildAdjacency(5, edges);
        std::uint32_t m = 0u;
        for (int v : s) m |= (std::uint32_t{1} << v);
        assert(isIndependent(m, adj));
        assert(((m >> 0) & 1u) == 0u);           // the hub cannot be chosen
    }

    // ----- Graph 3: triangle K3 -> MIS = 1 -----------------------------------
    {
        const std::vector<std::pair<int, int>> edges = {{0, 1}, {1, 2}, {0, 2}};
        int sz = 0;
        const std::vector<int> s = maxIndependentSet(3, edges, sz);
        assert(sz == 1);
        assert(maxIndependentSetBrute(3, edges) == 1);
    }

    // ----- Graph 4: 4-cycle C4 -> MIS = 2 ({0,2}) ----------------------------
    {
        const std::vector<std::pair<int, int>> edges = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0}};
        int sz = 0;
        const std::vector<int> s = maxIndependentSet(4, edges, sz);
        assert(sz == 2);
        assert(maxIndependentSetBrute(4, edges) == 2);
        const std::vector<std::uint32_t> adj = buildAdjacency(4, edges);
        std::uint32_t m = 0u;
        for (int v : s) m |= (std::uint32_t{1} << v);
        assert(isIndependent(m, adj));
    }

    // ----- Graph 5: edgeless graph -> MIS = n (edge case) --------------------
    {
        const std::vector<std::pair<int, int>> edges = {};
        int sz = 0;
        const std::vector<int> s = maxIndependentSet(4, edges, sz);
        assert(sz == 4 && s.size() == 4);
        assert(maxIndependentSetBrute(4, edges) == 4);
    }

    // ----- Duality check: alpha(G) == n - tau(G) on the path P4 --------------
    // Minimum vertex cover of P4 is 2, and n - 2 = 2 == MIS(P4). We verify the
    // identity indirectly: the complement of a maximum independent set is a
    // minimum vertex cover.
    {
        const std::vector<std::pair<int, int>> edges = {{0, 1}, {1, 2}, {2, 3}};
        int sz = 0;
        const std::vector<int> s = maxIndependentSet(4, edges, sz);
        std::uint32_t inSet = 0u;
        for (int v : s) inSet |= (std::uint32_t{1} << v);
        const std::uint32_t coverMask = ((std::uint32_t{1} << 4) - 1u) & ~inSet;
        // that complement must cover every edge
        bool covers = true;
        for (const auto& e : edges) {
            const bool cu = ((coverMask >> e.first) & 1u) != 0;
            const bool cv = ((coverMask >> e.second) & 1u) != 0;
            if (!cu && !cv) { covers = false; break; }
        }
        assert(covers);
        assert(popcount32(coverMask) == 4 - sz);
    }

    // ----- Demo output -------------------------------------------------------
    const std::vector<std::pair<int, int>> demo = {
        {0, 1}, {0, 2}, {0, 3}, {0, 4}};
    int sz = 0;
    const std::vector<int> s = maxIndependentSet(5, demo, sz);
    std::cout << "Maximum Independent Set demo (star, center 0)\n";
    std::cout << "  MIS size = " << sz << ", vertices = {";
    for (std::size_t i = 0; i < s.size(); ++i)
        std::cout << (i ? ", " : "") << s[i];
    std::cout << "}\n";
    std::cout << "All assertions passed.\n";
    return 0;
}
