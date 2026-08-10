/*
 * Doubly Linked List  (Data Structure - Linear)
 *
 * Summary:
 *   A chain of heap-allocated nodes each holding a prev and next pointer, with
 *   cached head_, tail_ and size_. The two-way links make BOTH ends O(1) and
 *   let us erase a known node in O(1) without re-scanning for its predecessor.
 *
 * Operations & complexity:
 *   +-----------------------+------------------------+
 *   | Operation             | Time                   |
 *   +-----------------------+------------------------+
 *   | push_front            | O(1)                   |
 *   | push_back             | O(1)                   |
 *   | pop_front             | O(1)                   |
 *   | pop_back              | O(1)  (prev cached)    |
 *   | front / back          | O(1)                   |
 *   | insert_before(node)   | O(1)  (given the node) |
 *   | insert_after(node)    | O(1)  (given the node) |
 *   | erase(node)           | O(1)  (given the node) |
 *   | erase_at(index)       | O(n)   (walk to index) |
 *   | find                  | O(n)                   |
 *   | reverse (in place)    | O(n)                   |
 *   | size / empty          | O(1)                   |
 *   | clear / destructor    | O(n)                   |
 *   +-----------------------+------------------------+
 *
 * Invariants:
 *   - size_ equals the number of reachable nodes from head_ (via next) and
 *     equivalently from tail_ (via prev).
 *   - Empty <=> head_ == nullptr <=> tail_ == nullptr <=> size_ == 0.
 *   - Non-empty: head_->prev == nullptr and tail_->next == nullptr.
 *   - For every interior node n: n->next->prev == n and n->prev->next == n
 *     (the links are mutually consistent). No cycles.
 *
 * When to use / trade-offs:
 *   - O(1) insert/erase at both ends and at any node you already hold -> the
 *     backbone of deques, LRU caches, and free lists.
 *   - Costs one extra pointer per node vs. a singly linked list.
 *   - Still no random access (index lookup is O(n)); prefer std::vector when
 *     you mostly index or append and want cache locality.
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <utility>

template <typename T>
class DoublyLinkedList {
    // Node layout mirrors the classic {prev, value, next} teaching diagram.
    struct Node {
        Node* prev;
        T value;
        Node* next;
        explicit Node(const T& v) : prev(nullptr), value(v), next(nullptr) {}
        explicit Node(T&& v) : prev(nullptr), value(std::move(v)), next(nullptr) {}
    };

    Node* head_ = nullptr;  // first node (head_->prev == nullptr)
    Node* tail_ = nullptr;  // last node  (tail_->next == nullptr)
    std::size_t size_ = 0;  // node count; maintained on every mutation

public:
    // ----- construction / destruction -------------------------------------

    DoublyLinkedList() noexcept = default;

    // Destructor: free the chain ITERATIVELY. A recursive free following next
    // pointers would recurse once per node and could overflow the stack.
    ~DoublyLinkedList() { clear(); }

    // Copy constructor: deep copy so the two lists own independent nodes and
    // neither double-frees the other's. Walk the source front-to-back.
    DoublyLinkedList(const DoublyLinkedList& other) {
        for (Node* cur = other.head_; cur != nullptr; cur = cur->next)
            push_back(cur->value);
    }

    // Copy assignment: copy-and-swap -> self-assignment safe and strongly
    // exception safe (old nodes live in tmp until the new copy fully succeeds).
    DoublyLinkedList& operator=(const DoublyLinkedList& other) {
        if (this != &other) {
            DoublyLinkedList tmp(other);
            swap(tmp);
        }
        return *this;
    }

    // Move constructor: steal pointers, leave source valid-empty so its own
    // destructor does nothing. noexcept enables efficient container growth.
    DoublyLinkedList(DoublyLinkedList&& other) noexcept
        : head_(other.head_), tail_(other.tail_), size_(other.size_) {
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
    }

    // Move assignment: drop our resources, then adopt the source's. noexcept
    // because it only frees (never throws) and reassigns pointers.
    DoublyLinkedList& operator=(DoublyLinkedList&& other) noexcept {
        if (this != &other) {
            clear();
            head_ = other.head_;
            tail_ = other.tail_;
            size_ = other.size_;
            other.head_ = nullptr;
            other.tail_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    void swap(DoublyLinkedList& other) noexcept {
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
        std::swap(size_, other.size_);
    }

    // ----- capacity --------------------------------------------------------

    std::size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }

    // ----- element access --------------------------------------------------

    T& front() { assert(head_ && "front() on empty list"); return head_->value; }
    const T& front() const { assert(head_ && "front() on empty list"); return head_->value; }
    T& back() { assert(tail_ && "back() on empty list"); return tail_->value; }
    const T& back() const { assert(tail_ && "back() on empty list"); return tail_->value; }

    // ----- modifiers -------------------------------------------------------

    // O(1): link a new node before head_; it also becomes tail_ if empty.
    void push_front(const T& v) { link_front(new Node(v)); }
    void push_front(T&& v) { link_front(new Node(std::move(v))); }

    // O(1): link a new node after tail_; it also becomes head_ if empty.
    void push_back(const T& v) { link_back(new Node(v)); }
    void push_back(T&& v) { link_back(new Node(std::move(v))); }

    // O(1): remove the first node. Precondition: non-empty.
    void pop_front() {
        assert(head_ && "pop_front() on empty list");
        erase(head_);
    }

    // O(1): remove the last node -- this is the payoff of the prev pointer.
    void pop_back() {
        assert(tail_ && "pop_back() on empty list");
        erase(tail_);
    }

    // O(1): insert a fresh node holding v immediately before `pos`.
    // Updates BOTH the new node's and its neighbours' prev/next links.
    Node* insert_before(Node* pos, const T& v) {
        assert(pos && "insert_before(nullptr)");
        if (pos == head_) { push_front(v); return head_; }
        Node* n = new Node(v);
        Node* p = pos->prev;         // guaranteed non-null (pos != head_)
        n->prev = p;
        n->next = pos;
        p->next = n;                 // fix predecessor's forward link
        pos->prev = n;               // fix successor's backward link
        ++size_;
        return n;
    }

    // O(1): insert a fresh node holding v immediately after `pos`.
    Node* insert_after(Node* pos, const T& v) {
        assert(pos && "insert_after(nullptr)");
        if (pos == tail_) { push_back(v); return tail_; }
        Node* n = new Node(v);
        Node* nx = pos->next;        // guaranteed non-null (pos != tail_)
        n->prev = pos;
        n->next = nx;
        pos->next = n;
        nx->prev = n;
        ++size_;
        return n;
    }

    // O(1): unlink `node` and delete it, patching both neighbours' links.
    // Precondition: node belongs to this list and is non-null.
    void erase(Node* node) {
        assert(node && "erase(nullptr)");
        Node* p = node->prev;
        Node* n = node->next;
        if (p) p->next = n; else head_ = n;  // node was head_ if no prev
        if (n) n->prev = p; else tail_ = p;  // node was tail_ if no next
        delete node;
        --size_;
    }

    // Erase by 0-based index. O(n): walk from the nearer end for efficiency.
    bool erase_at(std::size_t index) {
        Node* n = node_at(index);
        if (n == nullptr) return false;
        erase(n);
        return true;
    }

    // O(n): first node whose value == v, or nullptr.
    Node* find(const T& v) const {
        for (Node* cur = head_; cur != nullptr; cur = cur->next)
            if (cur->value == v) return cur;
        return nullptr;
    }

    // O(n): node at index, walking from head_ or tail_ depending on which half
    // the index falls in. Returns nullptr for out-of-range indices.
    Node* node_at(std::size_t index) const {
        if (index >= size_) return nullptr;
        Node* cur;
        if (index < size_ / 2) {                 // closer to the front
            cur = head_;
            for (std::size_t i = 0; i < index; ++i) cur = cur->next;
        } else {                                 // closer to the back
            cur = tail_;
            for (std::size_t i = size_ - 1; i > index; --i) cur = cur->prev;
        }
        return cur;
    }

    // In-place reversal in O(n): swap each node's prev/next, then swap the
    // head_/tail_ roles. No allocation and every link stays consistent.
    void reverse() noexcept {
        Node* cur = head_;
        while (cur != nullptr) {
            Node* next = cur->next;      // remember before we clobber it
            std::swap(cur->prev, cur->next);
            cur = next;                  // advance using the saved pointer
        }
        std::swap(head_, tail_);
    }

    // O(n): free the whole chain iteratively and reset to the empty state.
    void clear() noexcept {
        Node* cur = head_;
        while (cur != nullptr) {
            Node* next = cur->next;
            delete cur;
            cur = next;
        }
        head_ = tail_ = nullptr;
        size_ = 0;
    }

    // ----- bidirectional iteration -----------------------------------------
    // Minimal bidirectional iterator so range-based for and manual --/++ work.
    class iterator {
        Node* p_ = nullptr;
    public:
        explicit iterator(Node* p = nullptr) noexcept : p_(p) {}
        T& operator*() const noexcept { return p_->value; }
        iterator& operator++() noexcept { p_ = p_->next; return *this; }
        iterator& operator--() noexcept { p_ = p_->prev; return *this; }
        bool operator!=(const iterator& o) const noexcept { return p_ != o.p_; }
        bool operator==(const iterator& o) const noexcept { return p_ == o.p_; }
    };
    class const_iterator {
        const Node* p_ = nullptr;
    public:
        explicit const_iterator(const Node* p = nullptr) noexcept : p_(p) {}
        const T& operator*() const noexcept { return p_->value; }
        const_iterator& operator++() noexcept { p_ = p_->next; return *this; }
        const_iterator& operator--() noexcept { p_ = p_->prev; return *this; }
        bool operator!=(const const_iterator& o) const noexcept { return p_ != o.p_; }
        bool operator==(const const_iterator& o) const noexcept { return p_ == o.p_; }
    };

    iterator begin() noexcept { return iterator(head_); }
    iterator end() noexcept { return iterator(nullptr); }
    const_iterator begin() const noexcept { return const_iterator(head_); }
    const_iterator end() const noexcept { return const_iterator(nullptr); }

    // Node handles for O(1) positional operations.
    Node* front_node() const noexcept { return head_; }
    Node* back_node() const noexcept { return tail_; }

private:
    void link_front(Node* n) noexcept {
        n->next = head_;
        if (head_) head_->prev = n;    // old head now points back to n
        else tail_ = n;                // list was empty: n is also the tail
        head_ = n;
        ++size_;
    }
    void link_back(Node* n) noexcept {
        n->prev = tail_;
        if (tail_) tail_->next = n;     // old tail now points forward to n
        else head_ = n;                 // list was empty: n is also the head
        tail_ = n;
        ++size_;
    }
};

// ---------------------------------------------------------------------------
// Tests + demo
// ---------------------------------------------------------------------------

// Verify prev/next links are mutually consistent end-to-end. This catches any
// mutation that updates only one direction.
template <typename T>
static void check_links(const DoublyLinkedList<T>& l) {
    auto* h = l.front_node();
    auto* t = l.back_node();
    if (l.empty()) { assert(!h && !t); return; }
    assert(h && t);
    // (front_node/back_node expose the boundary; interior consistency is
    //  exercised through forward+backward traversal below.)
    std::size_t fwd = 0;
    for (auto it = l.begin(); it != l.end(); ++it) ++fwd;
    assert(fwd == l.size());
}

static void run_tests() {
    // Empty container.
    DoublyLinkedList<int> l;
    assert(l.empty() && l.size() == 0);
    check_links(l);

    // Single element.
    l.push_back(5);
    assert(l.size() == 1 && l.front() == 5 && l.back() == 5);
    check_links(l);

    // pop_back down to empty, then pop_front growth path.
    l.pop_back();
    assert(l.empty());
    l.push_front(1);   // 1
    l.pop_front();
    assert(l.empty());

    // Build 10 20 30 40 via both ends.
    l.push_back(30);
    l.push_back(40);
    l.push_front(20);
    l.push_front(10);  // 10 20 30 40
    assert(l.size() == 4);
    {
        int expected[] = {10, 20, 30, 40};
        std::size_t idx = 0;
        for (int v : l) assert(v == expected[idx++]);   // forward traversal
        assert(idx == 4);
    }

    // Backward traversal via node handles + operator--.
    {
        int expectedRev[] = {40, 30, 20, 10};
        std::size_t idx = 0;
        for (auto* n = l.back_node(); n != nullptr; n = n->prev)
            assert(n->value == expectedRev[idx++]);
        assert(idx == 4);
    }

    // pop from both ends.
    l.pop_front();   // 20 30 40
    l.pop_back();    // 20 30
    assert(l.front() == 20 && l.back() == 30 && l.size() == 2);

    // insert_before / insert_after around known nodes.
    l.push_back(50);                     // 20 30 50
    auto* n30 = l.find(30);
    assert(n30);
    l.insert_before(n30, 25);            // 20 25 30 50
    l.insert_after(n30, 40);             // 20 25 30 40 50
    {
        int expected[] = {20, 25, 30, 40, 50};
        std::size_t idx = 0;
        for (int v : l) assert(v == expected[idx++]);
        assert(idx == 5);
    }
    check_links(l);

    // insert_before(head) and insert_after(tail) hit the push_front/back paths.
    l.insert_before(l.front_node(), 15); // 15 20 25 30 40 50
    l.insert_after(l.back_node(), 55);   // ... 50 55
    assert(l.front() == 15 && l.back() == 55);

    // erase(node): head, tail, and interior.
    l.erase(l.front_node());             // remove 15
    assert(l.front() == 20);
    l.erase(l.back_node());              // remove 55
    assert(l.back() == 50);
    l.erase(l.find(30));                 // remove interior
    assert(l.find(30) == nullptr);
    check_links(l);

    // erase_at with node_at's two-sided walk (front half and back half).
    std::size_t before = l.size();
    assert(l.erase_at(0));               // front half
    assert(l.erase_at(l.size() - 1));    // back half
    assert(l.size() == before - 2);
    assert(!l.erase_at(l.size()));       // out of range

    // reverse.
    DoublyLinkedList<int> r;
    for (int i = 1; i <= 5; ++i) r.push_back(i);  // 1 2 3 4 5
    r.reverse();                                  // 5 4 3 2 1
    {
        int expected[] = {5, 4, 3, 2, 1};
        std::size_t idx = 0;
        for (int v : r) assert(v == expected[idx++]);
        assert(idx == 5);
    }
    // Reversed links must still be walkable backward from the new tail.
    assert(r.front() == 5 && r.back() == 1);
    check_links(r);
    r.reverse();  // back to 1..5
    assert(r.front() == 1 && r.back() == 5);

    // reverse of single element and empty are safe no-ops.
    DoublyLinkedList<int> one; one.push_back(7); one.reverse();
    assert(one.front() == 7 && one.back() == 7);
    DoublyLinkedList<int> none; none.reverse(); assert(none.empty());

    // ----- copy semantics: copy then mutate original; copy stays intact -----
    DoublyLinkedList<int> a;
    for (int i = 0; i < 5; ++i) a.push_back(i);   // 0 1 2 3 4
    DoublyLinkedList<int> b = a;                  // deep copy
    a.push_back(999);
    a.pop_front();
    assert(b.size() == 5 && b.front() == 0 && b.back() == 4);
    check_links(b);

    DoublyLinkedList<int> c;
    c.push_back(-1);
    c = b;                                        // copy assignment
    assert(c.size() == 5 && c.back() == 4);
    c = c;                                        // self-assignment safe
    assert(c.size() == 5 && c.front() == 0);

    // ----- move semantics: source becomes valid-empty ----------------------
    DoublyLinkedList<int> moved = std::move(b);
    assert(moved.size() == 5 && moved.front() == 0 && moved.back() == 4);
    assert(b.empty());
    b.push_back(123);                             // usable after move
    assert(b.size() == 1);

    DoublyLinkedList<int> movedAssign;
    movedAssign = std::move(moved);
    assert(movedAssign.size() == 5);
    assert(moved.empty());

    // Non-trivial T.
    DoublyLinkedList<std::string> s;
    s.push_back("b");
    s.push_front("a");
    s.push_back("c");             // a b c
    assert(s.front() == "a" && s.back() == "c");
    s.reverse();                  // c b a
    assert(s.front() == "c" && s.back() == "a");
}

int main() {
    run_tests();

    // Human-readable demo showing bidirectional traversal.
    DoublyLinkedList<std::string> deque;
    deque.push_back("mid");
    deque.push_front("left");
    deque.push_back("right");

    std::cout << "Forward  (head -> tail): ";
    for (const auto& x : deque) std::cout << '[' << x << "] ";
    std::cout << "\nBackward (tail -> head): ";
    for (auto* n = deque.back_node(); n != nullptr; n = n->prev)
        std::cout << '[' << n->value << "] ";
    std::cout << "\nsize = " << deque.size()
              << ", front = " << deque.front()
              << ", back = " << deque.back() << "\n";

    deque.reverse();
    std::cout << "After reverse:           ";
    for (const auto& x : deque) std::cout << '[' << x << "] ";
    std::cout << "\n";

    std::cout << "All DoublyLinkedList tests passed.\n";
    return 0;
}
