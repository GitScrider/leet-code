/*
 * Floyd-Warshall All-Pairs Shortest Paths (Algorithm - Graphs)
 *
 * Idea:
 *   Compute the shortest distance between EVERY ordered pair of vertices in a
 *   single dynamic program. Let dist[i][j] be the shortest path from i to j.
 *   Consider vertices as potential "intermediate" stops, one at a time. After
 *   allowing the first k vertices (0..k-1) to be used as intermediates, we open
 *   up vertex k:
 *       dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j])
 *   Meaning: the best i->j path either avoids k (the old value) or routes
 *   through k, splitting into a best i->k path and a best k->j path that
 *   themselves use only intermediates < k. Processing k = 0,1,...,V-1 in the
 *   OUTERMOST loop makes this DP correct in place (the k-th layer only reads
 *   values already finalized for smaller intermediate sets).
 *
 * Complexity:
 *   +--------+-----------+
 *   | Metric |   Bound   |
 *   +--------+-----------+
 *   | Time   | O(V^3)    |   three nested loops over vertices
 *   | Space  | O(V^2)    |   the distance matrix
 *   +--------+-----------+
 *
 * Key points / assumptions:
 *   - Works on directed OR undirected graphs; handles negative edges.
 *   - Must NOT contain a negative cycle for distances to be meaningful. Such a
 *     cycle is detectable AFTER the DP: if any dist[i][i] < 0, vertex i lies on
 *     a negative cycle (a zero-length loop was improved below 0).
 *   - INF is a sentinel for "no path". We SKIP the relaxation whenever either
 *     dist[i][k] or dist[k][j] is INF, so INF + INF (or INF + w) never overflows.
 *   - The diagonal starts at 0 (distance from a vertex to itself).
 */

#include <vector>
#include <limits>
#include <cassert>
#include <iostream>
#include <cstddef>
#include <tuple>

// Sentinel for "no path". Kept far from LLONG_MAX; combined with the explicit
// INF guards below, arithmetic never overflows.
constexpr long long INF = std::numeric_limits<long long>::max() / 4;

using Matrix = std::vector<std::vector<long long>>;

// Runs Floyd-Warshall IN PLACE on a V x V distance matrix.
// Precondition: dist[i][i] = 0, dist[i][j] = edge weight i->j or INF if absent.
void floydWarshall(Matrix& dist) {
    const std::size_t V = dist.size();
    for (std::size_t k = 0; k < V; ++k) {
        for (std::size_t i = 0; i < V; ++i) {
            // If i cannot reach k at all, no j can benefit from routing i->k->j.
            if (dist[i][k] == INF) {
                continue;
            }
            for (std::size_t j = 0; j < V; ++j) {
                // Skip when k cannot reach j: prevents INF + (finite/INF) overflow
                // and correctly means "no i->k->j path through k".
                if (dist[k][j] == INF) {
                    continue;
                }
                const long long through = dist[i][k] + dist[k][j];
                if (through < dist[i][j]) {
                    dist[i][j] = through;
                }
            }
        }
    }
}

// Reports whether the (already-processed) matrix reveals a negative cycle:
// a vertex whose distance to itself dropped below zero.
bool hasNegativeCycle(const Matrix& dist) {
    for (std::size_t i = 0; i < dist.size(); ++i) {
        if (dist[i][i] < 0) {
            return true;
        }
    }
    return false;
}

// Convenience: build a V x V matrix initialized with 0 on the diagonal and INF
// elsewhere, then apply the given directed edges {from, to, weight}.
Matrix makeMatrix(std::size_t V,
                  const std::vector<std::tuple<int, int, long long>>& edges) {
    Matrix dist(V, std::vector<long long>(V, INF));
    for (std::size_t i = 0; i < V; ++i) {
        dist[i][i] = 0;
    }
    for (const auto& [u, v, w] : edges) {
        // Keep the smallest weight if parallel edges are supplied.
        if (w < dist[static_cast<std::size_t>(u)][static_cast<std::size_t>(v)]) {
            dist[static_cast<std::size_t>(u)][static_cast<std::size_t>(v)] = w;
        }
    }
    return dist;
}

// -------------------------- Tests & demo --------------------------

