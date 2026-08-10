/*
 * Structure : Graph (Adjacency List)
 * Category  : Data Structure - Graph
 *
 * Summary   : A graph stored as one list of neighbors per vertex. Vertices are
 *             integer ids in [0, V). Supports directed and undirected graphs and
 *             the two canonical traversals, BFS and DFS.
 *
 * Operations & complexity (V = #vertices, E = #edges, deg(u) = degree of u):
 *   +----------------------+------------------+
 *   | Operation            | Time             |
 *   +----------------------+------------------+
 *   | addEdge(u, v)        | O(1) amortized   |  (push_back on a vector)
 *   | hasEdge(u, v)        | O(deg(u))        |  (linear scan of u's list)
 *   | neighbors(u)         | O(1)             |  (returns const reference)
 *   | vertexCount()        | O(1)             |
 *   | edgeCount()          | O(1)             |  (kept in a running counter)
 *   | BFS(source)          | O(V + E)         |
 *   | DFS(source)          | O(V + E)         |
 *   +----------------------+------------------+
 *   Space: O(V + E)  -- only the edges that actually exist are stored.
 *
 * Invariants / key ideas:
 *   - adj_.size() == V for the whole lifetime of the graph.
 *   - For an UNDIRECTED graph every edge {u,v} appears twice: v in adj_[u]
 *     AND u in adj_[v]. edgeCount() reports the number of logical edges, so
 *     undirected addEdge bumps the counter by ONE even though it stores two
 *     directed arcs.
 *   - BFS/DFS visit every vertex reachable from the source exactly once; a
 *     'visited' array enforces that and prevents infinite loops on cycles.
 *   - Neighbor order is deterministic: neighbors come out in the order they
 *     were added (insertion order), which makes traversal orders reproducible.
 *
 * When to use / trade-offs:
 *   - Ideal for SPARSE graphs (E << V^2): O(V + E) space beats an adjacency
 *     matrix's O(V^2) when most vertex pairs are NOT connected.
 *   - Iterating a vertex's neighbors is optimal (you touch only real edges).
 *   - hasEdge is O(deg(u)), not O(1); if you need constant-time edge queries
 *     on a dense graph, prefer an adjacency matrix instead.
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <queue>
#include <stack>
#include <vector>

class Graph {
public:
    // Construct a graph with 'vertexCount' vertices (ids 0..vertexCount-1).
    // 'directed' selects directed vs. undirected edge semantics.
    Graph(std::size_t vertexCount, bool directed)
        : adj_(vertexCount), directed_(directed), edgeCount_(0) {}

    // Rule of Zero: this class owns nothing but std::vector members, which
    // already implement correct destruction, deep copy, and move. We therefore
    // do NOT declare any of the special members and let the compiler generate
    // them. Adding hand-written ones here would be redundant and error-prone.

    // Add an edge u -> v. For an undirected graph we store BOTH directions so
    // that neighbors(u) and neighbors(v) each see the other endpoint.
    void addEdge(int u, int v) {
        assert(isValidVertex(u) && isValidVertex(v));
        adj_[static_cast<std::size_t>(u)].push_back(v);
        if (!directed_) {
            adj_[static_cast<std::size_t>(v)].push_back(u);
        }
        // One logical edge added regardless of representation (two arcs when
        // undirected). This keeps edgeCount() meaningful for both graph kinds.
        ++edgeCount_;
    }

    // True iff there is an arc u -> v. O(deg(u)) linear scan of u's list.
    bool hasEdge(int u, int v) const {
        assert(isValidVertex(u) && isValidVertex(v));
        const std::vector<int>& row = adj_[static_cast<std::size_t>(u)];
        for (int w : row) {
            if (w == v) return true;
        }
        return false;
    }

    // Const reference to u's neighbor list -- O(1), no copy.
    const std::vector<int>& neighbors(int u) const {
        assert(isValidVertex(u));
        return adj_[static_cast<std::size_t>(u)];
    }

    std::size_t vertexCount() const { return adj_.size(); }

    // Number of LOGICAL edges (undirected {u,v} counts once, see addEdge).
    std::size_t edgeCount() const { return edgeCount_; }

    // Breadth-first traversal from 'source'. Uses a FIFO queue: we enqueue the
    // source, then repeatedly dequeue a vertex, record it, and enqueue any not-
    // yet-visited neighbors. Marking visited AT ENQUEUE time (not at dequeue)
    // guarantees each vertex enters the queue at most once -> O(V + E).
    std::vector<int> BFS(int source) const {
        assert(isValidVertex(source));
        std::vector<bool> visited(adj_.size(), false);
        std::vector<int> order;
        std::queue<int> q;

        visited[static_cast<std::size_t>(source)] = true;
        q.push(source);

        while (!q.empty()) {
            int u = q.front();
            q.pop();
            order.push_back(u);
            for (int v : adj_[static_cast<std::size_t>(u)]) {
                if (!visited[static_cast<std::size_t>(v)]) {
                    visited[static_cast<std::size_t>(v)] = true;
                    q.push(v);
                }
            }
        }
        return order;
    }

    // Iterative depth-first traversal from 'source' using an explicit LIFO
    // stack (mirrors the call stack of the recursive version but cannot blow
    // the real stack on deep graphs). We mark visited AT POP time and skip
    // already-visited pops, so a vertex is recorded exactly once.
    //
    // To make the visit order match the natural recursion "visit neighbors in
    // insertion order", we push neighbors in REVERSE so the first neighbor sits
    // on top of the stack and is popped first.
    std::vector<int> DFS(int source) const {
        assert(isValidVertex(source));
        std::vector<bool> visited(adj_.size(), false);
        std::vector<int> order;
        std::stack<int> st;

        st.push(source);
        while (!st.empty()) {
            int u = st.top();
            st.pop();
            if (visited[static_cast<std::size_t>(u)]) {
                continue;  // May have been queued more than once; visit once.
            }
            visited[static_cast<std::size_t>(u)] = true;
            order.push_back(u);

            const std::vector<int>& row = adj_[static_cast<std::size_t>(u)];
            for (auto it = row.rbegin(); it != row.rend(); ++it) {
                if (!visited[static_cast<std::size_t>(*it)]) {
                    st.push(*it);
                }
            }
        }
        return order;
    }

    // Recursive DFS -- a compact alternative to the iterative version above.
    // Provided as a teaching addition; visits neighbors in insertion order.
    std::vector<int> DFSRecursive(int source) const {
        assert(isValidVertex(source));
        std::vector<bool> visited(adj_.size(), false);
        std::vector<int> order;
        dfsVisit(source, visited, order);
        return order;
    }

private:
    bool isValidVertex(int u) const {
        return u >= 0 && static_cast<std::size_t>(u) < adj_.size();
    }

    void dfsVisit(int u, std::vector<bool>& visited,
                  std::vector<int>& order) const {
        visited[static_cast<std::size_t>(u)] = true;
        order.push_back(u);
        for (int v : adj_[static_cast<std::size_t>(u)]) {
            if (!visited[static_cast<std::size_t>(v)]) {
                dfsVisit(v, visited, order);
            }
        }
    }

    std::vector<std::vector<int>> adj_;  // adj_[u] = neighbors of u
    bool directed_;
    std::size_t edgeCount_;  // number of logical edges
};

// ---------------------------------------------------------------------------
// Tests + demo
// ---------------------------------------------------------------------------
int main() {
    // -----------------------------------------------------------------------
    // UNDIRECTED graph. Known topology (edges added in this exact order):
    //
    //        0 ---- 1
    //        |     /|
    //        |    / |
    //        |   /  |
    //        2 -/   3
    //         \    /
    //          \  /
    //           4
    //
    // Edges: {0,1} {0,2} {1,2} {1,3} {2,4} {3,4}
    // -----------------------------------------------------------------------
    {
        Graph g(5, /*directed=*/false);
        g.addEdge(0, 1);
        g.addEdge(0, 2);
        g.addEdge(1, 2);
        g.addEdge(1, 3);
        g.addEdge(2, 4);
        g.addEdge(3, 4);

        assert(g.vertexCount() == 5);
        assert(g.edgeCount() == 6);  // 6 logical edges (12 stored arcs)

        // Undirected: edges are symmetric.
        assert(g.hasEdge(0, 1) && g.hasEdge(1, 0));
        assert(g.hasEdge(2, 4) && g.hasEdge(4, 2));
        assert(!g.hasEdge(0, 3));  // 0 and 3 are not adjacent
        assert(!g.hasEdge(0, 4));

        // Neighbor lists come out in insertion order.
        // adj[0] = [1, 2]; adj[1] = [0, 2, 3]; adj[2] = [0, 1, 4]; etc.
        assert((g.neighbors(0) == std::vector<int>{1, 2}));
        assert((g.neighbors(1) == std::vector<int>{0, 2, 3}));

        // BFS from 0: visit 0, then its neighbors 1,2 (in order), then their
        // unseen neighbors 3 (from 1) and 4 (from 2).
        //   queue trace: [0] -> pop 0 push 1,2 -> pop 1 push 3 -> pop 2 push 4
        //                -> pop 3 -> pop 4
        assert((g.BFS(0) == std::vector<int>{0, 1, 2, 3, 4}));

        // DFS from 0 (insertion neighbor order): 0 -> 1 -> 2 -> 4 -> 3.
        //   0 visits 1; 1 visits 2 (0 already seen); 2 visits 4 (0,1 seen);
        //   4 visits 3 (2 seen); 3 has no unseen neighbors -> backtrack.
        assert((g.DFS(0) == std::vector<int>{0, 1, 2, 4, 3}));

        // Iterative and recursive DFS must agree.
        assert(g.DFS(0) == g.DFSRecursive(0));
    }

    // -----------------------------------------------------------------------
    // DIRECTED graph. Same drawing but arcs point one way (added in order):
    //
    //        0 ---> 1 ---> 3
    //        |      |      |
    //        v      v      v
    //        2 ---> 4 <----+
    //
    // Arcs: 0->1, 0->2, 1->3, 1->4, 2->4, 3->4
    // -----------------------------------------------------------------------
    {
        Graph g(5, /*directed=*/true);
        g.addEdge(0, 1);
        g.addEdge(0, 2);
        g.addEdge(1, 3);
        g.addEdge(1, 4);
        g.addEdge(2, 4);
        g.addEdge(3, 4);

        assert(g.vertexCount() == 5);
        assert(g.edgeCount() == 6);  // directed: 6 arcs == 6 logical edges

        // Directed: hasEdge is one-directional.
        assert(g.hasEdge(0, 1));
        assert(!g.hasEdge(1, 0));  // no reverse arc
        assert(g.hasEdge(3, 4));
        assert(!g.hasEdge(4, 3));

        // Vertex 4 is a sink: no outgoing arcs.
        assert(g.neighbors(4).empty());

        // BFS from 0: 0 -> {1,2} -> from 1 push 3,4 -> from 2, 4 already seen.
        //   order: 0, 1, 2, 3, 4
        assert((g.BFS(0) == std::vector<int>{0, 1, 2, 3, 4}));

        // DFS from 0 (insertion order): 0 -> 1 -> 3 -> 4 -> back -> 2 (4 seen).
        //   order: 0, 1, 3, 4, 2
        assert((g.DFS(0) == std::vector<int>{0, 1, 3, 4, 2}));
        assert(g.DFS(0) == g.DFSRecursive(0));

        // Rule of Zero sanity check: the compiler-generated copy deep-copies
        // the underlying vectors, so the copy is independent of the original.
        Graph copy = g;
        assert(copy.hasEdge(0, 1) && !copy.hasEdge(1, 0));
        assert(copy.edgeCount() == g.edgeCount());
    }

    std::cout << "All assertions passed.\n\n";

    // Human-readable demo.
    Graph demo(5, /*directed=*/false);
    demo.addEdge(0, 1);
    demo.addEdge(0, 2);
    demo.addEdge(1, 2);
    demo.addEdge(1, 3);
    demo.addEdge(2, 4);
    demo.addEdge(3, 4);

    std::cout << "Undirected graph: " << demo.vertexCount()
              << " vertices, " << demo.edgeCount() << " edges\n";
    std::cout << "Adjacency lists:\n";
    for (std::size_t u = 0; u < demo.vertexCount(); ++u) {
        std::cout << "  " << u << " ->";
        for (int v : demo.neighbors(static_cast<int>(u))) {
            std::cout << ' ' << v;
        }
        std::cout << '\n';
    }

    std::cout << "BFS from 0:";
    for (int v : demo.BFS(0)) std::cout << ' ' << v;
    std::cout << '\n';

    std::cout << "DFS from 0:";
    for (int v : demo.DFS(0)) std::cout << ' ' << v;
    std::cout << '\n';

    return 0;
}
