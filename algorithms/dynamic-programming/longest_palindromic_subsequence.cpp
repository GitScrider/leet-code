/*
 * Longest Palindromic Subsequence (LPS)
 * Category: Algorithm - Dynamic Programming
 *
 * Idea
 * ----
 * Given a string s (length n), find the length of the longest subsequence of s
 * that reads the same forwards and backwards. (Subsequence: keep order, drop any
 * characters; NOT required to be contiguous.)
 *
 * Interval DP over substrings:
 *   State:      dp[i][j] = length of the LPS within the substring s[i..j]
 *                          (inclusive). Defined for i <= j.
 *   Recurrence: if s[i] == s[j]:
 *                   dp[i][j] = 2 + dp[i+1][j-1]          // both ends pair up
 *                              (dp[i+1][j-1] is 0 when i+1 > j-1)
 *               else:
 *                   dp[i][j] = max(dp[i+1][j], dp[i][j-1])   // drop one end
 *   Base cases: dp[i][i] = 1                             // single char
 *               empty substring contributes 0
 *
 * Why it is correct
 * -----------------
 * Optimal substructure: a palindromic subsequence of s[i..j] either uses both
 * matching ends (when s[i]==s[j], pairing them is always safe and adds 2 around
 * the best inner palindrome) or it discards at least one end, reducing to a
 * shorter interval. Overlapping subproblems: interval [i, j] is revisited by
 * many longer intervals, so we fill a table by increasing length.
 *
 * Relation to LCS: LPS(s) == LCS(s, reverse(s)). We compute the interval DP here
 * and cross-check against that identity in the tests.
 *
 * Complexity
 * ----------
 *   +-----------+-----------+-----------+
 *   | Approach  | Time      | Space     |
 *   +-----------+-----------+-----------+
 *   | Interval  | O(n^2)    | O(n^2)    |
 *   +-----------+-----------+-----------+
 * A rolling variant can bring space to O(n), but the 2-D table is clearest and
 * is what we use here.
 *
 * Key points
 * ----------
 *  - Bottom-up by increasing substring length so dp[i+1][j-1] is ready first.
 *  - dp[i+1][j-1] with j==i+1 refers to an empty interval -> treated as 0.
 *  - Careful std::size_t handling: never form i+1 or j-1 out of range; the inner
 *    term is only read when the interval length is at least 3.
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

// Interval DP for the LPS length.
std::size_t lpsLength(const std::string& s) {
    const std::size_t n = s.size();
    if (n == 0) return 0;

    // dp[i][j] for 0 <= i <= j < n. Lower triangle stays unused.
    std::vector<std::vector<std::size_t>> dp(n, std::vector<std::size_t>(n, 0));

    // Base case: every single character is a palindrome of length 1.
    for (std::size_t i = 0; i < n; ++i) dp[i][i] = 1;

    // Fill by increasing interval length (2 .. n) so inner intervals exist.
    for (std::size_t len = 2; len <= n; ++len) {
        for (std::size_t i = 0; i + len - 1 < n; ++i) {
            const std::size_t j = i + len - 1;
            if (s[i] == s[j]) {
                // Inner interval [i+1, j-1] is empty when len == 2 -> add 0.
                const std::size_t inner = (len > 2) ? dp[i + 1][j - 1] : 0;
                dp[i][j] = 2 + inner;
            } else {
                dp[i][j] = std::max(dp[i + 1][j], dp[i][j - 1]);
            }
        }
    }
    return dp[0][n - 1];
}

// Cross-check helper: LPS(s) == LCS(s, reverse(s)).
std::size_t lcsLength(const std::string& a, const std::string& b) {
    const std::size_t m = a.size();
    const std::size_t n = b.size();
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
    return dp[m][n];
}

std::size_t lpsViaLcs(const std::string& s) {
    std::string r(s.rbegin(), s.rend());
    return lcsLength(s, r);
}

// Brute-force reference: enumerate every subsequence via bitmask and keep the
// longest one that is a palindrome. Exponential; tiny inputs only.
static bool isPalindrome(const std::string& t) {
    std::size_t i = 0;
    std::size_t j = t.size();
    while (i + 1 < j) {  // compare t[i] with t[j-1]; avoids unsigned underflow
        if (t[i] != t[j - 1]) return false;
        ++i;
        --j;
    }
    return true;
}

std::size_t lpsBrute(const std::string& s) {
    const std::size_t n = s.size();
    std::size_t best = 0;
    for (std::size_t mask = 0; mask < (std::size_t{1} << n); ++mask) {
        std::string cand;
        for (std::size_t bit = 0; bit < n; ++bit) {
            if (mask & (std::size_t{1} << bit)) cand.push_back(s[bit]);
        }
        if (!cand.empty() && isPalindrome(cand)) {
            best = std::max(best, cand.size());
        }
    }
    return best;
}

int main() {
    // Guidance cases.
    assert(lpsLength("bbbab") == 4);  // "bbbb"
    assert(lpsLength("cbbd") == 2);   // "bb"
    assert(lpsLength("a") == 1);      // single char

    // Edge cases.
    assert(lpsLength("") == 0);
    assert(lpsLength("aa") == 2);
    assert(lpsLength("ab") == 1);

    // Known cases.
    assert(lpsLength("agbdba") == 5);   // "abdba"
    assert(lpsLength("abcde") == 1);
    assert(lpsLength("racecar") == 7);  // already a palindrome

    // LPS == LCS(s, reverse(s)) must hold.
    assert(lpsViaLcs("bbbab") == 4);
    assert(lpsViaLcs("cbbd") == 2);
    assert(lpsViaLcs("agbdba") == 5);

    // Cross-check interval DP, the LCS identity, and brute force on tiny cases.
    const char* pool[] = {"",     "a",    "ab",    "aa",   "aba",
                          "abba", "abca", "xyzyx", "abcb", "cbbd"};
    for (const char* p : pool) {
        const std::string s = p;
        const std::size_t viaDp = lpsLength(s);
        assert(viaDp == lpsBrute(s));
        assert(viaDp == lpsViaLcs(s));
    }

    // Short demo.
    std::cout << "LPS(\"bbbab\")   = " << lpsLength("bbbab") << " (e.g. bbbb)\n";
    std::cout << "LPS(\"cbbd\")    = " << lpsLength("cbbd") << " (e.g. bb)\n";
    std::cout << "LPS(\"racecar\") = " << lpsLength("racecar") << "\n";
    std::cout << "All LPS tests passed.\n";
    return 0;
}
