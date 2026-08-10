/*
 * ============================================================================
 * Cycle Detection (Undirected AND Directed graphs)
 * Category: Algorithm - Graphs (DFS)
 *
 * Idea:
 *   UNDIRECTED: run DFS carrying the PARENT of each vertex. If DFS from u
 *   reaches an already-visited neighbour w that is NOT the parent of u, then
 *   w gives a "back edge" closing a cycle. We must skip the parent because the
 *   edge we arrived on (parent-u) is the same undirected edge seen from the
 *   other side and is not, by itself, a cycle.
 *
 *   DIRECTED: a directed cycle exists iff DFS finds a back edge to a vertex
 *   currently ON the recursion stack. We use 3 colours:
 *     WHITE = unvisited, GRAY = in progress (on the current DFS path/stack),
 *     BLACK = fully finished (all descendants explored).
 *   Reaching a GRAY vertex means we looped back onto our own active path -> a
 *   cycle. Reaching a BLACK vertex is only a cross/forward edge into an already
 *   completed subtree, which is NOT a cycle. Colours are what make directed
 *   detection correct: "visited" alone is not enough for directed graphs.
 *
 * Complexity (V vertices, E edges):
 *   +-------+-----------+
 *   | Time  | O(V + E)  |   one DFS over the graph
 *   +-------+-----------+
 *   | Space | O(V)      |   colour/visited arrays + recursion stack
 *   +-------+-----------+
 *
 * Key points / assumptions:
 *   - Two SEPARATE routines: one assumes an undirected graph (edges stored
 *     both ways), the other a directed graph (edges stored one way).
 *   - Unweighted; both handle DISCONNECTED graphs by starting DFS from every
 *     unvisited vertex.
 *   - A self-loop counts as a cycle in both models.
 * ============================================================================
 */

#include <vector>
#include <cassert>
#include <iostream>

// Simple adjacency-list graph. For undirected use, add each edge both ways;
// for directed use, add it one way. Vertices are ints 0..V-1.
struct Graph {
    int V;
    std::vector<std::vector<int>> adj;
    explicit Graph(int n) : V(n), adj(n) {}
    void addDirectedEdge(int u, int v) { adj[u].push_back(v); }
    void addUndirectedEdge(int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
};

// -------------------- Undirected: DFS with parent tracking ------------------
static bool dfsUndirected(const Graph& g, int u, int parent,
                          std::vector<bool>& visited) {
    visited[u] = true;
    for (int w : g.adj[u]) {
        if (!visited[w]) {
            if (dfsUndirected(g, w, u, visited)) return true;  // cycle deeper
        } else if (w != parent) {
            // w is visited and is not the edge we came in on -> back edge.
            return true;
        }
    }
    return false;
}

bool hasCycleUndirected(const Graph& g) {
    std::vector<bool> visited(g.V, false);
    for (int s = 0; s < g.V; ++s) {          // handle disconnected graphs
        if (!visited[s] && dfsUndirected(g, s, -1, visited)) return true;
    }
    return false;
}

// -------------------- Directed: DFS with white/gray/black -------------------
enum Color { WHITE, GRAY, BLACK };

static bool dfsDirected(const Graph& g, int u, std::vector<Color>& color) {
    color[u] = GRAY;                         // u is now on the active path
    for (int w : g.adj[u]) {
        if (color[w] == GRAY) return true;   // back edge to active path = cycle
        if (color[w] == WHITE && dfsDirected(g, w, color)) return true;
        // color[w] == BLACK -> finished subtree, safe (no cycle from here)
    }
    color[u] = BLACK;                        // u fully explored, leaves stack
    return false;
}

bool hasCycleDirected(const Graph& g) {
    std::vector<Color> color(g.V, WHITE);
    for (int s = 0; s < g.V; ++s) {          // handle disconnected graphs
        if (color[s] == WHITE && dfsDirected(g, s, color)) return true;
    }
    return false;
}

int main() {
    // ================= UNDIRECTED =================
    // Positive: a triangle 0-1-2-0 is a cycle.
    //    0---1
    //     \ /
    //      2
    {
        Graph g(3);
        g.addUndirectedEdge(0, 1);
        g.addUndirectedEdge(1, 2);
        g.addUndirectedEdge(2, 0);
        assert(hasCycleUndirected(g) == true);
    }
    // Negative: a tree (path 0-1-2-3) has no cycle.
    //    0---1---2---3
    {
        Graph g(4);
        g.addUndirectedEdge(0, 1);
        g.addUndirectedEdge(1, 2);
        g.addUndirectedEdge(2, 3);
        assert(hasCycleUndirected(g) == false);
    }
    // Negative: a forest (two disjoint edges) has no cycle.
    {
        Graph g(4);
        g.addUndirectedEdge(0, 1);
        g.addUndirectedEdge(2, 3);
        assert(hasCycleUndirected(g) == false);
    }
    // Positive on a disconnected graph: clean edge + a separate triangle.
    {
        Graph g(5);
        g.addUndirectedEdge(0, 1);            // acyclic component
        g.addUndirectedEdge(2, 3);            // triangle component
        g.addUndirectedEdge(3, 4);
        g.addUndirectedEdge(4, 2);
        assert(hasCycleUndirected(g) == true);
    }

    // ================= DIRECTED =================
    // Positive: 0 -> 1 -> 2 -> 0 is a directed cycle.
    {
        Graph g(3);
        g.addDirectedEdge(0, 1);
        g.addDirectedEdge(1, 2);
        g.addDirectedEdge(2, 0);
        assert(hasCycleDirected(g) == true);
    }
    // Negative: a DAG.  0 -> 1 -> 3, 0 -> 2 -> 3  (diamond, no back edge)
    //      0
    //     / \
    //    1   2
    //     \ /
    //      3
    // Note edge 1->3 and 2->3 reach the same vertex but via forward/cross
    // edges into a FINISHED subtree, not a back edge -> not a cycle.
    {
        Graph g(4);
        g.addDirectedEdge(0, 1);
        g.addDirectedEdge(0, 2);
        g.addDirectedEdge(1, 3);
        g.addDirectedEdge(2, 3);
        assert(hasCycleDirected(g) == false);
    }
    // Positive on a disconnected digraph: a DAG part plus a 2-cycle part.
    {
        Graph g(5);
        g.addDirectedEdge(0, 1);              // acyclic part
        g.addDirectedEdge(3, 4);              // 2-cycle part
        g.addDirectedEdge(4, 3);
        assert(hasCycleDirected(g) == true);
    }
    // Self-loop counts as a directed cycle.
    {
        Graph g(2);
        g.addDirectedEdge(1, 1);
        assert(hasCycleDirected(g) == true);
    }

    std::cout << "Cycle Detection demo\n";
    std::cout << "  undirected triangle : cyclic\n";
    std::cout << "  directed diamond DAG: acyclic\n";
    std::cout << "All tests passed.\n";
    return 0;
}
