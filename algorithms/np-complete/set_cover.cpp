/*
 * Set Cover (Algorithm - NP-Complete / NP-Hard)
 * =============================================
 *
 * Decision problem (SET-COVER):
 *   Given a universe U = {0, 1, ..., n-1}, a collection S = {S_1, ..., S_m} of
 *   subsets of U (with union(S) = U), and an integer k, is there a sub-family
 *   of at most k sets whose union equals U?
 *
 * Optimization problem (MINIMUM SET COVER):
 *   Find the SMALLEST sub-family of S whose union is all of U.
 *
 * Complexity class:
 *   - SET-COVER (decision) is NP-complete: a chosen sub-family is a
 *     polynomial-time-checkable witness (so it is in NP), and it is NP-hard by
 *     a classic reduction from VERTEX COVER (each vertex -> the set of edges it
 *     covers; a vertex cover of size k <=> a set cover of size k).
 *   - MINIMUM SET COVER (optimization) is NP-hard for the same reason.
 *
 * Exact vs. greedy:
 *   +------------------+---------------------------------------------------------+
 *   | Exact (enumerate | O(2^m * m) : try every subset of the m sets, cheapest   |
 *   |  sub-families)   | valid cover wins. Feasible only for small m.            |
 *   |                  | Each candidate is a bitmask over the m sets; we OR the  |
 *   |                  | precomputed element-masks to test full coverage.        |
 *   +------------------+---------------------------------------------------------+
 *   | Greedy heuristic | O(m * n) per demo instance. Repeatedly pick the set     |
 *   |                  | covering the MOST still-uncovered elements.             |
 *   +------------------+---------------------------------------------------------+
 *   Greedy guarantee: greedy_size <= H(n) * OPT, where the harmonic number
 *   H(n) = 1 + 1/2 + ... + 1/n ~ ln n + 1. This ln n factor is essentially
 *   optimal: unless P = NP, no polynomial algorithm approximates set cover to
 *   within (1 - epsilon) * ln n -- the ln n inapproximability barrier
 *   (Dinur-Steurer). So greedy is, up to constants, the best we can do fast.
 *
 * Key points:
 *   - Elements-covered are tracked as a bitmask over std::uint32_t: bit i set
 *     means element i is covered. "Full universe" is the mask (1<<n) - 1.
 *   - The exact solver enumerates set-subsets (bitmask over m sets); the greedy
 *     one enumerates elements. Two different bit domains -- keep them straight.
 *   - Greedy is NOT always optimal; we assert only that it stays within the
 *     proven H(n) factor of the exact optimum (a strict cover is still valid).
 */

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <vector>

// A "set" is the list of universe elements it contains (0-indexed).
using SetList = std::vector<std::vector<int>>;

// Encode one set as a bitmask over its elements: bit e set => element e is in
// the set. Requires n <= 32 (we use uint32_t). Comment the bit trick inline.
static std::uint32_t maskOf(const std::vector<int> &s) {
    std::uint32_t m = 0;
    for (const int e : s) {
        // Set bit e: OR in a 1 shifted left by e. 1u forces unsigned so the
        // shift stays well-defined for e up to 31.
        m |= (1u << e);
    }
    return m;
}

// The bitmask representing the full universe {0, ..., n-1}: n low bits set.
// Guard n == 32 (shifting a 32-bit value by 32 is undefined behavior).
static std::uint32_t fullUniverse(std::size_t n) {
    if (n >= 32) return std::numeric_limits<std::uint32_t>::max();
    return (1u << n) - 1u; // e.g. n=3 -> 0b111
}

