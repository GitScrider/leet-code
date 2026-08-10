/*
 * ============================================================================
 * Hamiltonian Path & Hamiltonian Cycle
 * Category: Algorithm - NP-Complete
 *
 * DECISION PROBLEM:
 *   Given an undirected graph G = (V, E):
 *     - Hamiltonian PATH : is there a path that visits every vertex exactly
 *       once? (The two endpoints need not be adjacent.)
 *     - Hamiltonian CYCLE: is there a cycle that visits every vertex exactly
 *       once and returns to its start? (Needs |V| >= 3 to be a simple cycle.)
 *
 * COMPLEXITY CLASS:
 *   Both are NP-complete. Karp (1972) listed Hamiltonian Cycle among his 21
 *   NP-complete problems, reduced from Vertex Cover / 3-SAT; Hamiltonian Path
 *   reduces to and from it by simple gadget constructions. Note the sharp
 *   contrast with EULERIAN paths (use every EDGE once), which are decidable in
 *   linear time by a degree parity check -- "every vertex once" is the feature
 *   that makes the problem intractable.
 *
 * EXACT ALGORITHM (backtracking DFS that grows a partial path):
 *   +-----------------------+-------------------------------------------------+
 *   | Aspect                | Cost                                            |
 *   +-----------------------+-------------------------------------------------+
 *   | Time (worst case)     | O(n!) -- bounded by the number of orderings     |
 *   | Space                 | O(n) recursion + O(n) visited/path state        |
 *   +-----------------------+-------------------------------------------------+
 *   No approximation applies: these are yes/no decision problems, so there is
 *   nothing to approximate. (A faster O(2^n * n^2) Held-Karp-style DP exists,
 *   but backtracking is the clearest teaching form and prunes aggressively.)
 *
 * STATE SPACE & PRUNING:
 *   We extend a partial path one vertex at a time. From the current endpoint we
 *   may only step to an UNVISITED, ADJACENT vertex -- that adjacency test is the
 *   pruning that collapses the n! ordering tree to the reachable sub-tree. When
 *   the path length hits n we have a Hamiltonian path; for a cycle we then also
 *   require an edge from the last vertex back to the first.
 *
 * KEY POINTS:
 *   - Graph stored as a symmetric adjacency matrix (char 0/1) for O(1) edge tests.
 *   - Hamiltonian PATH may start anywhere, so we try every start vertex. A
 *     CYCLE is rotation-symmetric, so fixing the start at vertex 0 loses nothing.
 *   - Every returned witness is validated (a permutation of all vertices with
 *     each consecutive pair adjacent, plus the closing edge for a cycle) and the
 *     yes/no answer is cross-checked against a brute-force permutation oracle.
 * ============================================================================
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>

// Undirected graph as a symmetric adjacency matrix; g[u][v] == 1 iff edge u-v.
using Graph = std::vector<std::vector<char>>;

// Build a graph on n vertices from an undirected edge list.
static Graph makeGraph(int n, const std::vector<std::pair<int, int>>& edges) {
    Graph g(static_cast<std::size_t>(n),
            std::vector<char>(static_cast<std::size_t>(n), 0));
    for (const auto& e : edges) {
        g[static_cast<std::size_t>(e.first)][static_cast<std::size_t>(e.second)] = 1;
        g[static_cast<std::size_t>(e.second)][static_cast<std::size_t>(e.first)] = 1;
    }
    return g;
}

// Recursively try to extend `path` to cover all n vertices. `requireCycle`
// additionally demands an edge from the final vertex back to the first.
static bool extend(const Graph& g, std::vector<int>& path,
                   std::vector<char>& visited, int n, bool requireCycle) {
    if (static_cast<int>(path.size()) == n) {          // all vertices used
        if (!requireCycle) return true;                // a Hamiltonian PATH
        // A Hamiltonian CYCLE also needs to close back onto the start.
        return g[static_cast<std::size_t>(path.back())]
                [static_cast<std::size_t>(path.front())] != 0;
    }
    const int last = path.back();
    for (int v = 0; v < n; ++v) {
        if (visited[static_cast<std::size_t>(v)]) continue;      // no revisits
        if (!g[static_cast<std::size_t>(last)][static_cast<std::size_t>(v)])
            continue;                                            // must be adjacent
        // CHOOSE
        visited[static_cast<std::size_t>(v)] = 1;
        path.push_back(v);
        // EXPLORE
        if (extend(g, path, visited, n, requireCycle)) return true;
        // UNCHOOSE (backtrack)
        path.pop_back();
        visited[static_cast<std::size_t>(v)] = 0;
    }
    return false;
}

// Return a Hamiltonian path (as a vertex order) or an empty vector if none.
// A path may start at any vertex, so we try each possible start.
static std::vector<int> findHamiltonianPath(const Graph& g) {
    const int n = static_cast<int>(g.size());
    if (n == 0) return {};
    for (int start = 0; start < n; ++start) {
        std::vector<int> path{start};
        std::vector<char> visited(static_cast<std::size_t>(n), 0);
        visited[static_cast<std::size_t>(start)] = 1;
        if (extend(g, path, visited, n, false)) return path;
    }
    return {};
}

// Return a Hamiltonian cycle (vertex order; the closing edge back to the first
// is implicit) or an empty vector if none. A simple cycle needs n >= 3, and
// cycles are rotation-symmetric so we may fix the start at vertex 0.
static std::vector<int> findHamiltonianCycle(const Graph& g) {
    const int n = static_cast<int>(g.size());
    if (n < 3) return {};
    std::vector<int> path{0};
    std::vector<char> visited(static_cast<std::size_t>(n), 0);
    visited[0] = 1;
    if (extend(g, path, visited, n, true)) return path;
    return {};
}

// Validate a Hamiltonian path witness: right length, a permutation of all
// vertices, and every consecutive pair adjacent.
static bool isHamiltonianPath(const Graph& g, const std::vector<int>& seq) {
    const int n = static_cast<int>(g.size());
    if (n == 0 || static_cast<int>(seq.size()) != n) return false;
    std::vector<char> seen(static_cast<std::size_t>(n), 0);
    for (int v : seq) {
        if (v < 0 || v >= n) return false;
        if (seen[static_cast<std::size_t>(v)]) return false;      // duplicate
        seen[static_cast<std::size_t>(v)] = 1;
    }
    for (std::size_t k = 0; k + 1 < seq.size(); ++k)
        if (!g[static_cast<std::size_t>(seq[k])]
              [static_cast<std::size_t>(seq[k + 1])])
            return false;
    return true;
}

// Validate a Hamiltonian cycle witness: a valid Hamiltonian path (n >= 3) whose
// last vertex is also adjacent to the first (the closing edge).
static bool isHamiltonianCycle(const Graph& g, const std::vector<int>& seq) {
    if (static_cast<int>(g.size()) < 3) return false;
    if (!isHamiltonianPath(g, seq)) return false;
    return g[static_cast<std::size_t>(seq.back())]
            [static_cast<std::size_t>(seq.front())] != 0;
}

// Brute-force oracles: scan every vertex ordering. Exponential (O(n!)); used
// only to certify the backtracking finders' yes/no answers on small graphs.
static bool bruteHasPath(const Graph& g) {
    const int n = static_cast<int>(g.size());
    if (n == 0) return false;
    std::vector<int> perm(static_cast<std::size_t>(n));
    std::iota(perm.begin(), perm.end(), 0);
    do {
        bool ok = true;
        for (int k = 0; k + 1 < n && ok; ++k)
            if (!g[static_cast<std::size_t>(perm[static_cast<std::size_t>(k)])]
                  [static_cast<std::size_t>(perm[static_cast<std::size_t>(k + 1)])])
                ok = false;
        if (ok) return true;
    } while (std::next_permutation(perm.begin(), perm.end()));
    return false;
}

static bool bruteHasCycle(const Graph& g) {
    const int n = static_cast<int>(g.size());
    if (n < 3) return false;
    std::vector<int> perm(static_cast<std::size_t>(n));
    std::iota(perm.begin(), perm.end(), 0);
    do {
        bool ok = true;
        for (int k = 0; k + 1 < n && ok; ++k)
            if (!g[static_cast<std::size_t>(perm[static_cast<std::size_t>(k)])]
                  [static_cast<std::size_t>(perm[static_cast<std::size_t>(k + 1)])])
                ok = false;
        if (ok && g[static_cast<std::size_t>(perm[static_cast<std::size_t>(n - 1)])]
                   [static_cast<std::size_t>(perm[0])])
            return true;
    } while (std::next_permutation(perm.begin(), perm.end()));
    return false;
}

// --- Standard graph families used in the tests. ---
static Graph cycleGraph(int n) {          // C_n: 0-1-...-(n-1)-0
    std::vector<std::pair<int, int>> e;
    for (int i = 0; i < n; ++i) e.push_back({i, (i + 1) % n});
    return makeGraph(n, e);
}
static Graph pathGraph(int n) {           // P_n: 0-1-...-(n-1)
    std::vector<std::pair<int, int>> e;
    for (int i = 0; i + 1 < n; ++i) e.push_back({i, i + 1});
    return makeGraph(n, e);
}
static Graph completeGraph(int n) {       // K_n: every pair adjacent
    std::vector<std::pair<int, int>> e;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) e.push_back({i, j});
    return makeGraph(n, e);
}

int main() {
    // --- Cycle graph C_5: has BOTH a Hamiltonian path and cycle. ---
    {
        const Graph c5 = cycleGraph(5);
        const std::vector<int> path = findHamiltonianPath(c5);
        const std::vector<int> cyc = findHamiltonianCycle(c5);
        assert(!path.empty() && isHamiltonianPath(c5, path));
        assert(!cyc.empty() && isHamiltonianCycle(c5, cyc));
        assert(bruteHasPath(c5) && bruteHasCycle(c5));
    }

    // --- Path graph P_5: has a Hamiltonian path but NO Hamiltonian cycle
    //     (its two endpoints have degree 1, so no cycle can close). ---
    {
        const Graph p5 = pathGraph(5);
        const std::vector<int> path = findHamiltonianPath(p5);
        assert(!path.empty() && isHamiltonianPath(p5, path));
        assert(findHamiltonianCycle(p5).empty());  // none exists
        assert(bruteHasPath(p5) && !bruteHasCycle(p5));
    }

    // --- Star K_{1,3}: center 0 joined to leaves 1,2,3. Has NEITHER: any walk
    //     must pass back through the center to reach a second leaf. ---
    {
        const Graph star = makeGraph(4, {{0, 1}, {0, 2}, {0, 3}});
        assert(findHamiltonianPath(star).empty());
        assert(findHamiltonianCycle(star).empty());
        assert(!bruteHasPath(star) && !bruteHasCycle(star));
    }

    // --- Complete graph K_4: has BOTH (every ordering is a valid path). ---
    {
        const Graph k4 = completeGraph(4);
        const std::vector<int> path = findHamiltonianPath(k4);
        const std::vector<int> cyc = findHamiltonianCycle(k4);
        assert(!path.empty() && isHamiltonianPath(k4, path));
        assert(!cyc.empty() && isHamiltonianCycle(k4, cyc));
        assert(bruteHasPath(k4) && bruteHasCycle(k4));
    }

    // --- Edge cases on tiny graphs. ---
    {
        const Graph one = makeGraph(1, {});   // single vertex
        assert(isHamiltonianPath(one, findHamiltonianPath(one)));  // trivial path
        assert(findHamiltonianCycle(one).empty());                 // no cycle (<3)

        const Graph two = makeGraph(2, {{0, 1}}); // one edge
        assert(!findHamiltonianPath(two).empty());
        assert(findHamiltonianCycle(two).empty());                 // no cycle (<3)

        const Graph disc = makeGraph(3, {{0, 1}}); // vertex 2 isolated
        assert(findHamiltonianPath(disc).empty()); // cannot reach vertex 2
        assert(findHamiltonianCycle(disc).empty());
    }

    // --- Cross-check the backtracking finders against the oracle on assorted
    //     graphs; where a witness is returned, confirm it is genuinely valid. ---
    {
        const std::vector<Graph> suite = {
            cycleGraph(4), pathGraph(4), completeGraph(5),
            makeGraph(5, {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 0}, {1, 3}}),
            makeGraph(4, {{0, 1}, {1, 2}, {0, 2}}),   // triangle + isolated vertex 3
            makeGraph(5, {{0, 1}, {0, 2}, {0, 3}, {0, 4}}), // star K_{1,4}
        };
        for (const Graph& g : suite) {
            const std::vector<int> path = findHamiltonianPath(g);
            const std::vector<int> cyc = findHamiltonianCycle(g);
            assert(path.empty() != bruteHasPath(g));   // existence must agree
            assert(cyc.empty() != bruteHasCycle(g));
            if (!path.empty()) assert(isHamiltonianPath(g, path));
            if (!cyc.empty()) assert(isHamiltonianCycle(g, cyc));
        }
    }

    // --- Short demo. ---
    const Graph c5 = cycleGraph(5);
    const std::vector<int> cyc = findHamiltonianCycle(c5);
    std::cout << "Hamiltonian cycle in C_5: ";
    for (std::size_t k = 0; k < cyc.size(); ++k) std::cout << cyc[k] << " -> ";
    std::cout << cyc.front() << "\n";
    std::cout << "Path graph P_5 has a Hamiltonian cycle? "
              << (findHamiltonianCycle(pathGraph(5)).empty() ? "no" : "yes") << '\n';
    std::cout << "All Hamiltonian path/cycle tests passed.\n";
    return 0;
}
