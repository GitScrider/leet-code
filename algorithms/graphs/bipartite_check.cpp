/*
 * ============================================================================
 * Bipartite Check (2-coloring)
 * Category: Algorithm - Graphs (BFS traversal)
 *
 * Idea:
 *   A graph is BIPARTITE iff its vertices can be split into two sets such that
 *   every edge joins a vertex in one set to a vertex in the other -- i.e. it is
 *   2-colorable. We attempt exactly that colouring with BFS: pick an uncoloured
 *   vertex, colour it 0, and colour every neighbour the OPPOSITE colour. Each
 *   edge we traverse must connect two different colours. If we ever find an
 *   edge whose endpoints already carry the SAME colour, no valid 2-colouring
 *   exists and the graph is not bipartite.
 *
 *   WHY this is exactly the odd-cycle test: BFS layers alternate colours by
 *   distance parity from the source. A same-colour edge closes a walk of EVEN
 *   length back plus that edge => an ODD-length cycle. A graph is bipartite iff
 *   it contains NO odd cycle, so detecting a same-colour edge is precisely
 *   detecting an odd cycle.
 *
 * Complexity (V vertices, E edges; adjacency list):
 *   +-------+-----------+
 *   | Time  | O(V + E)  |   each vertex/edge examined a constant #times
 *   +-------+-----------+
 *   | Space | O(V)      |   colour array + BFS queue
 *   +-------+-----------+
 *
 * Key points / assumptions:
 *   - Graph is UNDIRECTED (edges stored both ways).
 *   - Unweighted; weights are irrelevant.
 *   - DISCONNECTED graphs: we restart BFS from every uncoloured vertex, so all
 *     components must be bipartite for the whole graph to be bipartite.
 *   - Trees and even-length cycles are bipartite; any odd cycle is not.
 * ============================================================================
 */

#include <vector>
#include <queue>
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

// Return true iff `g` is bipartite. If bipartite, `color` (resized to V) holds
// a valid 2-colouring with values 0/1; otherwise its contents are unspecified.
bool isBipartite(const Graph& g, std::vector<int>& color) {
    const int UNCOLORED = -1;
    color.assign(g.V, UNCOLORED);

    for (int start = 0; start < g.V; ++start) {
        if (color[start] != UNCOLORED) continue;  // component already coloured

        // BFS-colour this component starting from `start`.
        color[start] = 0;
        std::queue<int> q;
        q.push(start);
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (int w : g.adj[u]) {
                if (color[w] == UNCOLORED) {
                    color[w] = color[u] ^ 1;   // opposite colour (0<->1)
                    q.push(w);
                } else if (color[w] == color[u]) {
                    return false;              // same-colour edge -> odd cycle
                }
            }
        }
    }
    return true;
}

int main() {
    // --- Bipartite: an even cycle 0-1-2-3-0 (a 4-cycle / square). ---
    //    0 --- 1
    //    |     |
    //    3 --- 2
    {
        Graph g(4);
        g.addEdge(0, 1);
        g.addEdge(1, 2);
        g.addEdge(2, 3);
        g.addEdge(3, 0);
        std::vector<int> color;
        assert(isBipartite(g, color) == true);
        // Opposite ends of every edge differ in colour.
        assert(color[0] != color[1]);
        assert(color[1] != color[2]);
        assert(color[2] != color[3]);
        assert(color[3] != color[0]);
    }

    // --- Bipartite: a tree (path 0-1-2-3-4) is always bipartite. ---
    {
        Graph g(5);
        g.addEdge(0, 1);
        g.addEdge(1, 2);
        g.addEdge(2, 3);
        g.addEdge(3, 4);
        std::vector<int> color;
        assert(isBipartite(g, color) == true);
    }

    // --- NOT bipartite: an odd cycle (triangle 0-1-2-0). ---
    //    0 --- 1
    //     \   /
    //      \ /
    //       2
    {
        Graph g(3);
        g.addEdge(0, 1);
        g.addEdge(1, 2);
        g.addEdge(2, 0);
        std::vector<int> color;
        assert(isBipartite(g, color) == false);
    }

    // --- NOT bipartite: a 5-cycle (odd) 0-1-2-3-4-0. ---
    {
        Graph g(5);
        g.addEdge(0, 1);
        g.addEdge(1, 2);
        g.addEdge(2, 3);
        g.addEdge(3, 4);
        g.addEdge(4, 0);
        std::vector<int> color;
        assert(isBipartite(g, color) == false);
    }

    // --- Disconnected: a bipartite edge PLUS a triangle => not bipartite. ---
    // Every component must be bipartite; the triangle spoils it.
    {
        Graph g(5);
        g.addEdge(0, 1);          // bipartite component
        g.addEdge(2, 3);          // triangle component
        g.addEdge(3, 4);
        g.addEdge(4, 2);
        std::vector<int> color;
        assert(isBipartite(g, color) == false);
    }

    // --- Disconnected: two separate even structures => bipartite. ---
    {
        Graph g(6);
        g.addEdge(0, 1);          // an edge
        g.addEdge(2, 3);          // a square
        g.addEdge(3, 4);
        g.addEdge(4, 5);
        g.addEdge(5, 2);
        std::vector<int> color;
        assert(isBipartite(g, color) == true);
    }

    // --- Edge case: single vertex, no edges -> trivially bipartite. ---
    {
        Graph g(1);
        std::vector<int> color;
        assert(isBipartite(g, color) == true);
    }

    std::cout << "Bipartite Check (2-coloring) demo\n";
    std::cout << "  even cycle : bipartite\n";
    std::cout << "  triangle   : NOT bipartite\n";
    std::cout << "All tests passed.\n";
    return 0;
}
