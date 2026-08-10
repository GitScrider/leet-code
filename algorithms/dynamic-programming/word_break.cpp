/*
 * Word Break
 * Category: Algorithm - Dynamic Programming (1-D String DP)
 *
 * Problem:
 *   Given a string s and a dictionary of words, decide whether s can be
 *   segmented into a sequence of one or more dictionary words. Words may be
 *   reused any number of times.
 *
 * Idea (State / Recurrence / Base case):
 *   State:  dp[i] = true if the prefix s[0..i) (the first i characters) can be
 *           fully segmented into dictionary words.
 *   Recurrence:
 *           dp[i] = OR over all j in [0, i) of
 *                   ( dp[j] AND dict.contains(s[j..i)) )
 *           i.e. the length-i prefix is breakable if there is a split point j
 *           where the first j chars are breakable AND the remaining substring
 *           s[j..i) is itself a dictionary word.
 *   Base:   dp[0] = true (the empty prefix is trivially segmentable).
 *   Answer: dp[n], where n = s.size().
 *
 *   Why correct: any valid segmentation of s[0..i) ends with some FINAL word
 *   s[j..i); deleting it leaves a valid segmentation of s[0..j) (optimal
 *   substructure). The same prefix length i is queried by many longer prefixes,
 *   so subproblems overlap and tabulation/memoization avoids recomputation.
 *
 * Complexity:
 *   +-----------+---------------------------+
 *   | Time      | O(n^2 * L)                |
 *   | Space     | O(n)  (plus dictionary)   |
 *   +-----------+---------------------------+
 *   n = s.size(); the extra L is the cost of hashing/comparing a candidate
 *   substring of length up to n during each unordered_set lookup.
 *
 * Complexity derivation (DP states * work-per-transition, summation):
 *   States: dp[0..n] gives n+1 prefixes = O(n) states. The transition for dp[i]
 *   scans split points j = 0..i-1; each split builds the candidate s.substr(j,
 *   i-j) and hashes/compares it. Let L be the candidate length (i-j), at most n.
 *   Count the (i, j) inner iterations:
 *
 *       pairs(n) = SUM_{i=1}^{n} SUM_{j=0}^{i-1} 1
 *                = SUM_{i=1}^{n} i
 *                = n*(n+1)/2                    (arithmetic series, Gauss)
 *                = Theta(n^2)
 *   Spending O(L) to build + hash each candidate gives Theta(n^2) * O(L)
 *   = O(n^2 * L). Charging each candidate its TRUE length (i-j) yields the exact
 *   worst-case count:
 *
 *       SUM_{i=1}^{n} SUM_{j=0}^{i-1} (i - j)
 *            = SUM_{i=1}^{n} SUM_{k=1}^{i} k    (substitute k = i - j)
 *            = SUM_{i=1}^{n} i*(i+1)/2
 *            = Theta(n^3)
 *   i.e. O(n^2 * L) with L = Theta(n). dp is a single (n+1)-length array,
 *   so space is O(n) (plus the dictionary storage).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f = O(g)      iff  EXISTS c2, n0 :       f(n) <= c2*g(n)  for n >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   The inner "break" on the first valid split makes the cost data-dependent:
 *     WORST case (no early split ever succeeds) scans all n*(n+1)/2 pairs, each
 *                up to O(L) -> Theta(n^2 * L).
 *     BEST case still runs the outer loop n times and, because dp[0] is always
 *                true, pays at least the j=0 length-i substring hash on every i,
 *                a guaranteed SUM_{i=1}^{n} Theta(i) = Omega(n^2).
 *   So the overall running time is O(n^2 * L) (upper) and Omega(n^2) (lower); it
 *   is not a single Theta unless L is constant. Membership is decided by HASHING,
 *   not comparison sorting, so the Omega(n log n) comparison lower bound does not
 *   apply here.
 *
 * Key points:
 *   - Bottom-up tabulation over prefix lengths; dp is a simple 1-D boolean
 *     array, so there is no meaningful further space reduction.
 *   - A top-down memoized recursion on the START index computes the same table
 *     (shown here as wordBreakMemo).
 *   - An unordered_set gives average O(1) membership tests for substrings.
 */

