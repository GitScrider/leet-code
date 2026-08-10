/*
 * Prim's Minimum Spanning Tree (Algorithm - Graphs)
 *
 * Idea:
 *   Like Kruskal, Prim finds a Minimum Spanning Tree (MST) of a connected,
 *   weighted, UNDIRECTED graph: V-1 edges connecting every vertex with minimum
 *   total weight. But Prim is VERTEX-centric and grows a SINGLE tree outward
 *   from a chosen start vertex. Maintain a set of vertices already IN the tree
 *   ("visited"). Repeatedly pick the cheapest edge that crosses from the tree to
 *   a vertex outside it, add that vertex, and repeat until all V vertices are in.
 *   A min-heap (std::priority_queue) of candidate edges {weight, toVertex}
 *   supplies the cheapest crossing edge in O(log E) per pop. We may push the same
 *   vertex multiple times with different weights; a "visited" check when popping
 *   lazily discards stale (already-absorbed) entries.
 *
 *   Why the greedy is safe (the CUT PROPERTY):
 *     At every step the visited set S and its complement form a cut. Prim always
 *     adds the LIGHTEST edge crossing (S, V\S). The cut property guarantees this
 *     lightest crossing edge belongs to some MST, so extending the tree with it
 *     never overshoots the optimum. The visited set is exactly what defines the
 *     current cut and prevents re-adding a vertex (which would create a cycle).
 *
 *   Kruskal vs. Prim (same MST weight; different mechanics):
 *     - Kruskal: sort ALL edges globally, greedily add the next-cheapest edge
 *       that joins two different components (Union-Find). May grow many tree
 *       fragments at once that later merge. Best with an EDGE LIST / sparse graph.
 *     - Prim: keep ONE connected tree, always take the cheapest edge leaving it
 *       (priority queue + visited set). Best with an ADJACENCY LIST, and with a
 *       binary heap runs in O(E log V) on dense graphs too.
 *
 * Complexity (V vertices, E edges), binary-heap variant:
 *   +-----------+----------------------+
 *   |  Step     |  Time                |
 *   +-----------+----------------------+
 *   |  Pushes   |  O(E) edges pushed   |
 *   |  Pops     |  O(E log V)          |  (each pop/push is O(log(heap size)))
 *   |  Total    |  O(E log V)          |
 *   +-----------+----------------------+
 *   Space: O(V + E)  (adjacency list + heap holding up to O(E) entries)
 *
 * Complexity derivation (heap ops counted over the whole run):
 *   Lazy Prim on V vertices, E edges, using a binary min-heap.
 *   (1) ABSORPTIONS: a vertex is marked visited at most once, so exactly V pops
 *       do "useful" work; the remaining pops are stale entries discarded by the
 *       visited check. Either way every item that enters the heap leaves it once.
 *   (2) RELAX / PUSHES: when u is absorbed we scan its adjacency list and push
 *       each not-yet-visited neighbour. Each undirected edge {u,v} is stored on
 *       BOTH endpoints, so it is examined at most twice, bounding total pushes by
 *           P = SUM_{u in V} deg(u) = 2E = O(E).
 *       The heap therefore holds up to O(E) items and total pops = total pushes.
 *   (3) HEAP COST: each push and each pop is O(log(heap size)) = O(log E). A
 *       simple graph has E <= V^2, so log E <= 2*log V = O(log V). Summing:
 *           T(V,E) = O(E) pushes * O(log V)          (push work)
 *                  + O(E) pops   * O(log V)          (pop  work)
 *                  + SUM_{u} deg(u) = 2E             (adjacency scans)
 *                  = O(E log V) + O(E) = O(E log V).
 *   The linear O(E) adjacency-scan term is dominated by the O(E log V) heap term.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants; size = V + E):
 *     f = O(g)      iff  EXISTS c2, n0 :       f <= c2*g       for size >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g <= f            for size >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   The heap operations set the ceiling and the traversal sets the floor:
 *     upper  O:     T = O(E log V)   -- <= 2E pushes and <= 2E pops, each O(log V).
 *     lower  Omega: T = Omega(E + V) -- every edge is examined in a relax and V
 *                   vertices are popped, independent of weights.
 *   These are NOT the same order, so the running time is O(E log V) overall rather
 *   than a single Theta: the WORST case (e.g. a dense graph whose heap really does
 *   grow to Theta(E)) is Theta(E log V), while inputs with few heap collisions pop
 *   with less than log V comparisons and approach the Omega(E + V) floor. No
 *   comparison-sort Omega(E log E) bound applies -- Prim never sorts the edge set;
 *   it only compares heap keys, so weights are ordered lazily, not up front.
 *
 * Key points / assumptions:
 *   - Graph is UNDIRECTED and weighted; store each edge in BOTH directions in the
 *     adjacency list so the tree can grow either way across it.
 *   - Weights may be negative; Prim compares weights only.
 *   - Requires a CONNECTED graph to reach all V vertices. If it visits fewer than
 *     V vertices the graph is disconnected and no spanning tree exists.
 */

