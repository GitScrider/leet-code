/*
 * Topological Sort (Algorithm - Graphs)
 *
 * Idea:
 *   A topological order of a DIRECTED ACYCLIC GRAPH (DAG) lists the vertices so
 *   that for every edge u -> v, u appears before v. Two classic ways:
 *
 *   1. Kahn's algorithm (BFS on indegrees):
 *      Compute indegree[v] = #incoming edges. Vertices with indegree 0 have no
 *      prerequisites, so they may go first; put them in a queue. Repeatedly pop
 *      one, append it to the order, and "remove" it by decrementing each
 *      successor's indegree; any successor that drops to 0 is now free and is
 *      enqueued. If we output all V vertices the graph was acyclic; if some
 *      never reach indegree 0, they sit on a cycle -> no valid ordering.
 *
 *   2. DFS-based (reverse postorder):
 *      Run DFS; when a vertex FINISHES (all descendants done) push it onto a
 *      list. A vertex finishes only after everything it can reach, so reversing
 *      the finish order yields a topological order. A recursion-stack marker
 *      ("in progress") detects back edges, i.e. cycles.
 *
 * Complexity (V = number of vertices, E = number of edges):
 *   +-----------+------------------+
 *   |  Measure  |       Cost       |
 *   +-----------+------------------+
 *   |  Time     |   O(V + E)       |
 *   |  Space    |   O(V)           |
 *   +-----------+------------------+
 *   Kahn: build indegrees O(V+E), each edge relaxed once. DFS: each vertex/edge
 *   visited once. Space is the queue/stack + bookkeeping arrays, O(V).
 *
 * Key points / assumptions:
 *   - DIRECTED graph. A topological order exists IFF the graph is a DAG.
 *   - The order is generally NOT unique; any order respecting all edges is valid,
 *     so tests check the ordering CONSTRAINT (u before v), not one fixed sequence.
 *   - Both methods report cycles: Kahn via a short output, DFS via a gray marker.
 *   - Unweighted; weights are irrelevant to topological ordering.
 */

#include <vector>
#include <queue>
#include <cassert>
#include <iostream>
#include <cstddef>

// Directed adjacency list: adj[u] lists vertices v with an edge u -> v.
using Graph = std::vector<std::vector<int>>;

// ---- Kahn's algorithm ------------------------------------------------------
// On success sets 'order' to a topological order and returns true.
// On a cycle, returns false (and 'order' holds the partial, pre-cycle output).
bool topoSortKahn(const Graph& adj, std::vector<int>& order) {
    const std::size_t V = adj.size();
    std::vector<int> indeg(V, 0);
    for (std::size_t u = 0; u < V; ++u) {
        for (const int v : adj[u]) {
            ++indeg[static_cast<std::size_t>(v)];  // count incoming edges
        }
    }

    // Start with every source (indegree 0). Using indices in increasing order
    // makes the tie-breaking deterministic for the demo.
    std::queue<int> q;
    for (std::size_t v = 0; v < V; ++v) {
        if (indeg[v] == 0) q.push(static_cast<int>(v));
    }

    order.clear();
    while (!q.empty()) {
        const int u = q.front();
        q.pop();
        order.push_back(u);
        // Removing u frees its successors by one prerequisite each.
        for (const int v : adj[static_cast<std::size_t>(u)]) {
            if (--indeg[static_cast<std::size_t>(v)] == 0) {
                q.push(v);
            }
        }
    }
    // If we could not emit every vertex, the leftover ones form/feed a cycle.
    return order.size() == V;
}

// ---- DFS-based topological sort -------------------------------------------
// color: 0 = white (unseen), 1 = gray (on the recursion stack), 2 = black (done).
// Encountering a gray vertex means a back edge -> cycle.
bool topoDfsVisit(const Graph& adj, int u, std::vector<int>& color,
                  std::vector<int>& postorder) {
    color[static_cast<std::size_t>(u)] = 1;  // gray: currently being explored
    for (const int v : adj[static_cast<std::size_t>(u)]) {
        if (color[static_cast<std::size_t>(v)] == 1) {
            return false;  // back edge to an ancestor -> cycle
        }
        if (color[static_cast<std::size_t>(v)] == 0) {
            if (!topoDfsVisit(adj, v, color, postorder)) return false;
        }
    }
    color[static_cast<std::size_t>(u)] = 2;  // black: fully finished
    postorder.push_back(u);                  // record finish order
    return true;
}

