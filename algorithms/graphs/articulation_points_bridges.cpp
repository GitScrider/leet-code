/*
 * ============================================================================
 * Articulation Points and Bridges (Tarjan, single DFS)
 * Category: Algorithm - Graphs (DFS, discovery/low-link)
 *
 * Idea:
 *   In an UNDIRECTED connected graph, an ARTICULATION POINT (cut vertex) is a
 *   vertex whose removal increases the number of connected components; a BRIDGE
 *   (cut edge) is an edge whose removal does the same. A single DFS finds them
 *   all using two timestamps per vertex:
 *     disc[u] = the time u was first discovered (DFS preorder number).
 *     low[u]  = the smallest disc reachable from u's DFS subtree using tree
 *               edges downward plus AT MOST ONE back edge upward.
 *
 *   Low recurrence, computed as DFS returns from each child v of u:
 *     low[u] = min( low[u],           // initialised to disc[u]
 *                   low[v],           // v is a TREE child: inherit its reach
 *                   disc[w] )         // w is a BACK edge target (w != parent)
 *
 *   Decisions once low[v] is known for a tree child v of u:
 *     BRIDGE  (u,v)          iff  low[v] >  disc[u]
 *             -> v's subtree has NO back edge climbing above u, so the only way
 *                into it is through the edge (u,v): cutting it disconnects v.
 *     ARTICULATION (non-root u) iff  low[v] >= disc[u]
 *             -> v's subtree cannot bypass u to reach an ancestor, so removing
 *                u severs that subtree.
 *     ROOT special case: the DFS root is an articulation point IFF it has TWO
 *             OR MORE DFS tree children (its subtrees can only be linked
 *             through the root itself).
 *
 * Complexity (V vertices, E edges; adjacency list):
 *   +-------+-----------+
 *   | Time  | O(V + E)  |   one DFS
 *   +-------+-----------+
 *   | Space | O(V + E)  |   disc/low arrays + recursion stack
 *   +-------+-----------+
 *
 * Complexity derivation (DFS traversal counted over V and E):
 *   The recursion visits every vertex EXACTLY once: dfs(u) is entered only when
 *   disc[u] == -1, and its first line sets disc[u], so u is never re-entered.
 *   Inside dfs(u) the for-loop iterates over adj[u] once, i.e. deg(u) steps, and
 *   each step is O(1) work (a comparison, a std::min, maybe one emplace_back).
 *   Charging 1 unit to discover a vertex plus 1 unit per incident-edge scan:
 *
 *       C(V,E) = SUM_{u in V} ( 1 + deg(u) )
 *              = SUM_{u in V} 1   +   SUM_{u in V} deg(u)
 *              =        V         +         2E
 *              = V + 2E
 *              = O(V + E)
 *
 *   The middle step uses the handshake lemma: an undirected edge is stored at
 *   BOTH endpoints, so SUM_{u} deg(u) = 2E. The driver adds one O(V) loop to
 *   launch a DFS per unvisited vertex plus two O(V) result-collection scans, all
 *   dominated by the V + 2E traversal. (The final std::sort of the bridge list
 *   costs O(B log B) with B <= V-1 bridges -- a lower-order add-on for test
 *   determinism, not part of the core linear-time algorithm.)
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants; "size" = V + E):
 *     f = O(g)      iff  EXISTS c2, n0 :        f <= c2*g   for size >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g <= f          for size >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   The exact count is f(V,E) = V + 2E; take g(V,E) = V + E:
 *     upper  O:     V + 2E <= 2*(V + E)   for E >= 0  => O(V + E)     (c2 = 2)
 *     lower  Omega: V + 2E >=  1*(V + E)  for E >= 0  => Omega(V + E) (c1 = 1)
 *     tight  Theta: both hold (c1 = 1, c2 = 2)        => Theta(V + E)
 *   The traversal is INPUT-INDEPENDENT for a fixed (V,E): every vertex is
 *   discovered once and every edge scanned twice (once from each endpoint) no
 *   matter the graph's shape, so best = average = worst = Theta(V + E). No
 *   comparison-sort Omega(n log n) barrier applies here -- this is a linear
 *   graph scan, not a sort (the only sort, of the <= V-1 bridges, is an
 *   auxiliary output step outside the core traversal).
 *
 * Key points / assumptions:
 *   - Graph is UNDIRECTED (edges stored both ways), simple (no parallel edges
 *     or self-loops in the tests). Unweighted.
 *   - Handles DISCONNECTED graphs by running the DFS from every vertex; each
 *     DFS tree gets its own root special-case check.
 *   - Parent edge is skipped once (not treated as a back edge).
 * ============================================================================
 */

