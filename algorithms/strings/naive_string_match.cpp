/*
 * Naive String Match (Algorithm - Strings)
 *
 * Idea:
 *   The most direct way to find every occurrence of a pattern P inside a text
 *   T is to try aligning P at each possible start position of T and compare
 *   character by character. For a text of length n and a pattern of length m
 *   there are (n - m + 1) candidate alignments; each alignment is checked with
 *   a straightforward inner loop. There is no precomputation and no cleverness:
 *   this is the honest brute-force baseline against which smarter matchers
 *   (KMP, Rabin-Karp, Z-algorithm) are measured.
 *
 *   Overlapping matches are reported naturally: because we advance the start
 *   position by exactly one after each alignment, pattern "aa" in "aaaa"
 *   yields {0, 1, 2}.
 *
 * Complexity:
 *   +-----------+------------------+-------------------------------+
 *   | Aspect    | This algorithm   | Notes                         |
 *   +-----------+------------------+-------------------------------+
 *   | Time      | O(n * m) worst   | e.g. T="aaaa...a", P="aaa...b" |
 *   | Time      | O(n) best/typical| mismatch on first char often  |
 *   | Space     | O(1) extra       | only the output vector grows  |
 *   +-----------+------------------+-------------------------------+
 *   Smarter algorithms cut the worst case to O(n + m) by never re-scanning
 *   text characters they have already examined.
 *
 * Complexity derivation (nested-loop comparison count):
 *   Let n = |T|, m = |P|, and count character COMPARISONS. The outer loop runs
 *   over the (n - m + 1) alignments i = 0, 1, ..., n-m. At alignment i the inner
 *   while-loop makes c_i comparisons before it hits a mismatch or consumes all m
 *   characters, with 1 <= c_i <= m. The total is:
 *
 *       C(n, m) = SUM_{i=0}^{n-m} c_i
 *
 *   WORST case (e.g. T = "aa...a", P = "aa...ab"): every alignment matches the
 *   m-1 leading chars before failing, so c_i = m for all (n - m + 1) alignments:
 *
 *       C_worst = SUM_{i=0}^{n-m} m = m * (n - m + 1) = O(n * m)
 *
 *   (the product peaks at ~n^2/4 when m ~ n/2). BEST / typical case (mismatch on
 *   the FIRST compared char at every alignment) gives c_i = 1:
 *
 *       C_best = SUM_{i=0}^{n-m} 1 = n - m + 1 = O(n)
 *
 *   Nothing is remembered across alignments (the text pointer effectively moves
 *   backward after each mismatch), which is exactly why the worst case is O(n*m).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f = O(g)      iff  EXISTS c2, n0 :        f(n) <= c2*g(n)  for n >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)         for n >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   The cost is DATA-DEPENDENT, so bounds are given per case:
 *     WORST  C = m*(n-m+1): take g = m*(n-m+1), c1=c2=1 => Theta(n*m)  (tight).
 *     BEST   C = n-m+1:     => Theta(n - m + 1) = Theta(n) for m << n   (tight).
 *   Over ALL inputs the running time is O(n*m) (upper, from worst) and
 *   Omega(n - m + 1) = Omega(n) (lower, from best); it is NOT a single Theta
 *   because best != worst -- hence the per-case table. The comparison-sort floor
 *   Omega(n log n) is irrelevant: this is exact substring search, not sorting;
 *   the only universal floor is Omega(n), just to read the text.
 *
 * Key points:
 *   - No preprocessing; simplest possible correct matcher.
 *   - Advancing the window by 1 makes overlapping matches fall out for free.
 *   - Empty pattern convention (documented below): an empty pattern matches at
 *     every position 0..n inclusive, i.e. n + 1 matches, mirroring how
 *     std::string::find treats the empty needle as found everywhere.
 */

#include <string>
#include <vector>
#include <cstddef>
#include <cassert>
#include <iostream>

// Return the 0-indexed start positions of EVERY occurrence of `pattern`
// inside `text`, including overlapping occurrences.
//
// Empty-pattern convention: an empty pattern is considered to match at every
// index from 0 to text.size() inclusive, producing text.size() + 1 positions.
std::vector<std::size_t> naive_search(const std::string& text,
                                      const std::string& pattern) {
    std::vector<std::size_t> matches;
    const std::size_t n = text.size();
    const std::size_t m = pattern.size();

    // Empty pattern: matches at every gap, including the one past the end.
    if (m == 0) {
        for (std::size_t i = 0; i <= n; ++i) {
            matches.push_back(i);
        }
        return matches;
    }

    // If the pattern is longer than the text it can never fit.
    if (m > n) {
        return matches;
    }

    // Try each alignment start i in [0, n - m]. We compare P against the slice
    // T[i .. i+m-1]; on the first mismatch we abandon this alignment and slide
    // the window forward by one.
    for (std::size_t i = 0; i + m <= n; ++i) {
        std::size_t j = 0;
        while (j < m && text[i + j] == pattern[j]) {
            ++j;
        }
        if (j == m) {
            matches.push_back(i);  // full pattern consumed => a match at i
        }
    }
    return matches;
}

int main() {
    // ---- Assert-based tests ------------------------------------------------
    // Because this file IS the reference implementation, the tests here assert
    // against hand-computed expected results rather than another matcher.

    // Overlapping matches: "aa" in "aaaa" -> {0, 1, 2}.
    assert((naive_search("aaaa", "aa") == std::vector<std::size_t>{0, 1, 2}));

    // Single character repeated: "a" in "aaaa" -> {0, 1, 2, 3}.
    assert((naive_search("aaaa", "a") == std::vector<std::size_t>{0, 1, 2, 3}));

    // Typical multi-occurrence text.
    assert((naive_search("abababab", "ab") ==
            std::vector<std::size_t>{0, 2, 4, 6}));

    // Match at the very end of the text.
    assert((naive_search("hello world", "world") ==
            std::vector<std::size_t>{6}));

    // Pattern not present at all -> empty result.
    assert(naive_search("abcdef", "xyz").empty());

    // Pattern longer than text -> empty result.
    assert(naive_search("ab", "abc").empty());

    // Pattern equal to the whole text -> single match at 0.
    assert((naive_search("abc", "abc") == std::vector<std::size_t>{0}));

    // Empty pattern -> matches everywhere (0..n inclusive).
    assert((naive_search("abc", "") ==
            std::vector<std::size_t>{0, 1, 2, 3}));

    // Empty text with non-empty pattern -> no matches.
    assert(naive_search("", "a").empty());

    // Empty text with empty pattern -> one match at position 0.
    assert((naive_search("", "") == std::vector<std::size_t>{0}));

    // ---- Short demo --------------------------------------------------------
    const std::string text = "abracadabra";
    const std::string pattern = "abra";
    const std::vector<std::size_t> hits = naive_search(text, pattern);

    std::cout << "Naive string match demo\n";
    std::cout << "text    = \"" << text << "\"\n";
    std::cout << "pattern = \"" << pattern << "\"\n";
    std::cout << "matches at:";
    for (const std::size_t pos : hits) {
        std::cout << ' ' << pos;
    }
    std::cout << '\n';

    std::cout << "All assertions passed.\n";
    return 0;
}
