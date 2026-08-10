/*
 * Depth-First Search (Algorithm - Graphs)
 *
 * Idea:
 *   Go as deep as possible along one branch before backtracking. From a vertex
 *   u we recurse into the first unvisited neighbor, then the next, and so on;
 *   only when u has no unvisited neighbors left do we return to its caller. A
 *   boolean visited[] array prevents revisiting and (in a graph with cycles)
 *   prevents infinite loops. Two equivalent formulations are shown:
 *     1. Recursive  -- the call stack IS the traversal stack.
 *     2. Iterative  -- an explicit std::stack replaces the call stack.
 *   To make the iterative version reproduce the recursive visit order exactly,
 *   we push neighbors in REVERSE order so the first neighbor is popped first.
 *   A full traversal loops over all start vertices so disconnected components are
 *   covered, and we record the discovery (pre-order) sequence.
 *
 * Complexity (V = number of vertices, E = number of edges):
 *   +-----------+------------------+
 *   |  Measure  |       Cost       |
 *   +-----------+------------------+
 *   |  Time     |   O(V + E)       |
 *   |  Space    |   O(V)           |
 *   +-----------+------------------+
 *   Each vertex is marked visited once; each adjacency entry is examined once
 *   (2E scans for an undirected graph). Space is the visited array plus the
 *   recursion/explicit stack, worst case O(V) deep (a path graph).
 *
 * Key points / assumptions:
 *   - Works on directed or undirected graphs; the demo graph is undirected.
 *   - visited[] is essential: without it, any cycle would loop forever.
 *   - Neighbor iteration order is fixed, so the visit order is deterministic
 *     and both variants produce the SAME order (asserted below).
 *   - A component-spanning outer loop reaches vertices unreachable from vertex 0.
 */

#include <vector>
#include <stack>
#include <cassert>
#include <iostream>
#include <cstddef>

// Undirected adjacency list: adj[u] lists neighbors of u. Vertices are 0..V-1.
using Graph = std::vector<std::vector<int>>;

// ---- Recursive DFS from a single source -----------------------------------
// Appends vertices to 'order' the moment they are discovered (pre-order).
void dfsRecursive(const Graph& adj, int u,
                  std::vector<bool>& visited, std::vector<int>& order) {
    visited[static_cast<std::size_t>(u)] = true;
    order.push_back(u);  // discovery time recorded here
    for (const int v : adj[static_cast<std::size_t>(u)]) {
        if (!visited[static_cast<std::size_t>(v)]) {
            dfsRecursive(adj, v, visited, order);  // dive deeper, then backtrack
        }
    }
}

// ---- Iterative DFS from a single source (explicit stack) -------------------
// Mirrors the recursive order by pushing neighbors in reverse. We mark a vertex
// visited when we POP it (and skip if already visited, since a vertex may be
// pushed multiple times before it is first popped).
std::vector<int> dfsIterative(const Graph& adj, int source,
                              std::vector<bool>& visited) {
    std::vector<int> order;
    std::stack<int> st;
    st.push(source);
    while (!st.empty()) {
        const int u = st.top();
        st.pop();
        if (visited[static_cast<std::size_t>(u)]) {
            continue;  // was reached via an earlier branch; ignore the stale copy
        }
        visited[static_cast<std::size_t>(u)] = true;
        order.push_back(u);
        // Push neighbors in reverse so the first listed neighbor is on top and
        // therefore explored first -- matching the recursive version.
        const std::vector<int>& nbrs = adj[static_cast<std::size_t>(u)];
        for (std::size_t i = nbrs.size(); i-- > 0;) {
            if (!visited[static_cast<std::size_t>(nbrs[i])]) {
                st.push(nbrs[i]);
            }
        }
    }
    return order;
}

// ---- Full traversal over all components (recursive engine) -----------------
// Returns the global discovery order, restarting DFS at each still-unvisited
// vertex so that disconnected components are all covered.
std::vector<int> dfsFull(const Graph& adj) {
    const std::size_t V = adj.size();
    std::vector<bool> visited(V, false);
    std::vector<int> order;
    for (std::size_t s = 0; s < V; ++s) {
        if (!visited[s]) {
            dfsRecursive(adj, static_cast<int>(s), visited, order);
        }
    }
    return order;
}

void addUndirectedEdge(Graph& adj, int u, int v) {
    adj[static_cast<std::size_t>(u)].push_back(v);
    adj[static_cast<std::size_t>(v)].push_back(u);
}

int main() {
    // Known graph (undirected), two components. Neighbor insertion order is
    // fixed to make the DFS order deterministic.
    //
    //   Component A:            Component B:
    //        0                      5
    //       / \                     |
    //      1   2                    6
    //      |   |
    //      3   4  (3-4 also linked, forming a cycle 1-3-4-2-0-1)
    //
    Graph adj(7);
    addUndirectedEdge(adj, 0, 1);
    addUndirectedEdge(adj, 0, 2);
    addUndirectedEdge(adj, 1, 3);
    addUndirectedEdge(adj, 2, 4);
    addUndirectedEdge(adj, 3, 4);  // creates a cycle -> visited[] is exercised
    addUndirectedEdge(adj, 5, 6);  // separate component

    // Recursive traversal from vertex 0 (component A only).
    // Trace: 0 -> 1 -> 3 -> 4 (via 3-4) -> 2 (via 4-2). Back up; 0's neighbor 2
    // already visited. Order: 0,1,3,4,2.
    {
        std::vector<bool> visited(adj.size(), false);
        std::vector<int> order;
        dfsRecursive(adj, 0, visited, order);
        const std::vector<int> expected = {0, 1, 3, 4, 2};
        assert(order == expected);
    }

    // Iterative traversal from vertex 0 must match the recursive order exactly.
    {
        std::vector<bool> visited(adj.size(), false);
        const std::vector<int> order = dfsIterative(adj, 0, visited);
        const std::vector<int> expected = {0, 1, 3, 4, 2};
        assert(order == expected);
    }

    // Full traversal across BOTH components: after finishing {0,1,3,4,2} it
    // restarts at the next unvisited vertex (5), giving 5,6.
    {
        const std::vector<int> order = dfsFull(adj);
        const std::vector<int> expected = {0, 1, 3, 4, 2, 5, 6};
        assert(order == expected);
    }

    // Single-vertex graph edge case.
    {
        Graph solo(1);
        std::vector<bool> visited(1, false);
        std::vector<int> order;
        dfsRecursive(solo, 0, visited, order);
        assert(order == std::vector<int>({0}));
        assert(dfsFull(solo) == std::vector<int>({0}));
    }

    // Fully disconnected graph (no edges): each vertex is its own component,
    // discovered in index order.
    {
        Graph none(4);
        assert(dfsFull(none) == std::vector<int>({0, 1, 2, 3}));
    }

    // Short demo output.
    const std::vector<int> full = dfsFull(adj);
    std::cout << "DFS full discovery order:";
    for (const int v : full) std::cout << ' ' << v;
    std::cout << "\nAll DFS tests passed.\n";
    return 0;
}
