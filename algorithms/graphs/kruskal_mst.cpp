/*
 * Kruskal's Minimum Spanning Tree (Algorithm - Graphs)
 *
 * Idea:
 *   A Minimum Spanning Tree (MST) of a connected, weighted, UNDIRECTED graph is
 *   a subset of edges that connects all V vertices with no cycles and minimum
 *   total weight (exactly V-1 edges). Kruskal is a GLOBAL greedy: consider all
 *   edges sorted by ascending weight and add an edge to the tree iff its two
 *   endpoints currently lie in DIFFERENT components (adding it would not create a
 *   cycle). Component membership is tracked with a Disjoint Set Union (Union-Find)
 *   structure: find(x) returns the representative of x's component; union merges
 *   two components. After processing, if we accepted V-1 edges we have an MST.
 *
 *   Why the greedy is safe (the CUT PROPERTY):
 *     For any partition of the vertices into two non-empty sets (a "cut"), the
 *     lightest edge crossing the cut belongs to SOME MST. When Kruskal is about
 *     to add the next-cheapest edge e = (u, v) with u, v in different components,
 *     consider the cut that separates u's current component from everything else.
 *     Every edge already accepted stays inside a component, so it does NOT cross
 *     this cut; therefore e is the lightest edge crossing it that we have seen,
 *     and because we process edges in ascending order no cheaper crossing edge
 *     exists. By the cut property e is safe to add. Rejected edges join two
 *     vertices already in the same component -- adding them would form a cycle.
 *
 * Complexity (V vertices, E edges):
 *   +-----------+----------------------+
 *   |  Step     |  Time                |
 *   +-----------+----------------------+
 *   |  Sort     |  O(E log E)          |
 *   |  DSU ops  |  O(E * alpha(V))     |  (near-constant per op)
 *   |  Total    |  O(E log E)          |  = O(E log V) since E < V^2
 *   +-----------+----------------------+
 *   Space: O(V + E)  (parent/rank arrays + the edge list)
 *
 * Key points / assumptions:
 *   - Graph is UNDIRECTED and weighted; each undirected edge is stored once.
 *   - Weights may be negative; Kruskal only relies on relative ordering.
 *   - Edge-centric: shines on SPARSE graphs given as an edge list. (Contrast:
 *     Prim is vertex-centric and grows one tree from a start vertex.)
 *   - If fewer than V-1 edges are accepted the graph is disconnected: there is
 *     no spanning tree, only a minimum spanning FOREST.
 */

#include <vector>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <utility>   // std::swap
#include <cstddef>

// ------------------------- Union-Find (DSU) -------------------------
// Maintains a partition of {0..V-1} into disjoint sets. Two optimizations keep
// operations effectively O(alpha(V)) ~ O(1):
//   * union by RANK: attach the shorter tree under the taller one so trees stay
//     shallow (rank is an upper bound on tree height).
//   * PATH COMPRESSION: during find, re-point visited nodes straight at the root
//     so future queries are flat.
struct DisjointSet {
    std::vector<int> parent;   // parent[i] == i means i is a root (representative)
    std::vector<int> rank;     // rank[i] = height upper bound of the tree at root i

    explicit DisjointSet(std::size_t n) : parent(n), rank(n, 0) {
        // Each element starts in its own singleton set: parent[i] = i.
        std::iota(parent.begin(), parent.end(), 0);
    }

    // Return the representative of x's set, compressing the path on the way up.
    int find(int x) {
        while (parent[x] != x) {
            parent[x] = parent[parent[x]];  // path halving: point x at its grandparent
            x = parent[x];
        }
        return x;
    }

    // Merge the sets containing a and b. Returns true if they were distinct (a
    // real merge happened), false if they were already in the same set.
    bool unite(int a, int b) {
        int ra = find(a);
        int rb = find(b);
        if (ra == rb) {
            return false;                   // same component -> would form a cycle
        }
        // Union by rank: hang the lower-rank root beneath the higher-rank one.
        if (rank[ra] < rank[rb]) {
            std::swap(ra, rb);
        }
        parent[rb] = ra;
        if (rank[ra] == rank[rb]) {
            ++rank[ra];                     // equal ranks -> combined tree grows by 1
        }
        return true;
    }
};

// A weighted undirected edge.
struct Edge {
    int u;
    int v;
    int w;   // weight
};

// Result of a Kruskal run.
struct MstResult {
    int totalWeight = 0;             // sum of chosen edge weights
    std::vector<Edge> chosen;        // the MST edges, in the order they were added
    bool connected = false;          // true iff we accepted exactly V-1 edges
};

