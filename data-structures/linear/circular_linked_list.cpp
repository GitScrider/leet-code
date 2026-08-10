/*
 * Circular Linked List  (Data Structure - Linear, circular variant)
 *
 * Summary:
 *   A SINGLY linked list where the last node's `next` points back to the head
 *   instead of to nullptr, forming one closed ring. We cache a `tail_` pointer
 *   with the invariant `tail_->next == head_`, which makes BOTH push_front and
 *   push_back O(1) (the tail already knows where the head is).
 *
 * Operations & complexity:
 *   +----------------------+---------------------------------+
 *   | Operation            | Time                            |
 *   +----------------------+---------------------------------+
 *   | push_front           | O(1)                            |
 *   | push_back            | O(1)  (tail_ cached)            |
 *   | pop_front            | O(1)                            |
 *   | front / back         | O(1)                            |
 *   | insert(pos, value)   | O(pos)  (walk to predecessor)   |
 *   | erase(value)         | O(n)   (find predecessor)       |
 *   | rotate(k)            | O(k mod n)  (advance the head)  |
 *   | toVector / traversal | O(n)   (walk EXACTLY size_)     |
 *   | size / empty         | O(1)                            |
 *   | clear / destructor   | O(n)                            |
 *   +----------------------+---------------------------------+
 *
 * Invariants / key ideas:
 *   - size_ always equals the number of distinct nodes in the ring.
 *   - Empty list  <=>  head_ == nullptr == tail_  <=>  size_ == 0.
 *   - Non-empty list: `tail_->next == head_` ALWAYS holds (this is the whole
 *     point of the structure and must be re-established after every mutation).
 *   - Single-element list: head_ == tail_ and that node points to itself.
 *   - There is NO nullptr terminator inside a non-empty ring, so traversal must
 *     count size_ steps rather than watch for `next == nullptr`.
 *
 * When to use / trade-offs:
 *   - Round-robin scheduling: rotate(1) hands the "turn" to the next player in
 *     O(1) amortized while keeping everyone in the ring.
 *   - O(1) at both ends with only one pointer per node (cheaper than a doubly
 *     linked list) as long as you never need O(1) pop_back.
 *   - Pitfall-heavy: a naive walk loops forever and a naive delete/destroy
 *     either loops forever or leaks; the cycle must be broken deliberately.
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

template <typename T>
class CircularLinkedList {
    // A node owns nothing but its value; the list owns the ring of nodes.
    struct Node {
        T value;
        Node* next;
        explicit Node(const T& v, Node* n = nullptr) : value(v), next(n) {}
        explicit Node(T&& v, Node* n = nullptr) : value(std::move(v)), next(n) {}
    };

    Node* tail_ = nullptr;    // last node; tail_->next == head_ (the ring closes here)
    std::size_t size_ = 0;    // number of nodes; the ONLY safe traversal bound

    // Convenience: the head is always "the node after the tail". Keeping only a
    // tail_ pointer (instead of head_ + tail_) is enough because the ring lets
    // us reach the head in O(1); we still expose head() for readability.
    Node* head() const { return tail_ ? tail_->next : nullptr; }

public:
    CircularLinkedList() = default;

    // ---- Rule of Five ---------------------------------------------------
    // This class holds RAW OWNING pointers to a ring of heap nodes, so the
    // compiler-generated special members would shallow-copy the tail_ pointer
    // (double free) and never free the ring (leak). We must supply all five.

    // Destructor: free every node. clear() breaks the cycle first so the
    // iterative free terminates instead of chasing `next` around forever.
    ~CircularLinkedList() { clear(); }

    // Copy constructor: DEEP copy so the two lists own independent rings.
    // We rebuild by walking the source exactly size_ times and push_back'ing.
    CircularLinkedList(const CircularLinkedList& other) {
        Node* cur = other.head();
        for (std::size_t i = 0; i < other.size_; ++i) {
            push_back(cur->value);
            cur = cur->next;
        }
    }

    // Copy assignment: copy-and-swap gives strong exception safety and is
    // self-assignment safe (the temporary copy is made before we touch *this).
    CircularLinkedList& operator=(const CircularLinkedList& other) {
        CircularLinkedList tmp(other);  // may throw; *this untouched so far
        swap(tmp);                      // noexcept pointer swaps
        return *this;                   // tmp's destructor frees our old ring
    }

    // Move constructor: STEAL the source's ring and leave the source a valid
    // EMPTY list (head_ == tail_ == nullptr, size_ == 0). noexcept so that
    // containers like std::vector can move instead of copy on reallocation.
    CircularLinkedList(CircularLinkedList&& other) noexcept
        : tail_(other.tail_), size_(other.size_) {
        other.tail_ = nullptr;
        other.size_ = 0;
    }

    // Move assignment: free our own ring, then steal. Self-assignment safe via
    // the explicit guard (moving a list onto itself must not wipe it). noexcept.
    CircularLinkedList& operator=(CircularLinkedList&& other) noexcept {
        if (this != &other) {
            clear();                    // release our current ring first
            tail_ = other.tail_;
            size_ = other.size_;
            other.tail_ = nullptr;      // leave source empty & valid
            other.size_ = 0;
        }
        return *this;
    }

    // noexcept swap used by copy-assignment; just exchanges the two scalars.
    void swap(CircularLinkedList& other) noexcept {
        std::swap(tail_, other.tail_);
        std::swap(size_, other.size_);
    }

    // ---- Capacity -------------------------------------------------------
    std::size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }

    // ---- Element access -------------------------------------------------
    // Precondition: list is non-empty (asserted). Return by reference so the
    // caller can read or mutate the stored element in place.
    T& front() { assert(!empty()); return head()->value; }
    const T& front() const { assert(!empty()); return head()->value; }
    T& back() { assert(!empty()); return tail_->value; }
    const T& back() const { assert(!empty()); return tail_->value; }

    // ---- Mutators -------------------------------------------------------

    // push_front: new node becomes the head. We insert it AFTER the tail (so
    // the ring stays closed) and simply do NOT move tail_, which means the new
    // node is now the head. O(1) because tail_ already points at the head.
    void push_front(const T& value) { insertAfterTail(new Node(value)); }
    void push_front(T&& value) { insertAfterTail(new Node(std::move(value))); }

    // push_back: identical wiring to push_front, but we additionally advance
    // tail_ onto the new node, so the new node is the last element. O(1).
    void push_back(const T& value) {
        Node* n = new Node(value);
        insertAfterTail(n);
        tail_ = n;
    }
    void push_back(T&& value) {
        Node* n = new Node(std::move(value));
        insertAfterTail(n);
        tail_ = n;
    }

    // pop_front: remove and return the head. CRITICAL last-element case: when
    // size_ hits 0 we must reset head_ == tail_ == nullptr, otherwise a stale
    // tail_ would still "close" a freed node and every invariant would break.
    T pop_front() {
        assert(!empty());
        Node* h = head();
        T value = std::move(h->value);
        if (size_ == 1) {
            tail_ = nullptr;            // list becomes empty; no ring remains
        } else {
            tail_->next = h->next;      // re-close the ring past the old head
        }
        delete h;
        --size_;
        return value;                   // tail_->next == head_ still holds
    }

    // insert(pos, value): insert so the new element ends up at logical index
    // `pos` (0 == front, size_ == back). pos is clamped to [0, size_]. Walking
    // to the predecessor is O(pos); the wiring itself is O(1).
    void insert(std::size_t pos, const T& value) {
        if (pos == 0) { push_front(value); return; }
        if (pos >= size_) { push_back(value); return; }
        // Walk to the node currently at index (pos-1); splice after it.
        Node* prev = head();
        for (std::size_t i = 0; i + 1 < pos; ++i) prev = prev->next;
        prev->next = new Node(value, prev->next);
        ++size_;                        // interior insert: head_/tail_ unchanged
    }

    // erase(value): remove the FIRST node equal to `value`. Returns true if a
    // node was removed. We track a predecessor so we can relink around the
    // victim in O(1); finding it is O(n). Handles erasing the head and the tail
    // (both must keep tail_->next == head_) and the sole element (-> empty).
    bool erase(const T& value) {
        if (empty()) return false;
        Node* prev = tail_;             // predecessor of head() is the tail
        Node* cur = head();
        for (std::size_t i = 0; i < size_; ++i) {
            if (cur->value == value) {
                if (size_ == 1) {
                    tail_ = nullptr;    // erasing the only node -> empty list
                } else {
                    prev->next = cur->next;         // unlink cur from the ring
                    if (cur == tail_) tail_ = prev; // erased tail -> prev is new tail
                }
                delete cur;
                --size_;
                return true;
            }
            prev = cur;
            cur = cur->next;
        }
        return false;                   // value not present
    }

    // rotate(k): advance the LOGICAL head forward by k positions. Because the
    // list is a ring, "rotating" is just moving tail_ forward k steps -- no
    // node is copied or reallocated, which is the round-robin superpower.
    // Negative-free API: k is reduced mod size_ so large k is cheap-ish.
    void rotate(std::size_t k) {
        if (size_ <= 1) return;         // 0/1 elements: rotation is a no-op
        k %= size_;                     // full turns cancel out
        for (std::size_t i = 0; i < k; ++i) tail_ = tail_->next;
        // Moving tail_ forward by one makes the old head the new tail and the
        // old head->next the new head; the ring itself is never broken.
    }

    // toVector: snapshot the ring in logical order. MUST loop exactly size_
    // times -- a "walk until next == head()" style loop is fine too, but the
    // count is the robust bound and the reason a naive `next == nullptr` test
    // would spin forever on a non-empty ring.
    std::vector<T> toVector() const {
        std::vector<T> out;
        out.reserve(size_);
        Node* cur = head();
        for (std::size_t i = 0; i < size_; ++i) {
            out.push_back(cur->value);
            cur = cur->next;
        }
        return out;
    }

    // clear: free exactly size_ nodes iteratively. We deliberately do NOT rely
    // on reaching a nullptr (there is none in a ring); we count down instead.
    // Equivalent alternative: set tail_->next = nullptr to "cut" the ring and
    // then free until nullptr -- either way the cycle must be broken.
    void clear() noexcept {
        Node* cur = head();
        for (std::size_t i = 0; i < size_; ++i) {
            Node* nxt = cur->next;
            delete cur;
            cur = nxt;
        }
        tail_ = nullptr;
        size_ = 0;
    }

private:
    // Shared wiring for both pushes: link `n` in right after the tail so the
    // ring stays closed, creating the ring itself when the list was empty.
    void insertAfterTail(Node* n) {
        if (tail_ == nullptr) {
            n->next = n;                // single element points to itself
            tail_ = n;                  // head_ == tail_ == n
        } else {
            n->next = tail_->next;      // n->next = old head
            tail_->next = n;            // tail now precedes n (n is new head)
        }
        ++size_;
    }
};

// ---------------------------------------------------------------------------
// Tests + demo
// ---------------------------------------------------------------------------
int main() {
    // ---- Empty-list basics ------------------------------------------------
    {
        CircularLinkedList<int> l;
        assert(l.empty());
        assert(l.size() == 0);
        assert(l.toVector().empty());
    }

    // ---- push/pop at both ends -------------------------------------------
    {
        CircularLinkedList<int> l;
        l.push_back(2);        // [2]
        l.push_front(1);       // [1, 2]
        l.push_back(3);        // [1, 2, 3]
        assert((l.toVector() == std::vector<int>{1, 2, 3}));
        assert(l.front() == 1);
        assert(l.back() == 3);
        assert(l.size() == 3);

        assert(l.pop_front() == 1);                 // [2, 3]
        assert((l.toVector() == std::vector<int>{2, 3}));
        assert(l.front() == 2 && l.back() == 3);
    }

    // ---- Single-element list points to itself ----------------------------
    {
        CircularLinkedList<int> l;
        l.push_back(42);
        assert(l.size() == 1);
        assert(l.front() == 42 && l.back() == 42);  // head_ == tail_
        // Walk one full loop of length size_ and confirm we return home.
        auto v = l.toVector();
        assert((v == std::vector<int>{42}));
    }

    // ---- pop of the last remaining node empties the list correctly -------
    {
        CircularLinkedList<int> l;
        l.push_back(7);
        assert(l.pop_front() == 7);
        assert(l.empty() && l.size() == 0);
        // Reusing after emptying must still work (invariants were reset).
        l.push_front(9);
        l.push_back(10);
        assert((l.toVector() == std::vector<int>{9, 10}));
    }

    // ---- insert(pos, value) at front / middle / back / clamped -----------
    {
        CircularLinkedList<int> l;
        for (int x : {10, 20, 30}) l.push_back(x);  // [10, 20, 30]
        l.insert(0, 5);                             // [5, 10, 20, 30]
        l.insert(2, 15);                            // [5, 10, 15, 20, 30]
        l.insert(100, 99);                          // clamp -> push_back
        assert((l.toVector() == std::vector<int>{5, 10, 15, 20, 30, 99}));
        assert(l.front() == 5 && l.back() == 99);
    }

    // ---- erase: head, middle, tail, absent, and sole element -------------
    {
        CircularLinkedList<int> l;
        for (int x : {1, 2, 3, 4}) l.push_back(x);  // [1, 2, 3, 4]
        assert(l.erase(1));                         // erase head -> [2, 3, 4]
        assert((l.toVector() == std::vector<int>{2, 3, 4}));
        assert(l.front() == 2);
        assert(l.erase(3));                         // erase middle -> [2, 4]
        assert((l.toVector() == std::vector<int>{2, 4}));
        assert(l.erase(4));                         // erase tail -> [2]
        assert((l.toVector() == std::vector<int>{2}));
        assert(l.back() == 2 && l.front() == 2);    // tail_ correctly moved
        assert(!l.erase(999));                      // absent value
        assert(l.erase(2));                         // erase sole element
        assert(l.empty());
    }

    // ---- rotate changes the front and preserves the element multiset -----
    {
        CircularLinkedList<int> l;
        for (int x : {1, 2, 3, 4, 5}) l.push_back(x);   // [1,2,3,4,5]
        l.rotate(2);                                    // head advances by 2
        assert((l.toVector() == std::vector<int>{3, 4, 5, 1, 2}));
        assert(l.front() == 3 && l.back() == 2);
        l.rotate(5);                                    // full turn -> no change
        assert((l.toVector() == std::vector<int>{3, 4, 5, 1, 2}));
        l.rotate(8);                                    // 8 % 5 == 3
        assert((l.toVector() == std::vector<int>{1, 2, 3, 4, 5}));
        // Multiset check: sum is invariant under rotation.
        int sum = 0;
        for (int x : l.toVector()) sum += x;
        assert(sum == 1 + 2 + 3 + 4 + 5);
    }

    // ---- Copy independence: mutating the original must not touch the copy -
    {
        CircularLinkedList<int> a;
        for (int x : {1, 2, 3}) a.push_back(x);
        CircularLinkedList<int> b = a;              // deep copy
        a.push_back(4);
        a.pop_front();                              // a is now [2, 3, 4]
        assert((a.toVector() == std::vector<int>{2, 3, 4}));
        assert((b.toVector() == std::vector<int>{1, 2, 3}));  // unaffected

        CircularLinkedList<int> c;
        c.push_back(100);
        c = a;                                      // copy assignment
        assert((c.toVector() == std::vector<int>{2, 3, 4}));
        c = c;                                       // self-assignment safe
        assert((c.toVector() == std::vector<int>{2, 3, 4}));
    }

    // ---- Move leaves the source empty ------------------------------------
    {
        CircularLinkedList<int> a;
        for (int x : {1, 2, 3}) a.push_back(x);
        CircularLinkedList<int> b = std::move(a);   // steal a's ring
        assert((b.toVector() == std::vector<int>{1, 2, 3}));
        assert(a.empty() && a.size() == 0);         // source empty & valid
        a.push_back(7);                             // still usable
        assert((a.toVector() == std::vector<int>{7}));

        CircularLinkedList<int> c;
        c.push_back(8);
        c = std::move(b);                           // move assignment
        assert((c.toVector() == std::vector<int>{1, 2, 3}));
        assert(b.empty());
    }

    // ---- Works with a non-trivial element type ---------------------------
    {
        CircularLinkedList<std::string> l;
        l.push_back("beta");
        l.push_front("alpha");
        l.push_back("gamma");
        assert((l.toVector() ==
                std::vector<std::string>{"alpha", "beta", "gamma"}));
        l.rotate(1);
        assert(l.front() == "beta");
    }

    // ---- Human-readable demo ---------------------------------------------
    std::cout << "Circular Linked List demo\n";
    std::cout << "-------------------------\n";
    CircularLinkedList<std::string> players;
    for (const std::string& p : {"Ann", "Bob", "Cid", "Dan"})
        players.push_back(p);

    std::cout << "Players in the ring: ";
    for (const std::string& p : players.toVector()) std::cout << p << ' ';
    std::cout << "\n\nRound-robin turns (rotate(1) each round):\n";
    for (int round = 1; round <= 4; ++round) {
        std::cout << "  Round " << round << ": " << players.front()
                  << " takes the turn\n";
        players.rotate(1);              // O(1): hand the turn to the next player
    }
    std::cout << "\nAfter 4 rotations the order is restored: ";
    for (const std::string& p : players.toVector()) std::cout << p << ' ';
    std::cout << '\n';

    std::cout << "\nAll assertions passed.\n";
    return 0;
}
