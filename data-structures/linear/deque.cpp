/* ============================================================================
 * Deque<T>  --  Data Structure (Linear)
 *
 * Summary:
 *   Double-ended queue over a raw buffer used as a CIRCULAR (ring) buffer.
 *   Push/pop at BOTH ends in O(1) amortized, plus random access by logical
 *   index. Grows geometrically and re-linearizes when it runs out of room.
 *
 * Operations & complexity:
 *   +------------+----------------+
 *   | Operation  | Time           |
 *   +------------+----------------+
 *   | push_front | O(1) amortized |
 *   | push_back  | O(1) amortized |
 *   | pop_front  | O(1)           |
 *   | pop_back   | O(1)           |
 *   | operator[] | O(1)           |
 *   | front/back | O(1)           |
 *   | size/empty | O(1)           |
 *   | clear      | O(n)           |
 *   +------------+----------------+
 *
 * Invariants:
 *   - head_ in [0, capacity_)   : physical index of the logical front.
 *   - count_ in [0, capacity_]  : number of live elements.
 *   - Logical index i (0-based from the front) maps to physical slot
 *     (head_ + i) % capacity_; only those count_ slots hold live objects.
 *   - capacity_ == 0  iff  buf_ == nullptr  (empty, never-grown deque).
 *
 * When to use / trade-offs:
 *   - Use when you need efficient insert/remove at BOTH ends (sliding
 *     windows, work-stealing, undo/redo, monotonic-deque algorithms).
 *   - Random access is O(1) like a vector, but growth copies all elements.
 *   - A ring keeps both ends cheap; a plain array makes one end O(n).
 * ========================================================================== */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

template <typename T>
class Deque {
    // ::operator new + placement new give us RAW storage so we construct only
    // the slots in use (also supports non-default-constructible T).
    T*          buf_;       // owning raw storage (may be nullptr)
    std::size_t capacity_;  // number of slots in buf_
    std::size_t head_;      // physical index of the logical front
    std::size_t count_;     // number of live elements

    static constexpr std::size_t kInit = 4;  // first non-empty capacity

    static T*   allocate(std::size_t n) { return n ? static_cast<T*>(::operator new(n * sizeof(T))) : nullptr; }
    static void deallocate(T* p) noexcept { ::operator delete(p); }

    // Grow + RE-LINEARIZE: move live elements out in logical order so the front
    // lands at index 0. Needed because after wrap-around the elements straddle
    // the end of the ring and a flat copy would reorder them.
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

    Deque() noexcept : buf_(nullptr), capacity_(0), head_(0), count_(0) {}

    // Destructor: free everything. Elements destroyed in a LOOP (never deep
    // recursion) so an enormous deque cannot blow the call stack.
    ~Deque() { clear(); deallocate(buf_); }

    // Copy ctor: DEEP copy into our OWN buffer so the two deques share no
    // storage. Elements are re-linearized (copied front-first into [0..n)).
    Deque(const Deque& o)
        : buf_(allocate(o.capacity_)), capacity_(o.capacity_), head_(0), count_(o.count_) {
        for (std::size_t i = 0; i < count_; ++i)
            ::new (buf_ + i) T(o.buf_[(o.head_ + i) % o.capacity_]);
    }

    // Copy assignment: copy-and-swap. The temp does the deep copy; swap is
    // noexcept; inherently self-assignment safe and leak-free on throw.
    Deque& operator=(const Deque& o) {
        if (this != &o) { Deque tmp(o); swap(tmp); }
        return *this;
    }

    // Move ctor: O(1) steal of the buffer; leave source valid+empty.
    // noexcept lets generic code prefer moving over copying.
    Deque(Deque&& o) noexcept
        : buf_(o.buf_), capacity_(o.capacity_), head_(o.head_), count_(o.count_) {
        o.buf_ = nullptr; o.capacity_ = o.head_ = o.count_ = 0;
    }

    // Move assignment: free ours, then steal. Guarded against self-move so we
    // never free the buffer we are about to take. noexcept.
    Deque& operator=(Deque&& o) noexcept {
        if (this != &o) {
            clear(); deallocate(buf_);
            buf_ = o.buf_; capacity_ = o.capacity_; head_ = o.head_; count_ = o.count_;
            o.buf_ = nullptr; o.capacity_ = o.head_ = o.count_ = 0;
        }
        return *this;
    }

    void swap(Deque& o) noexcept {
        std::swap(buf_, o.buf_); std::swap(capacity_, o.capacity_);
        std::swap(head_, o.head_); std::swap(count_, o.count_);
    }

    bool        empty() const noexcept { return count_ == 0; }
    std::size_t size()  const noexcept { return count_; }
    void push_back(const T& v) { if (count_ == capacity_) grow(); ::new (buf_ + (head_ + count_) % capacity_) T(v); ++count_; }
    void push_back(T&& v)      { if (count_ == capacity_) grow(); ::new (buf_ + (head_ + count_) % capacity_) T(std::move(v)); ++count_; }

    // Decrement head_ modulo capacity_: add capacity_ BEFORE -1 so the unsigned
    // arithmetic never underflows and 0 wraps correctly to capacity_-1.
    void push_front(const T& v) { if (count_ == capacity_) grow(); head_ = (head_ + capacity_ - 1) % capacity_; ::new (buf_ + head_) T(v); ++count_; }
    void push_front(T&& v)      { if (count_ == capacity_) grow(); head_ = (head_ + capacity_ - 1) % capacity_; ::new (buf_ + head_) T(std::move(v)); ++count_; }

