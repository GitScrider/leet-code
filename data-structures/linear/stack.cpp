/*
 * Stack<T>  -  Data Structure - Linear
 *
 * Summary:
 *   A LIFO (Last-In, First-Out) container built directly on an owned, growable
 *   raw buffer (NOT a wrapper over std::vector). The most recently pushed item
 *   is always the first one popped.
 *
 * Operations & complexity:
 *   +-----------+------------------+
 *   | Operation | Time             |
 *   +-----------+------------------+
 *   | push      | O(1) amortized   |
 *   | pop       | O(1)             |
 *   | top       | O(1)             |
 *   | empty     | O(1)             |
 *   | size      | O(1)             |
 *   | clear     | O(n)             |
 *   +-----------+------------------+
 *
 * LIFO discipline:
 *   All growth and access happen at one end ("the top" == index size_-1). push
 *   appends there, pop removes there, top inspects it. Because we only ever add
 *   or remove at the end, no elements shift and every operation is O(1) (push is
 *   amortized O(1) due to geometric doubling: total copy work over n pushes is
 *   1+2+4+...+n < 2n, i.e. O(1) each on average).
 *
 * Invariants:
 *   - 0 <= size_ <= capacity_.
 *   - buf_ addresses raw storage for capacity_ objects; exactly the first size_
 *     slots are alive (constructed), the rest are uninitialized.
 *   - buf_ == nullptr  <=>  capacity_ == 0.
 *
 * When to use / trade-offs:
 *   - Ideal for recursion-to-iteration, undo stacks, expression evaluation,
 *     DFS, and balanced-bracket checks.
 *   - No random access and no iteration by design: only the top is reachable.
 *   - top()/pop() on an EMPTY stack is undefined behavior (reads/destroys a
 *     nonexistent slot) unless the caller first guards with empty(); we keep it
 *     unchecked to mirror std::stack and avoid paying for a check on the hot path.
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <new>       // placement new operator
#include <string>
#include <utility>

template <typename T>
class Stack {
    T* buf_;              // raw owning buffer (may contain uninitialized slots)
    std::size_t size_;    // number of live elements; top is buf_[size_-1]
    std::size_t capacity_;// slots available in buf_

    static T* allocate(std::size_t n) {
        return n == 0 ? nullptr
                      : static_cast<T*>(::operator new(n * sizeof(T)));
    }

    // Relocate live elements into a bigger raw buffer via placement new so that
    // spare capacity is never default-constructed. Moved-from originals are
    // destroyed, then the old raw storage is freed.
    void grow(std::size_t new_cap) {
        T* new_buf = allocate(new_cap);
        for (std::size_t i = 0; i < size_; ++i) {
            ::new (static_cast<void*>(new_buf + i)) T(std::move(buf_[i]));
            buf_[i].~T();
        }
        ::operator delete(buf_);
        buf_ = new_buf;
        capacity_ = new_cap;
    }

    void destroy_elements() noexcept {
        for (std::size_t i = 0; i < size_; ++i) buf_[i].~T();
        size_ = 0;
    }

public:
    // Default: empty stack; no allocation until the first push.
    Stack() noexcept : buf_(nullptr), size_(0), capacity_(0) {}

    // Destructor: destroy each live element (iteratively, no recursion), then
    // release the raw buffer. Required because we own heap storage.
    ~Stack() {
        destroy_elements();
        ::operator delete(buf_);
    }

    // Copy constructor: deep copy. The compiler default would duplicate the raw
    // pointer, producing two owners of one buffer (double free / aliasing bug).
    Stack(const Stack& other)
        : buf_(allocate(other.size_)), size_(0), capacity_(other.size_) {
        for (std::size_t i = 0; i < other.size_; ++i) {
            ::new (static_cast<void*>(buf_ + i)) T(other.buf_[i]);
            ++size_; // grow incrementally so a throwing copy stays destructible
        }
    }

    // Copy assignment: release ours, deep-copy theirs. Self-assignment guard
    // avoids wiping our own data before we read it.
    Stack& operator=(const Stack& other) {
        if (this != &other) {
            destroy_elements();
            ::operator delete(buf_);
            buf_ = allocate(other.size_);
            capacity_ = other.size_;
            for (std::size_t i = 0; i < other.size_; ++i) {
                ::new (static_cast<void*>(buf_ + i)) T(other.buf_[i]);
                ++size_;
            }
        }
        return *this;
    }

    // Move constructor: steal the buffer, leave source empty-but-valid. noexcept
    // so container relocations prefer moving over copying.
    Stack(Stack&& other) noexcept
        : buf_(other.buf_), size_(other.size_), capacity_(other.capacity_) {
        other.buf_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // Move assignment: free ours, steal theirs, blank the source. Self-safe and
    // noexcept for the same relocation reason.
    Stack& operator=(Stack&& other) noexcept {
        if (this != &other) {
            destroy_elements();
            ::operator delete(buf_);
            buf_ = other.buf_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.buf_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    // --- observers ---
    bool empty() const noexcept { return size_ == 0; }
    std::size_t size() const noexcept { return size_; }

    // Inspect the top without removing it. UB if empty (see class note); guard
    // with empty() when the stack may be empty.
    T& top() noexcept { return buf_[size_ - 1]; }
    const T& top() const noexcept { return buf_[size_ - 1]; }

    // --- modifiers ---
    void push(const T& value) {
        if (size_ == capacity_) grow(capacity_ == 0 ? 1 : capacity_ * 2);
        ::new (static_cast<void*>(buf_ + size_)) T(value);
        ++size_;
    }
    void push(T&& value) { // rvalue overload moves instead of copying
        if (size_ == capacity_) grow(capacity_ == 0 ? 1 : capacity_ * 2);
        ::new (static_cast<void*>(buf_ + size_)) T(std::move(value));
        ++size_;
    }

    // Remove the top element. Precondition: not empty (UB otherwise). Capacity
    // is retained so subsequent pushes need no reallocation.
    void pop() noexcept {
        buf_[size_ - 1].~T();
        --size_;
    }

    // Destroy all elements but keep the allocated capacity for reuse.
    void clear() noexcept { destroy_elements(); }
};

int main() {
    // --- empty stack ---
    Stack<int> s;
    assert(s.empty());
    assert(s.size() == 0);

    // --- single element ---
    s.push(42);
    assert(!s.empty());
    assert(s.size() == 1);
    assert(s.top() == 42);

    // top() is a reference: mutate in place
    s.top() = 43;
    assert(s.top() == 43);
    s.pop();
    assert(s.empty());

    // --- growth past initial capacity + LIFO order on drain ---
    const int N = 50;
    for (int i = 0; i < N; ++i) s.push(i);   // doubles 1,2,4,...,64
    assert(s.size() == static_cast<std::size_t>(N));
    assert(s.top() == N - 1);

    // --- removing down to empty verifies Last-In-First-Out ordering ---
    for (int i = N - 1; i >= 0; --i) {
        assert(s.top() == i);
        s.pop();
    }
    assert(s.empty());

    // clear() on a repopulated stack
    for (int i = 0; i < 10; ++i) s.push(i * i);
    s.clear();
    assert(s.empty() && s.size() == 0);
    s.push(7);                                // reuse retained capacity
    assert(s.top() == 7);

    // --- copy semantics: copy then mutate original, verify independence ---
    Stack<int> a;
    for (int i = 1; i <= 3; ++i) a.push(i);   // top == 3
    Stack<int> copy = a;                      // copy constructor
    a.push(99);
    a.top() = 1000;
    assert(copy.size() == 3 && copy.top() == 3);   // copy unaffected
    assert(a.top() == 1000 && a.size() == 4);

    Stack<int> assigned;
    assigned.push(-1);
    assigned = copy;                          // copy assignment
    assigned.pop();
    assert(assigned.size() == 2 && copy.size() == 3); // original intact

    // --- move semantics: source left empty, target owns the data ---
    Stack<int> moved = std::move(a);          // move constructor
    assert(moved.size() == 4 && moved.top() == 1000);
    assert(a.empty() && a.size() == 0);
    Stack<int> move_assigned;
    move_assigned = std::move(moved);         // move assignment
    assert(move_assigned.size() == 4 && moved.empty());

    // --- works with a non-trivial type (std::string) ---
    Stack<std::string> words;
    words.push("bottom");
    words.push(std::string("top"));
    assert(words.size() == 2 && words.top() == "top");

    // --- human-readable demo ---
    std::cout << "Stack demo\n";
    std::cout << "  pushing 3,1,4,1,5 then popping (LIFO): ";
    Stack<int> demo;
    demo.push(3); demo.push(1); demo.push(4); demo.push(1); demo.push(5);
    while (!demo.empty()) { std::cout << demo.top() << ' '; demo.pop(); }
    std::cout << "\n  string stack top: " << words.top() << "\n";
    std::cout << "All Stack assertions passed.\n";
    return 0;
}