// On success sets 'order' to a topological order and returns true; false on cycle.
bool topoSortDfs(const Graph& adj, std::vector<int>& order) {
    const std::size_t V = adj.size();
    std::vector<int> color(V, 0);
    std::vector<int> postorder;
    for (std::size_t s = 0; s < V; ++s) {
        if (color[s] == 0) {
            if (!topoDfsVisit(adj, static_cast<int>(s), color, postorder)) {
                return false;  // cycle detected
            }
        }
    }
    // Reverse postorder = topological order.
    order.assign(postorder.rbegin(), postorder.rend());
    return true;
}

// ---- Test helper -----------------------------------------------------------
// Verify 'order' is a permutation of 0..V-1 and that for every edge u -> v,
// u appears strictly before v.
bool isValidTopoOrder(const Graph& adj, const std::vector<int>& order) {
    const std::size_t V = adj.size();
    if (order.size() != V) return false;

    std::vector<int> pos(V, -1);
    for (std::size_t i = 0; i < order.size(); ++i) {
        const std::size_t vtx = static_cast<std::size_t>(order[i]);
        if (pos[vtx] != -1) return false;  // duplicate -> not a permutation
        pos[vtx] = static_cast<int>(i);
    }
    for (std::size_t u = 0; u < V; ++u) {
        for (const int v : adj[u]) {
            if (pos[u] >= pos[static_cast<std::size_t>(v)]) {
                return false;  // edge violated: u not before v
            }
        }
    }
    return true;
}

int main() {
    // Known DAG (directed). A classic "course prerequisites" style graph.
    //
    //     0 --> 1 --> 3
    //     |     ^     ^
    //     v     |     |
    //     2 ----+     |
    //     |           |
    //     +---------> 4 --> 5
    //
    // Edges: 0->1, 0->2, 1->3, 2->1, 2->4, 4->3, 4->5
    Graph dag(6);
    dag[0] = {1, 2};
    dag[1] = {3};
    dag[2] = {1, 4};
    dag[4] = {3, 5};

    // Kahn.
    std::vector<int> orderKahn;
    assert(topoSortKahn(dag, orderKahn));            // acyclic -> succeeds
    assert(isValidTopoOrder(dag, orderKahn));        // respects every edge

    // DFS.
    std::vector<int> orderDfs;
    assert(topoSortDfs(dag, orderDfs));
    assert(isValidTopoOrder(dag, orderDfs));

    // Disconnected DAG: two independent chains plus an isolated vertex.
    //   10:  0 -> 1        2 -> 3        4 (isolated)
    Graph disc(5);
    disc[0] = {1};
    disc[2] = {3};
    // vertex 4 isolated
    std::vector<int> o1, o2;
    assert(topoSortKahn(disc, o1) && isValidTopoOrder(disc, o1));
    assert(topoSortDfs(disc, o2) && isValidTopoOrder(disc, o2));

    // Single-vertex graph edge case.
    Graph solo(1);
    std::vector<int> os;
    assert(topoSortKahn(solo, os) && os == std::vector<int>({0}));
    assert(topoSortDfs(solo, os) && os == std::vector<int>({0}));

    // Cyclic graph: 0 -> 1 -> 2 -> 0. No valid ordering; BOTH methods report it.
    Graph cyc(3);
    cyc[0] = {1};
    cyc[1] = {2};
    cyc[2] = {0};
    std::vector<int> cKahn, cDfs;
    assert(!topoSortKahn(cyc, cKahn));  // Kahn: fewer than V emitted
    assert(!topoSortDfs(cyc, cDfs));    // DFS: back edge to a gray ancestor

    // Short demo output.
    std::cout << "Topological order (Kahn):";
    for (const int v : orderKahn) std::cout << ' ' << v;
    std::cout << "\nTopological order (DFS) :";
    for (const int v : orderDfs) std::cout << ' ' << v;
    std::cout << "\nCycle 0->1->2->0 has valid ordering? "
              << (topoSortKahn(cyc, cKahn) ? "yes" : "no") << '\n';
    std::cout << "All topological sort tests passed.\n";
    return 0;
}
