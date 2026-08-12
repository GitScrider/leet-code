/*
 * Binary Tree (general, NON-ordered)            Data Structure - Tree
 * ----------------------------------------------------------------------
 * Summary: A generic binary tree where each node holds a value and up to
 * two children. There is NO ordering invariant; the teaching focus is the
 * four canonical traversals that every other tree structure builds upon.
 *
 * Operations & complexity (n = number of nodes, h = height):
 *   Operation            | Time      | Space (aux)
 *   ---------------------+-----------+-------------------------------
 *   inorder   (rec)      | O(n)      | O(h) recursion stack
 *   preorder  (rec)      | O(n)      | O(h) recursion stack
 *   postorder (rec)      | O(n)      | O(h) recursion stack
 *   inorder   (iter)     | O(n)      | O(h) explicit std::stack
 *   levelOrder (BFS)     | O(n)      | O(w) queue, w = max width
 *   height()             | O(n)      | O(h)
 *   size()               | O(n)      | O(h)
 *
 * Invariants:
 *   NONE regarding key ordering. A node simply has an optional left child
 *   and an optional right child. (Ordered variants -- BST/AVL/red-black --
 *   add an ordering invariant on top of this same node/link foundation.)
 *
 * When to use / trade-offs:
 *   - Use when the SHAPE of the data is meaningful (expression trees,
 *     parse trees, Huffman trees) rather than sorted lookup.
 *   - Traversal order encodes intent: preorder = copy/serialize,
 *     inorder = (for BSTs) sorted output, postorder = free/evaluate,
 *     level-order = breadth-first / shortest edge count from root.
 *   - No self-balancing: this base type says nothing about performance of
 *     search, because there is no search key to exploit.
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <queue>
#include <stack>
#include <utility>  // std::move
#include <vector>

template <typename T>
class BinaryTree {
private:
    struct Node {
        T value;
        Node* left;
        Node* right;
        explicit Node(const T& v) : value(v), left(nullptr), right(nullptr) {}
    };

    Node* root_;

    // Recursive deep-copy used by the copy constructor / copy assignment.
    // Returns a freshly allocated clone of the subtree rooted at 'n'.
    static Node* cloneSubtree(const Node* n) {
        if (n == nullptr) return nullptr;
        Node* copy = new Node(n->value);
        copy->left = cloneSubtree(n->left);
        copy->right = cloneSubtree(n->right);
        return copy;
    }

    // Postorder deletion: children are freed BEFORE the parent, so we never
    // dereference a dangling pointer. Recursion depth is O(h); for a
    // pathological (degenerate) tree this could be O(n), but for teaching
    // trees this is clear and correct.
    static void destroySubtree(Node* n) {
        if (n == nullptr) return;
        destroySubtree(n->left);
        destroySubtree(n->right);
        delete n;
    }

    // --- Recursive traversal helpers: append values into 'out'. ---
    static void inorderRec(const Node* n, std::vector<T>& out) {
        if (n == nullptr) return;
        inorderRec(n->left, out);   // Left
        out.push_back(n->value);    // Node
        inorderRec(n->right, out);  // Right
    }

    static void preorderRec(const Node* n, std::vector<T>& out) {
        if (n == nullptr) return;
        out.push_back(n->value);     // Node first
        preorderRec(n->left, out);   // Left
        preorderRec(n->right, out);  // Right
    }

    static void postorderRec(const Node* n, std::vector<T>& out) {
        if (n == nullptr) return;
        postorderRec(n->left, out);   // Left
        postorderRec(n->right, out);  // Right
        out.push_back(n->value);      // Node last
    }

    static std::size_t sizeRec(const Node* n) {
        if (n == nullptr) return 0;
        return 1 + sizeRec(n->left) + sizeRec(n->right);
    }

    // Height = number of edges on the longest root-to-leaf path.
    // An empty tree is -1 so that a single node has height 0.
    static int heightRec(const Node* n) {
        if (n == nullptr) return -1;
        int lh = heightRec(n->left);
        int rh = heightRec(n->right);
        return 1 + (lh > rh ? lh : rh);
    }

public:
    BinaryTree() : root_(nullptr) {}

    // --- Rule of Five ---------------------------------------------------
    // We own raw Node pointers, so the compiler-generated copy would be a
    // SHALLOW copy: two trees pointing at the same nodes -> double-free on
    // destruction. We implement deep copy + move to keep ownership clean.
    ~BinaryTree() { destroySubtree(root_); }

    BinaryTree(const BinaryTree& other) : root_(cloneSubtree(other.root_)) {}

    BinaryTree& operator=(const BinaryTree& other) {
        if (this != &other) {
            Node* newRoot = cloneSubtree(other.root_);  // build first,
            destroySubtree(root_);                      // then release old,
            root_ = newRoot;                            // strong exception safety
        }
        return *this;
    }

    BinaryTree(BinaryTree&& other) noexcept : root_(other.root_) {
        other.root_ = nullptr;  // leave source empty but destructible
    }

    BinaryTree& operator=(BinaryTree&& other) noexcept {
        if (this != &other) {
            destroySubtree(root_);
            root_ = other.root_;
            other.root_ = nullptr;
        }
        return *this;
    }

    // Because this base tree has NO ordering, we cannot "insert by key".
    // Instead we expose explicit construction so main() can build a known,
    // fixed shape by hand. setRoot returns the root's opaque handle.
    using Handle = Node*;

    Handle setRoot(const T& value) {
        destroySubtree(root_);
        root_ = new Node(value);
        return root_;
    }

    Handle addLeft(Handle parent, const T& value) {
        assert(parent != nullptr && parent->left == nullptr);
        parent->left = new Node(value);
        return parent->left;
    }

    Handle addRight(Handle parent, const T& value) {
        assert(parent != nullptr && parent->right == nullptr);
        parent->right = new Node(value);
        return parent->right;
    }

    bool empty() const { return root_ == nullptr; }
    std::size_t size() const { return sizeRec(root_); }
    int height() const { return heightRec(root_); }

    std::vector<T> inorder() const {
        std::vector<T> out;
        inorderRec(root_, out);
        return out;
    }

    std::vector<T> preorder() const {
        std::vector<T> out;
        preorderRec(root_, out);
        return out;
    }

    std::vector<T> postorder() const {
        std::vector<T> out;
        postorderRec(root_, out);
        return out;
    }

    // Iterative inorder with an explicit stack. WHY: it makes the implicit
    // call stack of the recursive version visible. We walk as far left as
    // possible, pushing each node; when we can go no further we pop, visit,
    // and pivot to the right subtree.
    std::vector<T> inorderIterative() const {
        std::vector<T> out;
        std::stack<Node*> st;
        Node* cur = root_;
        while (cur != nullptr || !st.empty()) {
            while (cur != nullptr) {   // dive left, remembering the path
                st.push(cur);
                cur = cur->left;
            }
            cur = st.top();            // leftmost unvisited node
            st.pop();
            out.push_back(cur->value); // visit
            cur = cur->right;          // then explore its right subtree
        }
        return out;
    }

    // Level-order (breadth-first) traversal with a FIFO queue. WHY a queue:
    // nodes are visited in the exact order they are discovered, so all of
    // depth d is emitted before any node of depth d+1.
    std::vector<T> levelOrder() const {
        std::vector<T> out;
        if (root_ == nullptr) return out;
        std::queue<Node*> q;
        q.push(root_);
        while (!q.empty()) {
            Node* n = q.front();
            q.pop();
            out.push_back(n->value);
            if (n->left) q.push(n->left);
            if (n->right) q.push(n->right);
        }
        return out;
    }
};

int main() {
    // ---- Edge case: empty tree ----------------------------------------
    BinaryTree<int> empty;
    assert(empty.empty());
    assert(empty.size() == 0);
    assert(empty.height() == -1);          // convention: empty = -1
    assert(empty.inorder().empty());
    assert(empty.inorderIterative().empty());
    assert(empty.levelOrder().empty());

    // ---- Edge case: single node ---------------------------------------
    BinaryTree<int> single;
    single.setRoot(42);
    assert(single.size() == 1);
    assert(single.height() == 0);
    assert((single.inorder() == std::vector<int>{42}));
    assert((single.levelOrder() == std::vector<int>{42}));

    // ---- Build a KNOWN fixed tree by hand ------------------------------
    /*
     *            1
     *          /   \
     *         2     3
     *        / \     \
     *       4   5     6
     *          /
     *         7
     */
    BinaryTree<int> t;
    auto n1 = t.setRoot(1);
    auto n2 = t.addLeft(n1, 2);
    auto n3 = t.addRight(n1, 3);
    auto n4 = t.addLeft(n2, 4);
    auto n5 = t.addRight(n2, 5);
    (void)n4;
    t.addRight(n3, 6);
    t.addLeft(n5, 7);

    assert(t.size() == 7);
    assert(t.height() == 3);  // 1 -> 2 -> 5 -> 7 is 3 edges

    // Assert the EXACT traversal sequences for this fixed shape.
    const std::vector<int> expectedInorder   = {4, 2, 7, 5, 1, 3, 6};
    const std::vector<int> expectedPreorder  = {1, 2, 4, 5, 7, 3, 6};
    const std::vector<int> expectedPostorder = {4, 7, 5, 2, 6, 3, 1};
    const std::vector<int> expectedLevel     = {1, 2, 3, 4, 5, 6, 7};

    assert(t.inorder() == expectedInorder);
    assert(t.preorder() == expectedPreorder);
    assert(t.postorder() == expectedPostorder);
    assert(t.levelOrder() == expectedLevel);

    // The iterative inorder MUST match the recursive one exactly.
    assert(t.inorderIterative() == expectedInorder);

    // ---- Rule of Five: deep copy must be independent ------------------
    BinaryTree<int> copy = t;                 // copy constructor
    assert(copy.inorder() == expectedInorder);
    BinaryTree<int> assigned;
    assigned = t;                             // copy assignment
    assert(assigned.preorder() == expectedPreorder);
    // Mutating the original does not disturb the copies (separate nodes).
    t.setRoot(99);
    assert(t.size() == 1);
    assert(copy.inorder() == expectedInorder);
    assert(assigned.preorder() == expectedPreorder);

    // Move leaves the source empty but valid.
    BinaryTree<int> moved = std::move(copy);
    assert(moved.inorder() == expectedInorder);
    assert(copy.empty());

    // ---- Human-readable demo ------------------------------------------
    std::cout << "Binary Tree traversals for the fixed sample tree:\n";
    auto printSeq = [](const char* label, const std::vector<int>& v) {
        std::cout << "  " << label << ": ";
        for (std::size_t i = 0; i < v.size(); ++i)
            std::cout << v[i] << (i + 1 < v.size() ? ' ' : '\n');
    };
    printSeq("preorder    ", expectedPreorder);
    printSeq("inorder     ", expectedInorder);
    printSeq("inorder(it) ", assigned.inorderIterative());
    printSeq("postorder   ", expectedPostorder);
    printSeq("level-order ", expectedLevel);
    std::cout << "  size = " << assigned.size()
              << ", height = " << assigned.height() << "\n";
    std::cout << "All binary tree assertions passed.\n";
    return 0;
}
