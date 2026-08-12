/*
 * Breadth-First Search (Algorithm - Graphs)
 *
 * Idea:
 *   Explore the graph in "rings" of increasing distance from a source. A FIFO
 *   queue holds the frontier; a boolean visited[] array guarantees each vertex
 *   is enqueued at most once. We pop a vertex, then push every unvisited
 *   neighbor, marking it visited AT ENQUEUE TIME (not at dequeue time) so it can
 *   never be queued twice. Because the queue processes vertices in nondecreasing
 *   distance order, the first time we reach a vertex we have reached it by a
 *   shortest path (fewest edges). We record dist[v] = dist[u] + 1 and a
 *   parent[v] = u so any shortest path can be reconstructed by walking parents.
 *
 * Complexity (V = number of vertices, E = number of edges):
 *   +-----------+------------------+
 *   |  Measure  |       Cost       |
 *   +-----------+------------------+
 *   |  Time     |   O(V + E)       |
 *   |  Space    |   O(V)           |
 *   +-----------+------------------+
 *   Each vertex is enqueued/dequeued once (O(V)); each adjacency-list entry is
 *   scanned once (O(E), or 2E for an undirected graph). Space is the queue plus
 *   the visited/dist/parent arrays, all O(V).
 *
 * Complexity derivation (aggregate vertex + edge count over the BFS loop):
 *   Initialization assigns dist/parent/visited for all V vertices: c0*V ops.
 *   The outer while loop dequeues each vertex AT MOST ONCE, because a vertex is
 *   marked visited the instant it is enqueued and only unvisited vertices are
 *   ever pushed; hence the body runs once per reachable vertex, <= V times.
 *   When u is dequeued its inner for loop scans exactly deg(u) adjacency entries,
 *   each handled in O(1). Summing the per-vertex work over all dequeued vertices:
 *
 *       C(V,E) = c0*V + SUM_{u dequeued} (1 + deg(u))
 *              = c0*V + V + SUM_{u in V} deg(u)
 *              = c0*V + V + 2E     (undirected: each edge is seen from BOTH
 *                                   endpoints, so SUM_{u} deg(u) = 2E)
 *              = O(V + E)
 *
 *   For a DIRECTED graph SUM_{u} out-deg(u) = E, giving c0*V + V + E; either way
 *   the order is O(V + E). No vertex is dequeued twice and no adjacency entry is
 *   scanned twice, so the operation count is exact (not just an upper bound).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants) on size n = V + E:
 *     f(n) = O(g)      iff  EXISTS c2, n0 :       f(n) <= c2*g(n)  for n >= n0
 *     f(n) = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f(n) = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   Here f(V,E) = c0*V + V + 2E is linear in V + E; take g = V + E:
 *     upper  O:     f <= (c0 + 3) * (V + E)              => O(V + E)
 *     lower  Omega: f >=  1        * (V + E)             => Omega(V + E)
 *     tight  Theta: both hold (c1 = 1, c2 = c0 + 3)      => Theta(V + E)
 *   BFS initializes all V vertices and scans every reachable edge exactly once,
 *   so on a connected graph the work is input-INDEPENDENT and Theta(V + E) is the
 *   tight bound for best, average and worst case alike. If only part of the graph
 *   is reachable from source, only that component's V'+E' is paid, still bounded
 *   above by V + E.
 *
 * Key points / assumptions:
 *   - Works on directed or undirected graphs; here the demo graph is undirected
 *     (we insert both (u,v) and (v,u)).
 *   - UNWEIGHTED: BFS gives shortest paths in number of EDGES. For weighted
 *     shortest paths use Dijkstra / Bellman-Ford instead.
 *   - Unreachable vertices keep dist == INF and parent == -1 (no path exists).
 *   - Neighbor iteration order fixes the visit order, so tests are deterministic.
 */

#include <vector>
#include <queue>
#include <cassert>
#include <iostream>
#include <cstddef>

// Sentinel meaning "unreachable". Real distances are always >= 0, so the
// negative value -1 can never collide with a genuine (finite) distance.
constexpr int INF = -1;  // distances are >= 0, so -1 is an unambiguous "no path"

// Undirected adjacency list represented as vector<vector<int>>: adj[u] lists the
// neighbors of u. Vertices are ints 0..V-1.
using Graph = std::vector<std::vector<int>>;

// Result bundle: the order vertices were first visited (dequeued), plus the
// distance and parent arrays measured from 'source'.
struct BfsResult {
    std::vector<int> order;   // vertices in the order BFS first reaches them
    std::vector<int> dist;    // dist[v] = shortest #edges from source, or INF
    std::vector<int> parent;  // parent[v] on a shortest-path tree, or -1
};

