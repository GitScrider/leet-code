/*
 * Fenwick Tree / Binary Indexed Tree (BIT) (Data Structure - Tree)
 * -----------------------------------------------------------------------------
 * Summary:
 *   A compact 1-indexed array that maintains prefix sums under point updates.
 *   Each slot i stores the sum of a block of elements whose length is the
 *   lowest set bit of i, so both update and prefix query walk only O(log n)
 *   slots by flipping that bit.
 *
 * Operations & complexity (n = number of elements):
 *   Operation                | Time      | Space
 *   -------------------------+-----------+-------
 *   construct(n)             | O(n)      | O(n)
 *   update(index, delta)     | O(log n)  | O(1)
 *   prefixSum(index)         | O(log n)  | O(1)
 *   rangeSum(l, r)           | O(log n)  | O(1)
 *   at(index)                | O(log n)  | O(1)
 *
 * Invariants:
 *   - `bit_` is 1-indexed; bit_[0] is a permanently-unused sentinel so the
 *     low-bit arithmetic stays clean. The public API is 0-indexed and shifts
 *     by +1 internally.
 *   - Coverage invariant: bit_[i] == sum of the conceptual elements at indices
 *     (i - lowbit(i), i], i.e. the lowbit(i) elements ending at i, where
 *     lowbit(i) = i & (-i). This is exactly what makes prefixSum work by
 *     repeatedly stripping the lowest set bit.
 *
 * The low-bit trick, i & (-i):
 *   In two's complement, -i flips every bit above the lowest set bit and leaves
 *   that bit set, so i & (-i) isolates the lowest set bit (its numeric value,
 *   e.g. lowbit(12=0b1100) = 4). Two consequences drive the whole structure:
 *     * prefixSum: to sum [1..i], add bit_[i] then move to the block ending
 *       just before this one: i -= lowbit(i). Each step clears one set bit, so
 *       at most (popcount i) <= log2 n steps.
 *     * update: bit_[i] is responsible for index j, and so is the next block
 *       that swallows it: i += lowbit(i). Again O(log n) steps up to n.
 *
 * When to use / trade-offs:
 *   - A more compact, cache-friendly alternative to a segment tree when the
 *     aggregate forms a GROUP (invertible): sums work because rangeSum(l,r) =
 *     prefixSum(r) - prefixSum(l-1). Non-invertible ops like min/max cannot be
 *     range-queried this way -- reach for a segment tree there.
 *   - Half the memory of a segment tree and tiny constant factors; the code is
 *     just a few bit-twiddling loops.
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <utility>   // std::pair, std::move (used in the tests)
#include <vector>

// Note: index math uses signed long long internally for the i & (-i) trick,
// because negating an unsigned value would not isolate the low bit as intended.
template <typename T>
class FenwickTree {
public:
    FenwickTree() = default;

    // All-zero tree over n logical elements (0-indexed API: 0..n-1).
    explicit FenwickTree(std::size_t n) : n_(n), bit_(n + 1, T{}) {}

    // Build from initial values in O(n) using the "propagate to parent" trick:
    // after seeding each bit_[i] with its own element, push it once into the
    // block that covers it (i + lowbit(i)). Cheaper than n separate updates.
    explicit FenwickTree(const std::vector<T>& data)
        : n_(data.size()), bit_(data.size() + 1, T{}) {
        for (std::size_t i = 1; i <= n_; ++i) {
            bit_[i] += data[i - 1];
            std::size_t parent = i + lowbit(i);
            if (parent <= n_) bit_[parent] += bit_[i];
        }
    }

    // Value semantics: the only members are a std::vector and a size_t. vector
    // deep-copies and frees its own buffer, so the implicitly-defined copy/move/
    // destructor are already correct -- there is no owning raw pointer and thus
    // no double-free risk. Defaulted explicitly to show the choice is deliberate.
    FenwickTree(const FenwickTree&) = default;
    FenwickTree& operator=(const FenwickTree&) = default;
    FenwickTree(FenwickTree&&) noexcept = default;
    FenwickTree& operator=(FenwickTree&&) noexcept = default;
    ~FenwickTree() = default;

    std::size_t size() const { return n_; }
    bool empty() const { return n_ == 0; }

    // Add `delta` to the element at 0-indexed `index`. Climb to every block
    // that contains this index by repeatedly adding the low bit.
    void update(std::size_t index, const T& delta) {
        assert(index < n_ && "update index out of bounds");
        for (std::size_t i = index + 1; i <= n_; i += lowbit(i)) {
            bit_[i] += delta;
        }
    }

    // Sum of elements at 0-indexed positions [0, index]. Strip low bits to hop
    // backwards across the blocks that tile the prefix.
    T prefixSum(std::size_t index) const {
        assert(index < n_ && "prefixSum index out of bounds");
        T sum{};
        for (std::size_t i = index + 1; i > 0; i -= lowbit(i)) {
            sum += bit_[i];
        }
        return sum;
    }

    // Inclusive range sum on [l, r], via the group inverse (subtraction).
    // rangeSum(0, r) is just prefixSum(r); we special-case l == 0 to avoid
    // underflowing prefixSum(l - 1) on unsigned indices.
    T rangeSum(std::size_t l, std::size_t r) const {
        assert(l <= r && r < n_ && "rangeSum range out of bounds");
        if (l == 0) return prefixSum(r);
        return prefixSum(r) - prefixSum(l - 1);
    }

    // Single element value = rangeSum(index, index). O(log n).
    T at(std::size_t index) const {
        assert(index < n_ && "at index out of bounds");
        return rangeSum(index, index);
    }

private:
    std::size_t n_ = 0;
    std::vector<T> bit_;   // 1-indexed; bit_[0] is an unused sentinel.

    // Isolate the lowest set bit: i & (-i). Done in signed arithmetic because
    // unary minus on an unsigned type wraps mod 2^k and would not give the
    // intended "flip everything above the low bit" behaviour of two's complement.
    static std::size_t lowbit(std::size_t i) {
        long long s = static_cast<long long>(i);
        return static_cast<std::size_t>(s & (-s));
    }
};

// ---------------------------------------------------------------------------
// Tests + demo
// ---------------------------------------------------------------------------

int main() {
    // --- Edge case: empty tree ---------------------------------------------
    {
        FenwickTree<int> ft(std::size_t{0});
        assert(ft.empty());
        assert(ft.size() == 0);
        FenwickTree<int> ft2;   // default-constructed
        assert(ft2.empty());
    }

    // --- Edge case: single element -----------------------------------------
    {
        FenwickTree<int> ft(std::size_t{1});
        assert(ft.size() == 1);
        assert(ft.prefixSum(0) == 0);
        ft.update(0, 9);
        assert(ft.prefixSum(0) == 9);
        assert(ft.rangeSum(0, 0) == 9);
        assert(ft.at(0) == 9);
    }

    // --- Core: sequence of updates vs a brute-force reference array --------
    {
        const std::size_t n = 12;
        FenwickTree<long long> ft(n);
        std::vector<long long> ref(n, 0);

        // Apply a batch of point updates to both structures.
        const std::vector<std::pair<std::size_t, long long>> ops = {
            {0, 5}, {3, -2}, {7, 10}, {11, 4}, {3, 6}, {0, -1}, {9, 8}, {5, 3}
        };
        for (const auto& op : ops) {
            ft.update(op.first, op.second);
            ref[op.first] += op.second;
        }

        // prefixSum must match a running total of the reference at every index.
        long long running = 0;
        for (std::size_t i = 0; i < n; ++i) {
            running += ref[i];
            assert(ft.prefixSum(i) == running);
            assert(ft.at(i) == ref[i]);
        }

        // rangeSum must match a brute-force inner-loop sum for every [l, r].
        for (std::size_t l = 0; l < n; ++l) {
            for (std::size_t r = l; r < n; ++r) {
                long long expected = 0;
                for (std::size_t k = l; k <= r; ++k) expected += ref[k];
                assert(ft.rangeSum(l, r) == expected);
            }
        }
    }

    // --- Core: O(n) constructor from initial data matches manual updates ---
    {
        std::vector<int> data{3, 1, 4, 1, 5, 9, 2, 6};
        FenwickTree<int> built(data);          // fast build
        FenwickTree<int> incr(data.size());    // same content via updates
        for (std::size_t i = 0; i < data.size(); ++i) incr.update(i, data[i]);

        int running = 0;
        for (std::size_t i = 0; i < data.size(); ++i) {
            running += data[i];
            assert(built.prefixSum(i) == running);
            assert(incr.prefixSum(i) == running);
            assert(built.at(i) == data[i]);
        }
        assert(built.rangeSum(2, 5) == 4 + 1 + 5 + 9);
    }

    // --- Copy/move safety: copies are independent -------------------------
    {
        FenwickTree<int> a(std::vector<int>{1, 2, 3, 4});
        FenwickTree<int> b = a;         // deep copy
        b.update(0, 100);
        assert(a.prefixSum(0) == 1);    // original unaffected
        assert(b.prefixSum(0) == 101);
        FenwickTree<int> c = std::move(b);   // move
        assert(c.prefixSum(3) == 101 + 2 + 3 + 4);
    }

    std::cout << "All FenwickTree assertions passed.\n\n";

    // --- Human-readable demo -----------------------------------------------
    FenwickTree<int> ft(std::vector<int>{2, 4, 6, 8, 10});
    std::cout << "Array: [2, 4, 6, 8, 10]\n";
    std::cout << "  prefixSum(4) = " << ft.prefixSum(4) << "  (expected 30)\n";
    std::cout << "  rangeSum(1,3) = " << ft.rangeSum(1, 3) << "  (expected 18)\n";
    std::cout << "update: add 50 to index 2\n";
    ft.update(2, 50);
    std::cout << "  prefixSum(4) = " << ft.prefixSum(4) << "  (expected 80)\n";
    std::cout << "  at(2)        = " << ft.at(2) << "  (expected 56)\n";
    return 0;
}
