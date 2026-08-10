/*
 * ============================================================================
 * Union-Find (Disjoint Set Union, DSU)
 * Category: Data Structure - Graph (disjoint sets / connectivity)
 *
 * Summary:
 *   Maintains a partition of {0, 1, ..., n-1} into disjoint sets and answers
 *   "are these two elements in the same set?" while supporting fast merges.
 *   Uses BOTH classic optimizations: path compression and union by size.
 *
 * Operations & complexity (n = number of elements, alpha = inverse Ackermann):
 *   +-------------------+-------------------------------+
 *   | Operation         | Time                          |
 *   +-------------------+-------------------------------+
 *   | DSU(n)            | O(n)                          |
 *   | find(x)           | O(alpha(n)) amortized         |
 *   | unite(x, y)       | O(alpha(n)) amortized         |
 *   | connected(x, y)   | O(alpha(n)) amortized         |
 *   | count()           | O(1)                          |
 *   | componentSize(x)  | O(alpha(n)) amortized         |
 *   +-------------------+-------------------------------+
 *   With BOTH path compression AND union by size, the amortized cost per
 *   operation is O(alpha(n)) -- the inverse Ackermann function -- which is
 *   <= 4 for any n that fits in the observable universe, i.e. effectively
 *   constant. Neither optimization alone achieves this bound.
 *
 * Invariants / key ideas:
 *   - Each set is a rooted tree. parent[root] == root (self-loop marks a root).
 *   - size[r] is meaningful ONLY when r is a root; it holds the element count
 *     of that set. This lets union-by-size attach the smaller tree under the
 *     larger, keeping trees shallow.
 *   - Path compression: find(x) re-points every node on the path directly to
 *     the root, FLATTENING the tree so future queries are near O(1).
 *   - components == number of distinct roots, tracked incrementally.
 *
 * When to use / trade-offs:
 *   - Ideal for offline/online connectivity: Kruskal's MST, cycle detection in
 *     undirected graphs, grouping "friends of friends", percolation.
 *   - Near-constant amortized cost with tiny memory (two int vectors).
 *   - Supports UNION and FIND but NOT deletion or splitting of sets.
 *   - Element ids must be a dense range 0..n-1 (remap arbitrary keys first).
 * ============================================================================
 */

#include <vector>
#include <numeric>   // std::iota
#include <cstddef>   // std::size_t
#include <utility>   // std::swap
#include <cassert>
#include <iostream>

// Backed entirely by std::vector -> Rule of Zero: the compiler-generated
// destructor, copy, and move members are already correct, so we hand-write none.
class UnionFind {
public:
    // Constructor: create n singleton sets {0}, {1}, ..., {n-1}.
    // Each element starts as its own root (parent[i] == i) with size 1.
    explicit UnionFind(std::size_t n)
        : parent_(n), size_(n, 1), components_(n) {
        // std::iota fills parent_ with 0, 1, 2, ... so every node points to itself.
        std::iota(parent_.begin(), parent_.end(), std::size_t{0});
    }

    // find(x): return the representative (root) of x's set.
    // Iterative two-pass method (no recursion -> no stack overflow on long chains):
    //   Pass 1 walks up to the root; Pass 2 compresses every visited node to
    //   point straight at the root, FLATTENING the tree for future queries.
    std::size_t find(std::size_t x) {
        assert(x < parent_.size());
        std::size_t root = x;
        while (parent_[root] != root) {
            root = parent_[root];
        }
        // Path compression: re-point each node on the path directly to the root.
        while (parent_[x] != root) {
            std::size_t next = parent_[x];
            parent_[x] = root;
            x = next;
        }
        return root;
    }

    // unite(x, y): merge the sets containing x and y.
    // Returns false (a NO-OP) if they were already in the same set, true if a
    // real merge happened. Union by size: attach the SMALLER tree under the
    // LARGER root so the combined tree stays shallow.
    bool unite(std::size_t x, std::size_t y) {
        std::size_t rx = find(x);
        std::size_t ry = find(y);
        if (rx == ry) {
            return false;  // Already connected: merging would be a no-op.
        }
        // Ensure rx is the root of the larger set; swap if not.
        if (size_[rx] < size_[ry]) {
            std::swap(rx, ry);
        }
        parent_[ry] = rx;          // Smaller tree (ry) hangs under larger root (rx).
        size_[rx] += size_[ry];    // Root size accounts for the absorbed set.
        --components_;             // Two sets became one.
        return true;
    }

    // connected(x, y): true iff x and y share a root (same set).
    bool connected(std::size_t x, std::size_t y) {
        return find(x) == find(y);
    }

    // count(): number of disjoint sets currently in the structure.
    std::size_t count() const {
        return components_;
    }

