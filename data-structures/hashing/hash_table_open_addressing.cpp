/*
 * Hash Table (Open Addressing, Linear Probing)   Category: Data Structure - Hashing
 *
 * Summary:
 *   A generic key/value hash table that resolves collisions by OPEN ADDRESSING:
 *   every element lives directly inside one contiguous bucket array, and on a
 *   collision we probe successive slots until we find room. Deletion uses
 *   tombstones so that existing probe chains stay intact.
 *
 * Operations & complexity:
 *   +-----------+-----------+------------+-------------------------------------+
 *   | Operation | Average   | Worst case | Notes                               |
 *   +-----------+-----------+------------+-------------------------------------+
 *   | insert    | O(1)      | O(n)       | O(n) amortized rehash occasionally  |
 *   | find      | O(1)      | O(n)       | worst case = one long probe chain   |
 *   | erase     | O(1)      | O(n)       | marks a tombstone, no shifting      |
 *   +-----------+-----------+------------+-------------------------------------+
 *   Rehashing copies every live element into a larger table: a single insert
 *   can cost O(n), but the cost is spread ("amortized") over the many cheap
 *   inserts that preceded it, so the AVERAGE insert stays O(1).
 *
 * Invariants / key ideas:
 *   - Load factor alpha = (occupied + deleted) / capacity. We rehash (grow ~2x)
 *     as soon as alpha would exceed MAX_LOAD (~0.7). Keeping alpha bounded is
 *     what keeps probe chains short and therefore keeps operations O(1).
 *   - Collisions are resolved by LINEAR PROBING: probe index (h + step) % cap
 *     with step incremented by 1 each time.
 *   - Tombstone rule: erase marks a slot DELETED (never EMPTY). A DELETED slot
 *     means "something used to live here, KEEP PROBING" for lookups, but is a
 *     reusable landing spot for inserts. Turning it into EMPTY would truncate
 *     any probe chain that passes through it and hide later keys forever.
 *
 * When to use / trade-offs:
 *   - Great cache locality: everything is in one array, no per-node allocation.
 *   - Sensitive to load factor; performance degrades sharply as alpha -> 1.
 *   - Linear probing suffers PRIMARY CLUSTERING (see note near the probe loop).
 *   - Prefer separate chaining if you need very high load factors or frequent
 *     erases without periodic rehashing to clear tombstones.
 */

#include <cassert>
#include <cstddef>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

template <typename K, typename V, typename Hash = std::hash<K>>
class HashTableOpenAddressing {
public:
    // Rule of Zero: the only owning member is a std::vector, which already
    // implements correct copy/move/destroy. No hand-written special members.

    explicit HashTableOpenAddressing(std::size_t initial_capacity = 8)
        : slots_(initial_capacity <= 1 ? 2 : initial_capacity) {}

    // Insert a new key or overwrite the value of an existing one.
    // Returns true if a NEW key was inserted, false if an existing key was updated.
    bool insert(const K& key, const V& value) {
        // Grow BEFORE inserting so we never exceed the load-factor threshold and
        // so there is always at least one EMPTY slot to terminate probe chains.
        if (load_would_exceed_threshold()) {
            rehash(slots_.size() * 2);
        }

        const std::size_t cap = slots_.size();
        std::size_t idx = index_for(key);
        std::size_t step = 0;

        // Remember the first DELETED slot we pass: we may reuse it, but only
        // AFTER confirming the key is not already present further down the chain.
        bool have_reuse = false;
        std::size_t reuse_idx = 0;

        while (step < cap) {
            Slot& slot = slots_[idx];

            if (slot.state == State::EMPTY) {
                // End of the probe chain: the key is definitely absent.
                // Reuse an earlier tombstone if we saw one, else land here.
                std::size_t target = have_reuse ? reuse_idx : idx;
                slots_[target].state = State::OCCUPIED;
                slots_[target].key = key;
                slots_[target].value = value;
                ++occupied_;
                if (have_reuse) {
                    // We converted a tombstone back into a live slot.
                    --deleted_;
                }
                return true;
            }

            if (slot.state == State::DELETED) {
                if (!have_reuse) {
                    have_reuse = true;
                    reuse_idx = idx;
                }
                // Keep scanning: the key might still exist beyond this tombstone.
            } else if (key_eq_(slot.key, key)) {
                // OCCUPIED and same key -> update in place, size unchanged.
                slot.value = value;
                return false;
            }

            ++step;
            idx = (idx + 1) % cap;  // linear probe, step size 1
        }

        // We grow at ~0.7 load, so a free slot always exists; if the whole scan
        // saw only tombstones, reuse the first one. (Fallback grow is defensive.)
        if (have_reuse) {
            slots_[reuse_idx].state = State::OCCUPIED;
            slots_[reuse_idx].key = key;
            slots_[reuse_idx].value = value;
            ++occupied_;
            --deleted_;
            return true;
        }
        rehash(slots_.size() * 2);
        return insert(key, value);
    }

