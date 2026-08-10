/*
 * Red-Black Tree  --  Data Structure - Tree (self-balancing binary search tree)
 *
 * Summary:
 *   A BST in which every node carries one bit of color (red/black); a small set
 *   of coloring rules, restored by recolorings and rotations after each write,
 *   keeps the longest root-to-leaf path at most twice the shortest, hence the
 *   height stays O(log n). This is the structure behind std::map / std::set.
 *
 * Operations & complexity (n = number of stored keys):
 *   +-----------+-----------+-----------+
 *   | operation | average   | worst     |
 *   +-----------+-----------+-----------+
 *   | search    | O(log n)  | O(log n)  |
 *   | insert    | O(log n)  | O(log n)  |
 *   | remove    | O(log n)  | O(log n)  |
 *   | min / max | O(log n)  | O(log n)  |
 *   | inorder   | O(n)      | O(n)      |
 *   +-----------+-----------+-----------+
 *   Each insert/remove does O(1) rotations and O(log n) recolorings.
 *
 * Invariants (the five red-black properties):
 *   1. Every node is either red or black.
 *   2. The root is black.
 *   3. Every leaf (the shared sentinel NIL) is black.
 *   4. If a node is red, then both of its children are black (no two reds in a
 *      row on any path).
 *   5. For each node, every path down to a NIL leaf contains the same number of
 *      black nodes (equal "black-height").
 *   Together (4)+(5) force the longest path <= 2x the shortest => h = O(log n).
 *
 * When to use / trade-offs:
 *   - The standard workhorse ordered map/set: good all-round insert/delete/lookup.
 *   - Looser balance than AVL => fewer rotations on writes, marginally taller
 *     trees => marginally slower lookups. Prefer AVL for read-dominated loads.
 *   - Deletion fixup is intricate; a single shared black NIL sentinel removes
 *     most null checks and makes the rotations uniform.
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
class RedBlackTree {
    enum class Color { RED, BLACK };

    struct Node {
        T key;
        Color color;
        Node* left;
        Node* right;
        Node* parent;
        // Sentinel/default node: key value-initialized and unused.
        Node() : key(), color(Color::BLACK), left(nullptr), right(nullptr), parent(nullptr) {}
        explicit Node(const T& k)
            : key(k), color(Color::RED), left(nullptr), right(nullptr), parent(nullptr) {}
    };

    // Single shared sentinel: represents every NIL leaf AND the root's parent.
    // It is always black (property 3) and lets rotations/fixups avoid nullptr
    // special-cases -- we can freely read nil_->color, ->left, ->parent, etc.
    Node* nil_ = nullptr;
    Node* root_ = nullptr;
    std::size_t count_ = 0;

    // --- rotations ----------------------------------------------------------
    // Left rotation about x: its right child y moves up and x becomes y's left
    // child. y's old left subtree (keys between x and y) becomes x's new right
    // subtree, so BST order is preserved. Parent links are patched both ways.
    void leftRotate(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        if (y->left != nil_) y->left->parent = x;
        y->parent = x->parent;
        if (x->parent == nil_) root_ = y;
        else if (x == x->parent->left) x->parent->left = y;
        else x->parent->right = y;
        y->left = x;
        x->parent = y;
    }

    // Right rotation about x (mirror image of leftRotate).
    void rightRotate(Node* x) {
        Node* y = x->left;
        x->left = y->right;
        if (y->right != nil_) y->right->parent = x;
        y->parent = x->parent;
        if (x->parent == nil_) root_ = y;
        else if (x == x->parent->right) x->parent->right = y;
        else x->parent->left = y;
        y->right = x;
        x->parent = y;
    }

    // --- insertion fixup ----------------------------------------------------
    // A freshly inserted node is red, which can only break property 4 (red node
    // with a red parent). We climb toward the root fixing the local violation:
    //   * red uncle  -> recolor parent+uncle black, grandparent red, move up 2.
    //   * black uncle -> rotate (with an inner-case pre-rotation) so the black
    //                    node rises and the two reds end up on different paths.
    void insertFixup(Node* z) {
        while (z->parent->color == Color::RED) {
            if (z->parent == z->parent->parent->left) {
                Node* uncle = z->parent->parent->right;
                if (uncle->color == Color::RED) {
                    z->parent->color = Color::BLACK;
                    uncle->color = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {   // inner (LR) case
                        z = z->parent;
                        leftRotate(z);
                    }
                    z->parent->color = Color::BLACK;      // outer (LL) case
                    z->parent->parent->color = Color::RED;
                    rightRotate(z->parent->parent);
                }
            } else {  // mirror: parent is a right child
                Node* uncle = z->parent->parent->left;
                if (uncle->color == Color::RED) {
                    z->parent->color = Color::BLACK;
                    uncle->color = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {    // inner (RL) case
                        z = z->parent;
                        rightRotate(z);
                    }
                    z->parent->color = Color::BLACK;      // outer (RR) case
                    z->parent->parent->color = Color::RED;
                    leftRotate(z->parent->parent);
                }
            }
        }
        root_->color = Color::BLACK;  // property 2: root is always black
    }

    // --- deletion helpers ---------------------------------------------------
    // Replace the subtree rooted at u with the subtree rooted at v (parent side
    // only). We deliberately write v->parent even when v == nil_, because the
    // deletion fixup reads nil_'s parent to know where the "double black" sits.
    void transplant(Node* u, Node* v) {
        if (u->parent == nil_) root_ = v;
        else if (u == u->parent->left) u->parent->left = v;
        else u->parent->right = v;
        v->parent = u->parent;
    }

    Node* treeMinimum(Node* x) const {
        while (x->left != nil_) x = x->left;
        return x;
    }

    // Deleting a black node removes one black from every path through it,
    // breaking property 5. We push an extra "black token" onto x and let this
    // fixup discharge it via recolorings/rotations, mirroring the four cases
    // by the sibling w's color and the colors of w's children.
    void deleteFixup(Node* x) {
        while (x != root_ && x->color == Color::BLACK) {
            if (x == x->parent->left) {
                Node* w = x->parent->right;          // sibling
                if (w->color == Color::RED) {        // case 1: red sibling
                    w->color = Color::BLACK;
                    x->parent->color = Color::RED;
                    leftRotate(x->parent);
                    w = x->parent->right;
                }
                if (w->left->color == Color::BLACK && w->right->color == Color::BLACK) {
                    w->color = Color::RED;           // case 2: both nephews black
                    x = x->parent;
                } else {
                    if (w->right->color == Color::BLACK) {  // case 3: near nephew red
                        w->left->color = Color::BLACK;
                        w->color = Color::RED;
                        rightRotate(w);
                        w = x->parent->right;
                    }
                    w->color = x->parent->color;     // case 4: far nephew red
                    x->parent->color = Color::BLACK;
                    w->right->color = Color::BLACK;
                    leftRotate(x->parent);
                    x = root_;                       // done: drop the extra black
                }
            } else {  // mirror of the above (x is a right child)
                Node* w = x->parent->left;
                if (w->color == Color::RED) {
                    w->color = Color::BLACK;
                    x->parent->color = Color::RED;
                    rightRotate(x->parent);
                    w = x->parent->left;
                }
                if (w->right->color == Color::BLACK && w->left->color == Color::BLACK) {
                    w->color = Color::RED;
                    x = x->parent;
                } else {
                    if (w->left->color == Color::BLACK) {
                        w->right->color = Color::BLACK;
                        w->color = Color::RED;
                        leftRotate(w);
                        w = x->parent->left;
                    }
                    w->color = x->parent->color;
                    x->parent->color = Color::BLACK;
                    w->left->color = Color::BLACK;
                    rightRotate(x->parent);
                    x = root_;
                }
            }
        }
        x->color = Color::BLACK;  // absorb the extra black (or blacken the root)
    }

    void deleteNode(Node* z) {
        Node* y = z;                    // node actually removed or moved
        Color yOriginalColor = y->color;
        Node* x;                        // node that takes y's place
        if (z->left == nil_) {
            x = z->right;
            transplant(z, z->right);
        } else if (z->right == nil_) {
            x = z->left;
            transplant(z, z->left);
        } else {
            // Two children: y is z's inorder successor (min of right subtree).
            y = treeMinimum(z->right);
            yOriginalColor = y->color;
            x = y->right;
            if (y->parent == z) {
                x->parent = y;          // works even if x == nil_ (for fixup)
            } else {
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;        // y inherits z's color; net effect is
                                        // as if a node of yOriginalColor left.
        }
        delete z;
        if (yOriginalColor == Color::BLACK)
            deleteFixup(x);             // a black node vanished -> repair
    }

    // --- lookup / traversal / verification ---------------------------------
    Node* findNode(const T& key) const {
        Node* x = root_;
        while (x != nil_) {
            if (key < x->key) x = x->left;
            else if (x->key < key) x = x->right;
            else return x;
        }
        return nil_;
    }

    void destroy(Node* n) {
        if (n == nil_) return;          // sentinel is freed separately
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

    void collectInorder(const Node* n, std::vector<T>& out) const {
        if (n == nil_) return;
        collectInorder(n->left, out);
        out.push_back(n->key);
        collectInorder(n->right, out);
    }

    int heightNode(const Node* n) const {
        if (n == nil_) return 0;
        return 1 + std::max(heightNode(n->left), heightNode(n->right));
    }

    // Returns the black-height of the subtree (counting the black NIL leaf as
    // 1), or -1 if property 4 (no red-red) or property 5 (equal black-height)
    // is violated anywhere below n.
    int verifyNode(const Node* n) const {
        if (n == nil_) return 1;
        if (n->color == Color::RED &&
            (n->left->color == Color::RED || n->right->color == Color::RED))
            return -1;                                  // property 4
        const int lb = verifyNode(n->left);
        const int rb = verifyNode(n->right);
        if (lb < 0 || rb < 0 || lb != rb) return -1;    // property 5
        return lb + (n->color == Color::BLACK ? 1 : 0);
    }

public:
    RedBlackTree() {
        nil_ = new Node();               // the one shared black sentinel
        nil_->left = nil_->right = nil_->parent = nil_;
        root_ = nil_;                    // empty tree: root is the sentinel
    }

    ~RedBlackTree() {
        destroy(root_);                  // recursive: depth is O(log n), safe
        delete nil_;                     // delete nullptr is fine if moved-from
    }

    // Copy is deleted: nodes are raw-owned with parent links + a per-tree
    // sentinel, so a shallow copy would double-free. Move transfers ownership.
    RedBlackTree(const RedBlackTree&) = delete;
    RedBlackTree& operator=(const RedBlackTree&) = delete;

    RedBlackTree(RedBlackTree&& other) noexcept
        : nil_(other.nil_), root_(other.root_), count_(other.count_) {
        other.nil_ = nullptr;            // moved-from becomes null-but-safe:
        other.root_ = nullptr;           // destroy(nullptr) hits the nil_ base
        other.count_ = 0;                // case and delete nullptr is a no-op.
    }
    RedBlackTree& operator=(RedBlackTree&& other) noexcept {
        if (this != &other) {
            destroy(root_);
            delete nil_;
            nil_ = other.nil_;
            root_ = other.root_;
            count_ = other.count_;
            other.nil_ = nullptr;
            other.root_ = nullptr;
            other.count_ = 0;
        }
        return *this;
    }

    bool empty() const { return count_ == 0; }
    std::size_t size() const { return count_; }
    int height() const { return heightNode(root_); }

    // Black-height of the root (number of black nodes from root down to a NIL,
    // excluding the NIL). By property 5 this is the same along every path.
    int blackHeight() const {
        int bh = 0;
        for (const Node* n = root_; n != nil_; n = n->left)
            if (n->color == Color::BLACK) ++bh;
        return bh;
    }

    bool contains(const T& key) const { return findNode(key) != nil_; }

    // Returns true if inserted, false if the key already existed (set semantics).
    bool insert(const T& key) {
        Node* y = nil_;
        Node* x = root_;
        while (x != nil_) {              // descend to the insertion point
            y = x;
            if (key < x->key) x = x->left;
            else if (x->key < key) x = x->right;
            else return false;          // duplicate: reject
        }
        Node* z = new Node(key);        // new nodes start red
        z->parent = y;
        z->left = nil_;
        z->right = nil_;
        if (y == nil_) root_ = z;
        else if (z->key < y->key) y->left = z;
        else y->right = z;
        insertFixup(z);
        ++count_;
        return true;
    }

    // Returns true if the key was present and removed.
    bool remove(const T& key) {
        Node* z = findNode(key);
        if (z == nil_) return false;
        deleteNode(z);
        --count_;
        return true;
    }

    std::vector<T> inorder() const {
        std::vector<T> out;
        out.reserve(count_);
        collectInorder(root_, out);
        return out;
    }

    std::vector<T> levelOrder() const {
        std::vector<T> out;
        if (root_ == nil_) return out;
        std::queue<const Node*> q;
        q.push(root_);
        while (!q.empty()) {
            const Node* n = q.front();
            q.pop();
            out.push_back(n->key);
            if (n->left != nil_) q.push(n->left);
            if (n->right != nil_) q.push(n->right);
        }
        return out;
    }

    // Verify all five red-black properties plus BST ordering.
    bool checkInvariants() const {
        if (nil_->color != Color::BLACK) return false;   // property 3
        if (root_->color != Color::BLACK) return false;  // property 2
        if (verifyNode(root_) < 0) return false;         // properties 4 & 5
        const std::vector<T> v = inorder();              // BST ordering
        for (std::size_t i = 1; i < v.size(); ++i)
            if (!(v[i - 1] < v[i])) return false;
        return true;
        // Property 1 (every node red or black) is guaranteed by the enum type.
    }
};

// ---------------------------------------------------------------------------
static void runTests() {
    // Edge case: empty tree. Root is the black sentinel; all properties hold.
    {
        RedBlackTree<int> t;
        assert(t.empty());
        assert(t.size() == 0);
        assert(t.height() == 0);
        assert(!t.contains(1));
        assert(!t.remove(1));
        assert(t.inorder().empty());
        assert(t.checkInvariants());
    }

    // Edge case: single node -> must be black (root rule).
    {
        RedBlackTree<int> t;
        assert(t.insert(42));
        assert(!t.insert(42));            // duplicate rejected
        assert(t.size() == 1);
        assert(t.height() == 1);
        assert(t.blackHeight() == 1);
        assert(t.contains(42));
        assert(t.checkInvariants());
        assert(t.remove(42));
        assert(t.empty());
        assert(t.checkInvariants());
    }

    // Insert many keys; inorder must be sorted and all properties hold.
    {
        RedBlackTree<int> t;
        for (int v : {10, 20, 30, 15, 25, 5, 1, 17, 40, 35, 50, 45, 8, 12, 22})
            assert(t.insert(v));
        assert(t.checkInvariants());
        const std::vector<int> in = t.inorder();
        for (std::size_t i = 1; i < in.size(); ++i) assert(in[i - 1] < in[i]);

        // Deletions across all cases: leaf, one child, two children, and a
        // black node with a black successor (the hardest fixup path).
        for (int v : {10, 20, 30, 15, 25}) {
            assert(t.remove(v));
            assert(t.checkInvariants());  // invariant holds after EACH delete
        }
        assert(!t.remove(999));
        assert(t.checkInvariants());
        for (int v : {5, 1, 17, 40, 35, 50, 45, 8, 12, 22}) assert(t.remove(v));
        assert(t.empty());
        assert(t.checkInvariants());
    }

    // Ascending bulk insert would degenerate a plain BST; RB keeps it balanced.
    // Verify sorted inorder and a logarithmic height bound (h <= 2*log2(n+1)).
    {
        RedBlackTree<int> t;
        const int n = 1000;
        for (int i = 1; i <= n; ++i) assert(t.insert(i));
        assert(t.size() == static_cast<std::size_t>(n));
        assert(t.checkInvariants());

        const std::vector<int> in = t.inorder();
        for (int i = 0; i < n; ++i) assert(in[static_cast<std::size_t>(i)] == i + 1);

        const int bound = 2 * static_cast<int>(std::ceil(std::log2(static_cast<double>(n) + 1.0)));
        assert(t.height() <= bound);
        assert(t.height() < n);           // not a degenerate stick
    }

    // Pseudo-random insert/remove churn; invariant must survive throughout.
    {
        RedBlackTree<int> t;
        for (int i = 0; i < 500; ++i) t.insert((i * 131) % 997);
        assert(t.checkInvariants());
        for (int i = 0; i < 997; ++i) t.remove((i * 7) % 997);
        assert(t.checkInvariants());
        assert(t.empty());
    }

    // Move semantics: ownership transfers; moved-from is empty and destructible.
    {
        RedBlackTree<int> a;
        for (int v : {3, 1, 4, 1, 5, 9, 2, 6}) a.insert(v);  // note the dup '1'
        assert(a.size() == 7);
        RedBlackTree<int> b(std::move(a));
        assert(b.size() == 7);
        assert(b.checkInvariants());
        RedBlackTree<int> c;
        c = std::move(b);
        assert(c.size() == 7);
        assert(c.checkInvariants());
    }
}

int main() {
    runTests();

    // Human-readable demo.
    RedBlackTree<int> tree;
    std::cout << "Red-Black Tree demo -- inserting 1..15 in ascending order\n";
    for (int i = 1; i <= 15; ++i) tree.insert(i);

    std::cout << "size         = " << tree.size() << "\n";
    std::cout << "height       = " << tree.height()
              << "  (a plain BST would be 15 here)\n";
    std::cout << "black-height = " << tree.blackHeight() << "\n";

    std::cout << "inorder (sorted)    :";
    for (int v : tree.inorder()) std::cout << ' ' << v;
    std::cout << "\n";

    std::cout << "level-order (by row):";
    for (int v : tree.levelOrder()) std::cout << ' ' << v;
    std::cout << "\n";

    std::cout << "remove 8 (an internal node), then level-order:";
    tree.remove(8);
    for (int v : tree.levelOrder()) std::cout << ' ' << v;
    std::cout << "\n";

    std::cout << "all five RB properties hold : " << std::boolalpha
              << tree.checkInvariants() << "\n";
    std::cout << "All assertions passed.\n";
    return 0;
}
