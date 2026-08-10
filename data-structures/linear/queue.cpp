/* ============================================================================
 * Queue<T>  --  Data Structure (Linear)
 *
 * Summary:
 *   First-In-First-Out (FIFO) container over a raw buffer used as a CIRCULAR
 *   (ring) buffer, with geometric growth + re-linearization on overflow.
 *   enqueue at the back, dequeue from the front -- both O(1).
 *
 * Operations & complexity:
 *   +-----------+----------------+
 *   | Operation | Time           |
 *   +-----------+----------------+
 *   | enqueue   | O(1) amortized |  (O(n) only on the resize that grows)
 *   | dequeue   | O(1)           |
 *   | front     | O(1)           |
 *   | back      | O(1)           |
 *   | size      | O(1)           |
 *   | empty     | O(1)           |
 *   | clear     | O(n)           |  (destroys each live element)
 *   +-----------+----------------+
 *
 * Invariants:
 *   - head_ in [0, capacity_)   : physical index of the logical front.
 *   - count_ in [0, capacity_]  : number of live elements.
 *   - Live physical slots are exactly (head_ + i) % capacity_ for
 *     i in [0, count_); every other slot holds no constructed object.
 *   - capacity_ == 0  iff  buf_ == nullptr  (empty, never-grown queue).
 *
 * When to use / trade-offs:
 *   - Ideal for FIFO work queues / BFS frontiers: both ends are O(1).
 *   - The ring avoids the O(n) "shift everything left" cost of a naive array
 *     queue that pops from the front.
 *   - Contiguous & cache-friendly, but a grow copies all elements once.
 * ========================================================================== */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

template <typename T>
class Queue {
    // ::operator new + placement new give us RAW storage so we construct only
    // the slots actually in use (also supports non-default-constructible T).
    T*          buf_;       // owning raw storage (may be nullptr)
    std::size_t capacity_;  // number of slots in buf_
    std::size_t head_;      // physical index of the logical front
    std::size_t count_;     // number of live elements

    static constexpr std::size_t kInit = 4;  // first non-empty capacity

    static T*   allocate(std::size_t n) { return n ? static_cast<T*>(::operator new(n * sizeof(T))) : nullptr; }
    static void deallocate(T* p) noexcept { ::operator delete(p); }

    // Grow + RE-LINEARIZE: move live elements out in logical order so the front
    // lands at index 0. Needed because after wrap-around the elements straddle
    // the end of the ring and a flat copy would scramble their order.
    void grow() {
        std::size_t new_cap = capacity_ ? capacity_ * 2 : kInit;
        T* nb = allocate(new_cap);
        for (std::size_t i = 0; i < count_; ++i) {
            T* src = buf_ + (head_ + i) % capacity_;
            ::new (nb + i) T(std::move(*src));  // move into fresh storage
            src->~T();                          // destroy moved-from slot
        }
        deallocate(buf_);
        buf_ = nb; capacity_ = new_cap; head_ = 0;
    }

public:
    using value_type = T;

    Queue() noexcept : buf_(nullptr), capacity_(0), head_(0), count_(0) {}

    // Destructor: release everything. Elements destroyed in a LOOP (never deep
    // recursion) so a huge queue cannot overflow the stack.
    ~Queue() { clear(); deallocate(buf_); }

    // Copy ctor: DEEP copy -- each queue must own distinct storage, so we
    // allocate our own buffer and copy-construct every element re-linearized.
    Queue(const Queue& o)
        : buf_(allocate(o.capacity_)), capacity_(o.capacity_), head_(0), count_(o.count_) {
        for (std::size_t i = 0; i < count_; ++i)
            ::new (buf_ + i) T(o.buf_[(o.head_ + i) % o.capacity_]);
    }

    // Copy assignment: copy-and-swap. The temp does the deep copy; swap is
    // noexcept; this is inherently self-assignment safe and leak-free on throw.
    Queue& operator=(const Queue& o) {
        if (this != &o) { Queue tmp(o); swap(tmp); }
        return *this;
    }

    // Move ctor: O(1) steal of the buffer; leave source a valid empty queue.
    // noexcept so growth/relocations prefer moving over copying.
    Queue(Queue&& o) noexcept
        : buf_(o.buf_), capacity_(o.capacity_), head_(o.head_), count_(o.count_) {
        o.buf_ = nullptr; o.capacity_ = o.head_ = o.count_ = 0;
    }

    // Move assignment: free what we hold, then steal. Guarded against self-move
    // so we never free the buffer we are about to adopt. noexcept.
    Queue& operator=(Queue&& o) noexcept {
        if (this != &o) {
            clear(); deallocate(buf_);
            buf_ = o.buf_; capacity_ = o.capacity_; head_ = o.head_; count_ = o.count_;
            o.buf_ = nullptr; o.capacity_ = o.head_ = o.count_ = 0;
        }
        return *this;
    }

