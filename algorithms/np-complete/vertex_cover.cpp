/*
 * ============================================================================
 * Minimum Vertex Cover
 * Category: Algorithm - NP-Complete (decision) / NP-Hard (optimization)
 *
 * DECISION PROBLEM:
 *   Given an undirected graph G = (V, E) and an integer k, does there exist a
 *   set C of vertices with |C| <= k such that EVERY edge has at least one
 *   endpoint in C?  (The optimization version asks for the SMALLEST such C.)
 *
 * Complexity class:
 *   Vertex Cover (decision) is NP-COMPLETE; the optimization form is NP-HARD.
 *   WHY: Classic Karp reduction from 3-SAT via Independent Set / Clique. In
 *   fact the three problems are dual on the same graph:
 *        C is a vertex cover  <=>  V \ C is an independent set.
 *   Hence  min |vertex cover|  =  |V| - max |independent set|.  Because Maximum
 *   Independent Set is NP-hard, so is Minimum Vertex Cover.
 *
 * ALGORITHMS IMPLEMENTED:
 *   +-------------------------+-------------------------------+----------------+
 *   | Method                  | Time                          | Result         |
 *   +-------------------------+-------------------------------+----------------+
 *   | Brute force (reference) | O(2^n * m)                    | EXACT optimum  |
 *   | Branch & bound          | O(1.47^n) worst, much faster  | EXACT optimum  |
 *   | 2-approximation (match) | O(n + m)                      | <= 2 * optimum |
 *   +-------------------------+-------------------------------+----------------+
 *   n = |V|, m = |E|. The 2-approximation NEVER exceeds twice the optimum; on
 *   many instances it is strictly worse than optimal (it is only a heuristic).
 *
 * Complexity derivation (subset scan / branching recurrence / linear scan):
 *   (1) Brute force. The outer loop runs over every subset mask = 0 .. 2^n - 1,
 *       and for each mask the inner loop scans up to m edges, O(1) per edge:
 *
 *           C(n,m) = SUM_{mask=0}^{2^n - 1} (work to test one mask)
 *                  = SUM_{mask=0}^{2^n - 1} O(m)
 *                  = 2^n * O(m) = O(2^n * m).
 *
 *       (popcount of a feasible mask costs O(n) but is dominated by the m-edge
 *       scan, so it does not change the order.)
 *
 *   (2) Branch & bound (edge branching). For an uncovered edge (u,v) a cover must
 *       contain u OR v, a two-way branch. The refined worst-case recurrence
 *       (standard exact-VC analysis: take v, else take v's >= 2 other neighbours,
 *       after clearing degree-<=1 vertices by reduction) is
 *
 *           T(n) = T(n-1) + T(n-3) + O(m),      T(<=0) = O(1)   (base case)
 *
 *       Solve the homogeneous part via its characteristic equation x^3 = x^2 + 1,
 *       i.e. x^3 - x^2 - 1 = 0, whose real root is alpha ~= 1.4656. Unfolding the
 *       recursion tree, level d holds O(alpha^d) nodes, so
 *
 *           T(n) = O(alpha^n) = O(1.47^n).
 *
 *       NOTE: the plain two-way branch as literally coded, WITHOUT the degree
 *       reduction, has the weaker worst case T(n) = 2*T(n-1) + O(m) = O(2^n * m);
 *       O(1.47^n) is the refined bound the table cites.
 *
 *   (3) 2-approximation. One pass over the m edges (O(1) bit tests/sets each),
 *       then one pass over the n vertices to extract the cover:
 *
 *           C(n,m) = SUM_{e in E} O(1) + SUM_{i=0}^{n-1} O(1) = O(m) + O(n)
 *                  = O(n + m).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f = O(g)      iff  EXISTS c2, n0 :        f(n) <= c2*g(n)  for n >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   Per method:
 *     Brute force   visits all 2^n masks always => Theta(2^n) mask iterations;
 *                   with the O(m) edge scan the time is O(2^n * m).
 *     Branch&bound  data-dependent: best case (edgeless graph) finds no uncovered
 *                   edge at the root => Omega(m); worst case O(1.47^n). Overall it
 *                   is O(1.47^n) (worst) and Omega(m) (best), not a single Theta,
 *                   because pruning makes the cost input-dependent.
 *     2-approx      always one edge pass + one vertex pass => Theta(n + m) (tight).
 *   The comparison-sort lower bound Omega(n log n) does NOT apply here: this is a
 *   graph covering problem, not a comparison sort. The governing lower bound is the
 *   conjectured exponential hardness of the EXACT solution under P != NP.
 *
 * Key points:
 *   - Branching rule: for ANY still-uncovered edge (u,v), a cover MUST contain
 *     u or v -> recurse including u, then including v; keep the smaller cover.
 *   - The 2-approximation greedily builds a MAXIMAL MATCHING and takes BOTH
 *     endpoints of every matched edge. Any cover needs >= 1 vertex per matched
 *     edge, so optimum >= |matching|; we use 2*|matching| => ratio <= 2.
 *   - Subsets are encoded as bits of a std::uint32_t (vertex i <-> bit i).
 * ============================================================================
 */