#include <vector>
#include <cassert>
#include <iostream>
#include <queue>
#include <utility>      // std::pair
#include <functional>   // std::greater
#include <cstddef>

// Adjacency-list entry: an edge to 'to' with the given 'weight'.
struct Adj {
    int to;
    int weight;
};

using Graph = std::vector<std::vector<Adj>>;   // graph[u] = edges leaving u

// Add an UNDIRECTED weighted edge u<->v by recording it on both endpoints.
void addUndirectedEdge(Graph& g, int u, int v, int w) {
    g[u].push_back({v, w});
    g[v].push_back({u, w});
}

// Result of a Prim run.
struct MstResult {
    int totalWeight = 0;    // sum of chosen edge weights
    int edgeCount = 0;      // number of tree edges added (should be V-1 if connected)
    bool connected = false; // true iff all V vertices were reached from 'start'
};

// Compute an MST of 'g' (V = g.size()) starting the growth from vertex 'start'.
// The resulting tree and its weight are independent of the start vertex when the
// graph is connected (though the specific edges chosen among equal weights can
// vary). Vertices are ints 0..V-1.
MstResult primMst(const Graph& g, int start = 0) {
    const std::size_t V = g.size();
    MstResult result;
    if (V == 0) {
        result.connected = true;   // empty graph: vacuously an (empty) MST
        return result;
    }

    std::vector<bool> visited(V, false);   // vertices already absorbed into the tree

    // Min-heap of candidate crossing edges keyed by weight. Each item is
    // {weight, toVertex}; std::greater makes pair comparison pop the SMALLEST
    // weight first (pair compares by .first, i.e. weight, then .second).
    using Item = std::pair<int, int>;      // {weight, vertex}
    std::priority_queue<Item, std::vector<Item>, std::greater<Item>> pq;

    // Seed the tree with the start vertex; its "entry edge" has weight 0 so it is
    // absorbed for free and does not count toward the total.
    pq.push({0, start});

    while (!pq.empty()) {
        const Item top = pq.top();
        pq.pop();
        const int w = top.first;
        const int u = top.second;

        // Lazy deletion: if u is already in the tree, this is a stale entry left
        // over from when u had a cheaper (or now-superseded) crossing edge. Skip.
        if (visited[u]) {
            continue;
        }

        // Absorb u into the tree via the cheapest crossing edge that reached it.
        visited[u] = true;
        result.totalWeight += w;   // 0 for the very first (start) vertex
        if (w != 0 || u != start) {
            ++result.edgeCount;    // every vertex except the seed adds one edge
        }

        // Relax: offer every edge leaving u as a new candidate crossing edge.
        // We push unconditionally and rely on the visited check at pop time; this
        // "lazy Prim" keeps the code simple at the cost of up to O(E) heap entries.
        for (const Adj& e : g[u]) {
            if (!visited[e.to]) {
                pq.push({e.weight, e.to});
            }
        }
    }

    // Connected iff the tree absorbed every vertex, i.e. V-1 edges were added.
    result.connected =
        (static_cast<std::size_t>(result.edgeCount) + 1 == V);
    return result;
}

// ------------------------------ Tests & demo ------------------------------

