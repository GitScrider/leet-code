/*
 * Structure name & category: Graph (Adjacency Matrix) -- Data Structure / Graph.
 *
 * Summary:
 *   A graph stored as a dense V x V matrix where entry [u][v] holds the weight
 *   of the edge u->v (or a sentinel meaning "no edge"). Supports directed and
 *   undirected graphs and answers "is there an edge?" in constant time.
 *
 * Operations & complexity (V = #vertices):
 *   +----------------+-----------+----------------------------------------------+
 *   | Operation      | Big-O     | Why                                          |
 *   +----------------+-----------+----------------------------------------------+
 *   | addEdge        | O(1)      | direct index into the matrix                 |
 *   | removeEdge     | O(1)      | write sentinel back into the cell(s)         |
 *   | hasEdge        | O(1)      | single matrix read -- the headline strength  |
 *   | neighbors      | O(V)      | must scan one full row                       |
 *   | degree         | O(V)      | scan a row (out-degree)                      |
 *   | vertexCount    | O(1)      | stored integer                               |
 *   | edgeCount      | O(1)      | maintained incrementally on add/remove       |
 *   | (space)        | O(V^2)    | the whole matrix is materialized             |
 *   +----------------+-----------+----------------------------------------------+
 *
 * Invariants / key ideas:
 *   - matrix_ is exactly V rows by V columns; indices are vertex ids in [0, V).
 *   - NO_EDGE (a sentinel) marks a missing edge so we can also store weight 0.
 *   - Undirected graphs keep the matrix SYMMETRIC: [u][v] == [v][u] always.
 *   - edgeCount_ is kept in sync so edgeCount() is O(1); for undirected graphs
 *     a stored edge {u,v} is counted once even though it occupies two cells.
 *
 * When to use / trade-offs:
 *   - Great for DENSE graphs (E ~ V^2) and when you need O(1) edge queries.
 *   - Wasteful for SPARSE graphs: O(V^2) memory even with a handful of edges,
 *     and neighbors()/degree() are O(V) regardless of the real edge count.
 *   - Contrast with an adjacency LIST (graph_adjacency_list.cpp): a list uses
 *     O(V + E) space and iterates neighbors in O(deg(u)), but its hasEdge is
 *     O(deg(u)) rather than O(1). Pick the matrix when queries dominate and the
 *     graph is dense; pick the list when the graph is sparse and you iterate.
 */

#include <cassert>
#include <cstddef>   // std::size_t
#include <iostream>
#include <stdexcept>
#include <vector>

// Backed entirely by std::vector, so the Rule of Zero applies: the compiler-
// generated destructor, copy, and move members already do the right thing
// (deep-copy / move the underlying vectors). We deliberately hand-write NO
// special members below.
class GraphMatrix {
public:
    // Sentinel stored in a cell that has no edge. Using a distinct value (not 0)
    // lets a real edge legitimately carry weight 0.
    static constexpr int NO_EDGE = -1;

    // Construct a graph on `vertexCount` vertices (ids 0..vertexCount-1).
    // `directed == false` => undirected (the matrix is kept symmetric).
    explicit GraphMatrix(std::size_t vertexCount, bool directed = false)
        : directed_(directed),
          matrix_(vertexCount, std::vector<int>(vertexCount, NO_EDGE)) {}

    // ---- Mutators -----------------------------------------------------------

    // Add (or overwrite) edge u->v with weight w (default 1). Undirected graphs
    // set BOTH [u][v] and [v][u] to preserve symmetry. O(1).
    void addEdge(std::size_t u, std::size_t v, int w = 1) {
        checkBounds(u, v);
        if (w == NO_EDGE) {
            throw std::invalid_argument("weight collides with the NO_EDGE sentinel");
        }
        // Only bump edgeCount_ when this is a brand-new edge; re-weighting an
        // existing edge must not double-count it.
        if (matrix_[u][v] == NO_EDGE) {
            ++edgeCount_;
        }
        matrix_[u][v] = w;
        if (!directed_) {
            matrix_[v][u] = w; // symmetry invariant for undirected graphs
        }
    }

