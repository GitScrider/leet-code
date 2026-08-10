/*
 * Bin Packing - Algorithm - NP-Hard
 * =================================
 *
 * Decision problem (BIN-PACKING):
 *   Given item sizes s[0..n-1], a bin capacity C, and an integer k, can all
 *   items be packed into at most k bins so that no bin's contents exceed C?
 *   The optimization version asks for the MINIMUM number of bins.
 *
 * Complexity class:
 *   The decision version is NP-complete; the optimization version is NP-hard.
 *   WHY: the PARTITION problem reduces to it. Given numbers summing to 2B, ask
 *   "can they fit into k = 2 bins of capacity C = B?". A YES means the set
 *   splits into two equal halves -- exactly PARTITION. Since PARTITION is
 *   NP-complete, deciding "2 bins suffice" (hence BIN-PACKING) is too.
 *
 * Algorithms implemented here:
 *   +-----------------------------+------------------------+-------------------+
 *   | Method                      | Time                   | Guarantee         |
 *   +-----------------------------+------------------------+-------------------+
 *   | Exact branch-and-bound      | exponential (small n)  | optimal           |
 *   | First-Fit-Decreasing (FFD)  | O(n log n + n * bins)  | <= 11/9 OPT + 1   |
 *   +-----------------------------+------------------------+-------------------+
 *   FFD sorts items large-to-small and drops each into the first bin that has
 *   room (opening a new bin only when none fits). Johnson's classic bound is
 *   FFD(I) <= 11/9 * OPT(I) + 6/9; we assert the looser, safe 11/9 * OPT + 1.
 *   FFD is often optimal on small instances but is NOT optimal in general --
 *   worst-case tight families push the ratio to 11/9.
 *
 * Key points:
 *   - Exact solver assigns items (sorted descending) one at a time to an
 *     existing bin that fits or to a fresh bin, pruning whenever the partial
 *     bin count already reaches the best complete solution found.
 *   - Symmetry pruning: among currently open bins with identical fill levels
 *     we try only one -- placing the item in either yields an equivalent state.
 *   - Every packing produced is validated: capacity respected AND the multiset
 *     of packed items equals the input (nothing dropped or duplicated).
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>

using Packing = std::vector<std::vector<int>>; // bins -> list of item sizes

// ---- Validation: capacity respected and all items accounted for -----------
static bool validPacking(const Packing& bins, const std::vector<int>& items, int capacity) {
    for (const std::vector<int>& bin : bins) {
        long long load = 0;
        for (const int x : bin) load += x;
        if (load > capacity) return false;      // over capacity
    }
    std::vector<int> packed;
    for (const std::vector<int>& bin : bins)
        for (const int x : bin) packed.push_back(x);
    std::vector<int> expected = items;
    std::sort(packed.begin(), packed.end());
    std::sort(expected.begin(), expected.end());
    return packed == expected;                   // same multiset of items
}

// ---- Exact minimum number of bins via branch-and-bound ---------------------
struct ExactBinPacker {
    std::vector<int> sizes;   // sorted descending
    int capacity;
    std::size_t n;
    int best;                 // fewest bins found so far
    Packing bestPacking;      // a witness achieving `best`

    int solve(std::vector<int> s, int cap) {
        capacity = cap;
        for (const int x : s) assert(x >= 0 && x <= capacity && "item must fit in one bin");
        std::sort(s.begin(), s.end(), std::greater<int>()); // big items first -> stronger pruning
        sizes = std::move(s);
        n = sizes.size();

        best = static_cast<int>(n);   // trivial upper bound: one bin per item
        bestPacking.assign(n, {});
        for (std::size_t i = 0; i < n; ++i) bestPacking[i].push_back(sizes[i]);

        Packing bins;                 // current partial assignment (contents per bin)
        recurse(0, bins);
        return best;
    }

    void recurse(std::size_t idx, Packing& bins) {
        if (static_cast<int>(bins.size()) >= best) return;  // cannot beat the best
        if (idx == n) {                                     // all items placed
            best = static_cast<int>(bins.size());
            bestPacking = bins;
            return;
        }
        const int item = sizes[idx];

        // Try placing into an existing bin that still has room.
        std::vector<long long> triedLoads; // skip bins whose current load repeats
        for (std::size_t b = 0; b < bins.size(); ++b) {
            long long load = 0;
            for (const int x : bins[b]) load += x;
            if (load + item > capacity) continue;
            // Symmetry break: identical fill levels give equivalent branches.
            if (std::find(triedLoads.begin(), triedLoads.end(), load) != triedLoads.end()) continue;
            triedLoads.push_back(load);

            bins[b].push_back(item);
            recurse(idx + 1, bins);
            bins[b].pop_back();
        }

        // Try opening a fresh bin for this item.
        bins.push_back({item});
        recurse(idx + 1, bins);
        bins.pop_back();
    }
};

// ---- First-Fit-Decreasing heuristic ----------------------------------------
static int firstFitDecreasing(std::vector<int> items, int capacity, Packing* out = nullptr) {
    for (const int x : items) assert(x >= 0 && x <= capacity && "item must fit in one bin");
    std::sort(items.begin(), items.end(), std::greater<int>()); // decreasing order

    std::vector<long long> load;   // current fill per bin
    Packing bins;
    for (const int item : items) {
        bool placed = false;
        for (std::size_t b = 0; b < load.size(); ++b) {
            if (load[b] + item <= capacity) {      // first bin that fits
                load[b] += item;
                bins[b].push_back(item);
                placed = true;
                break;
            }
        }
        if (!placed) {                             // none fit -> open a new bin
            load.push_back(item);
            bins.push_back({item});
        }
    }
    if (out) *out = bins;
    return static_cast<int>(load.size());
}

// Simple, always-valid lower bound: ceil(total / capacity).
static int volumeLowerBound(const std::vector<int>& items, int capacity) {
    const long long total = std::accumulate(items.begin(), items.end(), 0LL);
    return static_cast<int>((total + capacity - 1) / capacity);
}

int main() {
    ExactBinPacker exact;

    // --- Edge cases ---------------------------------------------------------
    assert(exact.solve({}, 10) == 0);                 // no items -> 0 bins
    assert(exact.solve({10}, 10) == 1);               // single item filling a bin
    assert(exact.solve({3, 3, 3}, 3) == 3);           // each item needs its own bin

    // --- Known optimal instances -------------------------------------------
    struct Case { std::vector<int> items; int capacity; int expectedOpt; };
    const std::vector<Case> cases = {
        {{5, 5, 4, 4, 3, 3}, 10, 3},                  // (5,5)(4,4)(3,3) -> 3 bins; sum 24 needs >=3
        {{4, 8, 1, 4, 2, 1}, 10, 2},                  // 4+4+2 and 8+1+1 = 2 bins
        {{6, 6, 6, 4, 4, 4, 3, 3, 3, 3}, 10, 5},      // pair each 6 with a 4, then the 3s
        {{2, 5, 4, 7, 1, 3, 8}, 10, 3},               // total 30 -> 3 bins is optimal
    };

    for (const Case& c : cases) {
        const int opt = exact.solve(c.items, c.capacity);
        assert(opt == c.expectedOpt);

        // Exact witness must be a genuine, complete packing of the right size.
        assert(static_cast<int>(exact.bestPacking.size()) == opt);
        assert(validPacking(exact.bestPacking, c.items, c.capacity));

        // The optimum can never be below the volume lower bound.
        assert(opt >= volumeLowerBound(c.items, c.capacity));

        // FFD: valid packing, never better than optimal, within its guarantee.
        Packing ffdBins;
        const int ffd = firstFitDecreasing(c.items, c.capacity, &ffdBins);
        assert(validPacking(ffdBins, c.items, c.capacity));
        assert(static_cast<int>(ffdBins.size()) == ffd);
        assert(ffd >= opt);                            // a heuristic cannot beat OPT
        // FFD <= 11/9 * OPT + 1, written in overflow-free integer form:
        assert(9 * ffd <= 11 * opt + 9);
    }

    // --- Relationship to PARTITION -----------------------------------------
    // {8, 6, 4, 2} sums to 20; capacity 10 asks "do 2 bins suffice?" -> YES
    // means the set splits into two halves of 10 (8+2 and 6+4). This is the
    // PARTITION reduction that makes deciding "k = 2 bins" NP-complete.
    assert(exact.solve({8, 6, 4, 2}, 10) == 2);
    assert(exact.solve({8, 6, 4, 3}, 10) == 3);        // sum 21 -> no equal split, 3 bins

    // --- Short demo ---------------------------------------------------------
    const std::vector<int> demo = {5, 5, 4, 4, 3, 3};
    const int opt = exact.solve(demo, 10);
    Packing ffdBins;
    const int ffd = firstFitDecreasing(demo, 10, &ffdBins);
    std::cout << "Bin Packing  (capacity = 10)\n";
    std::cout << "items       = {5, 5, 4, 4, 3, 3}\n";
    std::cout << "OPT bins    = " << opt << "\n";
    std::cout << "FFD bins    = " << ffd << (ffd == opt ? "  (optimal here)" : "  (heuristic)") << "\n";
    std::cout << "FFD packing =";
    for (const std::vector<int>& bin : ffdBins) {
        std::cout << " [";
        for (std::size_t j = 0; j < bin.size(); ++j) std::cout << (j ? "," : "") << bin[j];
        std::cout << "]";
    }
    std::cout << "\nAll assertions passed.\n";
    return 0;
}