#include <vector>
#include <utility>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <iostream>

// Simple undirected graph (no self-loops, no parallel edges assumed).
struct Graph {
    int n;                                      // vertices are 0 .. n-1
    std::vector<std::pair<int, int>> edges;     // each edge stored once
};

// popcount via Kernighan's trick: x &= x-1 clears the lowest set bit, so the
// loop runs exactly once per set bit -> counts the bits of the subset.
static int popcount32(std::uint32_t x) {
    int c = 0;
    while (x) { x &= (x - 1); ++c; }
    return c;
}

// ---------------------------------------------------------------------------
// Validity check: does the given vertex set touch every edge?
// ---------------------------------------------------------------------------
static bool isVertexCover(const Graph& g, const std::vector<int>& cover) {
    std::vector<char> in(static_cast<std::size_t>(g.n), 0);
    for (int v : cover) in[static_cast<std::size_t>(v)] = 1;
    for (const auto& e : g.edges)
        if (!in[static_cast<std::size_t>(e.first)] &&
            !in[static_cast<std::size_t>(e.second)])
            return false;                        // this edge is uncovered
    return true;
}

// ---------------------------------------------------------------------------
// EXACT solver: branch & bound.
//   cover  = bitmask of vertices currently chosen
//   count  = |cover|  (kept alongside to avoid recomputing popcount)
//   best / bestCover = smallest cover found so far.
// ---------------------------------------------------------------------------
static void vcBranch(const Graph& g, std::uint32_t cover, int count,
                     int& best, std::uint32_t& bestCover) {
    if (count >= best) return;                   // cannot beat the incumbent

    // Find one edge not yet covered by `cover`.
    int eu = -1, ev = -1;
    for (const auto& e : g.edges) {
        const bool cu = ((cover >> e.first) & 1u) != 0;
        const bool cv = ((cover >> e.second) & 1u) != 0;
        if (!cu && !cv) { eu = e.first; ev = e.second; break; }
    }

    if (eu == -1) {                              // all edges covered -> feasible
        best = count;
        bestCover = cover;
        return;
    }

    // A minimum cover must contain u or v. Try both branches.
    vcBranch(g, cover | (std::uint32_t{1} << eu), count + 1, best, bestCover);
    vcBranch(g, cover | (std::uint32_t{1} << ev), count + 1, best, bestCover);
}

// Returns a minimum vertex cover; writes its size into `size`.
static std::vector<int> minVertexCover(const Graph& g, int& size) {
    int best = g.n;                 // trivial upper bound: all n vertices cover E
    std::uint32_t bestCover = 0;
    vcBranch(g, 0u, 0, best, bestCover);
    size = best;
    std::vector<int> res;
    for (int i = 0; i < g.n; ++i)
        if (((bestCover >> i) & 1u) != 0) res.push_back(i);
    return res;
}

// ---------------------------------------------------------------------------
// Reference solver: exhaustive subset scan (only for small n).
// ---------------------------------------------------------------------------
static int minVertexCoverBrute(const Graph& g) {
    assert(g.n <= 20 && "brute force limited to small graphs");
    int best = g.n;
    const std::uint32_t total = std::uint32_t{1} << g.n;
    for (std::uint32_t mask = 0; mask < total; ++mask) {
        bool ok = true;
        for (const auto& e : g.edges) {
            const bool cu = ((mask >> e.first) & 1u) != 0;
            const bool cv = ((mask >> e.second) & 1u) != 0;
            if (!cu && !cv) { ok = false; break; }
        }
        if (ok) {
            const int c = popcount32(mask);
            if (c < best) best = c;
        }
    }
    return best;
}

