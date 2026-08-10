/*
 * ============================================================================
 * Connected Components (Undirected Graph)
 * Category: Algorithm - Graphs (traversal / flood fill)
 *
 * Idea:
 *   In an UNDIRECTED graph, two vertices are in the same connected component
 *   iff there is a path between them. To find all components we sweep over
 *   every vertex; each time we meet a vertex that has not been labelled yet we
 *   start a fresh BFS "flood fill" from it, tagging every vertex reachable from
 *   it with the SAME component id, then move on. Because reachability in an
 *   undirected graph is symmetric and transitive, one flood fill discovers
 *   exactly one whole component. The number of times we have to start a new
 *   flood fill equals the number of connected components.
 *
 *   Invariant: when a flood fill from vertex s finishes, comp[v] == id for
 *   every v reachable from s and only for those v. The visited/label array
 *   (comp[v] != UNLABELLED) prevents re-processing and guarantees termination.
 *
 * Complexity (V vertices, E edges; adjacency list):
 *   +-------+------------------+
 *   | Time  | O(V + E)         |   each vertex/edge visited a constant #times
 *   +-------+------------------+
 *   | Space | O(V)             |   label array + BFS queue
 *   +-------+------------------+
 *
 * Key points / assumptions:
 *   - Graph is UNDIRECTED; store each edge (u,v) in BOTH adj[u] and adj[v].
 *   - Unweighted; weights are irrelevant to connectivity.
 *   - Isolated vertices (no edges) are their own singleton components.
 *   - A DFS or a Union-Find (DSU) would give the identical component count;
 *     BFS is used here to avoid deep recursion on large graphs.
 * ============================================================================
 */

#include <vector>
#include <queue>
#include <cassert>
#include <iostream>

// Undirected graph as an adjacency list. Vertices are ints 0..V-1.
struct Graph {
    int V;                              // number of vertices
    std::vector<std::vector<int>> adj;  // adj[u] = neighbours of u
    explicit Graph(int n) : V(n), adj(n) {}
    // Add an undirected edge: it appears on both endpoints' lists.
    void addEdge(int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
};

// Label every vertex with the id of the component it belongs to (0,1,2,...).
// Returns the total number of connected components. comp is resized to V.
int connectedComponents(const Graph& g, std::vector<int>& comp) {
    const int UNLABELLED = -1;
    comp.assign(g.V, UNLABELLED);
    int id = 0;  // next component id to hand out

    for (int start = 0; start < g.V; ++start) {
        if (comp[start] != UNLABELLED) continue;  // already part of a component

        // BFS flood fill: tag everything reachable from `start` with `id`.
        std::queue<int> q;
        comp[start] = id;
        q.push(start);
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (int w : g.adj[u]) {
                if (comp[w] == UNLABELLED) {  // first time we reach w
                    comp[w] = id;
                    q.push(w);
                }
            }
        }
        ++id;  // this flood fill covered exactly one component
    }
    return id;
}

int main() {
    // ------------------------------------------------------------------
    // Graph under test (8 vertices, undirected). Three components:
    //
    //   Component A: 0 - 1 - 2 - 0      (a triangle)
    //   Component B: 3 - 4              (a single edge)
    //   Component C: 5                  (isolated vertex)
    //   Component D: 6 - 7              (another single edge)
    //
    //   0---1        3---4     5     6---7
    //    \ /
    //     2
    // ------------------------------------------------------------------
    Graph g(8);
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 0);
    g.addEdge(3, 4);
    g.addEdge(6, 7);
    // vertex 5 is intentionally isolated (no edges)

    std::vector<int> comp;
    int count = connectedComponents(g, comp);

    // There must be exactly 4 components.
    assert(count == 4);

    // Two vertices share a label IFF they are in the same drawn component.
    // Same-component pairs:
    assert(comp[0] == comp[1] && comp[1] == comp[2]);  // triangle together
    assert(comp[3] == comp[4]);                        // edge together
    assert(comp[6] == comp[7]);                        // edge together
    // Different-component pairs must NOT share a label:
    assert(comp[0] != comp[3]);
    assert(comp[0] != comp[5]);
    assert(comp[3] != comp[5]);
    assert(comp[5] != comp[6]);
    assert(comp[3] != comp[6]);

    // Edge case: a graph of a single isolated vertex -> exactly 1 component.
    {
        Graph single(1);
        std::vector<int> c;
        assert(connectedComponents(single, c) == 1);
        assert(c[0] == 0);
    }

    // Edge case: fully connected chain 0-1-2-3 -> exactly 1 component.
    {
        Graph chain(4);
        chain.addEdge(0, 1);
        chain.addEdge(1, 2);
        chain.addEdge(2, 3);
        std::vector<int> c;
        assert(connectedComponents(chain, c) == 1);
        assert(c[0] == c[1] && c[1] == c[2] && c[2] == c[3]);
    }

    // Edge case: no edges at all -> every vertex is its own component.
    {
        Graph empty(5);
        std::vector<int> c;
        assert(connectedComponents(empty, c) == 5);
    }

    // Short demo.
    std::cout << "Connected Components demo\n";
    std::cout << "  #components = " << count << "\n  labels:";
    for (int v = 0; v < g.V; ++v) {
        std::cout << ' ' << v << "->" << comp[v];
    }
    std::cout << "\nAll tests passed.\n";
    return 0;
}