int main() {
    // Graph A -- same graph used by the Kruskal file, so the MST weight matches.
    //
    //         1
    //     (0)---(1)
    //      | \    |
    //     4|  3\  |2
    //      |    \ |
    //     (3)---(2)
    //      |  5
    //     6|
    //      |
    //     (4)      (2-4 has weight 7)
    //
    // Edges: 0-1:1, 0-2:3, 0-3:4, 1-2:2, 2-3:5, 3-4:6, 2-4:7
    // The unique MST is {0-1, 1-2, 0-3, 3-4} with total weight 13 (= Kruskal).
    {
        Graph g(5);
        addUndirectedEdge(g, 0, 1, 1);
        addUndirectedEdge(g, 0, 2, 3);
        addUndirectedEdge(g, 0, 3, 4);
        addUndirectedEdge(g, 1, 2, 2);
        addUndirectedEdge(g, 2, 3, 5);
        addUndirectedEdge(g, 3, 4, 6);
        addUndirectedEdge(g, 2, 4, 7);

        MstResult r = primMst(g, 0);
        assert(r.connected);
        assert(r.totalWeight == 13);        // identical optimum to Kruskal
        assert(r.edgeCount == 4);           // V - 1

        // Starting from a different vertex yields the same total weight.
        MstResult r2 = primMst(g, 4);
        assert(r2.connected);
        assert(r2.totalWeight == 13);
        assert(r2.edgeCount == 4);
    }

    // Graph B -- CLRS-style 4-vertex graph; optimum weight is 19.
    //   Edges: 0-1:10, 0-2:6, 0-3:5, 1-3:15, 2-3:4  ->  MST = {2-3, 0-3, 0-1}.
    {
        Graph g(4);
        addUndirectedEdge(g, 0, 1, 10);
        addUndirectedEdge(g, 0, 2, 6);
        addUndirectedEdge(g, 0, 3, 5);
        addUndirectedEdge(g, 1, 3, 15);
        addUndirectedEdge(g, 2, 3, 4);

        MstResult r = primMst(g, 0);
        assert(r.connected);
        assert(r.totalWeight == 19);
        assert(r.edgeCount == 3);
    }

    // Edge case: single vertex, no edges. Trivially connected, weight 0, 0 edges.
    {
        Graph g(1);
        MstResult r = primMst(g, 0);
        assert(r.connected);
        assert(r.totalWeight == 0);
        assert(r.edgeCount == 0);
    }

    // Edge case: DISCONNECTED graph. {0,1} joined by weight 5; {2} isolated.
    // Growth from 0 reaches only {0,1}, so not all V=3 vertices are absorbed.
    {
        Graph g(3);
        addUndirectedEdge(g, 0, 1, 5);
        MstResult r = primMst(g, 0);
        assert(!r.connected);               // vertex 2 unreachable -> no spanning tree
        assert(r.totalWeight == 5);
        assert(r.edgeCount == 1);
    }

    // Edge case: negative weights. 0-1:-2, 1-2:-3, 0-2:4 -> MST weight -5.
    {
        Graph g(3);
        addUndirectedEdge(g, 0, 1, -2);
        addUndirectedEdge(g, 1, 2, -3);
        addUndirectedEdge(g, 0, 2, 4);
        MstResult r = primMst(g, 0);
        assert(r.connected);
        assert(r.totalWeight == -5);
        assert(r.edgeCount == 2);
    }

    // -------------------------- Demo output --------------------------
    {
        Graph g(5);
        addUndirectedEdge(g, 0, 1, 1);
        addUndirectedEdge(g, 0, 2, 3);
        addUndirectedEdge(g, 0, 3, 4);
        addUndirectedEdge(g, 1, 2, 2);
        addUndirectedEdge(g, 2, 3, 5);
        addUndirectedEdge(g, 3, 4, 6);
        addUndirectedEdge(g, 2, 4, 7);

        MstResult r = primMst(g, 0);
        std::cout << "Prim MST grown from vertex 0:\n";
        std::cout << "  total weight = " << r.totalWeight
                  << ", edges = " << r.edgeCount
                  << ", connected = " << (r.connected ? "yes" : "no") << '\n';
    }

    std::cout << "All Prim MST tests passed.\n";
    return 0;
}
