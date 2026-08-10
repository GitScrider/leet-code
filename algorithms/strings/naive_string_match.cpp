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
