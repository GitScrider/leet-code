/*
 * AVL Tree  --  Data Structure - Tree (self-balancing binary search tree)
 *
 * Summary:
 *   A binary search tree that keeps its height in O(log n) by storing a height
 *   in every node and rebalancing with rotations after each insert/remove, so
 *   that no root-to-leaf path can grow much longer than any other.
 *
 * Operations & complexity (n = number of stored keys):
 *   +-------------+-----------+-----------+
 *   | operation   | average   | worst     |
 *   +-------------+-----------+-----------+
 *   | search      | O(log n)  | O(log n)  |
 *   | insert      | O(log n)  | O(log n)  |
 *   | remove      | O(log n)  | O(log n)  |
 *   | min / max   | O(log n)  | O(log n)  |
 *   | inorder     | O(n)      | O(n)      |
 *   +-------------+-----------+-----------+
 *   Unlike a plain BST (worst case O(n) on sorted input) the AVL bound holds
 *   even in the worst case because balancing is enforced on every mutation.
 *
 * Invariants:
 *   1. BST ordering: for any node, all keys in the left subtree are < node.key
 *      and all keys in the right subtree are > node.key (duplicates rejected).
 *   2. AVL balance: for every node, balanceFactor = height(left) - height(right)
 *      stays in {-1, 0, +1}. The stored height equals 1 + max(childHeights).
 *   This bounds the tree height h by ~1.44*log2(n+2), i.e. O(log n).
 *
 * When to use / trade-offs:
 *   - Prefer over a plain BST whenever inputs may be ordered/adversarial.
 *   - More rigidly balanced than a red-black tree => slightly faster lookups
 *     but slightly more rotations on write-heavy workloads.
 *   - Not a good fit if you need O(1) amortized structural sharing or ranks
 *     without extra bookkeeping.
 */

#include <algorithm>   // std::max
#include <cassert>
#include <cmath>       // std::log2, std::ceil
#include <cstddef>     // std::size_t
#include <iostream>
#include <queue>
#include <utility>     // std::move
#include <vector>

template <typename T>
class AVLTree {
    struct Node {
        T key;
        Node* left;
        Node* right;
        int height;  // height of the subtree rooted here; a leaf has height 1
        explicit Node(const T& k) : key(k), left(nullptr), right(nullptr), height(1) {}
    };

    Node* root_ = nullptr;
    std::size_t count_ = 0;

    // --- height / balance helpers ------------------------------------------
    // A null child has height 0 by convention, so a lone leaf has height 1.
    static int nodeHeight(const Node* n) { return n ? n->height : 0; }

    // Positive => left-heavy, negative => right-heavy.
    static int balanceFactor(const Node* n) {
        return n ? nodeHeight(n->left) - nodeHeight(n->right) : 0;
    }

    static void updateHeight(Node* n) {
        n->height = 1 + std::max(nodeHeight(n->left), nodeHeight(n->right));
    }

    // --- rotations ----------------------------------------------------------
    // Right rotation fixes a left-left imbalance. The left child x becomes the
    // new subtree root; y (old root) descends to x's right. x's old right
    // subtree (T2) is > x and < y, so it legally moves to y's left. BST order
    // is preserved and the tall left branch is lifted one level.
    static Node* rotateRight(Node* y) {
        Node* x = y->left;
        Node* T2 = x->right;
        x->right = y;
        y->left = T2;
        updateHeight(y);   // update the descended node first (now a child)
        updateHeight(x);   // then the new root
        return x;
    }

    // Left rotation fixes a right-right imbalance (mirror of the above).
    static Node* rotateLeft(Node* x) {
        Node* y = x->right;
        Node* T2 = y->left;
        y->left = x;
        x->right = T2;
        updateHeight(x);
        updateHeight(y);
        return y;
    }

    // Restore the AVL property at node using the four canonical cases.
    // We only ever get here off-balance by exactly 2, so one (LL/RR) or two
    // (LR/RL) rotations always suffice to bring every ancestor back to {-1,0,1}.
    static Node* rebalance(Node* node) {
        updateHeight(node);
        const int bf = balanceFactor(node);
        if (bf > 1) {                              // left-heavy
            if (balanceFactor(node->left) < 0)     // Left-Right: straighten first
                node->left = rotateLeft(node->left);
            return rotateRight(node);              // Left-Left
        }
        if (bf < -1) {                             // right-heavy
            if (balanceFactor(node->right) > 0)    // Right-Left: straighten first
                node->right = rotateRight(node->right);
            return rotateLeft(node);               // Right-Right
        }
        return node;                               // already balanced
    }

