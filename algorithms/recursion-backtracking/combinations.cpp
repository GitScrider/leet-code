/*
 * Combinations: all k-subsets of {1..n} - Algorithm - Recursion/Backtracking
 * =========================================================================
 *
 * Idea:
 *   Enumerate every way to choose k distinct numbers from {1, 2, ..., n}
 *   where ORDER DOES NOT MATTER (so {1,3} and {3,1} are the same combination,
 *   counted once). This is "n choose k" = C(n, k).
 *
 *   Backtracking structure (choose / explore / unchoose):
 *     - State: a partial combination `current` plus a `start` value marking
 *       the smallest number still allowed.
 *     - At each level we try every value v in [start, n], append it, recurse
 *       with start = v + 1, then remove it (backtrack).
 *     - START-INDEX PRUNING: by always continuing from v+1 we generate numbers
 *       in strictly increasing order. That is what prevents both duplicates
 *       ({1,3} vs {3,1}) and re-orderings -- each combination is produced once
 *       in its canonical ascending form.
 *     - BOUND PRUNING: if not enough numbers remain to finish a size-k
 *       combination, stop early (see the loop upper bound below).
 *
 * Complexity:
 *   +--------------------+----------------------+---------------------------+
 *   | Quantity           | Cost                 | Why                       |
 *   +--------------------+----------------------+---------------------------+
 *   | Time               | O(k * C(n, k))       | C(n,k) results, O(k) each |
 *   | Space (output)     | O(k * C(n, k))       | storing every combination |
 *   | Space (recursion)  | O(k)                 | recursion depth is k      |
 *   +--------------------+----------------------+---------------------------+
 *   C(n, k) can be exponential in n (it peaks around k = n/2, ~2^n / sqrt(n)),
 *   so listing all combinations is inherently expensive for large n. The bound
 *   pruning avoids descending into branches that can never reach size k.
 *
 * Complexity derivation (backtracking state-space tree):
 *   Each node appends one value; a root-to-leaf path has length k (depth = k),
 *   and each level branches over up to n candidates -- a NAIVE size bound is
 *   n^k nodes. START-INDEX pruning makes every path strictly ascending, so each
 *   distinct leaf is a distinct ascending k-subset: there are exactly C(n, k)
 *   leaves. Count the total work two ways:
 *
 *     Leaf copies: each of the C(n,k) full combinations is copied to `out`, and
 *       a combination holds k ints  ->  k * C(n, k).
 *
 *     Node visits: with BOUND pruning (last = n - needed + 1) we descend only
 *       into nodes that can still reach a size-k leaf, so every visited node is
 *       an ancestor of >= 1 leaf. Each leaf has exactly k ancestors, hence
 *           #nodes <= SUM_{leaves} (#ancestors) = k * C(n, k),
 *       and each node does O(1) choose/unchoose work -> O(k * C(n, k)).
 *
 *   Adding the two:  T(n,k) = O(k*C(n,k)) + O(k*C(n,k)) = O(k * C(n, k)).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f = O(g)      iff  EXISTS c2, n0 :       f(n) <= c2*g(n)  for n >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   The input is only the pair (n, k) -- there is no data to vary -- so the
 *   count is fixed: exactly C(n,k) combinations, k*C(n,k) integers of output.
 *   With g = k*C(n,k):
 *     upper  O:     work <= c2 * k*C(n,k)          => O(k * C(n, k))
 *     lower  Omega: emitting the output alone costs k*C(n,k) ints
 *                                                  => Omega(k * C(n, k))
 *     tight  Theta: both hold                      => Theta(k * C(n, k))
 *   So best = worst here (no data-dependent branch). The Omega is inherent
 *   (output size), not a property of this code; the comparison-sort Omega(n log
 *   n) bound does not apply -- this enumerates, it does not sort. Note C(n,k)
 *   itself is exponential near k = n/2 (~2^n / sqrt(n)).
 *
 * Key points / when to use:
 *   - Use when you need every fixed-size selection: lottery tickets, choosing
 *     committees, generating candidate feature subsets of a given size.
 *   - The start index is the whole trick: it enforces a canonical ordering so
 *     no combination is emitted twice.
 *   - Prefer this over "generate all subsets then filter by size" -- pruning
 *     to size k is dramatically cheaper.
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

// Exact C(n, k) computed iteratively without overflow for small inputs.
// Used only to VERIFY the enumeration count in the tests.
unsigned long long choose(unsigned n, unsigned k) {
    if (k > n) return 0;
    if (k > n - k) k = n - k;         // symmetry: C(n,k) == C(n,n-k)
    unsigned long long result = 1;
    for (unsigned i = 0; i < k; ++i) {
        result = result * (n - i) / (i + 1); // stays integral at each step
    }
    return result;
}

// Backtracking core. `start` is the smallest number we may still pick.
void combineRec(int n, int k, int start, std::vector<int>& current,
                std::vector<std::vector<int>>& out) {
    if (static_cast<int>(current.size()) == k) { // base case: a full combination
        out.push_back(current);
        return;
    }
    // Bound pruning: we still need (k - current.size()) more numbers. The
    // largest first value worth trying is n - (needed - 1); beyond that there
    // are not enough numbers left in [v, n] to complete the combination.
    const int needed = k - static_cast<int>(current.size());
    const int last = n - needed + 1;
    for (int v = start; v <= last; ++v) {
        current.push_back(v);                     // choose v
        combineRec(n, k, v + 1, current, out);    // explore: next value > v
        current.pop_back();                       // UNDO the choice (backtrack)
    }
}

std::vector<std::vector<int>> combine(int n, int k) {
    std::vector<std::vector<int>> out;
    std::vector<int> current;
    if (k >= 0 && k <= n) combineRec(n, k, 1, current, out);
    return out;
}

int main() {
    // --- Test 1: count equals C(n, k) across several cases ---
    struct Case { unsigned n, k; };
    for (const Case c : {Case{4, 2}, Case{5, 3}, Case{6, 0},
                         Case{6, 6}, Case{7, 3}, Case{10, 4}}) {
        const auto result = combine(static_cast<int>(c.n),
                                    static_cast<int>(c.k));
        assert(result.size() == choose(c.n, c.k));
    }

    // --- Test 2: every combination is strictly increasing (canonical form) ---
    // This is what guarantees no duplicates / no re-orderings.
    {
        const auto result = combine(5, 3);
        for (const auto& comb : result) {
            assert(static_cast<int>(comb.size()) == 3);
            for (std::size_t i = 1; i < comb.size(); ++i) {
                assert(comb[i - 1] < comb[i]);
            }
        }
    }

    // --- Test 3: exact expected result for a tiny instance C(4,2) = 6 ---
    {
        const std::vector<std::vector<int>> expected = {
            {1, 2}, {1, 3}, {1, 4}, {2, 3}, {2, 4}, {3, 4}
        };
        assert(combine(4, 2) == expected); // depends on the ascending traversal
    }

    // --- Test 4: k = 0 => exactly one (empty) combination; k > n => none ---
    {
        assert(combine(3, 0).size() == 1);
        assert(combine(3, 0).front().empty());
        assert(combine(3, 5).empty());
    }

    // --- Short demo: all 2-combinations of {1..4} ---
    std::cout << "C(4, 2) combinations:\n";
    for (const auto& comb : combine(4, 2)) {
        std::cout << "  { ";
        for (int x : comb) std::cout << x << ' ';
        std::cout << "}\n";
    }

    std::cout << "\nAll combination tests passed.\n";
    return 0;
}
