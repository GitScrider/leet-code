/*
 * Z-Algorithm String Match (Algorithm - Strings)
 *
 * Idea:
 *   For a string S of length n, the Z-array is defined by
 *       z[i] = length of the longest substring starting at i that is also a
 *              prefix of S      (by convention z[0] = 0, or n; we use 0).
 *   Example: S = "aabxaabxcaabxaabxay"
 *            z = [0,1,0,0,4,1,0,0,0,8,1,0,0,5,1,0,0,1,0]
 *
 *   The array is built in O(n) by maintaining the "Z-box" [l, r], the interval
 *   with the largest right end r that we know matches a prefix of S
 *   (S[l..r] == S[0..r-l]). For a new index i:
 *     - If i is inside the current box (i <= r), then S[i] corresponds to
 *       position (i - l) in the prefix, so we can COPY z[i - l] as a starting
 *       guess -- but not past the box, hence min(z[i - l], r - i + 1).
 *     - Then we extend by explicit comparison only beyond r.
 *     - If we ran past r, we move the box to [i, i + z[i] - 1].
 *   Each text character is examined O(1) amortized because r only ever moves
 *   forward, which is exactly why no position is re-scanned.
 *
 * Pattern matching:
 *   Build the combined string  C = pattern + '\x01' + text  where the
 *   separator is a sentinel that appears in neither pattern nor text. Compute
 *   the Z-array of C. Wherever z[i] == pattern.size(), the pattern occurs in
 *   the text at position (i - pattern.size() - 1).
 *
 * Complexity:
 *   +-----------+---------------+-----------------------------------------+
 *   | Aspect    | Z-algorithm   | vs naive O(n*m)                         |
 *   +-----------+---------------+-----------------------------------------+
 *   | Build Z   | O(n + m)      | single linear pass over pattern+text    |
 *   | Search    | O(n + m)      | scan the Z-array once                    |
 *   | Space     | O(n + m)      | the combined string and its Z-array     |
 *   +-----------+---------------+-----------------------------------------+
 *
 * Complexity derivation (amortized linear scan; N = m + 1 + n = |combined|):
 *   z_array runs one loop i = 1 .. N-1. Per i it does O(1) box bookkeeping,
 *   then a while-loop of explicit character comparisons. Split each i's
 *   comparisons into SUCCESSFUL ones (each does ++z[i]) and the single FAILING
 *   one that ends the while. Let e_i = number of successful matches at i. Every
 *   successful match advances the frontier max(r, i + z[i]) by 1, and that
 *   frontier is monotone non-decreasing and never exceeds N, so
 *
 *       SUM_{i=1}^{N-1} e_i <= N            (r only moves forward, 0 -> N-1).
 *
 *   Total operations for the build:
 *
 *       C(N) = SUM_{i=1}^{N-1} ( O(1) + e_i + 1 )
 *            = (N-1)*O(1) + SUM_{i} e_i + (N-1)
 *            <= (N-1) + N + (N-1)
 *            = O(N) = O(n + m).
 *
 *   The search then scans the Z-array once, SUM_{i=m+1}^{N-1} O(1) = O(N), and
 *   building the combined string is O(N) too, so overall time is O(n + m).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f(N) = O(g)      iff  EXISTS c2, n0 :       f(N) <= c2*g(N)  for N >= n0
 *     f(N) = Omega(g)  iff  EXISTS c1, n0 :  c1*g(N) <= f(N)        for N >= n0
 *     f(N) = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   Here f(N) = C(N) with N = n + m + 1 and g(N) = N. The cost is NOT
 *   data-dependent: every position must be read at least once (lower bound) and
 *   is touched O(1) amortized (upper bound). With c1 = 1, c2 = 4, n0 = 1,
 *       1*N <= f(N) <= 4*N   =>   build is Theta(N) = Theta(n + m),
 *   and best = average = worst all share this tight bound. This is exact string
 *   matching, not a comparison sort, so the Omega(n log n) sorting lower bound
 *   does NOT apply; the relevant lower bound is Omega(n + m), since any matcher
 *   must inspect every character of pattern and text at least once.
 *
 * Key points:
 *   - The Z-box [l, r] lets us reuse earlier comparisons; r never decreases.
 *   - A separator not in the alphabet prevents matches from straddling the
 *     pattern/text boundary.
 *   - Overlaps handled naturally: consecutive indices can each hit z == m.
 *   - Empty pattern convention: matches at every index 0..text.size() incl.
 */

