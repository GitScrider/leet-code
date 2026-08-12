/*
 * Binary Search Tree (BST)                       Data Structure - Tree
 * ----------------------------------------------------------------------
 * Summary: An ordered binary tree: for every node, all keys in the left
 * subtree are smaller and all keys in the right subtree are larger. This
 * ordering makes search, insert, and delete follow a single root-to-leaf
 * path.
 *
 * Operations & complexity (n = nodes, h = height):
 *   Operation      | Average   | Worst case
 *   ---------------+-----------+-------------------------------
 *   insert         | O(log n)  | O(n)   (degenerate/skewed tree)
 *   contains       | O(log n)  | O(n)
 *   remove         | O(log n)  | O(n)
 *   findMin/findMax| O(log n)  | O(n)
 *   inorder        | O(n)      | O(n)
 * Average assumes a reasonably balanced tree (h ~ log n). Worst case is a
 * fully skewed tree (h = n), e.g. inserting already-sorted input.
 *
 * Invariants:
 *   BST ordering property, enforced at EVERY node:
 *       key(left subtree)  <  key(node)  <  key(right subtree)
 *   Duplicates are rejected (this implementation keeps keys unique), so the
 *   inequalities are strict. A direct consequence: an inorder traversal
 *   visits keys in ascending sorted order.
 *
 * When to use / trade-offs:
 *   - Use for an ordered collection with fast lookup AND cheap sorted
 *     iteration / range queries / successor-predecessor.
 *   - No self-balancing here: adversarial or sorted input degrades to a
 *     linked list (O(n)). This is precisely what AVL and red-black trees
 *     fix by adding a balance invariant on top of this ordering.
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <utility>  // std::move
#include <vector>

template <typename T>
class BinarySearchTree {
private:
    struct Node {
        T key;
        Node* left;
        Node* right;
        explicit Node(const T& k) : key(k), left(nullptr), right(nullptr) {}
    };

    Node* root_;
    std::size_t count_;

    static Node* cloneSubtree(const Node* n) {
        if (n == nullptr) return nullptr;
        Node* copy = new Node(n->key);
        copy->left = cloneSubtree(n->left);
        copy->right = cloneSubtree(n->right);
        return copy;
    }

    // Postorder free: children released before the parent -> no dangling
    // access, no double-free. Depth O(h) recursion; acceptable for teaching
    // trees (a balanced tree keeps h ~ log n).
    static void destroySubtree(Node* n) {
        if (n == nullptr) return;
        destroySubtree(n->left);
        destroySubtree(n->right);
        delete n;
    }

    // Insert 'key' below 'n', returning the (possibly new) subtree root.
    // WHY link-by-return: it lets a parent re-point its child pointer in one
    // place, so we never juggle a separate "parent" pointer.
    static Node* insertRec(Node* n, const T& key, bool& inserted) {
        if (n == nullptr) {              // found the empty slot -> attach
            inserted = true;
            return new Node(key);
        }
        if (key < n->key)
            n->left = insertRec(n->left, key, inserted);
        else if (n->key < key)
            n->right = insertRec(n->right, key, inserted);
        else
            inserted = false;            // equal key already present: ignore
        return n;
    }

    static const Node* findMinNode(const Node* n) {
        assert(n != nullptr);
        while (n->left != nullptr) n = n->left;  // smallest = leftmost
        return n;
    }

    static const Node* findMaxNode(const Node* n) {
        assert(n != nullptr);
        while (n->right != nullptr) n = n->right; // largest = rightmost
        return n;
    }

    // Remove 'key' from the subtree, returning its new root. Handles the
    // three classic cases; see the two-children branch for the successor
    // trick that preserves the ordering invariant.
    static Node* removeRec(Node* n, const T& key, bool& removed) {
        if (n == nullptr) {              // key not found
            removed = false;
            return nullptr;
        }
        if (key < n->key) {
            n->left = removeRec(n->left, key, removed);
        } else if (n->key < key) {
            n->right = removeRec(n->right, key, removed);
        } else {
            // Found the node to delete.
            removed = true;
            if (n->left == nullptr) {          // Case 1a: no left child
                Node* r = n->right;            // (covers leaf: r == nullptr)
                delete n;
                return r;                      // splice right child up
            }
            if (n->right == nullptr) {         // Case 1b: only left child
                Node* l = n->left;
                delete n;
                return l;
            }
            // Case 2: two children. Replace this node's key with its inorder
            // SUCCESSOR (smallest key in the right subtree). The successor
            // has no left child, so removing it recursively falls into an
            // easy case. Copying its key up preserves BST ordering because
            // the successor is, by definition, the next-larger key.
            const Node* succ = findMinNode(n->right);
            n->key = succ->key;
            bool dummy = false;                // successor definitely exists
            n->right = removeRec(n->right, n->key, dummy);
        }
        return n;
    }

    static bool containsRec(const Node* n, const T& key) {
        while (n != nullptr) {
            if (key < n->key) n = n->left;
            else if (n->key < key) n = n->right;
            else return true;
        }
        return false;
    }

    static void inorderRec(const Node* n, std::vector<T>& out) {
        if (n == nullptr) return;
        inorderRec(n->left, out);
        out.push_back(n->key);   // visiting in-order yields ascending keys
        inorderRec(n->right, out);
    }

    static int heightRec(const Node* n) {
        if (n == nullptr) return -1;
        int lh = heightRec(n->left);
        int rh = heightRec(n->right);
        return 1 + (lh > rh ? lh : rh);
    }

    // Verify the ordering invariant for the whole tree: every key must lie
    // strictly within (lo, hi) inherited from ancestors. Used by tests.
    static bool isBSTRec(const Node* n, const T* lo, const T* hi) {
        if (n == nullptr) return true;
        if (lo != nullptr && !(*lo < n->key)) return false;
        if (hi != nullptr && !(n->key < *hi)) return false;
        return isBSTRec(n->left, lo, &n->key) &&
               isBSTRec(n->right, &n->key, hi);
    }

public:
    BinarySearchTree() : root_(nullptr), count_(0) {}

    // --- Rule of Five: we own raw nodes, so deep-copy + move (see below). -
    ~BinarySearchTree() { destroySubtree(root_); }

    BinarySearchTree(const BinarySearchTree& other)
        : root_(cloneSubtree(other.root_)), count_(other.count_) {}

    BinarySearchTree& operator=(const BinarySearchTree& other) {
        if (this != &other) {
            Node* newRoot = cloneSubtree(other.root_);  // build first
            destroySubtree(root_);                      // then free old
            root_ = newRoot;
            count_ = other.count_;
        }
        return *this;
    }

    BinarySearchTree(BinarySearchTree&& other) noexcept
        : root_(other.root_), count_(other.count_) {
        other.root_ = nullptr;
        other.count_ = 0;
    }

    BinarySearchTree& operator=(BinarySearchTree&& other) noexcept {
        if (this != &other) {
            destroySubtree(root_);
            root_ = other.root_;
            count_ = other.count_;
            other.root_ = nullptr;
            other.count_ = 0;
        }
        return *this;
    }

    bool empty() const { return root_ == nullptr; }
    std::size_t size() const { return count_; }
    int height() const { return heightRec(root_); }

    // Returns true if a new node was created, false if 'key' already existed.
    bool insert(const T& key) {
        bool inserted = false;
        root_ = insertRec(root_, key, inserted);
        if (inserted) ++count_;
        return inserted;
    }

    bool contains(const T& key) const { return containsRec(root_, key); }

    // Returns true if a node was actually removed.
    bool remove(const T& key) {
        bool removed = false;
        root_ = removeRec(root_, key, removed);
        if (removed) --count_;
        return removed;
    }

    const T& findMin() const {
        assert(root_ != nullptr && "findMin on empty tree");
        return findMinNode(root_)->key;
    }

    const T& findMax() const {
        assert(root_ != nullptr && "findMax on empty tree");
        return findMaxNode(root_)->key;
    }

    std::vector<T> inorder() const {
        std::vector<T> out;
        inorderRec(root_, out);
        return out;
    }

    bool isValidBST() const { return isBSTRec(root_, nullptr, nullptr); }
};

// Helper for tests: confirm a vector is strictly ascending (i.e. sorted
// with no duplicates), which is exactly what a BST inorder must produce.
template <typename T>
static bool isStrictlySorted(const std::vector<T>& v) {
    for (std::size_t i = 1; i < v.size(); ++i)
        if (!(v[i - 1] < v[i])) return false;
    return true;
}

int main() {
    // ---- Edge case: empty tree ----------------------------------------
    BinarySearchTree<int> empty;
    assert(empty.empty());
    assert(empty.size() == 0);
    assert(empty.height() == -1);
    assert(!empty.contains(5));
    assert(!empty.remove(5));           // removing from empty is a no-op
    assert(empty.inorder().empty());
    assert(empty.isValidBST());

    // ---- Edge case: single node ---------------------------------------
    BinarySearchTree<int> single;
    assert(single.insert(10));
    assert(single.size() == 1);
    assert(single.height() == 0);
    assert(single.findMin() == 10 && single.findMax() == 10);
    assert(single.remove(10));
    assert(single.empty());

    // ---- Build a tree and verify the ordering invariant ---------------
    /*
     *                50
     *              /    \
     *            30      70
     *           /  \    /  \
     *         20   40  60   80
     */
    BinarySearchTree<int> t;
    for (int k : {50, 30, 70, 20, 40, 60, 80})
        assert(t.insert(k));
    assert(t.size() == 7);
    assert(t.isValidBST());

    // Inorder of a BST is sorted ascending.
    const std::vector<int> sorted = {20, 30, 40, 50, 60, 70, 80};
    assert(t.inorder() == sorted);
    assert(isStrictlySorted(t.inorder()));
    assert(t.findMin() == 20 && t.findMax() == 80);

    // Duplicate handling: inserting an existing key is rejected, size stable.
    assert(!t.insert(50));
    assert(!t.insert(20));
    assert(t.size() == 7);

    // contains hits and misses.
    assert(t.contains(60) && t.contains(20));
    assert(!t.contains(45) && !t.contains(100));

    // ---- Remove: Case 1 - a leaf --------------------------------------
    assert(t.remove(20));
    assert(!t.contains(20));
    assert(t.isValidBST());
    assert((t.inorder() == std::vector<int>{30, 40, 50, 60, 70, 80}));

    // ---- Remove: Case 2 - a node with ONE child -----------------------
    // 30 currently has only the right child 40 (its left child 20 is gone).
    assert(t.remove(30));
    assert(!t.contains(30) && t.contains(40));  // 40 spliced up
    assert(t.isValidBST());
    assert((t.inorder() == std::vector<int>{40, 50, 60, 70, 80}));

    // ---- Remove: Case 3 - a node with TWO children --------------------
    // Rebuild a full tree so 70 has both children 60 and 80.
    BinarySearchTree<int> two;
    for (int k : {50, 30, 70, 60, 80}) two.insert(k);
    assert(two.remove(70));                     // successor is 80
    assert(!two.contains(70));
    assert(two.contains(60) && two.contains(80));
    assert(two.isValidBST());
    assert((two.inorder() == std::vector<int>{30, 50, 60, 80}));

    // ---- Remove the ROOT (also a two-children case) -------------------
    BinarySearchTree<int> r;
    for (int k : {50, 30, 70, 20, 40, 60, 80}) r.insert(k);
    assert(r.remove(50));                        // root replaced by succ 60
    assert(!r.contains(50));
    assert(r.isValidBST());
    assert((r.inorder() == std::vector<int>{20, 30, 40, 60, 70, 80}));
    assert(r.findMin() == 20 && r.findMax() == 80);

    // ---- Batch stress: invariant + sorted order must always hold ------
    BinarySearchTree<int> big;
    // Insert in an order that would skew a naive tree; correctness (not
    // balance) is what we assert here.
    for (int k : {8, 3, 10, 1, 6, 14, 4, 7, 13, 2, 5, 9, 11, 12, 0})
        big.insert(k);
    assert(big.isValidBST());
    assert(isStrictlySorted(big.inorder()));
    assert(big.size() == 15);
    // Remove a spread of keys, re-checking the invariant each time.
    for (int k : {8, 0, 14, 6, 3}) {
        assert(big.remove(k));
        assert(big.isValidBST());
        assert(isStrictlySorted(big.inorder()));
    }
    assert(big.size() == 10);

    // ---- Rule of Five: deep copy is independent -----------------------
    BinarySearchTree<int> copy = r;              // copy constructor
    assert(copy.inorder() == r.inorder());
    r.remove(60);                                // mutate original
    assert(copy.contains(60));                   // copy untouched
    BinarySearchTree<int> moved = std::move(copy);
    assert(moved.contains(60));
    assert(copy.empty());

    // ---- Human-readable demo ------------------------------------------
    std::cout << "Binary Search Tree demo:\n";
    std::cout << "  inorder (sorted): ";
    auto seq = two.inorder();
    for (std::size_t i = 0; i < seq.size(); ++i)
        std::cout << seq[i] << (i + 1 < seq.size() ? ' ' : '\n');
    std::cout << "  min = " << two.findMin()
              << ", max = " << two.findMax()
              << ", height = " << two.height()
              << ", size = " << two.size() << "\n";
    std::cout << "All BST assertions passed.\n";
    return 0;
}