    void swap(Queue& o) noexcept {
        std::swap(buf_, o.buf_); std::swap(capacity_, o.capacity_);
        std::swap(head_, o.head_); std::swap(count_, o.count_);
    }

    bool        empty() const noexcept { return count_ == 0; }
    std::size_t size()  const noexcept { return count_; }

    void enqueue(const T& v) { if (count_ == capacity_) grow(); ::new (buf_ + (head_ + count_) % capacity_) T(v); ++count_; }
    void enqueue(T&& v)      { if (count_ == capacity_) grow(); ::new (buf_ + (head_ + count_) % capacity_) T(std::move(v)); ++count_; }

    void dequeue() {
        assert(count_ > 0 && "dequeue() on empty queue");
        buf_[head_].~T();                  // destroy the front element
        head_ = (head_ + 1) % capacity_;   // advance front, wrapping around
        --count_;
    }

    void clear() noexcept {
        for (std::size_t i = 0; i < count_; ++i) buf_[(head_ + i) % capacity_].~T();
        count_ = 0; head_ = 0;
    }

    T&       front()       { assert(count_ > 0); return buf_[head_]; }
    const T& front() const { assert(count_ > 0); return buf_[head_]; }
    T&       back()        { assert(count_ > 0); return buf_[(head_ + count_ - 1) % capacity_]; }
    const T& back()  const { assert(count_ > 0); return buf_[(head_ + count_ - 1) % capacity_]; }

    // Iterator over the logical order (front -> back) so range-based for works.
    template <bool C>
    class Iter {
        using QPtr = std::conditional_t<C, const Queue*, Queue*>;
        using Ref  = std::conditional_t<C, const T&, T&>;
        QPtr q_; std::size_t i_;  // i_ = logical offset from the front
    public:
        Iter(QPtr q, std::size_t i) : q_(q), i_(i) {}
        Ref   operator*() const { return q_->buf_[(q_->head_ + i_) % q_->capacity_]; }
        Iter& operator++()      { ++i_; return *this; }
        bool  operator==(const Iter& o) const { return i_ == o.i_; }
        bool  operator!=(const Iter& o) const { return i_ != o.i_; }
    };
    using iterator = Iter<false>;
    using const_iterator = Iter<true>;
    iterator       begin()       { return {this, 0}; }
    iterator       end()         { return {this, count_}; }
    const_iterator begin() const { return {this, 0}; }
    const_iterator end()   const { return {this, count_}; }
};

int main() {
    Queue<int> q;                                   // empty
    assert(q.empty() && q.size() == 0u);

    q.enqueue(10);                                  // single element
    assert(!q.empty() && q.size() == 1u && q.front() == 10 && q.back() == 10);
    q.dequeue();
    assert(q.empty());

    for (int i = 0; i < 100; ++i) q.enqueue(i);     // growth past capacity
    assert(q.size() == 100u && q.front() == 0 && q.back() == 99);
    for (int i = 0; i < 100; ++i) { assert(q.front() == i); q.dequeue(); }
    assert(q.empty());                              // removed down to empty

    Queue<int> r;                                   // circular wrap-around
    for (int i = 0; i < 4; ++i) r.enqueue(i);       // fill capacity 4
    r.dequeue(); r.dequeue();                        // advance head_ to 2
    r.enqueue(100); r.enqueue(101);                  // tail wraps past the end
    int expect[] = {2, 3, 100, 101};
    std::size_t k = 0;
    for (int v : r) { assert(v == expect[k]); ++k; } // range-based for
    assert(k == 4u);

    Queue<int> c = r;                               // copy ctor: deep copy
    c.dequeue(); c.enqueue(999);                     // mutate the copy
    assert(r.front() == 2 && r.size() == 4u);        // original untouched

    Queue<int> c2; c2 = r;                          // copy assignment
    c2.enqueue(7);
    Queue<int>& alias = c2; c2 = alias;              // self-assignment safe
    assert(c2.size() == 5u && r.size() == 4u);

    Queue<int> m = std::move(c2);                   // move ctor
    assert(m.size() == 5u && c2.empty());
    Queue<int> m2; m2 = std::move(m);               // move assignment
    assert(m2.size() == 5u && m.empty());

    Queue<std::string> s;                           // generic over std::string
    s.enqueue("alpha"); s.enqueue("beta");
    assert(s.front() == "alpha" && s.back() == "beta");
    s.dequeue();
    assert(s.front() == "beta");

    r.clear();
    assert(r.empty());

    Queue<std::string> line;                        // human-readable demo
    line.enqueue("first"); line.enqueue("second"); line.enqueue("third");
    std::cout << "Queue (front -> back): ";
    for (const auto& name : line) std::cout << name << ' ';
    std::cout << "\nServing: " << line.front() << '\n';
    line.dequeue();
    std::cout << "New front: " << line.front() << ", size: " << line.size() << '\n';
    std::cout << "All Queue tests passed.\n";
    return 0;
}
