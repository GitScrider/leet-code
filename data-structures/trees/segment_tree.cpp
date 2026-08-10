/*
 * Segment Tree (Data Structure - Tree)
 * -----------------------------------------------------------------------------
 * Summary:
 *   An array-backed binary tree over a fixed-size sequence that answers
 *   range-sum queries on [l, r] and point updates, each in O(log n), by storing
 *   a partial aggregate (here: the sum) for every dyadic segment of the array.
 *
 * Operations & complexity (n = number of leaves / array size):
 *   Operation              | Time      | Space
 *   -----------------------+-----------+-------
 *   build(vector<T>)       | O(n)      | O(n)   -- one post-order pass
 *   query(l, r) sum        | O(log n)  | O(log n) recursion stack
 *   update(i, newValue)    | O(log n)  | O(log n) recursion stack
 *   size()                 | O(1)      | O(1)
 *
 * Invariants:
 *   - Fixed-topology complete binary tree laid out in a flat array `tree_`,
 *     root at index 1. For an internal node at index `node`, its children live
 *     at indices 2*node (left) and 2*node+1 (right). We reserve 4*n slots, the
 *     safe upper bound for a non-power-of-two n (the tree can have up to 2*2^ceil
 *     nodes; 4n always covers it).
 *   - Segment invariant: every node covers a contiguous index range [lo, hi] of
 *     the original array and stores tree_[node] == sum(original[lo..hi]).
 *     Leaves (lo == hi) hold a single element; internal nodes hold the sum of
 *     their two children. This is the property build/update restore and query
 *     relies on.
 *
 * When to use / trade-offs:
 *   - Best when you need many interleaved range queries AND point updates; a
 *     plain prefix-sum array gives O(1) queries but O(n) updates.
 *   - Generalizes to any *associative* combine (min, max, gcd, ...), unlike a
 *     Fenwick tree which needs an invertible group operation for range queries.
 *   - For range *updates* (add v to all of [l, r]) extend with LAZY
 *     PROPAGATION: store a pending delta per node and push it down on demand.
 *     Deliberately NOT implemented here to keep the point-update version simple
 *     and obviously correct.
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <utility>   // std::move (used in the tests)
#include <vector>

template <typename T>
class SegmentTree {
public:
    SegmentTree() = default;

    // Convenience: build straight from data.
    explicit SegmentTree(const std::vector<T>& data) { build(data); }

    // Value semantics: the ONLY data members are std::vectors and a size_t.
    // std::vector already performs deep copy/move and frees its own storage, so
    // the compiler-generated copy/move/destructor are correct here -- there is
    // no raw owning pointer and therefore no double-free hazard. We default them
    // explicitly to document that this is a considered choice, not an oversight.
    SegmentTree(const SegmentTree&) = default;
    SegmentTree& operator=(const SegmentTree&) = default;
    SegmentTree(SegmentTree&&) noexcept = default;
    SegmentTree& operator=(SegmentTree&&) noexcept = default;
    ~SegmentTree() = default;

    // Build the tree from an array in O(n): a single recursive post-order fill.
    void build(const std::vector<T>& data) {
        n_ = data.size();
        tree_.assign(n_ == 0 ? 0 : 4 * n_, T{});
        if (n_ > 0) {
            buildRec(1, 0, n_ - 1, data);
        }
    }

    std::size_t size() const { return n_; }
    bool empty() const { return n_ == 0; }

    // Sum over the inclusive range [l, r]. Precondition: 0 <= l <= r < n.
    T query(std::size_t l, std::size_t r) const {
        assert(!empty() && "query on empty tree");
        assert(l <= r && r < n_ && "query range out of bounds");
        return queryRec(1, 0, n_ - 1, l, r);
    }

    // Point update: set original[index] = newValue and repair every ancestor.
    void update(std::size_t index, const T& newValue) {
        assert(index < n_ && "update index out of bounds");
        updateRec(1, 0, n_ - 1, index, newValue);
    }

private:
    std::vector<T> tree_;   // 1-indexed; tree_[0] unused.
    std::size_t n_ = 0;     // number of leaves (original array length).

    // Post-order build: fill children first, then combine into the parent.
    // Each array position is written exactly once => O(n) total.
    void buildRec(std::size_t node, std::size_t lo, std::size_t hi,
                  const std::vector<T>& data) {
        if (lo == hi) {                 // leaf covers a single element
            tree_[node] = data[lo];
            return;
        }
        std::size_t mid = lo + (hi - lo) / 2;   // avoids overflow vs (lo+hi)/2
        buildRec(2 * node, lo, mid, data);
        buildRec(2 * node + 1, mid + 1, hi, data);
        tree_[node] = tree_[2 * node] + tree_[2 * node + 1];
    }

    // Three cases drive the O(log n) query:
    //   (1) [lo,hi] entirely outside [l,r]  -> contributes the identity (0).
    //   (2) [lo,hi] entirely inside  [l,r]  -> return the stored aggregate.
    //   (3) partial overlap                 -> recurse into both children.
    // Only O(log n) nodes are ever fully covered or split, bounding the work.
    T queryRec(std::size_t node, std::size_t lo, std::size_t hi,
               std::size_t l, std::size_t r) const {
        if (r < lo || hi < l) {         // (1) disjoint
            return T{};                 // additive identity
        }
        if (l <= lo && hi <= r) {       // (2) fully covered
            return tree_[node];
        }
        std::size_t mid = lo + (hi - lo) / 2;   // (3) split
        return queryRec(2 * node, lo, mid, l, r) +
               queryRec(2 * node + 1, mid + 1, hi, l, r);
    }

    // Walk down to the leaf holding `index`, overwrite it, then recompute each
    // ancestor on the way back up so the segment invariant is restored.
    void updateRec(std::size_t node, std::size_t lo, std::size_t hi,
                   std::size_t index, const T& newValue) {
        if (lo == hi) {                 // reached the target leaf
            tree_[node] = newValue;
            return;
        }
        std::size_t mid = lo + (hi - lo) / 2;
        if (index <= mid) {
            updateRec(2 * node, lo, mid, index, newValue);
        } else {
            updateRec(2 * node + 1, mid + 1, hi, index, newValue);
        }
        tree_[node] = tree_[2 * node] + tree_[2 * node + 1];  // repair parent
    }
};

// ---------------------------------------------------------------------------
// Tests + demo
// ---------------------------------------------------------------------------

// Brute-force reference so the tests validate the tree against ground truth.
template <typename T>
static T bruteSum(const std::vector<T>& a, std::size_t l, std::size_t r) {
    T s{};
    for (std::size_t i = l; i <= r; ++i) s += a[i];
    return s;
}

int main() {
    // --- Edge case: empty tree ---------------------------------------------
    {
        SegmentTree<int> st;
        assert(st.empty());
        assert(st.size() == 0);
        std::vector<int> none;
        st.build(none);
        assert(st.empty());
    }

    // --- Edge case: single node --------------------------------------------
    {
        SegmentTree<int> st(std::vector<int>{42});
        assert(st.size() == 1);
        assert(st.query(0, 0) == 42);
        st.update(0, 7);
        assert(st.query(0, 0) == 7);
    }

    // --- Core: build + every range sum vs brute force ----------------------
    {
        std::vector<int> data{5, -3, 8, 0, 2, 11, -6, 4};
        SegmentTree<int> st(data);
        assert(st.size() == data.size());
        for (std::size_t l = 0; l < data.size(); ++l) {
            for (std::size_t r = l; r < data.size(); ++r) {
                assert(st.query(l, r) == bruteSum(data, l, r));
            }
        }

        // --- Core: point updates, then re-assert every range ---------------
        st.update(0, 100);   data[0] = 100;
        st.update(6, 6);     data[6] = 6;
        st.update(3, -50);   data[3] = -50;
        for (std::size_t l = 0; l < data.size(); ++l) {
            for (std::size_t r = l; r < data.size(); ++r) {
                assert(st.query(l, r) == bruteSum(data, l, r));
            }
        }
        // Full-range and single-element spot checks.
        assert(st.query(0, data.size() - 1) == bruteSum(data, 0, data.size() - 1));
        assert(st.query(4, 4) == data[4]);
    }

    // --- Non-power-of-two length (exercises the 4n sizing) -----------------
    {
        std::vector<long long> data{1, 2, 3, 4, 5, 6, 7};  // n = 7
        SegmentTree<long long> st(data);
        for (std::size_t l = 0; l < data.size(); ++l) {
            for (std::size_t r = l; r < data.size(); ++r) {
                assert(st.query(l, r) == bruteSum(data, l, r));
            }
        }
        st.update(2, 30);  data[2] = 30;
        assert(st.query(0, 6) == bruteSum(data, 0, 6));
        assert(st.query(2, 5) == bruteSum(data, 2, 5));
    }

    // --- Copy/move safety: copies are independent, no shared state ---------
    {
        SegmentTree<int> a(std::vector<int>{1, 2, 3, 4});
        SegmentTree<int> b = a;          // deep copy
        b.update(0, 100);
        assert(a.query(0, 0) == 1);      // original untouched
        assert(b.query(0, 0) == 100);
        SegmentTree<int> c = std::move(b);   // move
        assert(c.query(0, 0) == 100);
    }

    std::cout << "All SegmentTree assertions passed.\n\n";

    // --- Human-readable demo -----------------------------------------------
    std::vector<int> demo{2, 4, 6, 8, 10};
    SegmentTree<int> st(demo);
    std::cout << "Array: [2, 4, 6, 8, 10]\n";
    std::cout << "  sum[0..4] = " << st.query(0, 4) << "  (expected 30)\n";
    std::cout << "  sum[1..3] = " << st.query(1, 3) << "  (expected 18)\n";
    std::cout << "update index 2 -> 60\n";
    st.update(2, 60);
    std::cout << "  sum[0..4] = " << st.query(0, 4) << "  (expected 84)\n";
    std::cout << "  sum[2..2] = " << st.query(2, 2) << "  (expected 60)\n";
    return 0;
}