// EXACT minimum set cover via subset enumeration over the m given sets.
// Returns the chosen set indices of a minimum cover, or empty if none exists
// (only possible when the sets do not jointly cover the universe).
static std::vector<int> exactSetCover(const SetList &sets, std::size_t n) {
    const std::size_t m = sets.size();
    assert(m <= 20 && "exact solver enumerates 2^m; keep m small");

    std::vector<std::uint32_t> setMask(m);
    for (std::size_t i = 0; i < m; ++i) setMask[i] = maskOf(sets[i]);

    const std::uint32_t target = fullUniverse(n);
    std::size_t bestCount = std::numeric_limits<std::size_t>::max();
    std::uint32_t bestChoice = 0;

    // Iterate over every sub-family, encoded as a bitmask over the m sets:
    // bit i set => set i is chosen. 2^m candidates total.
    const std::uint32_t combos = (m >= 32) ? 0u : (1u << m);
    for (std::uint32_t choice = 0; choice < combos; ++choice) {
        std::uint32_t covered = 0;
        std::size_t count = 0;
        for (std::size_t i = 0; i < m; ++i) {
            if (choice & (1u << i)) {   // is set i in this sub-family?
                covered |= setMask[i];  // union of chosen sets
                ++count;
            }
        }
        // A valid cover reaches every universe bit with fewer sets than best.
        if (covered == target && count < bestCount) {
            bestCount = count;
            bestChoice = choice;
        }
    }

    std::vector<int> result;
    if (bestCount == std::numeric_limits<std::size_t>::max())
        return result; // universe not coverable by the given sets
    for (std::size_t i = 0; i < m; ++i)
        if (bestChoice & (1u << i)) result.push_back(static_cast<int>(i));
    return result;
}

// GREEDY set cover: repeatedly take the set covering the most currently
// uncovered elements. H(n)-approximation. Returns chosen set indices, or empty
// if the sets cannot cover the universe.
static std::vector<int> greedySetCover(const SetList &sets, std::size_t n) {
    const std::size_t m = sets.size();
    std::vector<std::uint32_t> setMask(m);
    for (std::size_t i = 0; i < m; ++i) setMask[i] = maskOf(sets[i]);

    const std::uint32_t target = fullUniverse(n);
    std::uint32_t covered = 0;
    std::vector<int> chosen;
    std::vector<char> used(m, 0);

    while (covered != target) {
        std::size_t best = m;            // index of the greediest set
        int bestGain = 0;                // new elements it would cover
        for (std::size_t i = 0; i < m; ++i) {
            if (used[i]) continue;
            // Bits set in setMask[i] but NOT yet in covered = fresh elements.
            const std::uint32_t fresh = setMask[i] & ~covered;
            // __builtin_popcount counts set bits; portable manual count keeps
            // this standard-library-only (no compiler intrinsics required).
            int gain = 0;
            for (std::uint32_t b = fresh; b; b &= (b - 1)) ++gain; // clear lowest set bit
            if (gain > bestGain) { bestGain = gain; best = i; }
        }
        if (best == m) break; // no set adds anything new -> cannot finish
        used[best] = 1;
        covered |= setMask[best];
        chosen.push_back(static_cast<int>(best));
    }

    if (covered != target) return {}; // universe not coverable
    return chosen;
}

// Validate that a chosen sub-family really covers the whole universe.
static bool coversUniverse(const SetList &sets, std::size_t n,
                           const std::vector<int> &choice) {
    std::uint32_t covered = 0;
    for (const int i : choice) covered |= maskOf(sets[static_cast<std::size_t>(i)]);
    return covered == fullUniverse(n);
}

// Harmonic number H(n) = 1 + 1/2 + ... + 1/n, the greedy approximation factor.
static double harmonic(std::size_t n) {
    double h = 0.0;
    for (std::size_t i = 1; i <= n; ++i) h += 1.0 / static_cast<double>(i);
    return h;
}