// ---------------------------------------------------------------------------
// 2-APPROXIMATION: pick any uncovered edge, take BOTH endpoints, repeat.
// The chosen edges form a maximal matching (they are pairwise disjoint), so the
// returned set has size 2*|matching| and optimum >= |matching|  =>  ratio <= 2.
// ---------------------------------------------------------------------------
static std::vector<int> vertexCover2Approx(const Graph& g) {
    std::uint32_t cover = 0;
    for (const auto& e : g.edges) {
        const bool cu = ((cover >> e.first) & 1u) != 0;
        const bool cv = ((cover >> e.second) & 1u) != 0;
        if (!cu && !cv) {                        // uncovered => grab both ends
            cover |= (std::uint32_t{1} << e.first);
            cover |= (std::uint32_t{1} << e.second);
        }
    }
    std::vector<int> res;
    for (int i = 0; i < g.n; ++i)
        if (((cover >> i) & 1u) != 0) res.push_back(i);
    return res;
}

int main() {
    // ----- Graph 1: triangle K3 -> min cover = 2 -----------------------------
    Graph triangle{3, {{0, 1}, {1, 2}, {0, 2}}};
    {
        int sz = 0;
        const std::vector<int> cover = minVertexCover(triangle, sz);
        assert(sz == 2);
        assert(minVertexCoverBrute(triangle) == 2);
        assert(isVertexCover(triangle, cover));  // witness really covers all edges
        const std::vector<int> approx = vertexCover2Approx(triangle);
        assert(isVertexCover(triangle, approx));
        assert(static_cast<int>(approx.size()) <= 2 * sz);   // guarantee
    }

    // ----- Graph 2: path P4  0-1-2-3 -> min cover = 2 ({1,2}) ----------------
    Graph path{4, {{0, 1}, {1, 2}, {2, 3}}};
    {
        int sz = 0;
        const std::vector<int> cover = minVertexCover(path, sz);
        assert(sz == 2);
        assert(minVertexCoverBrute(path) == 2);
        assert(isVertexCover(path, cover));
        const std::vector<int> approx = vertexCover2Approx(path);
        assert(isVertexCover(path, approx));
        assert(static_cast<int>(approx.size()) <= 2 * sz);
    }

    // ----- Graph 3: star (center 0) -> min cover = 1 -------------------------
    Graph star{5, {{0, 1}, {0, 2}, {0, 3}, {0, 4}}};
    {
        int sz = 0;
        const std::vector<int> cover = minVertexCover(star, sz);
        assert(sz == 1);
        assert(cover.size() == 1 && cover[0] == 0);  // the hub is the whole cover
        assert(isVertexCover(star, cover));
        const std::vector<int> approx = vertexCover2Approx(star);
        // NOTE: the 2-approx takes BOTH endpoints of the first edge -> size 2,
        // strictly worse than the optimum 1 (still within the factor-2 bound).
        assert(isVertexCover(star, approx));
        assert(static_cast<int>(approx.size()) <= 2 * sz);
    }

    // ----- Graph 4: 4-cycle C4 -> min cover = 2 ------------------------------
    Graph cycle4{4, {{0, 1}, {1, 2}, {2, 3}, {3, 0}}};
    {
        int sz = 0;
        const std::vector<int> cover = minVertexCover(cycle4, sz);
        assert(sz == 2);
        assert(minVertexCoverBrute(cycle4) == 2);
        assert(isVertexCover(cycle4, cover));
    }

    // ----- Graph 5: edgeless graph -> min cover = 0 (edge case) --------------
    Graph empty{3, {}};
    {
        int sz = 99;
        const std::vector<int> cover = minVertexCover(empty, sz);
        assert(sz == 0 && cover.empty());
        assert(isVertexCover(empty, cover));
    }

    // ----- Demo output -------------------------------------------------------
    int sz = 0;
    const std::vector<int> cover = minVertexCover(path, sz);
    std::cout << "Minimum Vertex Cover demo (path 0-1-2-3)\n";
    std::cout << "  optimum size = " << sz << ", cover = {";
    for (std::size_t i = 0; i < cover.size(); ++i)
        std::cout << (i ? ", " : "") << cover[i];
    std::cout << "}\n";
    const std::vector<int> approx = vertexCover2Approx(path);
    std::cout << "  2-approx size = " << approx.size()
              << " (guaranteed <= " << 2 * sz << ")\n";
    std::cout << "All assertions passed.\n";
    return 0;
}