BfsResult bfs(const Graph& adj, int source) {
    const std::size_t V = adj.size();
    BfsResult r;
    r.dist.assign(V, INF);
    r.parent.assign(V, -1);
    std::vector<bool> visited(V, false);

    std::queue<int> q;
    // Seed the frontier with the source: distance 0, no parent.
    visited[static_cast<std::size_t>(source)] = true;
    r.dist[static_cast<std::size_t>(source)] = 0;
    q.push(source);

    while (!q.empty()) {
        const int u = q.front();
        q.pop();
        r.order.push_back(u);  // u is now finalized at its shortest distance

        // Relax every edge out of u. Marking neighbors visited HERE (at enqueue)
        // is what keeps each vertex out of the queue more than once.
        for (const int v : adj[static_cast<std::size_t>(u)]) {
            if (!visited[static_cast<std::size_t>(v)]) {
                visited[static_cast<std::size_t>(v)] = true;
                r.dist[static_cast<std::size_t>(v)] =
                    r.dist[static_cast<std::size_t>(u)] + 1;  // one more edge
                r.parent[static_cast<std::size_t>(v)] = u;
                q.push(v);
            }
        }
    }
    return r;
}

// Reconstruct the shortest path source -> target by following parents backward,
// then reversing. Returns an empty vector if target is unreachable.
std::vector<int> reconstructPath(const BfsResult& r, int source, int target) {
    if (r.dist[static_cast<std::size_t>(target)] == INF) {
        return {};  // no path
    }
    std::vector<int> path;
    for (int cur = target; cur != -1; cur = r.parent[static_cast<std::size_t>(cur)]) {
        path.push_back(cur);
        if (cur == source) break;
    }
    // We collected target..source; reverse to source..target.
    std::vector<int> forward(path.rbegin(), path.rend());
    return forward;
}

// Add an undirected edge (both directions) to keep the demo self-contained.
void addUndirectedEdge(Graph& adj, int u, int v) {
    adj[static_cast<std::size_t>(u)].push_back(v);
    adj[static_cast<std::size_t>(v)].push_back(u);
}

int main() {
    // Known graph (undirected). Vertex 6 is isolated (disconnected).
    /*
     *        0
     *       / \
     *      1   2
     *     / \   \
     *    3   4   5        6  (isolated)
     */
    // Adjacency built in a fixed order so BFS visit order is deterministic.
    Graph adj(7);
    addUndirectedEdge(adj, 0, 1);
    addUndirectedEdge(adj, 0, 2);
    addUndirectedEdge(adj, 1, 3);
    addUndirectedEdge(adj, 1, 4);
    addUndirectedEdge(adj, 2, 5);
    // vertex 6 has no edges

    const BfsResult r = bfs(adj, 0);

    // Visit order from source 0: level 0 -> {0}, level 1 -> {1,2},
    // level 2 -> {3,4} (children of 1) then {5} (child of 2).
    const std::vector<int> expectedOrder = {0, 1, 2, 3, 4, 5};
    assert(r.order == expectedOrder);

    // Distances in edges from source 0.
    assert(r.dist[0] == 0);
    assert(r.dist[1] == 1);
    assert(r.dist[2] == 1);
    assert(r.dist[3] == 2);
    assert(r.dist[4] == 2);
    assert(r.dist[5] == 2);
    assert(r.dist[6] == INF);  // vertex 6 is disconnected -> unreachable

    // Parent (shortest-path tree) checks.
    assert(r.parent[0] == -1);  // source has no parent
    assert(r.parent[3] == 1);
    assert(r.parent[5] == 2);
    assert(r.parent[6] == -1);  // unreachable -> no parent

    // Path reconstruction: 0 -> 1 -> 4.
    const std::vector<int> path04 = reconstructPath(r, 0, 4);
    const std::vector<int> expectedPath = {0, 1, 4};
    assert(path04 == expectedPath);

    // No path to the isolated vertex.
    assert(reconstructPath(r, 0, 6).empty());

    // Single-vertex graph edge case.
    Graph solo(1);
    const BfsResult rs = bfs(solo, 0);
    assert(rs.order == std::vector<int>({0}));
    assert(rs.dist[0] == 0);
    assert(rs.parent[0] == -1);

    // Short demo output.
    std::cout << "BFS from 0 visit order:";
    for (const int v : r.order) std::cout << ' ' << v;
    std::cout << "\nDistances (edges) from 0: ";
    for (std::size_t v = 0; v < r.dist.size(); ++v) {
        std::cout << v << ':';
        if (r.dist[v] == INF) std::cout << "INF ";
        else std::cout << r.dist[v] << ' ';
    }
    std::cout << "\nShortest path 0->4:";
    for (const int v : path04) std::cout << ' ' << v;
    std::cout << "\nAll BFS tests passed.\n";
    return 0;
}
