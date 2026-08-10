/*
 * Binary Heap (Array-backed MIN-heap)  -- Data Structure - Tree
 *
 * Summary:
 *   A binary heap is a *complete* binary tree (every level full except possibly
 *   the last, which fills left-to-right) stored contiguously in an array. This
 *   implementation is a MIN-heap: the smallest element is always at the root.
 *   Storing a complete tree in an array lets us navigate parent/child links with
 *   pure index arithmetic instead of pointers -- that compactness is the whole point.
 *
 * Operations & complexity:
 *   Operation      | Time
 *   ---------------+---------------------------------------------
 *   top / peek     | O(1)          (root is index 0)
 *   push           | O(log n)      (sift-up along one root-to-leaf path)
 *   pop/extractMin | O(log n)      (sift-down along one path)
 *   buildHeap      | O(n)          (bottom-up heapify -- see note below)
 *   size / empty   | O(1)
 *
 * Invariants:
 *   1. Shape:    the tree is complete; the backing vector has no gaps, so the
 *                node at array index i has children at 2i+1 and 2i+2 (if in range)
 *                and parent at (i-1)/2. These formulas hold *because* the tree is
 *                complete -- there are no holes to break the numbering.
 *   2. Heap order (MIN): every node is <= both of its children. Transitively the
 *                root is the global minimum. Note this is a PARTIAL order: siblings
 *                are unordered, so a heap is NOT a sorted array.
 *
 * When to use / trade-offs:
 *   - Priority queues: you repeatedly need the min (or max) and insert on the fly.
 *   - Heapsort, Dijkstra/Prim, k-smallest / streaming top-k.
 *   - Not for lookup/search of arbitrary elements (that is O(n)) -- use a BST/set.
 *   - Cache-friendly and pointer-free, but no cheap ordered iteration.
 */

#include <vector>
#include <cstddef>   // std::size_t
#include <utility>   // std::swap, std::move
#include <cassert>
#include <iostream>

// MinHeap<T> requires that T be less-than comparable (operator<).
template <typename T>
class MinHeap {
public:
    MinHeap() = default;

    // Convenience constructor: heapify an existing vector in O(n).
    explicit MinHeap(std::vector<T> items) : data_(std::move(items)) {
        buildHeap();
    }

    // This heap owns only a std::vector<T>, which already manages its own memory
    // with correct copy/move semantics. So the default special members are safe:
    // no raw pointers means no risk of a double-free. We keep Rule-of-Five defaults
    // explicit to document that the design is intentional, not accidental.
    ~MinHeap() = default;
    MinHeap(const MinHeap&) = default;
    MinHeap& operator=(const MinHeap&) = default;
    MinHeap(MinHeap&&) noexcept = default;
    MinHeap& operator=(MinHeap&&) noexcept = default;

    bool empty() const noexcept { return data_.empty(); }
    std::size_t size() const noexcept { return data_.size(); }

    // Smallest element. Precondition: heap is non-empty.
    const T& top() const {
        assert(!data_.empty() && "top() on empty heap");
        return data_.front();
    }

    // Insert a value, then restore heap order by sifting the new leaf up.
    void push(const T& value) {
        data_.push_back(value);
        siftUp(data_.size() - 1);
    }
    void push(T&& value) {
        data_.push_back(std::move(value));
        siftUp(data_.size() - 1);
    }

    // Remove and return the minimum.
    // Trick: swap the root with the last leaf, pop the leaf (now holding the old
    // min) off the back in O(1), then sift the moved element down to its place.
    T pop() {
        assert(!data_.empty() && "pop() on empty heap");
        T minValue = std::move(data_.front());
        data_.front() = std::move(data_.back());
        data_.pop_back();
        if (!data_.empty()) {
            siftDown(0);
        }
        return minValue;
    }

    // Bottom-up heapify of the current backing array in O(n).
    // Why O(n) and not O(n log n): siftDown cost is proportional to a node's
    // HEIGHT, and most nodes are near the bottom with tiny height. Summing
    // height over all nodes converges to O(n). We start at the last internal
    // node -- the parent of the last element -- and walk backwards to the root.
    void buildHeap() {
        if (data_.size() < 2) return;               // 0 or 1 element is already a heap
        // Last element index is size-1; its parent is the last node that has a child.
        std::size_t i = parent(data_.size() - 1);
        // Iterate down to and including index 0. Use a do/while-style guard because
        // std::size_t is unsigned and would wrap past 0 in a naive for-loop.
        while (true) {
            siftDown(i);
            if (i == 0) break;
            --i;
        }
    }

private:
    std::vector<T> data_;

    // --- Index arithmetic for a complete tree stored in an array ---
    // A node at index i has:
    //   parent    = (i - 1) / 2   (integer division; valid for i > 0)
    //   left child= 2*i + 1
    //   right child=2*i + 2
    static std::size_t parent(std::size_t i) noexcept { return (i - 1) / 2; }
    static std::size_t leftChild(std::size_t i) noexcept { return 2 * i + 1; }
    static std::size_t rightChild(std::size_t i) noexcept { return 2 * i + 2; }