int main() {
    // Graph 1: directed, all positive weights, NO negative cycle. Vertices 0..3.
    // Chosen so that many shortest paths are INDIRECT (a multi-hop route beats
    // the direct edge), which exercises the DP rather than just reading edges.
    //
    //            0
    //          /   \
    //      (3)/      \(7)
    //        v        v
    //        1 --(2)--> 2 --(1)--> 3 --(2)--> 0   (back edges close the cycles)
    //        ^          |
    //        |          |
    //        +--(8)-----+  (1->0 = 8, 2->0 = 5)
    //
    // Explicit edge list:
    //   0->1 (3), 0->3 (7), 1->0 (8), 1->2 (2), 2->0 (5), 2->3 (1), 3->0 (2)
    //
    // Hand-verified shortest distances (each row is "from i"):
    //   from 0: 0->1=3, 0->1->2=5, 0->1->2->3=6            -> [0, 3, 5, 6]
    //   from 1: 1->2=2, 1->2->3=3, 1->2->3->0=5 (beats 8)  -> [5, 0, 2, 3]
    //   from 2: 2->3=1, 2->3->0=3 (beats 5), 2->3->0->1=6  -> [3, 6, 0, 1]
    //   from 3: 3->0=2, 3->0->1=5, 3->0->1->2=7            -> [2, 5, 7, 0]
    {
        Matrix dist = makeMatrix(4, {
            {0, 1, 3}, {0, 3, 7},
            {1, 0, 8}, {1, 2, 2},
            {2, 0, 5}, {2, 3, 1},
            {3, 0, 2},
        });
        floydWarshall(dist);
        assert(!hasNegativeCycle(dist));

        const Matrix expected = {
            {0, 3, 5, 6},
            {5, 0, 2, 3},
            {3, 6, 0, 1},
            {2, 5, 7, 0},
        };
        assert(dist == expected);
    }

    // Graph 2: disconnected. Two vertices reachable, one isolated.
    //   0 -> 1 (4). Vertex 2 has no edges.
    {
        Matrix dist = makeMatrix(3, {{0, 1, 4}});
        floydWarshall(dist);
        assert(dist[0][0] == 0 && dist[1][1] == 0 && dist[2][2] == 0);
        assert(dist[0][1] == 4);
        assert(dist[1][0] == INF);   // no way back
        assert(dist[0][2] == INF);   // vertex 2 isolated
        assert(dist[2][0] == INF);
        assert(!hasNegativeCycle(dist));
    }

    // Graph 3: single vertex, no edges. The 1x1 matrix stays [[0]].
    {
        Matrix dist = makeMatrix(1, {});
        floydWarshall(dist);
        assert(dist.size() == 1 && dist[0][0] == 0);
        assert(!hasNegativeCycle(dist));
    }

    // Graph 4: NEGATIVE CYCLE detection via a negative diagonal.
    //   0 -> 1 (1), 1 -> 2 (-1), 2 -> 0 (-1): cycle weight 1 - 1 - 1 = -1 < 0.
    //   After the DP some dist[i][i] must be negative.
    {
        Matrix dist = makeMatrix(3, {{0, 1, 1}, {1, 2, -1}, {2, 0, -1}});
        floydWarshall(dist);
        assert(hasNegativeCycle(dist));
    }

    // Short demo on Graph 1.
    Matrix demo = makeMatrix(4, {
        {0, 1, 3}, {0, 3, 7},
        {1, 0, 8}, {1, 2, 2},
        {2, 0, 5}, {2, 3, 1},
        {3, 0, 2},
    });
    floydWarshall(demo);
    std::cout << "Floyd-Warshall all-pairs shortest distances:\n     ";
    for (std::size_t j = 0; j < demo.size(); ++j) std::cout << "  to" << j;
    std::cout << '\n';
    for (std::size_t i = 0; i < demo.size(); ++i) {
        std::cout << "from" << i << ":";
        for (std::size_t j = 0; j < demo.size(); ++j) {
            std::cout << ' ';
            if (demo[i][j] == INF) std::cout << " INF";
            else                   std::cout << ' ' << demo[i][j] << ' ';
        }
        std::cout << '\n';
    }

    std::cout << "All Floyd-Warshall tests passed.\n";
    return 0;
}