    // Returns a pointer to the stored value, or nullptr if the key is absent.
    // Const-correct overloads let callers read through a const table.
    const V* find(const K& key) const {
        const std::size_t slot = locate(key);
        return slot == npos ? nullptr : &slots_[slot].value;
    }

    V* find(const K& key) {
        const std::size_t slot = locate(key);
        return slot == npos ? nullptr : &slots_[slot].value;
    }

    bool contains(const K& key) const { return locate(key) != npos; }

    // Erase a key. Returns true if it was present.
    // Critically, we mark the slot DELETED (a tombstone), not EMPTY, so that
    // keys inserted later in the same probe chain remain reachable.
    bool erase(const K& key) {
        const std::size_t slot = locate(key);
        if (slot == npos) {
            return false;
        }
        slots_[slot].state = State::DELETED;
        --occupied_;
        ++deleted_;
        return true;
    }

    std::size_t size() const { return occupied_; }
    bool empty() const { return occupied_ == 0; }
    std::size_t bucket_count() const { return slots_.size(); }

    double load_factor() const {
        return static_cast<double>(occupied_ + deleted_) /
               static_cast<double>(slots_.size());
    }

private:
    enum class State { EMPTY, OCCUPIED, DELETED };

    struct Slot {
        State state = State::EMPTY;
        K key{};
        V value{};
    };

    static constexpr std::size_t npos = static_cast<std::size_t>(-1);
    static constexpr double MAX_LOAD = 0.7;

    std::vector<Slot> slots_;
    std::size_t occupied_ = 0;  // number of live (OCCUPIED) elements
    std::size_t deleted_ = 0;   // number of tombstones (count toward load factor)
    Hash hasher_{};             // hasher stored as a member, per spec
    std::equal_to<K> key_eq_{}; // key comparison

    std::size_t index_for(const K& key) const {
        return hasher_(key) % slots_.size();
    }

    // True if inserting one more element would push (occupied+deleted)/cap over
    // the threshold. We also count tombstones because they still lengthen probe
    // chains; rehashing is what finally sweeps them away.
    bool load_would_exceed_threshold() const {
        return static_cast<double>(occupied_ + deleted_ + 1) >
               MAX_LOAD * static_cast<double>(slots_.size());
    }

    // Find the index of an OCCUPIED slot holding key, or npos.
    // DELETED slots are treated as "keep probing"; an EMPTY slot ends the search.
    std::size_t locate(const K& key) const {
        const std::size_t cap = slots_.size();
        std::size_t idx = index_for(key);
        std::size_t step = 0;

        while (step < cap) {
            const Slot& slot = slots_[idx];
            if (slot.state == State::EMPTY) {
                return npos;  // chain ended without a match
            }
            if (slot.state == State::OCCUPIED && key_eq_(slot.key, key)) {
                return idx;
            }
            // OCCUPIED-but-different or DELETED: keep probing.
            ++step;
            idx = (idx + 1) % cap;
            //
            // PRIMARY CLUSTERING: with linear probing, keys that collide form
            // long contiguous runs, and those runs tend to merge and grow,
            // slowing every operation that lands in the cluster. Two classic
            // alternatives that spread probes out (NOT implemented here, to keep
            // the teaching example simple):
            //   * Quadratic probing: probe (h + c1*i + c2*i^2) % cap.
            //   * Double hashing:    step size = a second, independent hash of
            //                        the key, so different keys probe different
            //                        sequences.
        }
        return npos;
    }

    // Rebuild into a larger array, re-hashing every live element from scratch.
    // Copying only OCCUPIED slots naturally DROPS all tombstones and restores
    // short probe chains. Cost is spread over prior inserts: amortized O(1).
    void rehash(std::size_t new_capacity) {
        if (new_capacity < 2) {
            new_capacity = 2;
        }
        std::vector<Slot> old = std::move(slots_);
        slots_.assign(new_capacity, Slot{});
        occupied_ = 0;
        deleted_ = 0;

        for (const Slot& s : old) {
            if (s.state == State::OCCUPIED) {
                insert_no_resize(s.key, s.value);
            }
        }
    }

    // Insert used only during rehash: the destination is freshly allocated with
    // no tombstones, so the probe logic is a simple "find first EMPTY".
    void insert_no_resize(const K& key, const V& value) {
        const std::size_t cap = slots_.size();
        std::size_t idx = index_for(key);
        std::size_t step = 0;
        while (step < cap) {
            Slot& slot = slots_[idx];
            if (slot.state == State::EMPTY) {
                slot.state = State::OCCUPIED;
                slot.key = key;
                slot.value = value;
                ++occupied_;
                return;
            }
            ++step;
            idx = (idx + 1) % cap;
        }
    }
};

