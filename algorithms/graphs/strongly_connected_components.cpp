/*
 * ============================================================================
 * Strongly Connected Components (SCC) -- Kosaraju's algorithm
 * Category: Algorithm - Graphs (DFS, two passes)
 *
 * Idea (Kosaraju, chosen for its clear two-pass structure):
 *   A strongly connected component of a DIRECTED graph is a maximal set of
 *   vertices where every vertex can reach every other one. Kosaraju finds them
 *   with two depth-first searches:
 *
 *     PASS 1  On the ORIGINAL graph, run DFS and push each vertex onto a stack
 *             when it FINISHES (post-order). This orders vertices by decreasing
 *             finish time -- a topological-like order of the SCC "condensation".
 *
 *     PASS 2  Build the TRANSPOSE graph (reverse every edge). Pop vertices off
 *             the stack; each unvisited vertex starts a DFS on the transpose,
 *             and every vertex that DFS reaches forms one SCC.
 *
 *   WHY it works: reversing edges keeps SCCs intact (a<->b both directions
 *   survive) but flips the direction between DIFFERENT SCCs. Processing in
 *   decreasing finish time guarantees we start each transpose-DFS in a "sink"
 *   SCC of the condensation, so the DFS cannot leak into another component --
 *   it collects exactly one SCC before stopping.
 *
 * Complexity (V vertices, E edges; adjacency list):
 *   +-------+-----------+
 *   | Time  | O(V + E)  |   two DFS passes + building the transpose
 *   +-------+-----------+
 *   | Space | O(V + E)  |   transpose adjacency + stack + visited/id arrays
 *   +-------+-----------+
 *
 * Complexity derivation (three linear scans over V and E; count over V and E):
 *   Kosaraju runs three input-independent phases; sum their costs.
 *
 *     PASS 1  (fillOrder DFS on the original graph)  Each vertex is marked and
 *             entered once (SUM_v 1 = V) and its adjacency list is scanned once
 *             (SUM_v outdeg(v) = E)         =>  V + E.
 *     BUILD   (construct the transpose)  The double loop visits every vertex
 *             once and every out-edge once: SUM_v (1 + outdeg(v)) = V + E, and
 *             each gt.addEdge is O(1)       =>  V + E.
 *     PASS 2  (collect DFS on the transpose + popping the finish stack)  The
 *             transpose has the same V vertices and E edges; each vertex is
 *             labelled once and each reversed edge scanned once, and the while
 *             loop pops all V stack entries at O(1) each  =>  V + E.
 *
 *   Total:
 *       C = (V + E) + (V + E) + (V + E) = 3*(V + E) = O(V + E)
 *
 *   No branch depends on edge weights or vertex values, so every input of the
 *   same (V, E) costs the same up to constants -- the work is data-independent.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants; here "size" = V + E):
 *     f = O(g)      iff  EXISTS c2, n0 :        f(n) <= c2*g(n)  for n >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)         for n >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   The exact count is C = 3*(V + E). Take g = V + E:
 *     upper  O:     C <= 3*(V + E)             => T = O(V + E)
 *     lower  Omega: C >= 1*(V + E)             => T = Omega(V + E)
 *     tight  Theta: both hold (c1 = 1, c2 = 3) => T = Theta(V + E)
 *   There is no early exit and no data-dependent branch, so the SAME
 *   Theta(V + E) is the tight bound for best, average AND worst case.
 *
 * Key points / assumptions:
 *   - Graph is DIRECTED. In an undirected graph, SCCs == connected components.
 *   - Unweighted; weights do not affect strong connectivity.
 *   - Every vertex belongs to exactly one SCC (a lone vertex is its own SCC).
 *   - Two vertices share an SCC id iff each can reach the other.
 * ============================================================================
 */

#include <vector>
#include <stack>
#include <cassert>
#include <iostream>
#include <algorithm>

// Directed graph as an adjacency list. Vertices are ints 0..V-1.
struct Graph {
    int V;
    std::vector<std::vector<int>> adj;
    explicit Graph(int n) : V(n), adj(n) {}
    void addEdge(int u, int v) { adj[u].push_back(v); }  // directed u -> v
};

// PASS 1: DFS on the original graph, recording finish order.
static void fillOrder(const Graph& g, int u, std::vector<bool>& visited,
                      std::stack<int>& finishStack) {
    visited[u] = true;
    for (int w : g.adj[u]) {
        if (!visited[w]) fillOrder(g, w, visited, finishStack);
    }
    finishStack.push(u);  // u pushed AFTER all its descendants -> post-order
}

