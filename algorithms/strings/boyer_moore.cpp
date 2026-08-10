/*
 * Boyer-Moore String Search  (Algorithm - Strings)
 * -----------------------------------------------------------------------------
 * Problem : Report every 0-indexed start position where pattern P occurs in
 *           text T (overlaps included).
 *
 * Idea    : Align P under a window of T and compare RIGHT-TO-LEFT. On a
 *           mismatch we shift the window forward by MORE than one whenever
 *           possible, so most text characters are never inspected at all.
 *
 *           Bad-character heuristic (implemented here):
 *             Let the mismatch be pattern index j against text character c.
 *             Look up last[c] = right-most index of c inside P (or -1).
 *               * If c occurs in P to the LEFT of j, slide P so that occurrence
 *                 lines up under the text's c:   shift = j - last[c].
 *               * If c does not occur in P at all (last[c] = -1), we may slide
 *                 P entirely past c:             shift = j + 1.
 *               * If the only occurrences are to the RIGHT of j the formula
 *                 would go backwards, so we clamp the shift to at least 1.
 *
 *           Good-suffix heuristic (NOT coded, but it composes cleanly):
 *             When a suffix of P matched before the mismatch, that matched
 *             suffix may reappear earlier in P (or a prefix of P may equal a
 *             suffix of it). Precomputing those lets us take the LARGER of the
 *             two shifts, which is what yields Boyer-Moore's classic bound.
 *
 *   Complexity (n = |text|, m = |pattern|, sigma = alphabet size)
 *   +-----------------+------------------+-------------------------+
 *   | Case            | Boyer-Moore(BC)  | Naive                   |
 *   +-----------------+------------------+-------------------------+
 *   | Preprocess time | O(m + sigma)     | --                      |
 *   | Best  (search)  | O(n / m)         | O(n)                    |
 *   | Worst (search)  | O(n * m)         | O(n * m)                |
 *   | Space           | O(sigma)         | O(1)                    |
 *   +-----------------+------------------+-------------------------+
 *
 * Complexity derivation (window shifts; n=|text|, m=|pattern|, sigma=256):
 *   Preprocessing: last.fill(-1) writes sigma entries, then the loop over P
 *   runs SUM_{j=0}^{m-1} 1 = m iterations of O(1):
 *       P(m) = sigma + m = O(m + sigma)  (= O(m) since sigma = 256 is fixed).
 *   Search: let the window stop at alignments s_0=0, s_1, ..., s_{k-1}. At the
 *   t-th alignment the right-to-left loop does c_t comparisons (1 <= c_t <= m)
 *   and then shifts by delta_t >= 1. Total comparisons C = SUM_{t=0}^{k-1} c_t.
 *
 *     BEST case: every rightmost char mismatches and is absent from P, so
 *       c_t = 1 and delta_t = m. Then k = ceil((n-m+1)/m) windows and
 *           C_best = SUM_{t=0}^{k-1} 1 = k = O(n/m)  (sublinear: ~n/m chars).
 *     WORST case: e.g. P="a...a" in T="a...a"; each window compares the whole
 *       pattern (c_t = m) yet can shift only delta_t = 1, so k = n - m + 1 and
 *           C_worst = SUM_{t=0}^{n-m} m = (n - m + 1)*m = O(n*m).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f = O(g)     iff EXISTS c2,n0 :        f(n) <= c2*g(n)  for n >= n0
 *     f = Omega(g) iff EXISTS c1,n0 :  c1*g(n) <= f(n)         for n >= n0
 *     f = Theta(g) iff f = O(g) AND f = Omega(g)
 *   Boyer-Moore (bad-character only) is ADAPTIVE, so the bound is PER CASE:
 *     BEST  search  C_best  = ceil((n-m+1)/m)          => Theta(n/m)  (tight).
 *     WORST search  C_worst = (n-m+1)*m; with g = n*m and m <= n/2,
 *                   (1/2)*n*m <= C_worst <= n*m         => Theta(n*m)  (tight).
 *   Over ALL inputs the search is thus O(n*m) (upper bound, from the worst case)
 *   and Omega(n/m) (lower bound, from the best case); it is NOT a single Theta
 *   precisely because best != worst -- the whole reason the table splits
 *   Best/Worst. The bad-character rule alone cannot force a linear worst case;
 *   the good-suffix rule (+ Galil) would. This is exact matching, not sorting,
 *   so the comparison-sort Omega(n log n) lower bound is irrelevant here.
 *
 * Key points:
 *   - Sublinear best case: if every mismatched text char is absent from P we
 *     jump m positions each time, touching only ~n/m characters of the text.
 *   - Bad-character alone can degrade to O(n*m) (e.g. "aaaa" in "aaaa..."); the
 *     good-suffix rule fixes the worst case but is omitted here for clarity.
 *   - Empty pattern convention: it "matches" at every position 0..n (n+1 of
 *     them) -- exactly what the naive reference produces for m = 0.
 */