// ------------------------------- Tests / demo -------------------------------

int main() {
    // ---- Core operations with int keys ----
    HashTableOpenAddressing<int, int> t;
    assert(t.empty());
    assert(t.insert(1, 100));   // new key
    assert(t.insert(2, 200));
    assert(!t.insert(1, 111));  // update existing key -> returns false
    assert(t.size() == 2);      // size unchanged after update
    assert(*t.find(1) == 111);  // value overwritten
    assert(t.find(42) == nullptr);

    // ---- Force a collision cluster ----
    // With a small capacity, several keys hash to nearby slots, building one
    // contiguous linear-probing chain. We use a table sized so keys 3,11,19,27
    // (all == 3 mod 8) collide into the same starting bucket.
    HashTableOpenAddressing<int, std::string> chain(8);
    chain.insert(3, "a");
    chain.insert(11, "b");   // collides with 3, probes forward
    chain.insert(19, "c");   // collides, probes further
    chain.insert(27, "d");   // collides, probes further still
    assert(*chain.find(3) == "a");
    assert(*chain.find(11) == "b");
    assert(*chain.find(19) == "c");
    assert(*chain.find(27) == "d");

    // ---- Classic tombstone bug: erase a key in the MIDDLE of the chain ----
    // Removing 11 must NOT break the chain: 19 and 27 sit AFTER 11 and are only
    // reachable by probing THROUGH 11's slot. A correct tombstone keeps them
    // findable; a naive "mark EMPTY" would lose them.
    assert(chain.erase(11));
    assert(chain.find(11) == nullptr);
    assert(*chain.find(19) == "c");  // still reachable through the tombstone
    assert(*chain.find(27) == "d");
    assert(*chain.find(3) == "a");

    // ---- Tombstone reuse: re-inserting should land in the freed slot ----
    const std::size_t buckets_before = chain.bucket_count();
    chain.insert(11, "b2");
    assert(*chain.find(11) == "b2");
    assert(chain.bucket_count() == buckets_before);  // reused a tombstone, no growth
    assert(*chain.find(19) == "c");                  // rest of chain intact
    assert(*chain.find(27) == "d");

    // ---- Trigger at least one automatic REHASH and verify nothing is lost ----
    HashTableOpenAddressing<int, int> big(4);
    const std::size_t initial_buckets = big.bucket_count();
    const int N = 500;
    for (int i = 0; i < N; ++i) {
        big.insert(i, i * i);
    }
    assert(big.bucket_count() > initial_buckets);  // it grew at least once
    assert(big.size() == static_cast<std::size_t>(N));
    for (int i = 0; i < N; ++i) {
        const int* p = big.find(i);
        assert(p != nullptr);
        assert(*p == i * i);  // every key survived rehashing with the right value
    }
    // Interleave erases and inserts to create tombstones, then rehash again.
    for (int i = 0; i < N; i += 2) {
        assert(big.erase(i));
    }
    assert(big.size() == static_cast<std::size_t>(N / 2));
    for (int i = 0; i < N; ++i) {
        big.insert(i + N, -i);  // more inserts force another rehash past tombstones
    }
    for (int i = 1; i < N; i += 2) {
        assert(*big.find(i) == i * i);  // odd originals still correct
    }
    for (int i = 0; i < N; ++i) {
        assert(*big.find(i + N) == -i);  // newly inserted keys correct
    }

    // ---- std::string keys (uses std::hash<std::string> by default) ----
    HashTableOpenAddressing<std::string, int> words(4);
    const char* fruit[] = {"apple", "banana", "cherry", "date",
                           "elderberry", "fig", "grape", "honeydew"};
    for (int i = 0; i < 8; ++i) {
        words.insert(fruit[i], i);
    }
    for (int i = 0; i < 8; ++i) {
        assert(*words.find(fruit[i]) == i);  // survived string-key rehashing
    }
    assert(!words.insert("banana", 99));  // update existing string key
    assert(*words.find("banana") == 99);
    assert(words.size() == 8);
    assert(words.erase("cherry"));
    assert(words.find("cherry") == nullptr);
    assert(words.size() == 7);

    // ---- Human-readable demo ----
    std::cout << "HashTableOpenAddressing demo\n";
    std::cout << "  int table 'big': size=" << big.size()
              << ", bucket_count=" << big.bucket_count()
              << ", load_factor=" << big.load_factor() << "\n";
    std::cout << "  string table 'words': size=" << words.size()
              << ", bucket_count=" << words.bucket_count()
              << ", load_factor=" << words.load_factor() << "\n";
    std::cout << "  lookup words[\"grape\"] = " << *words.find("grape") << "\n";
    std::cout << "All assertions passed.\n";
    return 0;
}