    // --- recursive core -----------------------------------------------------
    Node* insertNode(Node* node, const T& key, bool& inserted) {
        if (!node) {
            inserted = true;
            return new Node(key);
        }
        if (key < node->key)
            node->left = insertNode(node->left, key, inserted);
        else if (node->key < key)
            node->right = insertNode(node->right, key, inserted);
        else {
            inserted = false;  // duplicate: reject, keep set semantics
            return node;
        }
        return rebalance(node);
    }

    static Node* minNode(Node* n) {
        while (n->left) n = n->left;
        return n;
    }

    Node* removeNode(Node* node, const T& key, bool& removed) {
        if (!node) {
            removed = false;
            return nullptr;
        }
        if (key < node->key) {
            node->left = removeNode(node->left, key, removed);
        } else if (node->key < key) {
            node->right = removeNode(node->right, key, removed);
        } else {
            removed = true;
            if (!node->left || !node->right) {
                // Zero or one child: splice the node out and return its child.
                Node* child = node->left ? node->left : node->right;
                delete node;
                return child;  // possibly nullptr
            }
            // Two children: copy the inorder successor's key up, then delete
            // that successor (which has at most one child) from the right side.
            Node* succ = minNode(node->right);
            node->key = succ->key;
            node->right = removeNode(node->right, succ->key, removed);
        }
        return rebalance(node);
    }

    // --- lifetime helpers ---------------------------------------------------
    static void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

    // Verify the AVL + height invariant; returns the true subtree height, or
    // -1 if any invariant is violated (imbalance or a stale stored height).
    static int verify(const Node* n) {
        if (!n) return 0;
        const int lh = verify(n->left);
        const int rh = verify(n->right);
        if (lh < 0 || rh < 0) return -1;
        const int diff = lh > rh ? lh - rh : rh - lh;
        if (diff > 1) return -1;
        const int h = 1 + std::max(lh, rh);
        if (h != n->height) return -1;  // stored height must be consistent
        return h;
    }

    static const Node* findNode(const Node* n, const T& key) {
        while (n) {
            if (key < n->key) n = n->left;
            else if (n->key < key) n = n->right;
            else return n;
        }
        return nullptr;
    }

    static void collectInorder(const Node* n, std::vector<T>& out) {
        if (!n) return;
        collectInorder(n->left, out);
        out.push_back(n->key);
        collectInorder(n->right, out);
    }

public:
    AVLTree() = default;
    ~AVLTree() { destroy(root_); }  // recursive free is safe: depth is O(log n)

    // Copy is deleted: the tree owns raw nodes, so a shallow copy would double
    // free. We provide move instead (cheap pointer steal) for ownership transfer.
    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    AVLTree(AVLTree&& other) noexcept : root_(other.root_), count_(other.count_) {
        other.root_ = nullptr;
        other.count_ = 0;
    }
    AVLTree& operator=(AVLTree&& other) noexcept {
        if (this != &other) {
            destroy(root_);
            root_ = other.root_;
            count_ = other.count_;
            other.root_ = nullptr;
            other.count_ = 0;
        }
        return *this;
    }

    bool empty() const { return count_ == 0; }
    std::size_t size() const { return count_; }
    int height() const { return nodeHeight(root_); }

    bool contains(const T& key) const { return findNode(root_, key) != nullptr; }

    // Returns true if a new key was inserted, false if it already existed.
    bool insert(const T& key) {
        bool inserted = false;
        root_ = insertNode(root_, key, inserted);
        if (inserted) ++count_;
        return inserted;
    }

    // Returns true if the key was present and removed.
    bool remove(const T& key) {
        bool removed = false;
        root_ = removeNode(root_, key, removed);
        if (removed) --count_;
        return removed;
    }

    std::vector<T> inorder() const {
        std::vector<T> out;
        out.reserve(count_);
        collectInorder(root_, out);
        return out;
    }

    std::vector<T> levelOrder() const {
        std::vector<T> out;
        if (!root_) return out;
        std::queue<const Node*> q;
        q.push(root_);
        while (!q.empty()) {
            const Node* n = q.front();
            q.pop();
            out.push_back(n->key);
            if (n->left) q.push(n->left);
            if (n->right) q.push(n->right);
        }
        return out;
    }

    // Full invariant check: BST ordering (via strictly increasing inorder)
    // plus AVL balance and stored-height consistency at every node.
    bool checkInvariants() const {
        if (verify(root_) < 0) return false;
        const std::vector<T> v = inorder();
        for (std::size_t i = 1; i < v.size(); ++i)
            if (!(v[i - 1] < v[i])) return false;
        return true;
    }
};

