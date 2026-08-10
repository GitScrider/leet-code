/*
 * Hash Table (Separate Chaining)            Category: Data Structure - Hashing
 * ---------------------------------------------------------------------------
 * Summary:
 *   A key/value map that resolves collisions by SEPARATE CHAINING: each bucket
 *   holds a small list of entries that hash to the same index. Automatic
 *   rehashing keeps the average chain short so operations stay ~O(1).
 *
 * Operations & complexity:
 *   +-----------------+-------------+---------------------------------------+
 *   | Operation       | Average     | Worst case                            |
 *   +-----------------+-------------+---------------------------------------+
 *   | put / insert    | O(1)        | O(n)  (all keys in one bucket)        |
 *   | get / contains  | O(1)        | O(n)                                  |
 *   | erase           | O(1)        | O(n)                                  |
 *   +-----------------+-------------+---------------------------------------+
 *   Rehash touches every element: O(n). But it happens rarely (only when the
 *   table roughly doubles), so amortized over all insertions each put is O(1).
 *
 * Invariants / key ideas:
 *   - Load factor = size / bucketCount. We rehash when it exceeds 0.75.
 *     A bounded load factor bounds the expected chain length, which is what
 *     keeps lookups O(1) on average -- if we never grew, chains would grow
 *     linearly with n and lookups would degrade to O(n).
 *   - Collisions are resolved by chaining: colliding keys coexist in the same
 *     bucket's list. No probing, so NO tombstones are needed (that concern is
 *     specific to open addressing).
 *   - A key appears at most once: put OVERWRITES an existing value rather than
 *     duplicating the key.
 *
 * When to use / trade-offs:
 *   - Simple, robust; tolerates high load factors and clustering gracefully.
 *   - Extra memory per node/list; poor cache locality vs open addressing.
 *   - Great default when deletions are frequent (no tombstone bookkeeping).
 *
 * Build: g++ -std=c++17 -Wall -Wextra hash_table_chaining.cpp -o demo
 */

#include <cassert>
#include <cstddef>
#include <functional>  // std::hash (default Hash template arg) and std::swap
#include <iostream>
#include <string>
#include <utility>
#include <vector>

// Rule of Zero: the only owning member is a std::vector of std::vectors, which
// manages its own memory and copies/moves/destroys correctly. We therefore do
// NOT declare any destructor, copy/move constructor, or assignment operator --
// the compiler-generated ones are correct and leak-free.
template <typename K, typename V, typename Hash = std::hash<K>>
class HashTableChaining {
public:
    // Start with a small prime bucket count. Prime sizes spread hashes more
    // evenly for typical std::hash outputs than powers of two would.
    explicit HashTableChaining(std::size_t initialBuckets = 7)
        : buckets_(initialBuckets == 0 ? 1 : initialBuckets), size_(0), hasher_() {}

    // Insert a new key or OVERWRITE the value of an existing one.
    // Returns true if a new key was inserted, false if an existing key updated.
    bool put(const K& key, const V& value) {
        // Update in place if the key already exists (keeps size unchanged and
        // avoids duplicating the key in its bucket).
        Entry* existing = findEntry(key);
        if (existing != nullptr) {
            existing->second = value;
            return false;
        }
        // New key: grow first if this insertion would push us over threshold.
        if (loadFactorAfterInsert() > kMaxLoadFactor) {
            rehash(nextCapacity(buckets_.size()));
        }
        buckets_[indexFor(key)].emplace_back(key, value);
        ++size_;
        return true;
    }

    // Lookup: return a pointer to the stored value, or nullptr if absent.
    // A pointer (rather than std::optional<V>) lets callers mutate in place and
    // avoids copying V; nullptr cleanly signals "not found".
    V* get(const K& key) {
        Entry* e = findEntry(key);
        return e != nullptr ? &e->second : nullptr;
    }
    const V* get(const K& key) const {
        const Entry* e = findEntry(key);
        return e != nullptr ? &e->second : nullptr;
    }

    bool contains(const K& key) const { return findEntry(key) != nullptr; }

    // Remove a key if present. Returns true if something was erased.
    bool erase(const K& key) {
        Bucket& bucket = buckets_[indexFor(key)];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == key) {
                // swap-with-last + pop_back: O(1) removal, order in a bucket
                // is irrelevant so we don't pay to shift the tail.
                std::swap(*it, bucket.back());
                bucket.pop_back();
                --size_;
                return true;
            }
        }
        return false;
    }

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    std::size_t bucketCount() const { return buckets_.size(); }
    double loadFactor() const {
        return static_cast<double>(size_) / static_cast<double>(buckets_.size());
    }