    // Move the element at index i up toward the root while it is smaller than its
    // parent. Restores the MIN-heap order after an insertion at a leaf.
    void siftUp(std::size_t i) {
        while (i > 0) {
            std::size_t p = parent(i);
            if (data_[i] < data_[p]) {   // heap order violated: child < parent
                std::swap(data_[i], data_[p]);
                i = p;                   // keep climbing from the parent's slot
            } else {
                break;                   // parent <= child: order restored
            }
        }
    }

    // Move the element at index i down toward the leaves while it is larger than
    // its smallest child. Restores order after a pop (or during buildHeap).
    void siftDown(std::size_t i) {
        const std::size_t n = data_.size();
        while (true) {
            std::size_t smallest = i;
            std::size_t l = leftChild(i);
            std::size_t r = rightChild(i);
            // Pick the smallest among node and its (existing) children. We must
            // swap with the SMALLER child, otherwise that child would end up
            // larger than its new sibling and violate the invariant.
            if (l < n && data_[l] < data_[smallest]) smallest = l;
            if (r < n && data_[r] < data_[smallest]) smallest = r;
            if (smallest == i) break;    // node <= both children: settled
            std::swap(data_[i], data_[smallest]);
            i = smallest;                // continue sinking from the child's slot
        }
    }
};

// ---------------------------------------------------------------------------
// Tests + demo
// ---------------------------------------------------------------------------
int main() {
    // --- Edge case: empty heap ---
    {
        MinHeap<int> h;
        assert(h.empty());
        assert(h.size() == 0);
    }

    // --- Edge case: single node ---
    {
        MinHeap<int> h;
        h.push(42);
        assert(!h.empty());
        assert(h.size() == 1);
        assert(h.top() == 42);
        assert(h.pop() == 42);
        assert(h.empty());
    }

    // --- push/pop yields ascending order (heapsort property) ---
    {
        MinHeap<int> h;
        int input[] = {5, 1, 8, 3, 9, 2, 7, 4, 6, 0};
        for (int x : input) h.push(x);
        assert(h.size() == 10);
        assert(h.top() == 0);   // global minimum sits at the root

        int prev = -1;          // all inputs are >= 0
        while (!h.empty()) {
            int cur = h.pop();
            assert(cur >= prev); // each pop is >= the previous -> ascending
            prev = cur;
        }
        assert(prev == 9);      // last popped is the maximum
    }

    // --- buildHeap on an unsorted array heapifies in O(n) ---
    {
        std::vector<int> raw = {9, 4, 7, 1, 3, 8, 2, 6, 5, 0, 10};
        MinHeap<int> h(raw);            // constructor calls buildHeap
        assert(h.size() == raw.size());
        assert(h.top() == 0);           // min bubbled to the root

        // Draining a valid heap must produce sorted (ascending) output.
        std::vector<int> drained;
        while (!h.empty()) drained.push_back(h.pop());
        for (std::size_t i = 1; i < drained.size(); ++i) {
            assert(drained[i - 1] <= drained[i]);
        }
        assert(drained.front() == 0 && drained.back() == 10);
    }

    // --- Duplicates are allowed; they simply coexist ---
    {
        MinHeap<int> h;
        for (int x : {5, 5, 1, 5, 1, 1}) h.push(x);
        assert(h.pop() == 1);
        assert(h.pop() == 1);
        assert(h.pop() == 1);
        assert(h.pop() == 5);
        assert(h.pop() == 5);
        assert(h.pop() == 5);
        assert(h.empty());
    }

    // --- Copy/move sanity: copies are independent (no shared state / double-free) ---
    {
        MinHeap<int> a;
        for (int x : {3, 1, 2}) a.push(x);
        MinHeap<int> b = a;             // deep copy via vector's copy
        assert(b.pop() == 1);
        assert(a.top() == 1);           // popping the copy did not disturb the original

        MinHeap<int> c = std::move(a);  // move leaves c holding a's data
        assert(c.top() == 1);
    }

    // --- Human-readable demo ---
    std::cout << "Binary MIN-heap demo\n";
    std::cout << "--------------------\n";
    std::vector<int> demo = {23, 4, 15, 8, 16, 42, 108};
    std::cout << "Input (unsorted): ";
    for (int x : demo) std::cout << x << ' ';
    std::cout << '\n';

    MinHeap<int> h(demo);
    std::cout << "Min after buildHeap: " << h.top() << '\n';
    std::cout << "Draining in priority order: ";
    while (!h.empty()) std::cout << h.pop() << ' ';
    std::cout << "\n(that is the input sorted ascending)\n";

    std::cout << "\nAll assertions passed.\n";
    return 0;
}
