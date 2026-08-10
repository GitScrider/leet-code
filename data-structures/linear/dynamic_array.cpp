/*
 * DynamicArray<T>  -  Data Structure - Linear
 *
 * Summary:
 *   A resizable, contiguous array (a simplified std::vector). It owns a raw
 *   buffer and grows geometrically so that appends are cheap on average.
 *
 * Operations & complexity:
 *   +-----------------------+------------------------+
 *   | Operation             | Time                   |
 *   +-----------------------+------------------------+
 *   | push_back             | O(1) amortized         |
 *   | pop_back              | O(1)                   |
 *   | operator[] / at       | O(1)                   |
 *   | front / back          | O(1)                   |
 *   | reserve               | O(n) (copy on grow)    |
 *   | insert(index, value)  | O(n) (shift right)     |
 *   | erase(index)          | O(n) (shift left)      |
 *   | clear                 | O(n) (destroy each)    |
 *   | size/capacity/empty   | O(1)                   |
 *   +-----------------------+------------------------+
 *
 * Amortized analysis of push_back:
 *   Doubling capacity means that inserting n elements triggers reallocations
 *   at sizes 1, 2, 4, ..., n. The total element-move work is 1+2+4+...+n < 2n,
 *   i.e. O(n) spread over n pushes => O(1) amortized per push_back. Growing by
 *   a constant instead would give O(n) copies per resize and O(n^2) total.
 *
 * Invariants:
 *   - 0 <= size_ <= capacity_.
 *   - data_ points to raw storage for capacity_ objects of T, of which exactly
 *     the first size_ are alive (constructed); the rest are uninitialized.
 *   - data_ == nullptr  <=>  capacity_ == 0.
 *
 * When to use / trade-offs:
 *   - Best default container: cache-friendly, O(1) random access and append.
 *   - Insert/erase in the middle is O(n) because elements must shift.
 *   - Growth may over-allocate; reserve() up front avoids repeated copies.
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <new>       // placement new operator
#include <stdexcept>
#include <string>
#include <utility>

template <typename T>
class DynamicArray {
    T* data_;             // raw owning buffer (may hold uninitialized slots)
    std::size_t size_;    // number of live (constructed) elements
    std::size_t capacity_;// number of slots the buffer can hold

    // Allocate raw, uninitialized storage for n objects (no T constructors run).
    static T* allocate(std::size_t n) {
        return n == 0 ? nullptr
                      : static_cast<T*>(::operator new(n * sizeof(T)));
    }

    // Move all live elements into a fresh buffer of new_cap slots. We separate
    // allocation from construction (placement new) so unused capacity stays raw.
    void reallocate(std::size_t new_cap) {
        T* new_data = allocate(new_cap);
        for (std::size_t i = 0; i < size_; ++i) {
            ::new (static_cast<void*>(new_data + i)) T(std::move(data_[i]));
            data_[i].~T();                 // destroy the moved-from original
        }
        ::operator delete(data_);          // free old raw storage (no dtors here)
        data_ = new_data;
        capacity_ = new_cap;
    }

    // Destroy every live element but keep the buffer (used by clear/assign/dtor).
    void destroy_elements() noexcept {
        for (std::size_t i = 0; i < size_; ++i) data_[i].~T();
        size_ = 0;
    }

public:
    using iterator = T*;             // raw-pointer iterators: contiguous storage
    using const_iterator = const T*; // makes range-based for "just work"

    // Default: empty array, no allocation until first push.
    DynamicArray() noexcept : data_(nullptr), size_(0), capacity_(0) {}

    // Destructor: destroy live elements, then release the raw buffer. The loop
    // is iterative (no recursion), so it is safe for any size_.
    ~DynamicArray() {
        destroy_elements();
        ::operator delete(data_);
    }

    // Copy constructor: deep copy. Needed because the default would copy the raw
    // pointer, giving two owners of one buffer (double free + aliasing).
    DynamicArray(const DynamicArray& other)
        : data_(allocate(other.size_)), size_(0), capacity_(other.size_) {
        for (std::size_t i = 0; i < other.size_; ++i) {
            ::new (static_cast<void*>(data_ + i)) T(other.data_[i]);
            ++size_; // increment as we go so a throwing copy leaves us destructible
        }
    }

    // Copy assignment: release ours, then deep-copy theirs. Self-assignment guard
    // prevents destroying our own data before reading from it.
    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) {
            destroy_elements();
            ::operator delete(data_);
            data_ = allocate(other.size_);
            capacity_ = other.size_;
            for (std::size_t i = 0; i < other.size_; ++i) {
                ::new (static_cast<void*>(data_ + i)) T(other.data_[i]);
                ++size_;
            }
        }
        return *this;
    }

    // Move constructor: steal the buffer and leave the source empty but valid.
    // noexcept lets containers relocate us without falling back to copies.
    DynamicArray(DynamicArray&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // Move assignment: free ours, steal theirs, blank the source. Self-safe and
    // noexcept for the same relocation reason.
    DynamicArray& operator=(DynamicArray&& other) noexcept {
        if (this != &other) {
            destroy_elements();
            ::operator delete(data_);
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    // --- capacity ---------------------------------------------------------
    std::size_t size() const noexcept { return size_; }
    std::size_t capacity() const noexcept { return capacity_; }
    bool empty() const noexcept { return size_ == 0; }

    // Grow the buffer to at least new_cap. Never shrinks (a no-op if smaller).
    void reserve(std::size_t new_cap) {
        if (new_cap > capacity_) reallocate(new_cap);
    }

    // --- element access (unchecked) --------------------------------------
    T& operator[](std::size_t i) noexcept { return data_[i]; }
    const T& operator[](std::size_t i) const noexcept { return data_[i]; }

    // Bounds-checked access; throws instead of invoking undefined behavior.
    T& at(std::size_t i) {
        if (i >= size_) throw std::out_of_range("DynamicArray::at index out of range");
        return data_[i];
    }
    const T& at(std::size_t i) const {
        if (i >= size_) throw std::out_of_range("DynamicArray::at index out of range");
        return data_[i];
    }

    T& front() noexcept { return data_[0]; }          // UB if empty (like std::vector)
    const T& front() const noexcept { return data_[0]; }
    T& back() noexcept { return data_[size_ - 1]; }
    const T& back() const noexcept { return data_[size_ - 1]; }

    // --- modifiers --------------------------------------------------------
    void push_back(const T& value) {
        if (size_ == capacity_) reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        ::new (static_cast<void*>(data_ + size_)) T(value);
        ++size_;
    }
    void push_back(T&& value) { // rvalue overload avoids a needless copy
        if (size_ == capacity_) reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        ::new (static_cast<void*>(data_ + size_)) T(std::move(value));
        ++size_;
    }

    // Precondition: not empty. Destroy the last element; capacity is unchanged.
    void pop_back() noexcept {
        data_[size_ - 1].~T();
        --size_;
    }

    // Insert before position index (index == size_ means append). O(n) shift.
    void insert(std::size_t index, const T& value) {
        if (index > size_) throw std::out_of_range("DynamicArray::insert index out of range");
        if (size_ == capacity_) reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        if (index == size_) {
            ::new (static_cast<void*>(data_ + size_)) T(value);
        } else {
            // The last slot is raw: move-construct into it first, then move-assign
            // the already-live slots backwards to open a hole at index.
            ::new (static_cast<void*>(data_ + size_)) T(std::move(data_[size_ - 1]));
            for (std::size_t i = size_ - 1; i > index; --i)
                data_[i] = std::move(data_[i - 1]);
            data_[index] = value;
        }
        ++size_;
    }

    // Remove element at index by shifting the tail left, then destroy the last.
    void erase(std::size_t index) {
        if (index >= size_) throw std::out_of_range("DynamicArray::erase index out of range");
        for (std::size_t i = index; i + 1 < size_; ++i)
            data_[i] = std::move(data_[i + 1]);
        data_[size_ - 1].~T();
        --size_;
    }

    // Destroy all elements but keep the allocated capacity for reuse.
    void clear() noexcept { destroy_elements(); }

    // --- iteration (raw pointers => range-based for works) ----------------
    iterator begin() noexcept { return data_; }
    iterator end() noexcept { return data_ + size_; }
    const_iterator begin() const noexcept { return data_; }
    const_iterator end() const noexcept { return data_ + size_; }
};

int main() {
    // --- empty container ---
    DynamicArray<int> a;
    assert(a.empty());
    assert(a.size() == 0);
    assert(a.capacity() == 0);

    // --- single element ---
    a.push_back(10);
    assert(!a.empty());
    assert(a.size() == 1);
    assert(a.front() == 10 && a.back() == 10);
    assert(a[0] == 10 && a.at(0) == 10);

    // --- growth / resize past initial capacity ---
    for (int i = 1; i < 20; ++i) a.push_back((i + 1) * 10); // a = [10,20,30,...,200]
    assert(a.size() == 20);
    assert(a.capacity() >= 20);       // doubled several times: 1,2,4,8,16,32
    assert(a.back() == 200);

    // at() bounds check throws
    bool threw = false;
    try { a.at(999); } catch (const std::out_of_range&) { threw = true; }
    assert(threw);

    // reserve never shrinks and preserves contents
    std::size_t cap_before = a.capacity();
    a.reserve(4);
    assert(a.capacity() == cap_before);
    a.reserve(100);
    assert(a.capacity() >= 100 && a.size() == 20 && a[5] == 60);

    // insert in the middle and at the end
    a.insert(0, -1);                  // front
    assert(a[0] == -1 && a[1] == 10 && a.size() == 21);
    a.insert(a.size(), 999);          // back
    assert(a.back() == 999 && a.size() == 22);
    a.insert(2, 15);                  // middle
    assert(a[2] == 15 && a[3] == 20);

    // erase from middle and ends
    a.erase(2);
    assert(a[2] == 20);
    a.erase(0);
    assert(a[0] == 10);

    // --- removing down to empty ---
    while (!a.empty()) a.pop_back();
    assert(a.empty() && a.size() == 0);

    // --- copy semantics: copy then mutate original, verify independence ---
    DynamicArray<int> src;
    for (int i = 0; i < 5; ++i) src.push_back(i);
    DynamicArray<int> copy = src;     // copy constructor
    src[0] = 777;
    src.push_back(555);
    assert(copy.size() == 5 && copy[0] == 0);   // copy unaffected
    assert(src[0] == 777 && src.size() == 6);

    DynamicArray<int> assigned;
    assigned = copy;                  // copy assignment
    assigned[1] = -9;
    assert(copy[1] == 1);             // original still intact

    // --- move semantics: source left empty, target owns the data ---
    DynamicArray<int> moved = std::move(src);   // move constructor
    assert(moved.size() == 6 && moved[0] == 777);
    assert(src.empty() && src.size() == 0);
    DynamicArray<int> move_assigned;
    move_assigned = std::move(moved); // move assignment
    assert(move_assigned.size() == 6 && moved.empty());

    // --- works with a non-trivial type (std::string) ---
    DynamicArray<std::string> words;
    words.push_back("hello");
    words.push_back(std::string("world"));
    assert(words.size() == 2 && words.back() == "world");

    // range-based for via begin()/end()
    long sum = 0;
    for (int v : move_assigned) sum += v;
    assert(sum == 777 + 1 + 2 + 3 + 4 + 555);

    // --- human-readable demo ---
    std::cout << "DynamicArray demo\n";
    std::cout << "  words: ";
    for (const std::string& w : words) std::cout << w << ' ';
    std::cout << "\n  numbers: ";
    for (int v : move_assigned) std::cout << v << ' ';
    std::cout << "\n  size=" << move_assigned.size()
              << " capacity=" << move_assigned.capacity() << "\n";
    std::cout << "All DynamicArray assertions passed.\n";
    return 0;
}