#include <string>
#include <vector>
#include <array>
#include <cassert>
#include <iostream>
#include <cstddef>

// ---- Naive O(n*m) reference used both as a fallback and as the test oracle. --
static std::vector<std::size_t> naiveSearch(const std::string& text,
                                             const std::string& pat) {
    std::vector<std::size_t> res;
    const std::size_t n = text.size(), m = pat.size();
    for (std::size_t i = 0; i + m <= n; ++i) {   // m == 0 -> i in [0, n]
        std::size_t j = 0;
        while (j < m && text[i + j] == pat[j]) ++j;
        if (j == m) res.push_back(i);
    }
    return res;
}

std::vector<std::size_t> boyerMoore(const std::string& text, const std::string& pat) {
    const std::size_t n = text.size(), m = pat.size();
    std::vector<std::size_t> res;

    if (m == 0) {                      // defined: empty pattern matches everywhere
        for (std::size_t i = 0; i <= n; ++i) res.push_back(i);
        return res;
    }
    if (m > n) return res;             // pattern longer than text -> no match

    // Bad-character table over all 256 byte values: last index of each byte in P.
    std::array<int, 256> last;
    last.fill(-1);
    for (std::size_t j = 0; j < m; ++j)
        last[static_cast<unsigned char>(pat[j])] = static_cast<int>(j);

    std::size_t s = 0;                 // window alignment: P sits over text[s .. s+m-1]
    while (s + m <= n) {
        std::size_t j = m;             // compare right-to-left; j counts down from m
        while (j > 0 && pat[j - 1] == text[s + j - 1]) --j;

        if (j == 0) {                  // full match at s
            res.push_back(s);
            ++s;                       // shift by 1 so overlapping matches are found
        } else {
            const int mismatchPos = static_cast<int>(j - 1);
            const int lastOcc = last[static_cast<unsigned char>(text[s + j - 1])];
            int shift = mismatchPos - lastOcc;   // = mismatchPos + 1 when lastOcc = -1
            if (shift < 1) shift = 1;            // never move backwards or stall
            s += static_cast<std::size_t>(shift);
        }
    }
    return res;
}

int main() {
    struct Case { const char* text; const char* pat; };
    const Case cases[] = {
        {"abacaabaccabacabaabb", "abac"},   // multiple, some near each other
        {"aaaa", "aa"},                      // overlapping -> {0,1,2}
        {"aaaaa", "aaa"},                    // overlapping -> {0,1,2}
        {"abcdef", "xyz"},                   // absent -> {}
        {"abc", "abcd"},                     // pattern longer than text -> {}
        {"hello world", ""},                 // empty pattern -> {0..n}
        {"mississippi", "issi"},             // overlapping "issi" at 1 and 4
        {"aabaabaaa", "aaba"},               // repeats
        {"GCTTCTGCTACCTTTTGCGCGCGCGCGGAA", "CGCG"}, // biology-ish, overlaps
        {"", ""},                            // empty text + empty pattern -> {0}
        {"", "a"},                           // empty text, non-empty pattern -> {}
    };

    for (const Case& c : cases) {
        std::string text(c.text), pat(c.pat);
        std::vector<std::size_t> got = boyerMoore(text, pat);
        std::vector<std::size_t> exp = naiveSearch(text, pat);
        assert(got == exp);
    }

    // Spot-check a couple of exact expected vectors.
    assert((boyerMoore("aaaa", "aa")  == std::vector<std::size_t>{0, 1, 2}));
    assert((boyerMoore("mississippi", "issi") == std::vector<std::size_t>{1, 4}));
    assert(boyerMoore("abcdef", "xyz").empty());

    std::cout << "Boyer-Moore demo\n";
    std::string text = "mississippi", pat = "issi";
    std::cout << "  pattern \"" << pat << "\" in \"" << text << "\" at:";
    for (std::size_t pos : boyerMoore(text, pat)) std::cout << ' ' << pos;
    std::cout << "\nAll Boyer-Moore assertions passed.\n";
    return 0;
}