    // Remove edge u->v (and v->u when undirected). No-op if it does not exist.
    // Returns true iff an edge was actually removed. O(1).
    bool removeEdge(std::size_t u, std::size_t v) {
        checkBounds(u, v);
        if (matrix_[u][v] == NO_EDGE) {
            return false; // nothing to remove
        }
        matrix_[u][v] = NO_EDGE;
        if (!directed_) {
            matrix_[v][u] = NO_EDGE;
        }
        --edgeCount_;
        return true;
    }

    // ---- Queries ------------------------------------------------------------

    // The headline operation: constant-time edge existence check. O(1).
    bool hasEdge(std::size_t u, std::size_t v) const {
        checkBounds(u, v);
        return matrix_[u][v] != NO_EDGE;
    }

    // Weight of edge u->v. Precondition: the edge exists (assert in debug).
    int weight(std::size_t u, std::size_t v) const {
        checkBounds(u, v);
        assert(matrix_[u][v] != NO_EDGE && "weight() called on a missing edge");
        return matrix_[u][v];
    }

    // All out-neighbors of u, in ascending id order. Must scan the whole row,
    // hence O(V) even if u has few neighbors -- a cost inherent to the matrix.
    std::vector<std::size_t> neighbors(std::size_t u) const {
        checkVertex(u);
        std::vector<std::size_t> result;
        for (std::size_t v = 0; v < size(); ++v) {
            if (matrix_[u][v] != NO_EDGE) {
                result.push_back(v);
            }
        }
        return result;
    }

    // Out-degree of u (for undirected graphs this equals the usual degree).
    // O(V): we count non-sentinel entries in the row.
    std::size_t degree(std::size_t u) const {
        checkVertex(u);
        std::size_t d = 0;
        for (std::size_t v = 0; v < size(); ++v) {
            if (matrix_[u][v] != NO_EDGE) {
                ++d;
            }
        }
        return d;
    }

    std::size_t vertexCount() const { return size(); }

    // Number of distinct edges. Maintained incrementally so this stays O(1);
    // for undirected graphs each {u,v} pair counts once (not twice).
    std::size_t edgeCount() const { return edgeCount_; }

    bool isDirected() const { return directed_; }

private:
    std::size_t size() const { return matrix_.size(); }

    void checkVertex(std::size_t u) const {
        if (u >= size()) {
            throw std::out_of_range("vertex id out of range");
        }
    }

    void checkBounds(std::size_t u, std::size_t v) const {
        checkVertex(u);
        checkVertex(v);
    }

    bool directed_;
    std::size_t edgeCount_ = 0;
    std::vector<std::vector<int>> matrix_; // V x V; NO_EDGE marks absence
};

// Out-of-class definition required for constexpr static data member in C++17
// when it is odr-used (e.g. bound to a reference); harmless otherwise.
constexpr int GraphMatrix::NO_EDGE;