// Compute an MST (or minimum spanning forest) of a graph on 'vertexCount'
// vertices described by an edge list. Vertices are ints 0..vertexCount-1.
MstResult kruskalMst(std::size_t vertexCount, std::vector<Edge> edges) {
    // Greedy order: process edges from lightest to heaviest.
    std::sort(edges.begin(), edges.end(),
              [](const Edge& a, const Edge& b) { return a.w < b.w; });

    DisjointSet dsu(vertexCount);
    MstResult result;
    result.chosen.reserve(vertexCount ? vertexCount - 1 : 0);

    for (const Edge& e : edges) {
        // Accept the edge iff it links two different components (no cycle).
        if (dsu.unite(e.u, e.v)) {
            result.chosen.push_back(e);
            result.totalWeight += e.w;
            // A spanning tree of V vertices has exactly V-1 edges; stop early.
            if (result.chosen.size() + 1 == vertexCount) {
                break;
            }
        }
    }

    // Connected iff a full spanning tree (V-1 edges) was formed. A single vertex
    // (V == 1) needs 0 edges and is trivially connected.
    result.connected =
        (vertexCount <= 1) || (result.chosen.size() + 1 == vertexCount);
    return result;
}

// ------------------------------ Tests & demo ------------------------------

int main() {
    // Graph A (connected, 5 vertices). Weights on each undirected edge.
    //
    //         1
    //     (0)---(1)
    //      | \    |
    //     4|  3\  |2
    //      |    \ |
    //     (3)---(2)
    //      |  5   \
    //     6|       \7
    //      |        \
    //     (4)--------+
    //   (edge 2-4 has weight 7, edge 3-4 has weight 6)
    //
    // Edges: 0-1:1, 0-2:3, 0-3:4, 1-2:2, 2-3:5, 3-4:6, 2-4:7
    // Kruskal picks (ascending): 0-1(1), 1-2(2), 0-2(3 -> cycle, reject),
    //   0-3(4), then need to reach vertex 4: 3-4(6) chosen over 2-4(7).
    // MST edges: {0-1, 1-2, 0-3, 3-4}, total = 1+2+4+6 = 13, count = 4 (= V-1).
    {
        const std::size_t V = 5;
        std::vector<Edge> edges = {
            {0, 1, 1}, {0, 2, 3}, {0, 3, 4},
            {1, 2, 2}, {2, 3, 5}, {3, 4, 6}, {2, 4, 7}
        };
        MstResult r = kruskalMst(V, edges);
        assert(r.connected);
        assert(r.totalWeight == 13);
        assert(r.chosen.size() == V - 1);   // exactly V-1 edges in a spanning tree
    }

    // Graph B (classic CLRS-style, 4 vertices) to double-check the optimum.
    //   Edges: 0-1:10, 0-2:6, 0-3:5, 1-3:15, 2-3:4
    //   Sorted: 2-3(4), 0-3(5), 0-2(6 -> 0 and 2 already joined via 3, reject),
    //           0-1(10). MST = {2-3, 0-3, 0-1}, total = 4+5+10 = 19.
    {
        const std::size_t V = 4;
        std::vector<Edge> edges = {
            {0, 1, 10}, {0, 2, 6}, {0, 3, 5}, {1, 3, 15}, {2, 3, 4}
        };
        MstResult r = kruskalMst(V, edges);
        assert(r.connected);
        assert(r.totalWeight == 19);
        assert(r.chosen.size() == 3);
    }

    // Edge case: single vertex, no edges. Trivially connected, weight 0.
    {
        MstResult r = kruskalMst(1, {});
        assert(r.connected);
        assert(r.totalWeight == 0);
        assert(r.chosen.empty());
    }

    // Edge case: DISCONNECTED graph. Two vertices {0,1} joined, {2} isolated.
    // Only 1 edge can be accepted but a spanning tree of 3 vertices needs 2,
    // so 'connected' must be false (this is a spanning forest, not a tree).
    {
        const std::size_t V = 3;
        std::vector<Edge> edges = {{0, 1, 5}};
        MstResult r = kruskalMst(V, edges);
        assert(!r.connected);
        assert(r.totalWeight == 5);
        assert(r.chosen.size() == 1);       // forest: only V - (#components) edges
    }

    // Edge case: negative weights are handled (ordering is all that matters).
    // Edges: 0-1:-2, 1-2:-3, 0-2:4  -> MST = {1-2(-3), 0-1(-2)}, total = -5.
    {
        const std::size_t V = 3;
        std::vector<Edge> edges = {{0, 1, -2}, {1, 2, -3}, {0, 2, 4}};
        MstResult r = kruskalMst(V, edges);
        assert(r.connected);
        assert(r.totalWeight == -5);
        assert(r.chosen.size() == 2);
    }

    // -------------------------- Demo output --------------------------
    {
        const std::size_t V = 5;
        std::vector<Edge> edges = {
            {0, 1, 1}, {0, 2, 3}, {0, 3, 4},
            {1, 2, 2}, {2, 3, 5}, {3, 4, 6}, {2, 4, 7}
        };
        MstResult r = kruskalMst(V, edges);
        std::cout << "Kruskal MST edges (u - v : w):\n";
        for (const Edge& e : r.chosen) {
            std::cout << "  " << e.u << " - " << e.v << " : " << e.w << '\n';
        }
        std::cout << "Total MST weight = " << r.totalWeight
                  << "  (edges = " << r.chosen.size() << ")\n";
    }

    std::cout << "All Kruskal MST tests passed.\n";
    return 0;
}