#include <vector>
#include <set>
#include <algorithm>
#include <utility>      // std::pair
#include <cassert>
#include <iostream>

// Undirected graph as an adjacency list. Vertices are ints 0..V-1.
struct Graph {
    int V;
    std::vector<std::vector<int>> adj;
    explicit Graph(int n) : V(n), adj(n) {}
    void addEdge(int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
};

// Carries all DFS state so the recursion signature stays small.
struct APState {
    const Graph& g;
    std::vector<int> disc;              // discovery time, -1 = unvisited
    std::vector<int> low;               // low-link value
    std::vector<bool> isArticulation;   // result: articulation-point flags
    std::vector<std::pair<int, int>> bridges;  // result: bridge edges (u<v)
    int timer = 0;

    explicit APState(const Graph& graph)
        : g(graph), disc(graph.V, -1), low(graph.V, 0),
          isArticulation(graph.V, false) {}
};

// DFS from u whose DFS-tree parent is `parent` (-1 for a root).
static void dfs(APState& s, int u, int parent) {
    s.disc[u] = s.low[u] = s.timer++;   // discover u; low starts at its own disc
    int children = 0;                   // number of DFS-tree children of u
    bool parentSkipped = false;         // skip the edge back to parent ONCE

    for (int v : s.g.adj[u]) {
        if (v == parent && !parentSkipped) {
            parentSkipped = true;       // ignore the tree edge we arrived on
            continue;                   // (a 2nd parallel edge to parent, if
        }                               //  any, falls through as a back edge)
        if (s.disc[v] == -1) {          // (u,v) is a TREE edge
            ++children;
            dfs(s, v, u);
            s.low[u] = std::min(s.low[u], s.low[v]);   // inherit child's reach

            // Bridge: v's subtree cannot climb above u.
            if (s.low[v] > s.disc[u]) {
                s.bridges.emplace_back(std::min(u, v), std::max(u, v));
            }
            // Articulation (non-root): subtree cannot bypass u to an ancestor.
            if (parent != -1 && s.low[v] >= s.disc[u]) {
                s.isArticulation[u] = true;
            }
        } else {                        // (u,v) is a BACK edge to an ancestor
            s.low[u] = std::min(s.low[u], s.disc[v]);
        }
    }

    // Root special case: a root is an articulation point iff it has >= 2
    // DFS-tree children (its subtrees can meet only through the root itself).
    if (parent == -1 && children >= 2) {
        s.isArticulation[u] = true;
    }
}

// Compute articulation points and bridges of the whole (possibly
// disconnected) graph. `arts` and `bridges` receive the sorted results.
void findArticulationAndBridges(const Graph& g,
                                std::vector<int>& arts,
                                std::vector<std::pair<int, int>>& bridges) {
    APState s(g);
    for (int v = 0; v < g.V; ++v) {
        if (s.disc[v] == -1) dfs(s, v, -1);   // new DFS tree, v is its root
    }
    arts.clear();
    for (int v = 0; v < g.V; ++v) {
        if (s.isArticulation[v]) arts.push_back(v);
    }
    bridges = s.bridges;
    std::sort(bridges.begin(), bridges.end());
}

// Helpers to compare results against known sets regardless of order.
static std::set<int> toSet(const std::vector<int>& v) {
    return std::set<int>(v.begin(), v.end());
}
static std::set<std::pair<int, int>> toSet(
        const std::vector<std::pair<int, int>>& v) {
    return std::set<std::pair<int, int>>(v.begin(), v.end());
}

int main() {
    // Graph 1: a simple chain 0 - 1 - 2 - 3 - 4.
    // Every INTERNAL vertex (1,2,3) is an articulation point; the two ends
    // (0,4) are not. EVERY edge is a bridge.
    {
        Graph g(5);
        g.addEdge(0, 1);
        g.addEdge(1, 2);
        g.addEdge(2, 3);
        g.addEdge(3, 4);
        std::vector<int> arts;
        std::vector<std::pair<int, int>> bridges;
        findArticulationAndBridges(g, arts, bridges);
        assert(toSet(arts) == (std::set<int>{1, 2, 3}));
        std::set<std::pair<int, int>> expectedBridges{
            {0, 1}, {1, 2}, {2, 3}, {3, 4}};
        assert(toSet(bridges) == expectedBridges);
    }

    // Graph 2: a triangle 0-1-2-0 with a tail 2-3.  0---1 / \ 2---3
    // The cycle {0,1,2} has no internal cut, but vertex 2 holds the tail on,
    // so 2 is the only articulation point and (2,3) the only bridge.
    {
        Graph g(4);
        g.addEdge(0, 1);
        g.addEdge(1, 2);
        g.addEdge(2, 0);
        g.addEdge(2, 3);
        std::vector<int> arts;
        std::vector<std::pair<int, int>> bridges;
        findArticulationAndBridges(g, arts, bridges);
        assert(toSet(arts) == (std::set<int>{2}));
        std::set<std::pair<int, int>> expectedBridges{{2, 3}};
        assert(toSet(bridges) == expectedBridges);
    }

    // Graph 3: a single cycle (square) 0-1-2-3-0 has NO articulation points
    // and NO bridges: removing any one vertex or edge leaves it connected.
    {
        Graph g(4);
        g.addEdge(0, 1);
        g.addEdge(1, 2);
        g.addEdge(2, 3);
        g.addEdge(3, 0);
        std::vector<int> arts;
        std::vector<std::pair<int, int>> bridges;
        findArticulationAndBridges(g, arts, bridges);
        assert(arts.empty());
        assert(bridges.empty());
    }

    // Graph 4 (root special case + disconnected): 1---0---2   and   3---4.
    // Root 0 has two DFS children -> articulation point. The lone edge {3,4}
    // has no articulation point but edge (3,4) is a bridge.
    // Expected articulation points {0}; bridges (0,1),(0,2),(3,4).
    {
        Graph g(5);
        g.addEdge(0, 1);
        g.addEdge(0, 2);
        g.addEdge(3, 4);
        std::vector<int> arts;
        std::vector<std::pair<int, int>> bridges;
        findArticulationAndBridges(g, arts, bridges);
        assert(toSet(arts) == (std::set<int>{0}));
        std::set<std::pair<int, int>> expectedBridges{{0, 1}, {0, 2}, {3, 4}};
        assert(toSet(bridges) == expectedBridges);
    }

    // Edge case: single vertex -> nothing to cut.
    {
        Graph g(1);
        std::vector<int> arts;
        std::vector<std::pair<int, int>> bridges;
        findArticulationAndBridges(g, arts, bridges);
        assert(arts.empty());
        assert(bridges.empty());
    }

    std::cout << "Articulation Points & Bridges (Tarjan) demo\n";
    std::cout << "  chain 0-1-2-3-4: cut vertices {1,2,3}, all edges bridges\n";
    std::cout << "  square 0-1-2-3-0: no cut vertices, no bridges\n";
    std::cout << "All tests passed.\n";
    return 0;
}
