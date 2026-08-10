/*
 * Dijkstra's Shortest Paths (Algorithm - Graphs)
 *
 * Idea:
 *   Single-source shortest paths on a graph with NON-NEGATIVE edge weights.
 *   Maintain a tentative distance dist[v] for every vertex (dist[src] = 0,
 *   everything else = INF). Repeatedly pick the not-yet-finalized vertex u with
 *   the smallest tentative distance, declare dist[u] final, and RELAX every
 *   outgoing edge (u -> v, w): if dist[u] + w < dist[v], improve dist[v].
 *   A min-heap (std::priority_queue) supplies the smallest-distance vertex in
 *   O(log V). We use LAZY DELETION: instead of decreasing a key inside the heap
 *   (which the STL heap cannot do), we simply push the improved (dist, vertex)
 *   pair. A vertex may therefore sit in the heap several times; when we pop a
 *   pair whose stored distance is stale (greater than the current dist[u]) we
 *   skip it. A 'settled' guard also ensures each vertex is expanded once.
 *
 *   Correctness (greedy invariant): when u is popped with the minimum tentative
 *   distance, no not-yet-settled vertex can offer a shorter route into u,
 *   because all edge weights are >= 0 -- any alternative path would first leave
 *   the settled set through some frontier vertex whose distance is already >=
 *   dist[u], and adding non-negative weights cannot reduce it. Hence dist[u] is
 *   final the moment it is extracted.
 *
 * Complexity:
 *   +--------+--------------------+
 *   | Metric |       Bound        |
 *   +--------+--------------------+
 *   | Time   | O((V + E) log V)   |   each edge may push once; each push O(log V)
 *   | Space  | O(V + E)           |   adjacency list + dist[] + heap
 *   +--------+--------------------+
 *
 * Complexity derivation (heap-based single-source; count in V and E):
 *   Charge the work to graph elements. Each vertex is EXTRACTED as a fresh
 *   minimum (settled) exactly once; the 'settled' guard and the stale-key test
 *   (d > dist[u]) discard every duplicate pop in O(1). When a vertex u is
 *   settled its outgoing edges are scanned once, and a scan relaxes at most one
 *   edge -> at most one heap push. Counting a base "extract" per vertex plus one
 *   unit per outgoing edge:
 *
 *       Work_core = SUM_{u in V} (1 + outdeg(u))
 *                 = V + SUM_{u in V} outdeg(u)
 *                 = V + E                          (sum of out-degrees = E)
 *
 *   Every unit above touches the binary heap: settling u is a POP, a successful
 *   relaxation is a PUSH, and at most 1 + E pairs are ever pushed, so the heap
 *   holds <= E + 1 = O(E) entries. One heap operation therefore costs
 *   O(log(E+1)) = O(log V), because E <= V^2 => log E <= 2*log V. Multiplying
 *   the (V + E) operations by the per-operation heap cost:
 *
 *       T(V,E) = (V + E) * O(log V) = O((V + E) log V)
 *
 *   For a connected graph E >= V-1, so V + E = O(E) and this reads O(E log V);
 *   the explicit V term keeps the bound valid for sparse/disconnected graphs.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants; here the "size" is the
 *   pair (V, E) and g(V,E) = (V + E) log V):
 *     f = O(g)      iff  EXISTS c2, n0 :        f(V,E) <= c2*g(V,E)
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g(V,E) <= f(V,E)
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   Upper  O:     <= (1 + E) pushes and (1 + E) pops, each O(log V), plus the E
 *                 edge scans and V settles => f <= c2*(V + E)*log2 V
 *                 => T = O((V + E) log V).
 *   Lower  Omega: the source and every reachable vertex is popped (>= 1 heap op
 *                 of cost up to log V) and, in the worst case (dense graph where
 *                 every edge improves a distance) all E edges force a push =>
 *                 f >= c1*(V + E)*log2 V => T = Omega((V + E) log V) (worst case).
 *   Tight  Theta: the heap operations dominate and the worst case attains the
 *                 upper bound => Theta((V + E) log V) for this binary-heap form.
 *   This is a graph traversal, NOT a comparison sort, so the Omega(n log n)
 *   sorting lower bound does not apply; the log V factor comes solely from the
 *   heap. A Fibonacci-heap variant (O(1) amortized decrease-key) lowers this to
 *   O(E + V log V).
 *
 * Key points / assumptions:
 *   - Works on directed OR undirected graphs (store an undirected edge twice).
 *   - Edge weights MUST be non-negative. With a negative edge the greedy
 *     invariant breaks: a vertex settled early could later be reached more
 *     cheaply through a negative edge, but Dijkstra never revisits it (see the
 *     documented counterexample in main). Use Bellman-Ford for negative edges.
 *   - INF is a sentinel for "unreachable"; we never add w to INF, so no overflow.
 */

#include <vector>
#include <queue>
#include <functional>   // std::greater (heap comparator)
#include <utility>
#include <limits>
#include <cassert>
#include <iostream>
#include <cstddef>

// Weighted adjacency list: graph[u] holds {neighbor, weight} pairs.
using Edge  = std::pair<int, int>;      // {neighbor, weight}
using Graph = std::vector<std::vector<Edge>>;

// Sentinel for "no path". Chosen well below INT_MAX so comparisons are safe;
// we still guard every relaxation so INF is never used as an operand of '+'.
constexpr long long INF = std::numeric_limits<long long>::max() / 4;

