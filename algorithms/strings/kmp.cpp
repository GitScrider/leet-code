/*
 * Knuth-Morris-Pratt (KMP) String Match (Algorithm - Strings)
 *
 * Idea:
 *   The naive matcher throws away work: after a partial match that fails, it
 *   slides the pattern by one and re-compares characters it has already seen.
 *   KMP precomputes, for the pattern alone, a "failure function" (also called
 *   the LPS array: Longest proper Prefix which is also a Suffix).
 *
 *   lps[i] = the length of the longest proper prefix of pattern[0..i] that is
 *   also a suffix of pattern[0..i]. "Proper" means it is not the whole string.
 *   Intuitively, if we have matched pattern[0..i] and the next text character
 *   fails, lps[i] tells us the longest already-matched prefix we can reuse, so
 *   we can resume comparison from pattern[lps[i]] WITHOUT moving the text
 *   pointer backwards.
 *
 *   LPS recurrence (len = length of the current best prefix-suffix):
 *     - if pattern[i] == pattern[len]: lps[i] = ++len            (extend)
 *     - else if len > 0:               len = lps[len - 1]; retry (fall back)
 *     - else:                          lps[i] = 0                (restart)
 *   The fall-back never rescans: `len` only ever decreases here, and it
 *   increases at most once per index, giving linear total work.
 *
 * Complexity:
 *   +-----------+---------------+-----------------------------------------+
 *   | Aspect    | KMP           | vs naive O(n*m)                         |
 *   +-----------+---------------+-----------------------------------------+
 *   | Preprocess| O(m)          | build LPS from the pattern              |
 *   | Search    | O(n)          | text pointer never moves backwards      |
 *   | Total     | O(n + m)      | naive can be O(n*m) on e.g. a^n / a^*b   |
 *   | Space     | O(m)          | the LPS array                           |
 *   +-----------+---------------+-----------------------------------------+
 *
 * Complexity derivation (amortized potential argument):
 *   PREPROCESS build_lps: the while-loop uses index i and prefix length len.
 *   Each iteration is exactly one of:
 *     (a) extend  : pattern[i]==pattern[len] -> i++, len++     (len rises by 1)
 *     (b) fallback: mismatch, len>0 -> len = lps[len-1]        (len drops >= 1)
 *     (c) restart : mismatch, len==0 -> lps[i]=0, i++          (len unchanged)
 *   Use potential Phi = len >= 0. i strictly increases only in (a)+(c) and runs
 *   1..m-1, so #(a)+#(c) = m-1. Only (a) raises Phi, by 1 each, so the total
 *   rise <= m-1; since Phi stays >= 0 the total fall over (b) steps is <= the
 *   rise, hence #(b) <= m-1. Iterations <= #(a)+#(b)+#(c) <= 2(m-1) = O(m).
 *
 *   SEARCH kmp_search: same accounting with text index i and match length j. i
 *   increases on a match or a j==0 mismatch and runs 0..n, so those steps are
 *   <= n; j rises by 1 only on a match (<= n times) and Phi = j >= 0, so the
 *   fallback steps (j = lps[j-1], incl. after a full match) are <= n. Total
 *   iterations <= 2n = O(n). Combining the two phases:
 *
 *       T(n, m) = O(m) [build_lps] + O(n) [scan] = O(n + m)
 *
 *   The text index i NEVER decreases -- that is the whole win over naive O(n*m).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f = O(g)      iff  EXISTS c2, n0 :        f <= c2*g(n)  for n >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f         for n >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   The work is DATA-INDEPENDENT. build_lps always runs m-1 outer advances of i
 *   => Theta(m); the scan must drive i from 0 to n => Omega(n), and amortization
 *   caps it at O(n) => Theta(n). Hence total = Theta(n + m) with the SAME tight
 *   bound for best, average and worst case (no per-case split needed); constants
 *   c1=1, c2=2 per phase. The Omega(n log n) comparison-sort floor does not apply
 *   -- this is a single linear scan, not sorting.
 *
 * Key points:
 *   - The text index i is monotonically non-decreasing: no back-tracking.
 *   - LPS is built by matching the pattern against itself.
 *   - Overlaps handled: after a full match we set j = lps[m-1] and continue.
 *   - Empty pattern convention: matches at every index 0..n inclusive.
 */

#include <string>
#include <vector>
#include <utility>
#include <cstddef>
#include <cassert>
#include <iostream>

