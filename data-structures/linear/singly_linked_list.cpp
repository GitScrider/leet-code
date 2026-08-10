/*
 * Singly Linked List  (Data Structure - Linear)
 *
 * Summary:
 *   A chain of heap-allocated nodes, each owning the next via a raw pointer.
 *   We cache head_, tail_ and size_ so both ends are O(1) to reach while the
 *   list still uses only forward links.
 *
 * Operations & complexity:
 *   +---------------------+------------------------+
 *   | Operation           | Time                   |
 *   +---------------------+------------------------+
 *   | push_front          | O(1)                   |
 *   | push_back           | O(1)  (tail_ cached)   |
 *   | pop_front           | O(1)                   |
 *   | front / back        | O(1)                   |
 *   | insert_after(pos)   | O(1)  (given the node) |
 *   | erase (by value)    | O(n)                   |
 *   | erase (position it) | O(n)  (need prev link) |
 *   | find                | O(n)                   |
 *   | reverse (in place)  | O(n)                   |
 *   | size / empty        | O(1)                   |
 *   | clear / destructor  | O(n)                   |
 *   +---------------------+------------------------+
 *   Note: pop_back would be O(n) on a singly linked list (no prev link to the
 *   new tail), so it is intentionally NOT provided.
 *
 * Invariants:
 *   - size_ always equals the number of reachable nodes from head_.
 *   - Empty list <=> head_ == nullptr <=> tail_ == nullptr <=> size_ == 0.
 *   - Non-empty list: tail_->next == nullptr, tail_ is reachable from head_.
 *   - Exactly one node (the tail) has next == nullptr; no cycles.
 *
 * When to use / trade-offs:
 *   - O(1) push/pop at the front and O(1) push at the back with tiny per-node
 *     overhead (one pointer) -> good as a stack or FIFO building block.
 *   - No random access: index/erase-in-the-middle costs O(n) because you must
 *     walk from head_ to find the predecessor.
 *   - Prefer std::vector for cache locality; prefer this when you splice a lot
 *     at the front or need stable node addresses.
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <utility>

template <typename T>
class SinglyLinkedList {
    // A node owns nothing but its value; the list owns the node chain.
    struct Node {
        T value;
        Node* next;
        explicit Node(const T& v, Node* n = nullptr) : value(v), next(n) {}
        explicit Node(T&& v, Node* n = nullptr) : value(std::move(v)), next(n) {}
    };

    Node* head_ = nullptr;  // first node, or nullptr when empty
    Node* tail_ = nullptr;  // last node (cached so push_back is O(1))
    std::size_t size_ = 0;  // number of nodes; kept in sync on every mutation

public:
    // ----- construction / destruction -------------------------------------

    SinglyLinkedList() noexcept = default;

    // Destructor: release every node. We free ITERATIVELY rather than relying
    // on a recursive ~Node(); a long list would otherwise blow the call stack.
    ~SinglyLinkedList() { clear(); }

    // Copy constructor: deep copy is required because two lists must not share
    // (and later double-free) the same nodes. Walk the source iteratively.
    SinglyLinkedList(const SinglyLinkedList& other) {
        for (Node* cur = other.head_; cur != nullptr; cur = cur->next)
            push_back(cur->value);
    }

    // Copy assignment: needed for the same ownership reason as the copy ctor.
    // copy-and-swap gives us self-assignment safety and the strong guarantee
    // for free (the temporary holds the old resources and dies at the brace).
    SinglyLinkedList& operator=(const SinglyLinkedList& other) {
        if (this != &other) {
            SinglyLinkedList tmp(other);  // deep copy first (may throw)
            swap(tmp);                    // then commit with a noexcept swap
        }
        return *this;
    }

    // Move constructor: steal the pointers so no allocation/copy happens, and
    // leave the source in a valid empty state so its destructor is a no-op.
    SinglyLinkedList(SinglyLinkedList&& other) noexcept
        : head_(other.head_), tail_(other.tail_), size_(other.size_) {
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
    }

    // Move assignment: release our own resources, then steal the source's.
    // noexcept because it only shuffles pointers (clear() frees, never throws).
    SinglyLinkedList& operator=(SinglyLinkedList&& other) noexcept {
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

    void swap(SinglyLinkedList& other) noexcept {
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

    // O(1): new node becomes head_; it also becomes tail_ if the list was empty.
    void push_front(const T& v) { emplace_front(new Node(v)); }
    void push_front(T&& v) { emplace_front(new Node(std::move(v))); }

    // O(1): link after tail_ (or set both ends if empty), then advance tail_.
    void push_back(const T& v) { emplace_back(new Node(v)); }
    void push_back(T&& v) { emplace_back(new Node(std::move(v))); }

    // O(1): unlink and delete the head. Precondition: list is non-empty.
    void pop_front() {
        assert(head_ && "pop_front() on empty list");
        Node* old = head_;
        head_ = head_->next;
        if (head_ == nullptr) tail_ = nullptr;  // list became empty
        delete old;
        --size_;
    }

    // O(1) given the predecessor node: splice a fresh node after `pos`.
    // If pos was the tail, the new node becomes tail_. Returns the new node so
    // callers can chain inserts. pos == nullptr is treated as "insert at front".
    Node* insert_after(Node* pos, const T& v) {
        if (pos == nullptr) { push_front(v); return head_; }
        Node* n = new Node(v, pos->next);
        pos->next = n;
        if (n->next == nullptr) tail_ = n;  // inserted after the old tail
        ++size_;
        return n;
    }

    // Erase the first node holding `value`. Returns true if one was removed.
    // O(n): we must find the predecessor to relink around the victim.
    bool erase(const T& value) {
        Node* prev = nullptr;
        for (Node* cur = head_; cur != nullptr; prev = cur, cur = cur->next) {
            if (cur->value == value) {
                unlink(prev, cur);
                return true;
            }
        }
        return false;
    }

    // Erase the node at 0-based index. Returns true if the index was valid.
    // O(n): walk to the predecessor of the target.
    bool erase_at(std::size_t index) {
        if (index >= size_) return false;
        Node* prev = nullptr;
        Node* cur = head_;
        for (std::size_t i = 0; i < index; ++i) { prev = cur; cur = cur->next; }
        unlink(prev, cur);
        return true;
    }

    // O(n): return the first node whose value == v, or nullptr if none.
    Node* find(const T& v) const {
        for (Node* cur = head_; cur != nullptr; cur = cur->next)
            if (cur->value == v) return cur;
        return nullptr;
    }

    // In-place reversal in O(n) time, O(1) extra space. We flip every next
    // pointer, then swap the head_/tail_ roles. No nodes are (de)allocated.
    void reverse() noexcept {
        Node* prev = nullptr;
        Node* cur = head_;
        tail_ = head_;  // the old head becomes the new tail
        while (cur != nullptr) {
            Node* next = cur->next;
            cur->next = prev;
            prev = cur;
            cur = next;
        }
        head_ = prev;  // the old tail becomes the new head
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

    // ----- iteration -------------------------------------------------------
    // A minimal forward iterator so range-based for works naturally.
    class iterator {
        Node* p_ = nullptr;
    public:
        explicit iterator(Node* p = nullptr) noexcept : p_(p) {}
        T& operator*() const noexcept { return p_->value; }
        iterator& operator++() noexcept { p_ = p_->next; return *this; }
        bool operator!=(const iterator& o) const noexcept { return p_ != o.p_; }
        bool operator==(const iterator& o) const noexcept { return p_ == o.p_; }
    };
    class const_iterator {
        const Node* p_ = nullptr;
    public:
        explicit const_iterator(const Node* p = nullptr) noexcept : p_(p) {}
        const T& operator*() const noexcept { return p_->value; }
        const_iterator& operator++() noexcept { p_ = p_->next; return *this; }
        bool operator!=(const const_iterator& o) const noexcept { return p_ != o.p_; }
        bool operator==(const const_iterator& o) const noexcept { return p_ == o.p_; }
    };

    iterator begin() noexcept { return iterator(head_); }
    iterator end() noexcept { return iterator(nullptr); }
    const_iterator begin() const noexcept { return const_iterator(head_); }
    const_iterator end() const noexcept { return const_iterator(nullptr); }

    // Expose head as a starting node handle for insert_after-based callers.
    Node* head_node() const noexcept { return head_; }

private:
    void emplace_front(Node* n) noexcept {
        n->next = head_;
        head_ = n;
        if (tail_ == nullptr) tail_ = n;  // first element: it is also the tail
        ++size_;
    }
    void emplace_back(Node* n) noexcept {
        if (tail_ == nullptr) head_ = tail_ = n;  // was empty
        else { tail_->next = n; tail_ = n; }
        ++size_;
    }
    // Remove `cur`, whose predecessor is `prev` (nullptr means cur == head_).
    void unlink(Node* prev, Node* cur) noexcept {
        if (prev == nullptr) head_ = cur->next;   // erasing the head
        else prev->next = cur->next;              // bridge over cur
        if (cur == tail_) tail_ = prev;           // erasing the tail
        delete cur;
        --size_;
    }
};

// ---------------------------------------------------------------------------
// Tests + demo
// ---------------------------------------------------------------------------

static void run_tests() {
    // Empty container.
    SinglyLinkedList<int> l;
    assert(l.empty());
    assert(l.size() == 0);

    // Single element: front and back must agree.
    l.push_back(42);
    assert(!l.empty());
    assert(l.size() == 1);
    assert(l.front() == 42 && l.back() == 42);

    // pop down to empty, then confirm empty-state invariants.
    l.pop_front();
    assert(l.empty() && l.size() == 0);

    // Growth: mix push_front / push_back -> expect 3 2 1 10 20 30.
    for (int i = 1; i <= 3; ++i) l.push_front(i * 10);  // 30 20 10
    l.reverse();                                        // 10 20 30
    for (int i = 1; i <= 3; ++i) l.push_front(i);       // 3 2 1 10 20 30
    assert(l.size() == 6);
    {
        int expected[] = {3, 2, 1, 10, 20, 30};
        std::size_t idx = 0;
        for (int v : l) assert(v == expected[idx++]);   // range-based for
        assert(idx == 6);
    }

    // find + insert_after: place 99 right after the node holding 1.
    auto* n1 = l.find(1);
    assert(n1 != nullptr);
    l.insert_after(n1, 99);                             // 3 2 1 99 10 20 30
    assert(l.size() == 7);
    assert(l.find(99) != nullptr);

    // insert_after the tail keeps tail_ correct (back() must update).
    l.insert_after(l.find(30), 100);                    // ... 30 100
    assert(l.back() == 100);

    // erase by value (head, middle, tail).
    assert(l.erase(3));      // remove head
    assert(l.front() == 2);
    assert(l.erase(99));     // remove middle
    assert(l.erase(100));    // remove tail
    assert(l.back() == 30);
    assert(!l.erase(12345)); // absent value -> false

    // erase_at bounds.
    assert(!l.erase_at(l.size()));  // out of range
    std::size_t before = l.size();
    assert(l.erase_at(0));          // remove current head
    assert(l.size() == before - 1);

    // reverse a single element is a no-op on values.
    SinglyLinkedList<int> one;
    one.push_back(7);
    one.reverse();
    assert(one.front() == 7 && one.back() == 7);

    // ----- copy semantics: copy then mutate original; copy stays intact -----
    SinglyLinkedList<int> a;
    for (int i = 0; i < 5; ++i) a.push_back(i);   // 0 1 2 3 4
    SinglyLinkedList<int> b = a;                  // deep copy
    a.push_back(999);
    a.pop_front();
    assert(b.size() == 5);
    assert(b.front() == 0 && b.back() == 4);      // b untouched by a's edits

    SinglyLinkedList<int> c;
    c.push_back(-1);
    c = b;                                        // copy assignment
    assert(c.size() == 5 && c.back() == 4);
    c = c;                                        // self-assignment is safe
    assert(c.size() == 5 && c.front() == 0);

    // ----- move semantics: source becomes valid-empty -----------------------
    SinglyLinkedList<int> moved = std::move(b);
    assert(moved.size() == 5 && moved.front() == 0);
    assert(b.empty());                            // NOLINT: use-after-move check
    b.push_back(123);                             // still usable after move
    assert(b.size() == 1 && b.front() == 123);

    SinglyLinkedList<int> movedAssign;
    movedAssign = std::move(moved);
    assert(movedAssign.size() == 5);
    assert(moved.empty());

    // Works for a non-trivial T (std::string) too.
    SinglyLinkedList<std::string> s;
    s.push_back("hello");
    s.push_front("say");
    assert(s.front() == "say" && s.back() == "hello");
    s.reverse();
    assert(s.front() == "hello" && s.back() == "say");
}

int main() {
    run_tests();

    // Human-readable demo.
    SinglyLinkedList<std::string> playlist;
    playlist.push_back("intro");
    playlist.push_back("verse");
    playlist.push_back("chorus");
    playlist.push_front("silence");

    std::cout << "Playlist (front -> back): ";
    for (const auto& track : playlist) std::cout << '[' << track << "] ";
    std::cout << "\nsize = " << playlist.size() << "\n";

    playlist.reverse();
    std::cout << "After reverse:            ";
    for (const auto& track : playlist) std::cout << '[' << track << "] ";
    std::cout << "\nfront = " << playlist.front()
              << ", back = " << playlist.back() << "\n";

    std::cout << "All SinglyLinkedList tests passed.\n";
    return 0;
}
