/*
 * Bellman-Ford Shortest Paths (Algorithm - Graphs)
 *
 * Idea:
 *   Single-source shortest paths that ALLOW negative edge weights. Start with
 *   dist[src] = 0 and all others = INF, then RELAX every edge V-1 times:
 *       if dist[u] + w < dist[v]  then  dist[v] = dist[u] + w
 *   Why V-1 rounds suffice: any shortest path in a graph with no negative cycle
 *   is simple (visits each vertex at most once), so it uses at most V-1 edges.
 *   Claim (induction on k): after k full passes, dist[v] is correct for every
 *   vertex whose shortest path uses at most k edges. Pass k+1 extends every such
 *   path by one more edge, so after V-1 passes all shortest paths are found.
 *
 *   NEGATIVE-CYCLE DETECTION: run ONE more relaxation pass. If any edge can
 *   still be relaxed, some path keeps getting cheaper without bound -- a
 *   negative cycle is reachable from the source. Shortest distances are then
 *   undefined (they diverge to -INF around the cycle), so we report it.
 *
 * Complexity:
 *   +--------+-----------+
 *   | Metric |   Bound   |
 *   +--------+-----------+
 *   | Time   | O(V * E)  |   V-1 passes, each scanning all E edges
 *   | Space  | O(V + E)  |   edge list + dist[]
 *   +--------+-----------+
 *
 * Key points / assumptions:
 *   - Works on directed graphs; an undirected negative edge is itself a negative
 *     2-cycle, so undirected + negative weight is generally ill-posed.
 *   - Handles negative edges (unlike Dijkstra) and DETECTS negative cycles.
 *   - Slower than Dijkstra; use it only when negative weights are possible.
 *   - INF is a sentinel for unreachable; we SKIP relaxing from a vertex whose
 *     dist is still INF so that INF + w never overflows.
 */

#include <vector>
#include <utility>
#include <limits>
#include <cassert>
#include <iostream>
#include <cstddef>

// Directed edge list -- the natural representation for Bellman-Ford, which
// simply scans every edge each pass.
struct Edge {
    int from;
    int to;
    long long weight;
};

// Sentinel for "unreachable". Kept far from LLONG_MAX so that ordinary
// comparisons never overflow; we additionally guard against relaxing from INF.
constexpr long long INF = std::numeric_limits<long long>::max() / 4;

// Result of a run: the distance array plus whether a reachable negative cycle
// was detected. When hasNegativeCycle is true, 'dist' is not meaningful.
struct BellmanFordResult {
    std::vector<long long> dist;
    bool hasNegativeCycle;
};

BellmanFordResult bellmanFord(std::size_t V, const std::vector<Edge>& edges, int src) {
    std::vector<long long> dist(V, INF);
    dist[static_cast<std::size_t>(src)] = 0;

    // Relax all edges V-1 times.
    for (std::size_t pass = 0; pass + 1 < V; ++pass) {
        bool changed = false;
        for (const Edge& e : edges) {
            // Skip if 'from' is unreachable so far: INF + weight would overflow
            // and is meaningless (you cannot extend a path that doesn't exist).
            if (dist[static_cast<std::size_t>(e.from)] == INF) {
                continue;
            }
            const long long cand = dist[static_cast<std::size_t>(e.from)] + e.weight;
            if (cand < dist[static_cast<std::size_t>(e.to)]) {
                dist[static_cast<std::size_t>(e.to)] = cand;
                changed = true;
            }
        }
        // Optimization: if a full pass changes nothing, distances have converged
        // early and further passes are pointless.
        if (!changed) {
            break;
        }
    }

    // One extra pass: any successful relaxation now proves a reachable negative
    // cycle (a simple shortest path can improve for at most V-1 passes).
    bool hasNegativeCycle = false;
    for (const Edge& e : edges) {
        if (dist[static_cast<std::size_t>(e.from)] == INF) {
            continue;
        }
        if (dist[static_cast<std::size_t>(e.from)] + e.weight
                < dist[static_cast<std::size_t>(e.to)]) {
            hasNegativeCycle = true;
            break;
        }
    }

    return {dist, hasNegativeCycle};
}

// -------------------------- Tests & demo --------------------------