    void pop_front() {
        assert(count_ > 0 && "pop_front() on empty deque");
        buf_[head_].~T();
        head_ = (head_ + 1) % capacity_;   // advance front, wrapping around
        --count_;
    }
    void pop_back() {
        assert(count_ > 0 && "pop_back() on empty deque");
        buf_[(head_ + count_ - 1) % capacity_].~T();  // destroy last; head_ stays
        --count_;
    }

    void clear() noexcept {
        for (std::size_t i = 0; i < count_; ++i) buf_[(head_ + i) % capacity_].~T();
        count_ = 0; head_ = 0;
    }

    // Logical index i -> physical slot (head_ + i) % capacity_.
    T&       operator[](std::size_t i)       { assert(i < count_); return buf_[(head_ + i) % capacity_]; }
    const T& operator[](std::size_t i) const { assert(i < count_); return buf_[(head_ + i) % capacity_]; }

    T&       front()       { assert(count_ > 0); return buf_[head_]; }
    const T& front() const { assert(count_ > 0); return buf_[head_]; }
    T&       back()        { assert(count_ > 0); return buf_[(head_ + count_ - 1) % capacity_]; }
    const T& back()  const { assert(count_ > 0); return buf_[(head_ + count_ - 1) % capacity_]; }

    // Iterator over the logical order (front -> back) so range-based for works.
    template <bool C>
    class Iter {
        using DPtr = std::conditional_t<C, const Deque*, Deque*>;
        using Ref  = std::conditional_t<C, const T&, T&>;
        DPtr d_; std::size_t i_;  // i_ = logical offset from the front
    public:
        Iter(DPtr d, std::size_t i) : d_(d), i_(i) {}
        Ref   operator*() const { return d_->buf_[(d_->head_ + i_) % d_->capacity_]; }
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
    Deque<int> d;                                   // empty
    assert(d.empty() && d.size() == 0u);
    d.push_back(5);                                 // single via each end
    assert(d.size() == 1u && d.front() == 5 && d.back() == 5);
    d.pop_back(); assert(d.empty());
    d.push_front(9); assert(d.front() == 9 && d.back() == 9);
    d.pop_front(); assert(d.empty());
    d.push_back(4); d.push_back(5); d.push_back(6);  // build 1..6 from both ends
    d.push_front(3); d.push_front(2); d.push_front(1);
    assert(d.size() == 6u && d.front() == 1 && d.back() == 6);
    for (std::size_t i = 0; i < d.size(); ++i) assert(d[i] == static_cast<int>(i) + 1);
    d.pop_front(); d.pop_back();                     // pop toward empty
    assert(d.front() == 2 && d.back() == 5 && d.size() == 4u);
    Deque<int> g;                                    // growth (front) + relinearize
    for (int i = 0; i < 100; ++i) g.push_front(i);   // 99,98,...,0
    assert(g.size() == 100u && g.front() == 99 && g.back() == 0);
    for (int i = 0; i < 100; ++i) assert(g[static_cast<std::size_t>(i)] == 99 - i);
    while (!g.empty()) { g.pop_front(); if (!g.empty()) g.pop_back(); }
    assert(g.empty());                               // removed down to empty
    Deque<int> w;                                    // circular wrap-around
    for (int i = 0; i < 4; ++i) w.push_back(i);      // fill capacity 4: 0 1 2 3
    w.pop_front(); w.pop_front();                     // head_ advances to 2
    w.push_back(10);                                  // tail wraps to physical 0
    w.push_front(20);                                 // head_ wraps to the end
    int expect[] = {20, 2, 3, 10};
    std::size_t k = 0;
    for (int v : w) { assert(v == expect[k]); ++k; }  // range-based for
    assert(k == 4u && w[0] == 20 && w[3] == 10);
    Deque<int> c = w;                                // copy ctor: deep copy
    c.push_back(777); c.pop_front();
    assert(w.size() == 4u && w.front() == 20);        // original untouched
    Deque<int> c2; c2 = w;                           // copy assignment
    Deque<int>& alias = c2; c2 = alias;               // self-assignment safe
    c2.push_front(-1);
    assert(c2.size() == 5u && w.size() == 4u);
    Deque<int> m = std::move(c2);                    // move ctor
    assert(m.size() == 5u && c2.empty());
    Deque<int> m2; m2 = std::move(m);                // move assignment
    assert(m2.size() == 5u && m.empty());
    Deque<std::string> s;                            // generic over std::string
    s.push_back("mid"); s.push_front("start"); s.push_back("end");
    assert(s.front() == "start" && s.back() == "end" && s[1] == "mid");
    w.clear(); assert(w.empty());
    Deque<std::string> dq;                           // human-readable demo
    dq.push_back("B"); dq.push_front("A"); dq.push_back("C");
    std::cout << "Deque (front -> back): ";
    for (const auto& x : dq) std::cout << x << ' ';
    std::cout << "\nfront=" << dq.front() << " back=" << dq.back() << " size=" << dq.size() << '\n';
    dq.pop_front();
    std::cout << "After pop_front, front=" << dq.front() << '\n';
    std::cout << "All Deque tests passed.\n";
    return 0;
}