// ---------------------------------------------------------------------------
static void runTests() {
    // Edge case: empty tree.
    {
        AVLTree<int> t;
        assert(t.empty());
        assert(t.size() == 0);
        assert(t.height() == 0);
        assert(!t.contains(42));
        assert(!t.remove(42));
        assert(t.checkInvariants());
        assert(t.inorder().empty());
    }

    // Edge case: single node.
    {
        AVLTree<int> t;
        assert(t.insert(10));
        assert(!t.insert(10));          // duplicate rejected
        assert(t.size() == 1);
        assert(t.height() == 1);
        assert(t.contains(10));
        assert(t.checkInvariants());
        assert(t.remove(10));
        assert(t.empty());
        assert(t.checkInvariants());
    }

    // Each rotation case, checked individually.
    {
        AVLTree<int> ll; ll.insert(30); ll.insert(20); ll.insert(10); // LL
        assert(ll.levelOrder() == (std::vector<int>{20, 10, 30}));
        assert(ll.checkInvariants());

        AVLTree<int> rr; rr.insert(10); rr.insert(20); rr.insert(30); // RR
        assert(rr.levelOrder() == (std::vector<int>{20, 10, 30}));
        assert(rr.checkInvariants());

        AVLTree<int> lr; lr.insert(30); lr.insert(10); lr.insert(20); // LR
        assert(lr.levelOrder() == (std::vector<int>{20, 10, 30}));
        assert(lr.checkInvariants());

        AVLTree<int> rl; rl.insert(10); rl.insert(30); rl.insert(20); // RL
        assert(rl.levelOrder() == (std::vector<int>{20, 10, 30}));
        assert(rl.checkInvariants());
    }

    // Ascending insert would make a plain BST a linked list of height n.
    // Assert the AVL height stays logarithmic and inorder comes out sorted.
    {
        AVLTree<int> t;
        const int n = 1023;
        for (int i = 1; i <= n; ++i) assert(t.insert(i));
        assert(t.size() == static_cast<std::size_t>(n));
        assert(t.checkInvariants());

        const std::vector<int> in = t.inorder();
        for (int i = 0; i < n; ++i) assert(in[static_cast<std::size_t>(i)] == i + 1);

        // AVL height <= ~1.44*log2(n+2); a generous 2*log2 bound proves balance.
        const int bound = 2 * static_cast<int>(std::ceil(std::log2(static_cast<double>(n) + 1.0)));
        assert(t.height() <= bound);
        assert(t.height() < n);  // definitively not a degenerate stick
    }

    // Deletions that trigger rebalancing, including a two-children delete.
    {
        AVLTree<int> t;
        for (int v : {50, 25, 75, 10, 30, 60, 80, 5, 15, 27, 55}) t.insert(v);
        assert(t.checkInvariants());

        assert(t.remove(50));   // internal node with two children (root)
        assert(!t.contains(50));
        assert(t.checkInvariants());

        assert(t.remove(80));   // forces rebalancing on the right spine
        assert(t.remove(75));
        assert(t.remove(60));
        assert(t.checkInvariants());

        assert(!t.remove(9999));  // absent key
        assert(t.checkInvariants());
    }

    // Insert/remove churn: invariant must hold throughout and end empty.
    {
        AVLTree<int> t;
        for (int i = 0; i < 200; ++i) t.insert((i * 37) % 211);
        assert(t.checkInvariants());
        for (int i = 0; i < 211; ++i) t.remove(i);
        assert(t.empty());
        assert(t.checkInvariants());
    }

    // Move semantics: ownership transfers, source left valid and empty.
    {
        AVLTree<int> a;
        for (int v : {4, 2, 6, 1, 3}) a.insert(v);
        AVLTree<int> b(std::move(a));
        assert(b.size() == 5);
        assert(b.checkInvariants());
        assert(a.empty());  // moved-from is safely destructible
    }
}

int main() {
    runTests();

    // Human-readable demo.
    AVLTree<int> tree;
    std::cout << "AVL Tree demo -- inserting 1..7 in ascending order\n";
    for (int i = 1; i <= 7; ++i) tree.insert(i);

    std::cout << "size   = " << tree.size() << "\n";
    std::cout << "height = " << tree.height()
              << "  (a plain BST would be 7 here)\n";

    std::cout << "inorder (sorted)    :";
    for (int v : tree.inorder()) std::cout << ' ' << v;
    std::cout << "\n";

    std::cout << "level-order (by row):";
    for (int v : tree.levelOrder()) std::cout << ' ' << v;
    std::cout << "\n";

    std::cout << "remove 4 (two children), then level-order:";
    tree.remove(4);
    for (int v : tree.levelOrder()) std::cout << ' ' << v;
    std::cout << "\n";

    std::cout << "AVL invariant holds : " << std::boolalpha
              << tree.checkInvariants() << "\n";
    std::cout << "All assertions passed.\n";
    return 0;
}
