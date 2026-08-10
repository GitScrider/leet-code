/*
 * Rod Cutting - Algorithm - Dynamic Programming
 * =============================================
 *
 * Idea:
 *   A rod of integer length n can be cut into integer pieces. A piece of length
 *   c sells for price[c] (price is 1-indexed: price[1..n]). Choose cuts to
 *   maximize total revenue. Pieces are reusable in the sense that any length may
 *   appear any number of times -- this is UNBOUNDED knapsack with weight == the
 *   piece length and capacity == n.
 *
 *   State:  dp[l] = maximum revenue obtainable from a rod of length l.
 *   Base:   dp[0] = 0 (a length-0 rod earns nothing).
 *   Recur:  dp[l] = max over first-cut length c in [1..l] of
 *                       price[c] + dp[l - c]
 *   Why correct: every optimal cutting of length l makes some FIRST piece of
 *   length c (1 <= c <= l); the remaining length l - c is then cut optimally
 *   and independently (optimal substructure). Trying every possible first piece
 *   and reusing the already-computed dp[l - c] gives the optimum. Subproblems
 *   overlap heavily (dp[l - c] is shared across many l), so we tabulate.
 *
 *   RECONSTRUCTION:
 *     Store cut[l] = the length c that achieved dp[l]. Then peel pieces:
 *     l -> l - cut[l] -> ... -> 0, collecting cut[l] each step.
 *
 * Complexity:
 *   +---------------------+-----------------+---------------------------------+
 *   | Quantity            | Time            | Space                           |
 *   +---------------------+-----------------+---------------------------------+
 *   | Revenue DP          | O(n^2)          | O(n)                            |
 *   | + reconstruction    | O(n) extra      | O(n) for the cut[] table        |
 *   +---------------------+-----------------+---------------------------------+
 *
 * Key points:
 *   - Unbounded knapsack specialized to value == price[length]; the length loop
 *     is naturally ascending so pieces may repeat.
 *   - Keeping a `cut[]` choice array turns "how much" into "which cuts".
 *   - Verified against the classic CLRS price table (n=4 -> 10) and brute force.
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

// Maximum revenue for a rod of length n. `price` is 1-indexed: price[c] is the
// value of a piece of length c, for c in [1..n] (price.size() must be >= n+1).
int rodCutting(const std::vector<int>& price, int n) {
    std::vector<int> dp(static_cast<std::size_t>(n) + 1, 0);
    for (int l = 1; l <= n; ++l) {
        int best = 0;
        for (int c = 1; c <= l; ++c) {
            const int cand = price[static_cast<std::size_t>(c)] +
                             dp[static_cast<std::size_t>(l - c)];
            best = std::max(best, cand);
        }
        dp[static_cast<std::size_t>(l)] = best;
    }
    return dp[static_cast<std::size_t>(n)];
}

// Same DP but also records the first-cut choice so we can list the pieces.
// `cuts` is filled with piece lengths that sum to n and realize the optimum.
int rodCuttingWithCuts(const std::vector<int>& price, int n,
                       std::vector<int>& cuts) {
    std::vector<int> dp(static_cast<std::size_t>(n) + 1, 0);
    std::vector<int> choice(static_cast<std::size_t>(n) + 1, 0);  // best first cut
    for (int l = 1; l <= n; ++l) {
        int best = 0;
        int bestC = 0;
        for (int c = 1; c <= l; ++c) {
            const int cand = price[static_cast<std::size_t>(c)] +
                             dp[static_cast<std::size_t>(l - c)];
            if (cand > best) {
                best = cand;
                bestC = c;
            }
        }
        dp[static_cast<std::size_t>(l)] = best;
        choice[static_cast<std::size_t>(l)] = bestC;
    }
    // Peel pieces from length n down to 0 using the recorded choices.
    cuts.clear();
    int l = n;
    while (l > 0) {
        const int c = choice[static_cast<std::size_t>(l)];
        cuts.push_back(c);
        l -= c;
    }
    return dp[static_cast<std::size_t>(n)];
}

// Brute-force oracle: try every first cut recursively. Exponential; small n.
int rodCuttingBrute(const std::vector<int>& price, int n) {
    if (n == 0) return 0;
    int best = 0;
    for (int c = 1; c <= n; ++c) {
        best = std::max(best,
                        price[static_cast<std::size_t>(c)] +
                            rodCuttingBrute(price, n - c));
    }
    return best;
}

int main() {
    // Classic CLRS price table (index 0 unused; price[i] for length i).
    //   len :  1  2  3  4   5   6   7   8
    //   $   :  1  5  8  9  10  17  17  20
    const std::vector<int> price = {0, 1, 5, 8, 9, 10, 17, 17, 20};

    // --- Known CLRS answers ---
    assert(rodCutting(price, 4) == 10);  // two pieces of length 2 (5 + 5)
    assert(rodCutting(price, 8) == 22);  // 2 + 6 (5 + 17)
    assert(rodCutting(price, 1) == 1);
    assert(rodCutting(price, 0) == 0);   // zero-length rod

    // --- Reconstruction: pieces must sum to n and realize the revenue ---
    {
        std::vector<int> cuts;
        const int rev = rodCuttingWithCuts(price, 4, cuts);
        assert(rev == 10);
        int sum = 0;
        for (const int c : cuts) sum += c;
        assert(sum == 4);            // pieces tile the whole rod
        // Recompute revenue from the returned pieces to confirm consistency.
        int check = 0;
        for (const int c : cuts) check += price[static_cast<std::size_t>(c)];
        assert(check == rev);
    }

    // --- Cross-check against brute force over the whole table range ---
    for (int n = 0; n + 1 <= static_cast<int>(price.size()); ++n) {
        assert(rodCutting(price, n) == rodCuttingBrute(price, n));
    }

    // --- A table where NOT cutting is optimal (super-additive prices) ---
    {
        const std::vector<int> p = {0, 1, 5, 8, 9, 10, 17, 17, 100};  // len 8 = 100
        std::vector<int> cuts;
        const int rev = rodCuttingWithCuts(p, 8, cuts);
        assert(rev == 100);
        assert(cuts.size() == 1 && cuts[0] == 8);  // keep the rod whole
    }

    // --- Demo ---
    std::vector<int> cuts;
    const int rev = rodCuttingWithCuts(price, 8, cuts);
    std::cout << "rodCutting(CLRS price, n=4) = " << rodCutting(price, 4)
              << "  (expected 10)\n";
    std::cout << "rodCutting(CLRS price, n=8) = " << rev << "  (expected 22)\n";
    std::cout << "  optimal pieces for n=8: { ";
    for (const int c : cuts) std::cout << c << ' ';
    std::cout << "}\n";

    std::cout << "\nAll rod-cutting tests passed.\n";
    return 0;
}
