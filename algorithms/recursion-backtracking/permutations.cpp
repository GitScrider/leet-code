/*
 * ============================================================================
 * Permutations (with duplicate handling -> DISTINCT permutations only)
 * Category: Algorithm - Backtracking
 *
 * Idea (choose / explore / unchoose):
 *   Build a permutation one position at a time. At each step we CHOOSE an
 *   unused element to occupy the current slot, EXPLORE (recurse to fill the
 *   remaining slots), then UNCHOOSE (undo the choice) so the next iteration can
 *   try a different element in that slot. When every slot is filled we have one
 *   complete permutation -- the BASE CASE.
 *
 *   Handling DUPLICATES: naively permuting [1,1,2] would emit the same tuple
 *   several times. Two standard fixes, both shown below:
 *     (A) used[] array + sort: sort first, then at each slot skip a value equal
 *         to its predecessor when that predecessor is NOT currently used. This
 *         forces equal elements to be placed in their sorted left-to-right
 *         order, so each distinct arrangement is generated exactly once.
 *     (B) swap-based: fix each position by swapping candidates into it; at a
 *         given slot keep a "seen" set of values already tried there and skip
 *         repeats. No sorting needed; produces the same DISTINCT set.
 *
 * Complexity:
 *   +-----------+------------------------------------------------------------+
 *   | Time      | O(n * P) where P = number of DISTINCT permutations =       |
 *   |           | n! / (m1! * m2! * ...). All-distinct input => O(n * n!).   |
 *   | Space     | O(n) recursion depth + O(n) for used[]/seen; O(n * P) to   |
 *   |           | STORE all results.                                         |
 *   +-----------+------------------------------------------------------------+
 *   WHY (near-)factorial / exponential: there are P leaves in the search tree
 *   and copying each length-n permutation into the output costs O(n). The skip
 *   rules PRUNE branches that would regenerate an already-seen arrangement,
 *   bringing the count down from n! to exactly n!/(product of multiplicities!).
 *
 * Key points / when to use:
 *   - Canonical backtracking template: choose -> recurse -> undo.
 *   - Sort + "skip equal-to-previous-when-unused" is the idiomatic dedup rule.
 *   - Prefer the used[] form for clarity; the swap form avoids an extra array
 *     but mutates (and restores) the input in place.
 * ============================================================================
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <set>
#include <unordered_set>
#include <vector>

// n! as an exact integer (n small here). Used only to derive the expected
// number of distinct permutations for the tests.
unsigned long long factorial(unsigned n) {
    unsigned long long f = 1;
    for (unsigned k = 2; k <= n; ++k) f *= k;
    return f;
}

// Expected number of DISTINCT permutations = n! / (m1! * m2! * ...), where mi
// are the multiplicities of the distinct values.
unsigned long long distinctPermCount(std::vector<int> nums) {
    std::sort(nums.begin(), nums.end());
    unsigned long long total = factorial(static_cast<unsigned>(nums.size()));
    std::size_t i = 0;
    while (i < nums.size()) {
        std::size_t j = i;
        while (j < nums.size() && nums[j] == nums[i]) ++j;   // run of equal values
        total /= factorial(static_cast<unsigned>(j - i));    // divide out mi!
        i = j;
    }
    return total;
}

// --------------------------------------------------------------------------
// Approach (A): used[] array + sort + skip.
// --------------------------------------------------------------------------
void permuteUsedImpl(const std::vector<int>& nums, std::vector<bool>& used,
                     std::vector<int>& current,
                     std::vector<std::vector<int>>& out) {
    if (current.size() == nums.size()) {        // BASE CASE: a full permutation
        out.push_back(current);
        return;
    }
    for (std::size_t i = 0; i < nums.size(); ++i) {
        if (used[i]) continue;                  // this element is already placed
        // DEDUP PRUNE: skip a duplicate value whose identical predecessor has
        // not been used on this path -- that arrangement is produced by the
        // branch where the predecessor comes first, so taking it here repeats.
        if (i > 0 && nums[i] == nums[i - 1] && !used[i - 1]) continue;

        used[i] = true;                         // CHOOSE
        current.push_back(nums[i]);
        permuteUsedImpl(nums, used, current, out);  // EXPLORE
        current.pop_back();                     // UNCHOOSE (backtrack)
        used[i] = false;
    }
}
std::vector<std::vector<int>> permuteUnique(std::vector<int> nums) {
    std::sort(nums.begin(), nums.end());        // required for the skip rule
    std::vector<bool> used(nums.size(), false);
    std::vector<int> current;
    std::vector<std::vector<int>> out;
    permuteUsedImpl(nums, used, current, out);
    return out;
}

// --------------------------------------------------------------------------
// Approach (B): swap-based. Fix position `start` by swapping each distinct
// candidate into it, recurse on the suffix, then swap back to restore state.
// --------------------------------------------------------------------------
void permuteSwapImpl(std::vector<int>& nums, std::size_t start,
                     std::vector<std::vector<int>>& out) {
    if (start == nums.size()) {                 // BASE CASE: all slots fixed
        out.push_back(nums);
        return;
    }
    std::unordered_set<int> seen;               // values already tried at `start`
    for (std::size_t i = start; i < nums.size(); ++i) {
        if (seen.count(nums[i])) continue;      // DEDUP PRUNE: same value here already
        seen.insert(nums[i]);

        std::swap(nums[start], nums[i]);        // CHOOSE: put nums[i] at `start`
        permuteSwapImpl(nums, start + 1, out);  // EXPLORE the remaining suffix
        std::swap(nums[start], nums[i]);        // UNCHOOSE (backtrack the swap)
    }
}
std::vector<std::vector<int>> permuteUniqueSwap(std::vector<int> nums) {
    std::vector<std::vector<int>> out;
    permuteSwapImpl(nums, 0, out);
    return out;
}

// Convert a list of permutations into a set (also proves internal distinctness
// when compared against the list length).
std::set<std::vector<int>> toSet(const std::vector<std::vector<int>>& v) {
    return std::set<std::vector<int>>(v.begin(), v.end());
}

// A produced tuple must be a genuine rearrangement of the input multiset.
bool isPermutationOf(std::vector<int> a, std::vector<int> b) {
    std::sort(a.begin(), a.end());
    std::sort(b.begin(), b.end());
    return a == b;
}

int main() {
    const std::vector<std::vector<int>> inputs = {
        {},                 // 0! = 1 (the single empty permutation)
        {7},                // 1
        {1, 2, 3},          // 3! = 6, all distinct
        {1, 2, 3, 4},       // 4! = 24
        {1, 1, 2},          // 3! / 2! = 3
        {1, 1, 2, 2},       // 4! / (2! 2!) = 6
        {2, 2, 2},          // 3! / 3! = 1
        {3, 3, 1, 1},       // 4! / (2! 2!) = 6
    };

    for (const auto& in : inputs) {
        const unsigned long long expected = distinctPermCount(in);

        auto viaUsed = permuteUnique(in);
        auto viaSwap = permuteUniqueSwap(in);

        // (1) Count matches n! / (product of multiplicities!).
        assert(viaUsed.size() == expected);
        assert(viaSwap.size() == expected);

        // (2) Every result is internally DISTINCT (set size == list size).
        auto usedSet = toSet(viaUsed);
        auto swapSet = toSet(viaSwap);
        assert(usedSet.size() == viaUsed.size());
        assert(swapSet.size() == viaSwap.size());

        // (3) Both approaches enumerate exactly the SAME set of permutations.
        assert(usedSet == swapSet);

        // (4) Each tuple is a true rearrangement of the input multiset.
        for (const auto& p : viaUsed) assert(isPermutationOf(p, in));
    }

    // Explicit expected set for a small duplicate case (order-independent).
    {
        auto perms = toSet(permuteUnique({1, 1, 2}));
        std::set<std::vector<int>> expected = {
            {1, 1, 2}, {1, 2, 1}, {2, 1, 1}
        };
        assert(perms == expected);
    }

    // The used[] approach returns results in lexicographically sorted order
    // (a nice side effect of sorting the input first).
    {
        auto perms = permuteUnique({3, 1, 2});
        assert(std::is_sorted(perms.begin(), perms.end()));
        assert(perms.front() == (std::vector<int>{1, 2, 3}));
        assert(perms.back() == (std::vector<int>{3, 2, 1}));
    }

    // ---- short demo --------------------------------------------------------
    std::cout << "Permutations demo\n";
    std::cout << "  distinct permutations of [1,1,2] ("
              << distinctPermCount({1, 1, 2}) << " total):\n";
    for (const auto& p : permuteUnique({1, 1, 2})) {
        std::cout << "    [";
        for (std::size_t i = 0; i < p.size(); ++i) {
            std::cout << p[i] << (i + 1 < p.size() ? ", " : "");
        }
        std::cout << "]\n";
    }
    std::cout << "  count of permutations of [1,2,3,4] = "
              << permuteUnique({1, 2, 3, 4}).size() << "  (4! = 24)\n";
    std::cout << "All assertions passed.\n";
    return 0;
}