private:
    using Entry  = std::pair<K, V>;
    using Bucket = std::vector<Entry>;

    static constexpr double kMaxLoadFactor = 0.75;

    std::vector<Bucket> buckets_;
    std::size_t         size_;
    Hash                hasher_;  // store the hasher as a member (may be stateful)

    std::size_t indexFor(const K& key) const {
        // Map the (possibly huge) hash into [0, bucketCount).
        return hasher_(key) % buckets_.size();
    }

    double loadFactorAfterInsert() const {
        return static_cast<double>(size_ + 1) / static_cast<double>(buckets_.size());
    }

    // Non-const and const overloads share the same scan logic.
    Entry* findEntry(const K& key) {
        Bucket& bucket = buckets_[indexFor(key)];
        for (Entry& e : bucket) {
            if (e.first == key) return &e;
        }
        return nullptr;
    }
    const Entry* findEntry(const K& key) const {
        const Bucket& bucket = buckets_[indexFor(key)];
        for (const Entry& e : bucket) {
            if (e.first == key) return &e;
        }
        return nullptr;
    }

    // Choose the next bucket count: roughly double, then bump to the next odd
    // number. Odd (ideally prime) sizes avoid the clustering that even sizes
    // cause when hashes share common factors with the table size.
    static std::size_t nextCapacity(std::size_t current) {
        std::size_t next = current * 2 + 1;  // *2+1 is guaranteed odd
        return next;
    }

    // Rebuild the table with a larger bucket array and re-insert every entry.
    // Rehashing is why operations stay O(1): by keeping the load factor bounded
    // we keep the expected chain length constant regardless of how large n gets.
    void rehash(std::size_t newBucketCount) {
        std::vector<Bucket> fresh(newBucketCount == 0 ? 1 : newBucketCount);
        for (Bucket& bucket : buckets_) {
            for (Entry& e : bucket) {
                // Recompute the index against the NEW bucket count; move to
                // avoid copying keys/values during the rebuild.
                std::size_t idx = hasher_(e.first) % fresh.size();
                fresh[idx].emplace_back(std::move(e));
            }
        }
        buckets_ = std::move(fresh);
    }
};

// ---------------------------------------------------------------------------
// Tests + human-readable demo
// ---------------------------------------------------------------------------

// A deliberately terrible hasher that funnels EVERY key into bucket 0, so we
// can exercise the collision (long-chain) code path directly.
struct AllCollideHash {
    std::size_t operator()(int) const { return 0; }
};

int main() {
    // --- int keys: basic operations ---------------------------------------
    {
        HashTableChaining<int, std::string> table;
        assert(table.empty());
        assert(table.size() == 0);

        assert(table.put(1, "one") == true);
        assert(table.put(2, "two") == true);
        assert(table.put(3, "three") == true);
        assert(table.size() == 3);
        assert(!table.empty());

        // get returns a pointer to the live value.
        const std::string* v = table.get(2);
        assert(v != nullptr && *v == "two");
        assert(table.get(999) == nullptr);
        assert(table.contains(1) && !table.contains(42));

        // Update existing key: value overwritten, size UNCHANGED, no duplicate.
        assert(table.put(2, "TWO") == false);
        assert(table.size() == 3);
        assert(*table.get(2) == "TWO");

        // Erase.
        assert(table.erase(1) == true);
        assert(!table.contains(1));
        assert(table.size() == 2);
        assert(table.erase(1) == false);  // erasing again is a no-op
    }

    // --- Forced collisions: every key lands in the same bucket -------------
    {
        HashTableChaining<int, int, AllCollideHash> table(8);
        for (int i = 0; i < 20; ++i) {
            table.put(i, i * 10);
        }
        // All 20 coexist in one long chain; every one must still be findable.
        assert(table.size() == 20);
        for (int i = 0; i < 20; ++i) {
            const int* got = table.get(i);
            assert(got != nullptr && *got == i * 10);
        }
        // Update and erase still work correctly inside a crowded bucket.
        table.put(5, 555);
        assert(*table.get(5) == 555 && table.size() == 20);
        assert(table.erase(10) && table.size() == 19 && !table.contains(10));
    }

    // --- Rehash: insert enough keys to trigger AT LEAST ONE grow -----------
    {
        HashTableChaining<int, int> table(7);  // small, so growth happens early
        const std::size_t startBuckets = table.bucketCount();

        const int N = 500;
        for (int i = 0; i < N; ++i) {
            table.put(i, i + 1000);
        }
        // Growth must have occurred and load factor must stay under threshold.
        assert(table.bucketCount() > startBuckets);
        assert(table.loadFactor() <= 0.75);
        assert(table.size() == static_cast<std::size_t>(N));

        // CRITICAL: after rehashing, every key still maps to its value.
        for (int i = 0; i < N; ++i) {
            const int* got = table.get(i);
            assert(got != nullptr && *got == i + 1000);
        }
    }

    // --- std::string keys (default std::hash<std::string>) -----------------
    {
        HashTableChaining<std::string, int> ages;
        ages.put("alice", 30);
        ages.put("bob", 25);
        ages.put("carol", 41);
        assert(ages.size() == 3);
        assert(*ages.get("bob") == 25);

        ages.put("bob", 26);  // overwrite
        assert(ages.size() == 3 && *ages.get("bob") == 26);

        assert(ages.erase("alice"));
        assert(!ages.contains("alice") && ages.size() == 2);

        // Force a rehash with many string keys, then verify all survive.
        HashTableChaining<std::string, int> big(3);
        const int M = 300;
        for (int i = 0; i < M; ++i) {
            big.put("key#" + std::to_string(i), i);
        }
        assert(big.size() == static_cast<std::size_t>(M));
        for (int i = 0; i < M; ++i) {
            const int* got = big.get("key#" + std::to_string(i));
            assert(got != nullptr && *got == i);
        }
    }

    // --- Human-readable demo ----------------------------------------------
    HashTableChaining<std::string, int> demo;
    demo.put("apple", 3);
    demo.put("banana", 7);
    demo.put("cherry", 5);
    demo.put("apple", 4);  // overwrite

    std::cout << "Separate-chaining hash table demo\n";
    std::cout << "  size          = " << demo.size() << "\n";
    std::cout << "  bucket count  = " << demo.bucketCount() << "\n";
    std::cout << "  load factor   = " << demo.loadFactor() << "\n";
    std::cout << "  apple -> " << *demo.get("apple") << " (overwritten from 3)\n";
    std::cout << "  contains banana? " << (demo.contains("banana") ? "yes" : "no") << "\n";

    std::cout << "All assertions passed.\n";
    return 0;
}
