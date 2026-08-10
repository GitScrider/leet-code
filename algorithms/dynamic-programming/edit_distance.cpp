/*
 * Edit Distance (Levenshtein Distance)
 * Category: Algorithm - Dynamic Programming
 *
 * Idea
 * ----
 * Minimum number of single-character edits -- INSERT, DELETE, or REPLACE -- to
 * turn string A (length m) into string B (length n). Each edit costs 1.
 *
 *   State:      dp[i][j] = edit distance between the prefixes A[0..i-1] and
 *                          B[0..j-1] (i chars of A, j chars of B).
 *   Recurrence: if A[i-1] == B[j-1]:
 *                   dp[i][j] = dp[i-1][j-1]                 // no edit needed
 *               else:
 *                   dp[i][j] = 1 + min( dp[i-1][j],         // delete A[i-1]
 *                                       dp[i][j-1],         // insert B[j-1]
 *                                       dp[i-1][j-1] )      // replace
 *   Base cases: dp[i][0] = i   (delete all i chars of A to reach empty)
 *               dp[0][j] = j   (insert all j chars of B into empty A)
 *
 * Why it is correct
 * -----------------
 * Optimal substructure: consider the last operation that aligns the two
 * prefixes. Either the last characters match (fall through to dp[i-1][j-1]) or
 * the cheapest final edit is a delete, insert, or replace, each reducing to a
 * strictly smaller prefix pair. Overlapping subproblems: prefix pair (i, j) is
 * reached by many edit orderings, so we memoize it once.
 *
 * Complexity
 * ----------
 *   +-----------+-----------+---------------------------------+
 *   | Approach  | Time      | Space                           |
 *   +-----------+-----------+---------------------------------+
 *   | Tabulation| O(m * n)  | O(m * n)                         |
 *   | Rolling   | O(m * n)  | O(min(m, n))  (two rows only)   |
 *   +-----------+-----------+---------------------------------+
 * Only dp[i-1][*] and the current row are ever needed, so two rows of length
 * min(m,n)+1 suffice -- the O(min(m,n)) space optimization implemented below.
 *
 * Complexity derivation (double-loop cell count)
 * ----------------------------------------------
 *   The table has (m+1)*(n+1) cells. Base initialization sets row 0 and column 0
 *   in (m+1) + (n+1) steps of O(1). The fill then runs i = 1..m and, for each i,
 *   j = 1..n, doing O(1) work per cell (one char compare, then a 3-way min).
 *   Counting cell computations:
 *
 *       C(m,n) = SUM_{i=1}^{m} SUM_{j=1}^{n} c
 *              = SUM_{i=1}^{m} (c * n)
 *              = c * m * n
 *              = O(m * n)
 *
 *   The O(m + n) base setup is dominated by c*m*n. The rolling version computes
 *   the SAME m*n cells (only reusing two length-(min(m,n)+1) rows), so its time
 *   is identical O(m*n); it merely lowers space to O(min(m,n)). When both strings
 *   have length Theta(N) this reads as Theta(N^2).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight)
 * ------------------------------------------------------------
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f(N) = O(g)      iff  EXISTS c2, n0 :       f(N) <= c2*g(N)  for N >= n0
 *     f(N) = Omega(g)  iff  EXISTS c1, n0 :  c1*g(N) <= f(N)        for N >= n0
 *     f(N) = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   Both loops always run to their bounds: neither the a[i-1]==b[j-1] branch nor
 *   the string contents can shorten the m*n iterations (there is no early exit).
 *   Thus f(m,n) = c*m*n EXACTLY for every input. With g = m*n,
 *       c1*(m*n) <= f(m,n) <= c2*(m*n)      (here c1 = c2 = c)
 *   so the running time is Theta(m*n) -- best, average and worst all coincide.
 *
 * Key points
 * ----------
 *  - Bottom-up full table is the clearest form and can reconstruct operations.
 *  - The rolling-array version keeps only two rows; we assert it matches.
 *  - Indices use std::size_t; the string is only read with i-1/j-1 inside the
 *    i>0 && j>0 region, avoiding unsigned underflow.
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

// Full O(m*n) table version.
std::size_t editDistance(const std::string& a, const std::string& b) {
    const std::size_t m = a.size();
    const std::size_t n = b.size();

    std::vector<std::vector<std::size_t>> dp(m + 1,
                                             std::vector<std::size_t>(n + 1, 0));

    // Base cases: transforming to/from an empty prefix.
    for (std::size_t i = 0; i <= m; ++i) dp[i][0] = i;  // delete i chars
    for (std::size_t j = 0; j <= n; ++j) dp[0][j] = j;  // insert j chars

    for (std::size_t i = 1; i <= m; ++i) {
        for (std::size_t j = 1; j <= n; ++j) {
            if (a[i - 1] == b[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];  // characters already agree
            } else {
                const std::size_t del = dp[i - 1][j];      // delete A[i-1]
                const std::size_t ins = dp[i][j - 1];      // insert B[j-1]
                const std::size_t rep = dp[i - 1][j - 1];  // replace
                dp[i][j] = 1 + std::min({del, ins, rep});
            }
        }
    }
    return dp[m][n];
}

// Space-optimized O(min(m,n)) version using two rolling rows.
std::size_t editDistanceRolling(const std::string& a, const std::string& b) {
    // Iterate columns over the shorter string so each row has min(m,n)+1 cells.
    const std::string& s = (a.size() >= b.size()) ? a : b;  // rows indexed by s
    const std::string& t = (a.size() >= b.size()) ? b : a;  // cols indexed by t
    const std::size_t m = s.size();
    const std::size_t n = t.size();

    std::vector<std::size_t> prev(n + 1, 0);
    std::vector<std::size_t> cur(n + 1, 0);
    for (std::size_t j = 0; j <= n; ++j) prev[j] = j;  // base: empty s prefix

    for (std::size_t i = 1; i <= m; ++i) {
        cur[0] = i;  // base: empty t prefix, delete i chars of s
        for (std::size_t j = 1; j <= n; ++j) {
            if (s[i - 1] == t[j - 1]) {
                cur[j] = prev[j - 1];
            } else {
                cur[j] = 1 + std::min({prev[j], cur[j - 1], prev[j - 1]});
            }
        }
        std::swap(prev, cur);  // cur becomes prev for the next row
    }
    return prev[n];
}

// Brute-force reference: exhaustive recursion over the three edits (plus the
// free match). Exponential; only for tiny strings in tests.
std::size_t editBrute(const std::string& a, std::size_t i, const std::string& b,
                      std::size_t j) {
    if (i == a.size()) return b.size() - j;  // insert the rest of b
    if (j == b.size()) return a.size() - i;  // delete the rest of a
    if (a[i] == b[j]) return editBrute(a, i + 1, b, j + 1);
    const std::size_t del = editBrute(a, i + 1, b, j);
    const std::size_t ins = editBrute(a, i, b, j + 1);
    const std::size_t rep = editBrute(a, i + 1, b, j + 1);
    return 1 + std::min({del, ins, rep});
}

int main() {
    // Guidance cases.
    assert(editDistance("horse", "ros") == 3);
    assert(editDistance("intention", "execution") == 5);

    // Empty-string edges.
    assert(editDistance("", "") == 0);
    assert(editDistance("abc", "") == 3);   // three deletes
    assert(editDistance("", "abcd") == 4);  // four inserts

    // A few more known values.
    assert(editDistance("abc", "abc") == 0);
    assert(editDistance("kitten", "sitting") == 3);
    assert(editDistance("sunday", "saturday") == 3);

    // Rolling version must agree with the full table on all of the above.
    assert(editDistanceRolling("horse", "ros") == 3);
    assert(editDistanceRolling("intention", "execution") == 5);
    assert(editDistanceRolling("", "") == 0);
    assert(editDistanceRolling("abc", "") == 3);
    assert(editDistanceRolling("", "abcd") == 4);
    assert(editDistanceRolling("kitten", "sitting") == 3);

    // Cross-check table, rolling, and brute force on many tiny pairs.
    const char* pool[] = {"", "a", "ab", "ba", "abc",
                          "acb", "aa", "abab", "baa", "cab"};
    for (const char* pa : pool) {
        for (const char* pb : pool) {
            const std::string sa = pa;
            const std::string sb = pb;
            const std::size_t viaDp = editDistance(sa, sb);
            assert(viaDp == editDistanceRolling(sa, sb));
            assert(viaDp == editBrute(sa, 0, sb, 0));
        }
    }

    // Short demo.
    std::cout << "editDistance(\"horse\", \"ros\")           = "
              << editDistance("horse", "ros") << "\n";
    std::cout << "editDistance(\"intention\", \"execution\") = "
              << editDistance("intention", "execution") << "\n";
    std::cout << "All edit-distance tests passed.\n";
    return 0;
}