#include <string>
#include <vector>
#include <utility>
#include <cstddef>
#include <cassert>
#include <iostream>

// Compute the Z-array of `s`. z[0] is defined as 0 here; z[i] for i>0 is the
// length of the longest common prefix of s and the suffix s[i..].
std::vector<std::size_t> z_array(const std::string& s) {
    const std::size_t n = s.size();
    std::vector<std::size_t> z(n, 0);
    if (n == 0) return z;

    std::size_t l = 0;  // left end of the current Z-box
    std::size_t r = 0;  // right end (inclusive) of the current Z-box
    for (std::size_t i = 1; i < n; ++i) {
        if (i <= r) {
            // i is inside the known box: copy from the mirror position,
            // but do not read beyond the box's right edge.
            const std::size_t mirror = i - l;
            const std::size_t remaining = r - i + 1;
            z[i] = (z[mirror] < remaining) ? z[mirror] : remaining;
        }
        // Extend the match explicitly beyond whatever we could copy.
        while (i + z[i] < n && s[z[i]] == s[i + z[i]]) {
            ++z[i];
        }
        // If this match reaches further right than any before, move the box.
        if (i + z[i] > 0 && i + z[i] - 1 > r) {
            l = i;
            r = i + z[i] - 1;
        }
    }
    return z;
}

// Return 0-indexed start positions of every occurrence (incl. overlaps).
// Empty-pattern convention: matches at every index 0..text.size() inclusive.
std::vector<std::size_t> z_search(const std::string& text,
                                  const std::string& pattern) {
    std::vector<std::size_t> matches;
    const std::size_t n = text.size();
    const std::size_t m = pattern.size();

    if (m == 0) {
        for (std::size_t i = 0; i <= n; ++i) matches.push_back(i);
        return matches;
    }
    if (m > n) return matches;

    // Sentinel byte '\x01' assumed absent from ordinary text/pattern.
    const std::string combined = pattern + '\x01' + text;
    const std::vector<std::size_t> z = z_array(combined);

    // A prefix match of full length m that starts after the separator marks a
    // pattern occurrence. Offset by (m + 1) to translate into text coordinates.
    for (std::size_t i = m + 1; i < combined.size(); ++i) {
        if (z[i] == m) {
            matches.push_back(i - (m + 1));
        }
    }
    return matches;
}

// Naive O(n*m) reference used only to validate z_search.
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
    // ---- Assert the Z-array on a known string ------------------------------
    assert((z_array("aabxaabxcaabxaabxay") ==
            std::vector<std::size_t>{
                0, 1, 0, 0, 4, 1, 0, 0, 0, 8, 1, 0, 0, 5, 1, 0, 0, 1, 0}));
    // Simple repeated char: "aaaaa" -> z = [0,4,3,2,1].
    assert((z_array("aaaaa") == std::vector<std::size_t>{0, 4, 3, 2, 1}));

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
        {"mississippi", "issi"},   // overlaps -> {1,4}
        {"aaabaaa", "aaa"},        // {0,4}
    };
    for (const auto& c : cases) {
        assert(z_search(c.first, c.second) == naive_ref(c.first, c.second));
    }

    // Spot-check specific expected values.
    assert((z_search("aaaa", "aa") == std::vector<std::size_t>{0, 1, 2}));
    assert((z_search("mississippi", "issi") ==
            std::vector<std::size_t>{1, 4}));

    // ---- Short demo --------------------------------------------------------
    const std::string text = "aabxaabxcaabxaabxay";
    const std::string pattern = "aabx";
    std::cout << "Z-algorithm string match demo\n";
    std::cout << "text    = \"" << text << "\"\n";
    std::cout << "pattern = \"" << pattern << "\"\n";
    std::cout << "matches at:";
    for (const std::size_t pos : z_search(text, pattern)) {
        std::cout << ' ' << pos;
    }
    std::cout << '\n';
    std::cout << "All assertions passed.\n";
    return 0;
}