#include <string>
#include <vector>
#include <unordered_set>
#include <cstddef>
#include <cassert>
#include <iostream>

using std::size_t;
using Dict = std::unordered_set<std::string>;

// Bottom-up tabulation.
bool wordBreak(const std::string& s, const Dict& dict) {
    const size_t n = s.size();
    // dp[i] = can the prefix of length i be segmented. dp[0] = empty prefix.
    std::vector<char> dp(n + 1, false);
    dp[0] = true;
    for (size_t i = 1; i <= n; ++i) {
        for (size_t j = 0; j < i; ++j) {
            // Consider s[j..i): substring starting at j with length (i - j).
            if (dp[j] && dict.count(s.substr(j, i - j)) > 0) {
                dp[i] = true;
                break; // one valid split is enough to mark this prefix breakable
            }
        }
    }
    return dp[n] != 0;
}

// Top-down memoized recursion on the start index (same subproblems).
// We use signed char for the memo so the -1 "unknown" sentinel is portable
// (plain char may be unsigned on some platforms, which would break -1).
bool wordBreakMemoHelper(const std::string& s, size_t start,
                         const Dict& dict, std::vector<signed char>& memo) {
    const size_t n = s.size();
    if (start == n) return true;               // consumed the entire string
    if (memo[start] != -1) return memo[start] != 0;

    bool ok = false;
    for (size_t end = start + 1; end <= n; ++end) {
        // s[start..end) is a candidate first word; recurse on the remainder.
        if (dict.count(s.substr(start, end - start)) > 0 &&
            wordBreakMemoHelper(s, end, dict, memo)) {
            ok = true;
            break;
        }
    }
    memo[start] = ok ? 1 : 0;
    return ok;
}

bool wordBreakMemo(const std::string& s, const Dict& dict) {
    // -1 = unknown, 0 = false, 1 = true. Sized to s.size(); start == n returns
    // early, so index start is always in range [0, n).
    std::vector<signed char> memo(s.size(), -1);
    return wordBreakMemoHelper(s, 0, dict, memo);
}

// Brute-force reference (exponential): plain recursion without memoization.
bool wordBreakBrute(const std::string& s, size_t start, const Dict& dict) {
    const size_t n = s.size();
    if (start == n) return true;
    for (size_t end = start + 1; end <= n; ++end) {
        if (dict.count(s.substr(start, end - start)) > 0 &&
            wordBreakBrute(s, end, dict)) {
            return true;
        }
    }
    return false;
}

int main() {
    struct Case {
        std::string s;
        Dict dict;
        bool expected;
    };

    const std::vector<Case> cases = {
        // "leetcode" = "leet" + "code" -> true.
        {"leetcode", {"leet", "code"}, true},
        // "applepenapple" = "apple" + "pen" + "apple" (reuse) -> true.
        {"applepenapple", {"apple", "pen"}, true},
        // "catsandog" cannot be fully segmented -> false.
        {"catsandog", {"cats", "dog", "sand", "and", "cat"}, false},
        // Edge: empty string is segmentable into zero words -> true.
        {"", {"a", "b"}, true},
        // No-solution: non-empty string, empty dictionary -> false.
        {"a", {}, false},
        // Single-letter dictionary that tiles the whole string -> true.
        {"aaaa", {"a", "aa"}, true},
    };

    for (const auto& c : cases) {
        const bool got = wordBreak(c.s, c.dict);
        assert(got == c.expected);
        // All three implementations must agree.
        assert(wordBreakMemo(c.s, c.dict) == c.expected);
        assert(wordBreakBrute(c.s, 0, c.dict) == c.expected);
    }

    std::cout << "wordBreak(\"leetcode\", {leet, code}) = "
              << std::boolalpha << wordBreak("leetcode", {"leet", "code"})
              << "\n";
    std::cout << "wordBreak(\"catsandog\", {...}) = "
              << wordBreak("catsandog",
                           {"cats", "dog", "sand", "and", "cat"})
              << "\n";
    std::cout << "All word-break tests passed.\n";
    return 0;
}