// Returns the array of shortest distances from 'src' to every vertex.
// Unreachable vertices keep the value INF.
std::vector<long long> dijkstra(const Graph& graph, int src) {
    const std::size_t V = graph.size();
    std::vector<long long> dist(V, INF);
    std::vector<bool> settled(V, false);   // true once a vertex's distance is final

    // Min-heap of (distance, vertex). std::priority_queue is a max-heap by
    // default, so we use std::greater to pop the SMALLEST distance first.
    using State = std::pair<long long, int>;   // {distance, vertex}
    std::priority_queue<State, std::vector<State>, std::greater<State>> pq;

    dist[src] = 0;
    pq.push({0, src});

    while (!pq.empty()) {
        const auto [d, u] = pq.top();
        pq.pop();

        // Lazy deletion: this pair is a stale leftover (a better distance for u
        // was found after this pair was pushed), or u is already finalized.
        if (settled[u] || d > dist[u]) {
            continue;
        }
        settled[u] = true;                 // dist[u] is now final

        // Relax every outgoing edge of u.
        for (const auto& [v, w] : graph[static_cast<std::size_t>(u)]) {
            // dist[u] is finite here (u was reached), so dist[u] + w is safe.
            if (!settled[v] && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});     // push improved key; old one becomes stale
            }
        }
    }
    return dist;
}

// -------------------------- Tests & demo --------------------------

int main() {
    // Graph 1 (directed, non-negative weights). Vertices 0..4.
    //
    //          (10)
    //      0 ---------> 1
    //      |            |
    //   (3)|            |(2)
    //      v            v
    //      2 ---------> 3 ---> 4        edges: 2->3 (4), 3->4 (7)
    //          (4)                      plus a shortcut 2->1 (1)
    //
    // Full edge list:
    //   0->1 (10), 0->2 (3), 2->1 (1), 2->3 (4), 1->3 (2), 3->4 (7)
    // Hand-computed shortest distances from source 0:
    //   0: 0
    //   1: 0->2->1        = 3 + 1     = 4   (beats direct 0->1 = 10)
    //   2: 0->2           = 3
    //   3: 0->2->1->3     = 3 + 1 + 2 = 6   (beats 0->2->3 = 7)
    //   4: 0->2->1->3->4  = 6 + 7     = 13
    {
        Graph g(5);
        g[0] = {{1, 10}, {2, 3}};
        g[2] = {{1, 1}, {3, 4}};
        g[1] = {{3, 2}};
        g[3] = {{4, 7}};

        const std::vector<long long> dist = dijkstra(g, 0);
        const std::vector<long long> expected = {0, 4, 3, 6, 13};
        assert(dist == expected);
    }

    // Graph 2: disconnected. Vertex 3 has no incoming edge from {0,1,2}.
    //   0 -> 1 (5), 1 -> 2 (5).  Vertex 3 is isolated => unreachable.
    {
        Graph g(4);
        g[0] = {{1, 5}};
        g[1] = {{2, 5}};
        // g[3] intentionally empty and unreachable.
        const std::vector<long long> dist = dijkstra(g, 0);
        assert(dist[0] == 0);
        assert(dist[1] == 5);
        assert(dist[2] == 10);
        assert(dist[3] == INF);   // unreachable stays at the sentinel
    }

    // Graph 3: single vertex, no edges. Distance to itself is 0.
    {
        Graph g(1);
        const std::vector<long long> dist = dijkstra(g, 0);
        assert(dist.size() == 1 && dist[0] == 0);
    }

    // Graph 4: undirected triangle (store each edge in both directions).
    //   0 --(1)-- 1 --(2)-- 2 --(4)-- 0
    //   From 0:  d[1] = 1, d[2] = min(4, 1+2) = 3.
    {
        Graph g(3);
        auto addUndirected = [&](int a, int b, int w) {
            g[a].push_back({b, w});
            g[b].push_back({a, w});
        };
        addUndirected(0, 1, 1);
        addUndirected(1, 2, 2);
        addUndirected(0, 2, 4);
        const std::vector<long long> dist = dijkstra(g, 0);
        const std::vector<long long> expected = {0, 1, 3};
        assert(dist == expected);
    }

    // WHY DIJKSTRA FAILS WITH NEGATIVE EDGES (illustrative, not asserted as the
    // "true" answer -- it shows the WRONG result Dijkstra would produce):
    //   0 -> 1 (2), 0 -> 2 (5), 1 -> 2 (-4)
    //   True shortest to 2 is 0->1->2 = 2 + (-4) = -2.
    //   But Dijkstra settles vertex 2 with dist 5 as soon as it is the heap min
    //   (it is popped before the improving edge 1->2 is relaxed in some orders),
    //   or more precisely it can finalize a vertex before a later negative edge
    //   would lower it. The non-negativity assumption is exactly what guarantees
    //   a settled distance is never improved again. Use Bellman-Ford instead.

    // Short demo.
    Graph demo(5);
    demo[0] = {{1, 10}, {2, 3}};
    demo[2] = {{1, 1}, {3, 4}};
    demo[1] = {{3, 2}};
    demo[3] = {{4, 7}};
    const std::vector<long long> d = dijkstra(demo, 0);
    std::cout << "Dijkstra distances from vertex 0:\n";
    for (std::size_t v = 0; v < d.size(); ++v) {
        std::cout << "  0 -> " << v << " = ";
        if (d[v] == INF) std::cout << "INF (unreachable)";
        else             std::cout << d[v];
        std::cout << '\n';
    }

    std::cout << "All Dijkstra tests passed.\n";
    return 0;
}
