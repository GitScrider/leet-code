/*
 * Subsets (Power Set) - Algorithm - Recursion/Backtracking
 * ========================================================
 *
 * Idea:
 *   The power set of a set of n elements is the collection of ALL its subsets.
 *   We build subsets with the classic INCLUDE / EXCLUDE recursion: at every
 *   index i we make a binary choice for element a[i]:
 *       - EXCLUDE it: recurse to i+1 without adding it, or
 *       - INCLUDE it: add it, recurse to i+1, then REMOVE it (backtrack).
 *   Because each of the n elements is independently in-or-out, the recursion
 *   tree has 2^n leaves, one per subset.
 *
 *   Handling duplicates (unique subsets):
 *   When the input may contain repeated values we first SORT it so equal
 *   elements are adjacent. We then use the "subsets-with-index" enumeration
 *   (choose how many copies / from which start position) and skip a candidate
 *   at the same recursion depth when it equals the previous, already-tried
 *   sibling. That prunes the duplicate branches so each distinct subset is
 *   emitted exactly once.
 *
 * Complexity:
 *   +--------------------+------------------+-------------------------------+
 *   | Quantity           | Cost             | Why                           |
 *   +--------------------+------------------+-------------------------------+
 *   | Time (distinct)    | O(n * 2^n)       | 2^n subsets, O(n) to copy one |
 *   | Space (output)     | O(n * 2^n)       | storing every subset          |
 *   | Space (recursion)  | O(n)             | depth of the call stack       |
 *   +--------------------+------------------+-------------------------------+
 *   Exponential is INHERENT here: there are 2^n subsets, so no algorithm can
 *   list them faster than that. Duplicate pruning only removes redundant work,
 *   it does not change the asymptotic class for distinct input.
 *
 * Key points / when to use:
 *   - Use when you must ENUMERATE all subsets (feature selection, bitmask DP
 *     seeds, brute-force search over combinations of options).
 *   - Two mental models: include/exclude recursion vs. start-index expansion;
 *     the latter makes duplicate handling natural.
 *   - Sort + "skip equal sibling" is the canonical de-duplication trick used
 *     across subsets / combinations / permutations with repeats.
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

// -------- Version 1: include/exclude recursion (distinct input) -----------
// At index `i` we decide element a[i]: recurse excluding it, then recurse
// including it. `current` is shared and backtracked (no per-call copies).
void subsetsIncludeExclude(const std::vector<int>& a, std::size_t i,
                           std::vector<int>& current,
                           std::vector<std::vector<int>>& out) {
    if (i == a.size()) {          // base case: a decision was made for every element
        out.push_back(current);   // record this leaf of the 2^n recursion tree
        return;
    }
    // Choice A: EXCLUDE a[i] -- do not touch `current`, just move on.
    subsetsIncludeExclude(a, i + 1, current, out);

    // Choice B: INCLUDE a[i].
    current.push_back(a[i]);                    // make the choice
    subsetsIncludeExclude(a, i + 1, current, out); // explore
    current.pop_back();                         // UNDO the choice (backtrack)
}

std::vector<std::vector<int>> subsets(const std::vector<int>& a) {
    std::vector<std::vector<int>> out;
    std::vector<int> current;
    subsetsIncludeExclude(a, 0, current, out);
    return out;
}

// -------- Version 2: start-index expansion with duplicate pruning ---------
// Input is sorted. From position `start` we try each element as the next one
// to append. Skipping equal siblings (j > start && a[j] == a[j-1]) removes
// duplicate subsets when the input has repeated values.
void subsetsUniqueRec(const std::vector<int>& a, std::size_t start,
                      std::vector<int>& current,
                      std::vector<std::vector<int>>& out) {
    out.push_back(current);       // every node (not just leaves) is a valid subset
    for (std::size_t j = start; j < a.size(); ++j) {
        // Pruning: at this depth, do not start a branch with a value we already
        // used as a sibling -- it would regenerate an identical subset.
        if (j > start && a[j] == a[j - 1]) continue;

        current.push_back(a[j]);                     // choose a[j]
        subsetsUniqueRec(a, j + 1, current, out);    // explore the rest
        current.pop_back();                          // backtrack
    }
}

std::vector<std::vector<int>> subsetsUnique(std::vector<int> a) {
    std::sort(a.begin(), a.end());   // bring equal elements together for pruning
    std::vector<std::vector<int>> out;
    std::vector<int> current;
    subsetsUniqueRec(a, 0, current, out);
    return out;
}

int main() {
    // --- Test 1: distinct input => exactly 2^n subsets ---
    {
        const std::vector<int> a = {1, 2, 3};
        const auto result = subsets(a);
        const std::size_t expected = std::size_t(1) << a.size(); // 2^3 = 8
        assert(result.size() == expected);
    }
    {
        const std::vector<int> a = {5, 6, 7, 8, 9}; // n = 5
        const auto result = subsets(a);
        assert(result.size() == (std::size_t(1) << a.size())); // 2^5 = 32
    }

    // --- Test 2: the two versions agree in COUNT on distinct input ---
    {
        const std::vector<int> a = {1, 2, 3, 4};
        assert(subsets(a).size() == subsetsUnique(a).size()); // both 16
    }

    // --- Test 3: duplicates => only UNIQUE subsets ---
    // {1,2,2}: unique subsets are {},{1},{2},{1,2},{2,2},{1,2,2} = 6.
    {
        const std::vector<int> a = {1, 2, 2};
        const auto result = subsetsUnique(a);
        assert(result.size() == 6);
        // Confirm every produced subset is genuinely unique.
        auto sorted = result;
        std::sort(sorted.begin(), sorted.end());
        assert(std::unique(sorted.begin(), sorted.end()) == sorted.end());
    }

    // --- Test 4: empty set => exactly one subset, the empty subset ---
    {
        const std::vector<int> a;
        const auto result = subsets(a);
        assert(result.size() == 1);
        assert(result.front().empty());
    }

    // --- Short demo: print the power set of {1,2,3} ---
    std::cout << "Power set of {1, 2, 3}:\n";
    for (const auto& subset : subsets({1, 2, 3})) {
        std::cout << "  { ";
        for (int x : subset) std::cout << x << ' ';
        std::cout << "}\n";
    }

    std::cout << "\nUnique subsets of {1, 2, 2}:\n";
    for (const auto& subset : subsetsUnique({1, 2, 2})) {
        std::cout << "  { ";
        for (int x : subset) std::cout << x << ' ';
        std::cout << "}\n";
    }

    std::cout << "\nAll subset tests passed.\n";
    return 0;
}