// ---------------------------------------------------------------------------
// Tests + demo
// ---------------------------------------------------------------------------
int main() {
    // ---- Undirected graph: symmetry, O(1) hasEdge, neighbors, degree -------
    {
        GraphMatrix g(5, /*directed=*/false);
        assert(g.vertexCount() == 5);
        assert(g.edgeCount() == 0);
        assert(!g.hasEdge(0, 1)); // O(1) lookup on an empty graph

        g.addEdge(0, 1);
        g.addEdge(0, 4);
        g.addEdge(1, 2);
        g.addEdge(2, 3);

        // O(1) existence checks in both directions confirm symmetry.
        assert(g.hasEdge(0, 1) && g.hasEdge(1, 0));
        assert(g.hasEdge(0, 4) && g.hasEdge(4, 0));
        assert(!g.hasEdge(0, 2) && !g.hasEdge(2, 0));
        assert(g.edgeCount() == 4); // each undirected edge counted once

        // The underlying matrix must be symmetric everywhere.
        for (std::size_t u = 0; u < g.vertexCount(); ++u) {
            for (std::size_t v = 0; v < g.vertexCount(); ++v) {
                assert(g.hasEdge(u, v) == g.hasEdge(v, u));
            }
        }

        // neighbors(u) returns exactly the right set, in ascending order.
        std::vector<std::size_t> n0 = g.neighbors(0);
        assert((n0 == std::vector<std::size_t>{1, 4}));
        std::vector<std::size_t> n2 = g.neighbors(2);
        assert((n2 == std::vector<std::size_t>{1, 3}));

        assert(g.degree(0) == 2);
        assert(g.degree(3) == 1);
        assert(g.degree(4) == 1);

        // removeEdge clears BOTH directions and returns true; a second remove
        // is a no-op returning false.
        assert(g.removeEdge(0, 1));
        assert(!g.hasEdge(0, 1) && !g.hasEdge(1, 0));
        assert(g.degree(0) == 1);
        assert(g.edgeCount() == 3);
        assert(!g.removeEdge(0, 1)); // already gone
        assert(g.edgeCount() == 3);  // unchanged
    }

    // ---- Directed + weighted graph: asymmetry, re-weighting, weight 0 ------
    {
        GraphMatrix d(3, /*directed=*/true);
        d.addEdge(0, 1, 7);
        assert(d.hasEdge(0, 1));
        assert(!d.hasEdge(1, 0)); // direction matters; no symmetry
        assert(d.weight(0, 1) == 7);
        assert(d.edgeCount() == 1);

        // Re-weighting an existing edge must NOT change the edge count.
        d.addEdge(0, 1, 42);
        assert(d.weight(0, 1) == 42);
        assert(d.edgeCount() == 1);

        // Weight 0 is a valid edge thanks to the NO_EDGE sentinel being -1.
        d.addEdge(2, 0, 0);
        assert(d.hasEdge(2, 0));
        assert(d.weight(2, 0) == 0);
        assert(d.edgeCount() == 2);

        assert((d.neighbors(0) == std::vector<std::size_t>{1}));
        assert(d.degree(0) == 1); // out-degree
        assert(d.degree(1) == 0);
    }

    // ---- Defensive checks: bounds and the sentinel-collision guard ---------
    {
        GraphMatrix g(2, false);
        bool threw = false;
        try { g.addEdge(0, 5); } catch (const std::out_of_range&) { threw = true; }
        assert(threw);

        threw = false;
        try { g.addEdge(0, 1, GraphMatrix::NO_EDGE); }
        catch (const std::invalid_argument&) { threw = true; }
        assert(threw);
    }

    // ---- Human-readable demo ------------------------------------------------
    GraphMatrix demo(4, /*directed=*/false);
    demo.addEdge(0, 1);
    demo.addEdge(0, 2);
    demo.addEdge(1, 2);
    demo.addEdge(2, 3);

    std::cout << "Undirected graph, " << demo.vertexCount() << " vertices, "
              << demo.edgeCount() << " edges.\n";
    std::cout << "Adjacency matrix (1 = edge, . = none):\n    ";
    for (std::size_t v = 0; v < demo.vertexCount(); ++v) std::cout << v << ' ';
    std::cout << '\n';
    for (std::size_t u = 0; u < demo.vertexCount(); ++u) {
        std::cout << "  " << u << ' ';
        for (std::size_t v = 0; v < demo.vertexCount(); ++v) {
            std::cout << (demo.hasEdge(u, v) ? '1' : '.') << ' ';
        }
        std::cout << '\n';
    }
    for (std::size_t u = 0; u < demo.vertexCount(); ++u) {
        std::cout << "  neighbors(" << u << ") = {";
        std::vector<std::size_t> ns = demo.neighbors(u);
        for (std::size_t i = 0; i < ns.size(); ++i) {
            std::cout << ns[i] << (i + 1 < ns.size() ? ", " : "");
        }
        std::cout << "}  degree=" << demo.degree(u) << '\n';
    }

    std::cout << "All assertions passed.\n";
    return 0;
}