    // componentSize(x): number of elements in the set that contains x.
    std::size_t componentSize(std::size_t x) {
        return size_[find(x)];  // size_ is valid only at the root, so query it there.
    }

private:
    std::vector<std::size_t> parent_;  // parent_[i] = parent of i; root iff parent_[i]==i.
    std::vector<std::size_t> size_;    // size_[r] = element count of set rooted at r.
    std::size_t components_;           // Live count of disjoint sets.
};

int main() {
    // ---- Test 1: n singletons, count() == n initially, none connected. ----
    UnionFind uf(6);
    assert(uf.count() == 6);
    for (std::size_t i = 0; i < 6; ++i) {
        assert(uf.componentSize(i) == 1);          // Each singleton has size 1.
        assert(uf.connected(i, i));                // An element is always self-connected.
        for (std::size_t j = i + 1; j < 6; ++j) {
            assert(!uf.connected(i, j));           // Distinct singletons are separate.
        }
    }

    // ---- Test 2: uniting pairs connects them and decrements count(). ----
    assert(uf.unite(0, 1) == true);                // Real merge.
    assert(uf.connected(0, 1));
    assert(uf.count() == 5);                        // 6 -> 5 sets.
    assert(uf.componentSize(0) == 2 && uf.componentSize(1) == 2);

    assert(uf.unite(2, 3) == true);
    assert(uf.count() == 4);

    // ---- Test 3: uniting already-connected elements is a NO-OP. ----
    assert(uf.unite(0, 1) == false);               // Same set -> returns false.
    assert(uf.count() == 4);                        // Count unchanged.
    // Merge {0,1} with {2,3}; then re-uniting internal members is still a no-op.
    assert(uf.unite(1, 2) == true);
    assert(uf.count() == 3);
    assert(uf.componentSize(0) == 4);              // {0,1,2,3}
    assert(uf.unite(3, 0) == false);               // Already connected across the merge.
    assert(uf.count() == 3);

    // ---- Test 4: componentSize grows correctly as sets merge. ----
    assert(uf.componentSize(0) == 4);
    assert(uf.componentSize(1) == 4);
    assert(uf.componentSize(2) == 4);
    assert(uf.componentSize(3) == 4);
    assert(uf.componentSize(4) == 1);              // 4 and 5 untouched so far.
    assert(uf.componentSize(5) == 1);

    // ---- Test 5: two-component connectivity scenario. ----
    // Component A = {0,1,2,3} (built above). Component B = {4,5}.
    assert(uf.unite(4, 5) == true);
    assert(uf.count() == 2);                        // Exactly two components remain.
    assert(uf.connected(4, 5));
    assert(uf.componentSize(4) == 2 && uf.componentSize(5) == 2);
    // Cross-component elements must NOT be connected.
    for (std::size_t a : {std::size_t{0}, std::size_t{1}, std::size_t{2}, std::size_t{3}}) {
        for (std::size_t b : {std::size_t{4}, std::size_t{5}}) {
            assert(!uf.connected(a, b));
        }
    }

    // ---- Test 6: merging the two components leaves a single set. ----
    assert(uf.unite(3, 5) == true);
    assert(uf.count() == 1);                        // All 6 elements unified.
    assert(uf.componentSize(0) == 6);
    for (std::size_t i = 0; i < 6; ++i) {
        for (std::size_t j = 0; j < 6; ++j) {
            assert(uf.connected(i, j));            // Everything is now connected.
        }
    }

    // ---- Test 7: path compression correctness on a long chain. ----
    // A deliberately lopsided series of unites should still answer queries
    // correctly and stay flat thanks to compression + union by size.
    UnionFind chain(1000);
    for (std::size_t i = 0; i + 1 < 1000; ++i) {
        chain.unite(i, i + 1);
    }
    assert(chain.count() == 1);
    assert(chain.componentSize(0) == 1000);
    assert(chain.connected(0, 999));

    // ---- Human-readable demo. ----
    std::cout << "Union-Find (DSU) demo\n";
    std::cout << "---------------------\n";
    UnionFind demo(5);
    std::cout << "Start: " << demo.count() << " singleton sets {0}{1}{2}{3}{4}\n";
    demo.unite(0, 2);
    demo.unite(4, 2);
    std::cout << "After unite(0,2), unite(4,2): " << demo.count()
              << " sets; component of 0 has size " << demo.componentSize(0) << "\n";
    demo.unite(1, 3);
    std::cout << "After unite(1,3): " << demo.count() << " sets\n";
    std::cout << "connected(0, 4)? " << (demo.connected(0, 4) ? "yes" : "no") << "\n";
    std::cout << "connected(0, 1)? " << (demo.connected(0, 1) ? "yes" : "no") << "\n";
    std::cout << "unite(3, 4) merges the two groups... ";
    demo.unite(3, 4);
    std::cout << demo.count() << " set left, size "
              << demo.componentSize(0) << "\n";

    std::cout << "\nAll assertions passed.\n";
    return 0;
}