// PASS 2: DFS on the transpose, labelling every reached vertex with `id`.
static void collect(const Graph& gt, int u, std::vector<int>& comp, int id) {
    comp[u] = id;
    for (int w : gt.adj[u]) {
        if (comp[w] == -1) collect(gt, w, comp, id);
    }
}

// Compute SCCs. Fills comp[v] with the SCC id of v (0-based); returns the
// number of SCCs found.
int stronglyConnectedComponents(const Graph& g, std::vector<int>& comp) {
    // ---- Pass 1: order vertices by finish time on the original graph. ----
    std::vector<bool> visited(g.V, false);
    std::stack<int> finishStack;
    for (int v = 0; v < g.V; ++v) {
        if (!visited[v]) fillOrder(g, v, visited, finishStack);
    }

    // ---- Build the transpose (reverse every edge). ----
    Graph gt(g.V);
    for (int u = 0; u < g.V; ++u) {
        for (int w : g.adj[u]) gt.addEdge(w, u);
    }

    // ---- Pass 2: DFS on transpose in decreasing finish-time order. ----
    comp.assign(g.V, -1);
    int id = 0;
    while (!finishStack.empty()) {
        int v = finishStack.top();
        finishStack.pop();
        if (comp[v] == -1) {          // starts a new SCC (a sink in condensation)
            collect(gt, v, comp, id);
            ++id;
        }
    }
    return id;
}

int main() {
    // ------------------------------------------------------------------
    // Known digraph with exactly 3 SCCs:
    //
    //   0 -> 1 -> 2 -> 0    (cycle)          => SCC {0,1,2}
    //   3 -> 4 -> 5 -> 3    (cycle)          => SCC {3,4,5}
    //   2 -> 3             (one-way bridge between the two cycles)
    //   6 -> 0            (6 reaches the rest, nothing reaches 6) => SCC {6}
    //
    //        +-----------------+        +-----------------+
    //   6 -> | 0 -> 1 -> 2 --. |  2->3  | 3 -> 4 -> 5 --. |
    //        | ^-----------' | ------->  | ^-----------' |
    //        +-----------------+        +-----------------+
    //   The one-way edges 2->3 and 6->0 keep the three SCCs separate.
    // ------------------------------------------------------------------
    Graph g(7);
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 0);   // {0,1,2} cycle
    g.addEdge(2, 3);   // bridge into the next SCC (one directional)
    g.addEdge(3, 4);
    g.addEdge(4, 5);
    g.addEdge(5, 3);   // {3,4,5} cycle
    g.addEdge(6, 0);   // 6 points into {0,1,2} but nothing points back to 6

    std::vector<int> comp;
    int sccCount = stronglyConnectedComponents(g, comp);

    // Exactly 3 SCCs: {0,1,2}, {3,4,5}, {6}.
    assert(sccCount == 3);

    // Membership: vertices in the same drawn cycle share an id.
    assert(comp[0] == comp[1] && comp[1] == comp[2]);
    assert(comp[3] == comp[4] && comp[4] == comp[5]);
    // Distinct SCCs have distinct ids.
    assert(comp[0] != comp[3]);
    assert(comp[6] != comp[0]);
    assert(comp[6] != comp[3]);
    // 6 is alone -> no other vertex shares its id.
    for (int v = 0; v < g.V; ++v) {
        if (v != 6) assert(comp[v] != comp[6]);
    }

    // Edge case: single vertex is its own SCC.
    {
        Graph one(1);
        std::vector<int> c;
        assert(stronglyConnectedComponents(one, c) == 1);
        assert(c[0] == 0);
    }

    // Edge case: a pure DAG (no cycles) -> every vertex is its own SCC.
    //   0 -> 1 -> 2
    {
        Graph dag(3);
        dag.addEdge(0, 1);
        dag.addEdge(1, 2);
        std::vector<int> c;
        assert(stronglyConnectedComponents(dag, c) == 3);
        // All three ids distinct.
        assert(c[0] != c[1] && c[1] != c[2] && c[0] != c[2]);
    }

    // Edge case: one big cycle over all vertices -> exactly 1 SCC.
    {
        Graph ring(4);
        ring.addEdge(0, 1);
        ring.addEdge(1, 2);
        ring.addEdge(2, 3);
        ring.addEdge(3, 0);
        std::vector<int> c;
        assert(stronglyConnectedComponents(ring, c) == 1);
    }

    std::cout << "Strongly Connected Components (Kosaraju) demo\n";
    std::cout << "  #SCC = " << sccCount << "\n  labels:";
    for (int v = 0; v < g.V; ++v) std::cout << ' ' << v << "->" << comp[v];
    std::cout << "\nAll tests passed.\n";
    return 0;
}
