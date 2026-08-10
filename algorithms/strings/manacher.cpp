/*
 * Manacher's Algorithm  (Algorithm - Strings)
 * -----------------------------------------------------------------------------
 * Problem : Find the LONGEST PALINDROMIC SUBSTRING of a string in linear time.
 *
 * Idea    : "Expand around center" is naturally O(n^2): every one of the 2n-1
 *           possible centers (n characters + n-1 gaps) may expand up to O(n).
 *           Manacher removes the re-scanning by reusing palindrome symmetry.
 *
 *           1) Transform  "abc"  ->  "^#a#b#c#$".
 *              '#' separators make EVERY palindrome odd-length, so even and odd
 *              cases are handled uniformly. '^' and '$' are unique sentinels so
 *              expansion can never fall off either end (no bounds checks).
 *
 *           2) Keep the palindrome with the right-most reach: its center C and
 *              its right boundary R (= C + radius). For a new index i < R its
 *              mirror is  m = 2C - i.  The palindrome at i is at least as long
 *              as the one at m, clipped so it does not cross R:
 *                   p[i] = min(R - i, p[m])
 *              Only the part beyond R must be verified by actual comparisons.
 *              Because R only moves forward, total extra work is O(n).
 *
 *           The radius p[i] in the transformed string equals the LENGTH of the
 *           palindrome in the ORIGINAL string (each real char is flanked by #).
 *
 *   Complexity (n = length of the input string)
 *   +-----------+-----------------+------------------------------+
 *   | Operation | Manacher        | Naive expand-around-center   |
 *   +-----------+-----------------+------------------------------+
 *   | Time      | O(n)            | O(n^2)                       |
 *   | Space     | O(n)            | O(1)                         |
 *   +-----------+-----------------+------------------------------+
 *
 * Key points:
 *   - The separator trick unifies even/odd palindromes; sentinels kill bounds
 *     checks and any risk of size_t underflow while expanding.
 *   - p[i] is both the radius in the padded string AND the length in the
 *     original string; the original start index is (center - p[i]) / 2.
 *   - We also expose the full per-center radius array p[] for inspection.
 */

#include <string>
#include <vector>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <cstddef>
#include <utility>

struct ManacherResult {
    std::string longest;             // the longest palindromic substring itself
    std::size_t start = 0;           // its start index in the ORIGINAL string
    std::size_t length = 0;          // its length
    std::vector<std::size_t> radii;  // p[]: radius per center in the padded string
};

ManacherResult manacher(const std::string& s) {
    // Build "^#a#b#c#$": sentinels at the ends, '#' between every character.
    std::string t;
    t.reserve(2 * s.size() + 3);
    t.push_back('^');
    for (char c : s) { t.push_back('#'); t.push_back(c); }
    t.push_back('#');
    t.push_back('$');

    const std::size_t m = t.size();
    std::vector<std::size_t> p(m, 0);   // p[i] = palindrome radius centered at i
    std::size_t center = 0;             // center of the current right-most palindrome
    std::size_t right = 0;              // its right boundary: center + p[center]

    // Skip the two sentinel positions (index 0 and m-1).
    for (std::size_t i = 1; i + 1 < m; ++i) {
        if (i < right) {
            const std::size_t mirror = 2 * center - i;   // reflection of i across center
            p[i] = std::min(right - i, p[mirror]);       // reuse the mirror's radius
        }
        // Expand only the unknown part. The sentinels guarantee we stop before
        // reading out of range, so (i - p[i] - 1) never underflows here.
        while (t[i + p[i] + 1] == t[i - p[i] - 1]) ++p[i];
        if (i + p[i] > right) { center = i; right = i + p[i]; }
    }

    std::size_t bestLen = 0, bestCenter = 0;
    for (std::size_t i = 1; i + 1 < m; ++i)
        if (p[i] > bestLen) { bestLen = p[i]; bestCenter = i; }

    ManacherResult r;
    r.length = bestLen;
    r.start  = (bestCenter - bestLen) / 2;   // map padded center back to the original
    r.longest = s.substr(r.start, r.length);
    r.radii  = std::move(p);
    return r;
}

// ---- Naive O(n^3) reference: length of the longest palindromic substring. ----
static std::size_t naiveLongestPalindromeLength(const std::string& s) {
    const std::size_t n = s.size();
    std::size_t best = 0;
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = i; j < n; ++j) {
            std::size_t lo = i, hi = j;
            bool pal = true;
            while (lo < hi) { if (s[lo] != s[hi]) { pal = false; break; } ++lo; --hi; }
            if (pal && j - i + 1 > best) best = j - i + 1;
        }
    return best;
}

int main() {
    // Known longest-palindrome results (the guidance's examples).
    ManacherResult r1 = manacher("babad");
    assert(r1.length == 3);
    assert(r1.longest == "bab" || r1.longest == "aba");   // both are valid answers

    ManacherResult r2 = manacher("cbbd");
    assert(r2.length == 2 && r2.longest == "bb");

    assert(manacher("a").longest == "a");                 // single character
    assert(manacher("aaaa").longest == "aaaa");           // all-same
    assert(manacher("").length == 0);                     // empty -> empty palindrome
    assert(manacher("abc").length == 1);                  // no repeat -> length 1

    // Cross-check the length against the naive reference on several strings.
    const char* samples[] = {
        "", "a", "ab", "aa", "abcba", "forgeeksskeegfor",
        "banana", "abacabad", "xxyxxyx", "civicracecar"
    };
    for (const char* text : samples) {
        std::string s(text);
        ManacherResult r = manacher(s);
        assert(r.length == naiveLongestPalindromeLength(s));
        // The reported substring must really be a palindrome of the stated length.
        assert(r.longest.size() == r.length);
        assert(std::equal(r.longest.begin(), r.longest.end(), r.longest.rbegin()));
        // And it must occur at the reported position inside the original string.
        assert(s.substr(r.start, r.length) == r.longest);
    }

    std::cout << "Manacher demo\n";
    for (const std::string& demo : {std::string("forgeeksskeegfor"), std::string("babad")}) {
        ManacherResult r = manacher(demo);
        std::cout << "  \"" << demo << "\" -> \"" << r.longest
                  << "\" (start=" << r.start << ", len=" << r.length << ")\n";
    }
    std::cout << "All Manacher assertions passed.\n";
    return 0;
}