// Build the LPS / failure array for `pattern`.
// lps[i] = length of the longest proper prefix of pattern[0..i] that is also
// a suffix of pattern[0..i].
std::vector<std::size_t> build_lps(const std::string& pattern) {
    const std::size_t m = pattern.size();
    std::vector<std::size_t> lps(m, 0);
    std::size_t len = 0;   // length of the previous longest prefix-suffix
    std::size_t i = 1;     // lps[0] is always 0, so start at 1
    while (i < m) {
        if (pattern[i] == pattern[len]) {
            // Current char extends the running prefix-suffix by one.
            lps[i] = ++len;
            ++i;
        } else if (len > 0) {
            // Mismatch but we still have a shorter candidate to fall back to.
            // Do NOT advance i; retry with the next shorter prefix-suffix.
            len = lps[len - 1];
        } else {
            // No prefix-suffix at all for this position.
            lps[i] = 0;
            ++i;
        }
    }
    return lps;
}

// Return 0-indexed start positions of every occurrence (incl. overlaps).
// Empty-pattern convention: matches at every index 0..text.size() inclusive.
std::vector<std::size_t> kmp_search(const std::string& text,
                                    const std::string& pattern) {
    std::vector<std::size_t> matches;
    const std::size_t n = text.size();
    const std::size_t m = pattern.size();

    if (m == 0) {
        for (std::size_t i = 0; i <= n; ++i) matches.push_back(i);
        return matches;
    }
    if (m > n) return matches;

    const std::vector<std::size_t> lps = build_lps(pattern);

    std::size_t i = 0;  // index into text  (never decreases)
    std::size_t j = 0;  // index into pattern (how much is currently matched)
    while (i < n) {
        if (text[i] == pattern[j]) {
            ++i;
            ++j;
            if (j == m) {
                // Full match ends at i-1, so it starts at i - m.
                matches.push_back(i - m);
                // Reuse the longest prefix-suffix so overlaps are found.
                j = lps[j - 1];
            }
        } else if (j > 0) {
            // Mismatch after some progress: fall back within the pattern only.
            j = lps[j - 1];
        } else {
            // Mismatch on the very first pattern char: advance the text.
            ++i;
        }
    }
    return matches;
}

// Naive O(n*m) reference used only to validate kmp_search in the tests.
static std::vector<std::size_t> naive_ref(const std::string& text,
                                          const std::string& pattern) {
    std::vector<std::size_t> matches;
    const std::size_t n = text.size();
    const std::size_t m = pattern.size();
    if (m == 0) {
        for (std::size_t i = 0; i <= n; ++i) matches.push_back(i);
        return matches;
    }
    if (m > n) return matches;
    for (std::size_t i = 0; i + m <= n; ++i) {
        std::size_t j = 0;
        while (j < m && text[i + j] == pattern[j]) ++j;
        if (j == m) matches.push_back(i);
    }
    return matches;
}

int main() {
    // ---- LPS sanity checks -------------------------------------------------
    // For "aabaaab": prefixes-suffixes computed by hand.
    assert((build_lps("aabaaab") ==
            std::vector<std::size_t>{0, 1, 0, 1, 2, 2, 3}));
    // Classic "ababaca".
    assert((build_lps("ababaca") ==
            std::vector<std::size_t>{0, 0, 1, 2, 3, 0, 1}));

    // ---- Cross-check every match against the naive reference ---------------
    const std::vector<std::pair<std::string, std::string>> cases = {
        {"aaaa", "aa"},            // overlaps -> {0,1,2}
        {"aaaa", "a"},             // {0,1,2,3}
        {"abababab", "ab"},        // {0,2,4,6}
        {"abracadabra", "abra"},   // {0,7}
        {"abcdef", "xyz"},         // none
        {"ab", "abc"},             // pattern longer than text
        {"", "a"},                 // empty text
        {"abc", ""},               // empty pattern
        {"", ""},                  // both empty
        {"mississippi", "issip"},  // {4}
        {"aaabaaa", "aaa"},        // {0,4}
    };
    for (const auto& c : cases) {
        assert(kmp_search(c.first, c.second) == naive_ref(c.first, c.second));
    }

    // Spot-check specific expected values.
    assert((kmp_search("aaaa", "aa") == std::vector<std::size_t>{0, 1, 2}));
    assert((kmp_search("abracadabra", "abra") ==
            std::vector<std::size_t>{0, 7}));

    // ---- Short demo --------------------------------------------------------
    const std::string text = "abababab";
    const std::string pattern = "abab";
    std::cout << "KMP string match demo\n";
    std::cout << "text    = \"" << text << "\"\n";
    std::cout << "pattern = \"" << pattern << "\"\n";
    std::cout << "matches at:";
    for (const std::size_t pos : kmp_search(text, pattern)) {
        std::cout << ' ' << pos;
    }
    std::cout << '\n';
    std::cout << "All assertions passed.\n";
    return 0;
}
