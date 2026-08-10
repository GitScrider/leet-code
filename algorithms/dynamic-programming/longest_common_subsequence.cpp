/*
 * Longest Common Subsequence (LCS)
 * Category: Algorithm - Dynamic Programming
 *
 * Idea
 * ----
 * Given two strings A (length m) and B (length n), find the longest sequence
 * that appears in BOTH as a subsequence (characters in order, not necessarily
 * contiguous).
 *
 *   State:      dp[i][j] = length of the LCS of the prefixes A[0..i-1] and
 *                          B[0..j-1] (i characters of A, j characters of B).
 *   Recurrence: if A[i-1] == B[j-1]:
 *                   dp[i][j] = dp[i-1][j-1] + 1        // extend the match
 *               else:
 *                   dp[i][j] = max(dp[i-1][j], dp[i][j-1])
 *   Base cases: dp[0][j] = dp[i][0] = 0                // empty prefix -> LCS 0
 *
 * Why it is correct
 * -----------------
 * Optimal substructure: the LCS of two prefixes either ends by matching the two
 * last characters (when they are equal, that pairing is always safe to take) or
 * it drops the last character of one of the prefixes. Overlapping subproblems:
 * the same (i, j) prefix pair is reached along many decision paths, so we cache
 * each once in the dp table.
 *
 * Complexity
 * ----------
 *   +-----------+--------------------+----------------------------------+
 *   | Approach  | Time               | Space                            |
 *   +-----------+--------------------+----------------------------------+
 *   | Tabulation| O(m * n)           | O(m * n)  (needed to reconstruct)|
 *   | Rolling   | O(m * n)           | O(min(m, n)) for length only     |
 *   +-----------+--------------------+----------------------------------+
 * Reconstruction requires the full table (or a re-derivation), so this file
 * keeps the O(m*n) table; the rolling-row trick only gives the length.
 *
 * Key points
 * ----------
 *  - Bottom-up tabulation with a (m+1) x (n+1) table; row/col 0 are the base.
 *  - Reconstruct one LCS by walking the table from dp[m][n] back to dp[0][0].
 *  - Indices use std::size_t; we always index the table with i/j and the string
 *    with i-1/j-1 inside the "i>0 && j>0" region, so no unsigned underflow.
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

// Returns the length of the LCS and, via out-parameter, one LCS string.
std::size_t lcsLength(const std::string& a, const std::string& b,
                      std::string& out) {
    const std::size_t m = a.size();
    const std::size_t n = b.size();

    // dp[i][j] over prefixes; extra row/column of zeros form the base case.
    std::vector<std::vector<std::size_t>> dp(m + 1,
                                             std::vector<std::size_t>(n + 1, 0));

    for (std::size_t i = 1; i <= m; ++i) {
        for (std::size_t j = 1; j <= n; ++j) {
            if (a[i - 1] == b[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
            }
        }
    }

    // Reconstruct one LCS by tracing the decisions backwards.
    std::string result;
    std::size_t i = m;
    std::size_t j = n;
    while (i > 0 && j > 0) {
        if (a[i - 1] == b[j - 1]) {
            result.push_back(a[i - 1]);  // this char is part of an LCS
            --i;
            --j;
        } else if (dp[i - 1][j] >= dp[i][j - 1]) {
            --i;  // move in the direction that preserved the optimum
        } else {
            --j;
        }
    }
    std::reverse(result.begin(), result.end());  // we built it back-to-front

    out = result;
    return dp[m][n];
}

// Space-optimized length-only variant using two rolling rows: O(min(m,n)) space.
std::size_t lcsLengthRolling(const std::string& a, const std::string& b) {
    // Ensure b is the shorter string so the rows have min(m,n)+1 entries.
    const std::string& s = (a.size() >= b.size()) ? a : b;
    const std::string& t = (a.size() >= b.size()) ? b : a;
    const std::size_t m = s.size();
    const std::size_t n = t.size();

    std::vector<std::size_t> prev(n + 1, 0);
    std::vector<std::size_t> cur(n + 1, 0);
    for (std::size_t i = 1; i <= m; ++i) {
        for (std::size_t j = 1; j <= n; ++j) {
            if (s[i - 1] == t[j - 1]) {
                cur[j] = prev[j - 1] + 1;
            } else {
                cur[j] = std::max(prev[j], cur[j - 1]);
            }
        }
        std::swap(prev, cur);
        std::fill(cur.begin(), cur.end(), std::size_t{0});
    }
    return prev[n];
}

// Brute-force reference: enumerate every subsequence of the shorter string and
// check whether it is also a subsequence of the other. Exponential; tiny inputs.
static bool isSubsequence(const std::string& sub, const std::string& s) {
    std::size_t k = 0;
    for (std::size_t idx = 0; idx < s.size() && k < sub.size(); ++idx) {
        if (s[idx] == sub[k]) ++k;
    }
    return k == sub.size();
}

std::size_t lcsBrute(const std::string& a, const std::string& b) {
    const std::string& shorter = (a.size() <= b.size()) ? a : b;
    const std::string& other = (a.size() <= b.size()) ? b : a;
    const std::size_t len = shorter.size();
    std::size_t best = 0;
    // Each bitmask selects a subset of positions (a candidate subsequence).
    for (std::size_t mask = 0; mask < (std::size_t{1} << len); ++mask) {
        std::string cand;
        for (std::size_t bit = 0; bit < len; ++bit) {
            if (mask & (std::size_t{1} << bit)) cand.push_back(shorter[bit]);
        }
        if (isSubsequence(cand, other)) best = std::max(best, cand.size());
    }
    return best;
}

int main() {
    // Guidance case: LCS("AGGTAB","GXTXAYB") = 4, one answer is "GTAB".
    std::string lcs;
    assert(lcsLength("AGGTAB", "GXTXAYB", lcs) == 4);
    assert(lcs.size() == 4);
    assert(isSubsequence(lcs, "AGGTAB") && isSubsequence(lcs, "GXTXAYB"));

    // Edge cases: empty strings yield length 0.
    std::string tmp;
    assert(lcsLength("", "", tmp) == 0 && tmp.empty());
    assert(lcsLength("ABC", "", tmp) == 0 && tmp.empty());
    assert(lcsLength("", "XYZ", tmp) == 0 && tmp.empty());

    // Known small cases.
    assert(lcsLength("ABCBDAB", "BDCAB", tmp) == 4);   // e.g. "BCAB"
    assert(lcsLength("abc", "abc", tmp) == 3 && tmp == "abc");
    assert(lcsLength("abc", "xyz", tmp) == 0);

    // Rolling variant agrees with the table on length.
    assert(lcsLengthRolling("AGGTAB", "GXTXAYB") == 4);
    assert(lcsLengthRolling("", "XYZ") == 0);

    // Verify length against brute force for many tiny random-ish pairs.
    const char* pool[] = {"", "a", "ab", "aab", "abc", "bca",
                          "abab", "baba", "abcab", "cbabc"};
    for (const char* pa : pool) {
        for (const char* pb : pool) {
            std::string dummy;
            const std::size_t viaDp = lcsLength(pa, pb, dummy);
            assert(viaDp == lcsBrute(pa, pb));
            assert(viaDp == lcsLengthRolling(pa, pb));
        }
    }

    // Short demo.
    std::string demoLcs;
    const std::size_t len = lcsLength("AGGTAB", "GXTXAYB", demoLcs);
    std::cout << "LCS of \"AGGTAB\" and \"GXTXAYB\": length " << len
              << ", one LCS = \"" << demoLcs << "\"\n";
    std::cout << "All LCS tests passed.\n";
    return 0;
}