int main() {
    // Graph 1: negative edge but NO negative cycle (directed). Vertices 0..4.
    //
    //      0 --(-1)--> 1 --(3)--> 2
    //      |           |          ^
    //   (4)|        (2)|          |(5)   also 1 --(2)--> 3, 3 --(-3)--> 2 wait...
    //      v           v
    //      2 <-- ...   3 --(1)--> 4
    //
    // Explicit edge list (classic CLRS-style example):
    //   0->1 (-1), 0->2 (4), 1->2 (3), 1->3 (2), 1->4 (2),
    //   3->2 (5),  3->1 (1), 4->3 (-3)
    // Hand-computed shortest distances from source 0:
    //   0: 0
    //   1: 0->1                 = -1
    //   2: 0->1->2              = -1 + 3 = 2
    //   3: 0->1->4->3           = -1 + 2 + (-3) = -2
    //   4: 0->1->4              = -1 + 2 = 1
    {
        const std::vector<Edge> edges = {
            {0, 1, -1}, {0, 2, 4},
            {1, 2, 3},  {1, 3, 2}, {1, 4, 2},
            {3, 2, 5},  {3, 1, 1},
            {4, 3, -3},
        };
        const auto res = bellmanFord(5, edges, 0);
        assert(!res.hasNegativeCycle);
        const std::vector<long long> expected = {0, -1, 2, -2, 1};
        assert(res.dist == expected);
    }

    // Graph 2: contains a NEGATIVE CYCLE reachable from the source.
    //   0 -> 1 (1), 1 -> 2 (-1), 2 -> 3 (-1), 3 -> 1 (-1)
    //   The cycle 1 -> 2 -> 3 -> 1 has total weight -1 - 1 - 1 = -3 < 0.
    //   Distances diverge, so detection must trigger.
    {
        const std::vector<Edge> edges = {
            {0, 1, 1}, {1, 2, -1}, {2, 3, -1}, {3, 1, -1},
        };
        const auto res = bellmanFord(4, edges, 0);
        assert(res.hasNegativeCycle);
    }

    // Graph 3: a negative cycle that is NOT reachable from the source must NOT
    // be reported (Bellman-Ford only flags cycles reachable from src).
    //   Reachable part:  0 -> 1 (2).
    //   Separate component with a negative cycle: 2 -> 3 (-1), 3 -> 2 (-1).
    {
        const std::vector<Edge> edges = {
            {0, 1, 2}, {2, 3, -1}, {3, 2, -1},
        };
        const auto res = bellmanFord(4, edges, 0);
        assert(!res.hasNegativeCycle);      // the negative cycle is unreachable
        assert(res.dist[0] == 0);
        assert(res.dist[1] == 2);
        assert(res.dist[2] == INF);         // component {2,3} unreachable from 0
        assert(res.dist[3] == INF);
    }

    // Graph 4: disconnected with a single source, plus a single-vertex edge case.
    {
        // Single vertex, no edges: distance to itself is 0.
        const auto res1 = bellmanFord(1, {}, 0);
        assert(!res1.hasNegativeCycle);
        assert(res1.dist.size() == 1 && res1.dist[0] == 0);

        // Two vertices, no edge between them: vertex 1 unreachable.
        const auto res2 = bellmanFord(2, {}, 0);
        assert(res2.dist[0] == 0);
        assert(res2.dist[1] == INF);
    }

    // Short demo on Graph 1.
    const std::vector<Edge> demo = {
        {0, 1, -1}, {0, 2, 4},
        {1, 2, 3},  {1, 3, 2}, {1, 4, 2},
        {3, 2, 5},  {3, 1, 1},
        {4, 3, -3},
    };
    const auto res = bellmanFord(5, demo, 0);
    std::cout << "Bellman-Ford distances from vertex 0"
              << (res.hasNegativeCycle ? " (NEGATIVE CYCLE!)" : "") << ":\n";
    for (std::size_t v = 0; v < res.dist.size(); ++v) {
        std::cout << "  0 -> " << v << " = ";
        if (res.dist[v] == INF) std::cout << "INF (unreachable)";
        else                    std::cout << res.dist[v];
        std::cout << '\n';
    }

    std::cout << "All Bellman-Ford tests passed.\n";
    return 0;
}