int main() {
    // --- Instance 1: greedy is FORCED to be suboptimal (classic example) ---
    // Universe {0..5}. Two sets of 3 perfectly partition it (OPT = 2), but a
    // greedy tie/size bias can be lured into extra picks on harder inputs.
    {
        const std::size_t n = 6;
        SetList sets = {
            {0, 1, 2},          // set 0
            {3, 4, 5},          // set 1
            {0, 1, 2, 3},       // set 2 (largest -> greedy grabs it first)
            {4, 5}              // set 3
        };
        std::vector<int> exact = exactSetCover(sets, n);
        assert(exact.size() == 2); // {0,1} covers everything with 2 sets
        assert(coversUniverse(sets, n, exact));

        std::vector<int> greedy = greedySetCover(sets, n);
        assert(coversUniverse(sets, n, greedy));        // witness is valid
        // Greedy first grabs S2 (4 fresh elements), then S1 (adds 4,5); here it
        // matches OPT, but in general it may exceed it -- so we assert only the
        // one-directional facts: never better than exact, and within H(n)*OPT.
        assert(greedy.size() >= exact.size());           // never beats optimum
        assert(static_cast<double>(greedy.size()) <=
               harmonic(n) * static_cast<double>(exact.size())); // H(n) bound
    }

    // --- Instance 2: known unique optimum of size 3 ---
    // Universe {0..4}; only way to cover with 3 sets.
    {
        const std::size_t n = 5;
        SetList sets = {
            {0, 1},     // set 0
            {2, 3},     // set 1
            {4},        // set 2
            {0},        // set 3
            {1, 2}      // set 4
        };
        std::vector<int> exact = exactSetCover(sets, n);
        assert(exact.size() == 3);
        assert(coversUniverse(sets, n, exact));

        std::vector<int> greedy = greedySetCover(sets, n);
        assert(coversUniverse(sets, n, greedy));
        assert(greedy.size() >= exact.size());
        assert(static_cast<double>(greedy.size()) <=
               harmonic(n) * static_cast<double>(exact.size()));
    }

    // --- Instance 3: a single set already covers everything -> OPT = 1 ---
    {
        const std::size_t n = 4;
        SetList sets = {{0}, {0, 1, 2, 3}, {1, 2}};
        std::vector<int> exact = exactSetCover(sets, n);
        assert(exact.size() == 1);
        assert(coversUniverse(sets, n, exact));

        std::vector<int> greedy = greedySetCover(sets, n);
        assert(greedy.size() == 1); // greedy grabs the all-covering set first
        assert(coversUniverse(sets, n, greedy));
    }

    // --- Instance 4: sets that CANNOT cover the universe (element missing) ---
    {
        const std::size_t n = 3;                 // universe {0,1,2}
        SetList sets = {{0}, {1}};               // element 2 never appears
        std::vector<int> exact = exactSetCover(sets, n);
        assert(exact.empty());                   // no cover exists
        std::vector<int> greedy = greedySetCover(sets, n);
        assert(greedy.empty());                  // greedy also reports failure
    }

    // --- Edge case: empty universe is covered by the empty sub-family ---
    {
        const std::size_t n = 0;
        SetList sets = {{}, {}};
        std::vector<int> exact = exactSetCover(sets, n);
        assert(exact.empty());                   // 0 sets needed
        assert(coversUniverse(sets, n, exact));  // vacuously covers {}
    }

    // --- Randomized-ish cross-check: greedy is valid & within H(n) of exact ---
    {
        const std::size_t n = 5;
        const std::vector<SetList> instances = {
            {{0, 1, 2}, {2, 3}, {3, 4}, {0, 4}, {1}},
            {{0}, {1}, {2}, {3}, {4}, {0, 1, 2, 3, 4}},
            {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 0}},
        };
        for (const auto &sets : instances) {
            std::vector<int> exact = exactSetCover(sets, n);
            std::vector<int> greedy = greedySetCover(sets, n);
            assert(!exact.empty());
            assert(coversUniverse(sets, n, exact));
            assert(coversUniverse(sets, n, greedy));
            assert(greedy.size() >= exact.size());
            assert(static_cast<double>(greedy.size()) <=
                   harmonic(n) * static_cast<double>(exact.size()));
        }
    }

    // --- Short std::cout demo ---
    {
        const std::size_t n = 6;
        SetList sets = {{0, 1, 2}, {3, 4, 5}, {0, 1, 2, 3}, {4, 5}};
        std::vector<int> exact = exactSetCover(sets, n);
        std::vector<int> greedy = greedySetCover(sets, n);
        std::cout << "Universe size " << n << ", " << sets.size() << " sets.\n";
        std::cout << "Exact minimum cover uses " << exact.size() << " sets: {";
        for (std::size_t i = 0; i < exact.size(); ++i)
            std::cout << (i ? ", " : "") << "S" << exact[i];
        std::cout << "}\n";
        std::cout << "Greedy cover uses " << greedy.size() << " sets: {";
        for (std::size_t i = 0; i < greedy.size(); ++i)
            std::cout << (i ? ", " : "") << "S" << greedy[i];
        std::cout << "}\n";
        std::cout << "Greedy guarantee H(" << n << ") = " << harmonic(n)
                  << " => greedy <= " << harmonic(n) * exact.size()
                  << " sets (heuristic may exceed the optimum).\n";
    }

    std::cout << "All assertions passed.\n";
    return 0;
}
