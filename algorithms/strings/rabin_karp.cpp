/*
 * Rabin-Karp String Match (Algorithm - Strings)
 *
 * Idea:
 *   Comparing the pattern against every window of the text costs O(m) per
 *   window. Rabin-Karp replaces most of those O(m) comparisons with an O(1)
 *   hash comparison. Treat a length-m string as a base-B number modulo a large
 *   value:
 *       hash(s) = (s[0]*B^(m-1) + s[1]*B^(m-2) + ... + s[m-1]*B^0) mod M
 *
 *   The trick is the ROLLING update: once we know the hash of window
 *   T[i .. i+m-1], the hash of the next window T[i+1 .. i+m] is obtained in
 *   O(1) by removing the leading character's contribution and appending the
 *   new trailing character:
 *       h' = ( (h - T[i]*B^(m-1)) * B + T[i+m] ) mod M
 *
 *   A hash match is only PROBABLE evidence, never proof: two different strings
 *   can share a hash (a "collision"). So on every hash hit we VERIFY the window
 *   character by character. This keeps the algorithm exact while collisions
 *   merely cost an occasional wasted verification.
 *
 * Collision handling:
 *   We use a large prime modulus M and a random-ish base B to make collisions
 *   rare, and we ALWAYS verify on a hash hit, so correctness never depends on
 *   the hash being collision-free -- only performance does. (A uint64 natural-
 *   overflow variant is possible but is easier to attack adversarially, so the
 *   explicit prime modulus is used here.)
 *
 * Complexity:
 *   +-----------+-------------------+--------------------------------------+
 *   | Aspect    | Rabin-Karp        | vs naive O(n*m)                      |
 *   +-----------+-------------------+--------------------------------------+
 *   | Preprocess| O(m)              | pattern hash + B^(m-1)               |
 *   | Search    | O(n + m) expected | O(n*m) worst if every window collides|
 *   | Space     | O(1) extra        | a few integer accumulators           |
 *   +-----------+-------------------+--------------------------------------+
 *
 * Key points:
 *   - Rolling hash makes each window transition O(1).
 *   - Always verify on a hash hit -> exact matching despite collisions.
 *   - Modular arithmetic in std::uint64_t; the "+M" before mod avoids a
 *     transient negative when removing the leading term.
 *   - Empty pattern convention: matches at every index 0..n inclusive.
 */

#include <string>
#include <vector>
#include <utility>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <iostream>

namespace {
constexpr std::uint64_t kBase = 256;         // treat each char as a base-256 digit
constexpr std::uint64_t kMod  = 1000000007;  // large prime modulus
}  // namespace

// Return 0-indexed start positions of every occurrence (incl. overlaps).
// Empty-pattern convention: matches at every index 0..text.size() inclusive.
std::vector<std::size_t> rabin_karp_search(const std::string& text,
                                           const std::string& pattern) {
    std::vector<std::size_t> matches;
    const std::size_t n = text.size();
    const std::size_t m = pattern.size();

    if (m == 0) {
        for (std::size_t i = 0; i <= n; ++i) matches.push_back(i);
        return matches;
    }
    if (m > n) return matches;

    // highPow = B^(m-1) mod M -- the weight of the leading character.
    std::uint64_t high_pow = 1;
    for (std::size_t k = 0; k + 1 < m; ++k) {
        high_pow = (high_pow * kBase) % kMod;
    }

    // Hash of the pattern and of the first text window T[0..m-1].
    std::uint64_t pattern_hash = 0;
    std::uint64_t window_hash = 0;
    for (std::size_t k = 0; k < m; ++k) {
        pattern_hash = (pattern_hash * kBase +
                        static_cast<std::uint8_t>(pattern[k])) % kMod;
        window_hash = (window_hash * kBase +
                       static_cast<std::uint8_t>(text[k])) % kMod;
    }

    for (std::size_t i = 0; i + m <= n; ++i) {
        if (window_hash == pattern_hash) {
            // Hash hit: VERIFY to rule out a collision.
            std::size_t j = 0;
            while (j < m && text[i + j] == pattern[j]) ++j;
            if (j == m) matches.push_back(i);
        }
        // Roll the hash forward to the window starting at i+1 (if any).
        if (i + m < n) {
            const std::uint64_t lead =
                static_cast<std::uint8_t>(text[i]);
            const std::uint64_t tail =
                static_cast<std::uint8_t>(text[i + m]);
            // Remove leading char, shift left by one base digit, add new tail.
            // Add kMod before subtracting to keep the value non-negative.
            window_hash = (window_hash + kMod - (lead * high_pow) % kMod) % kMod;
            window_hash = (window_hash * kBase + tail) % kMod;
        }
    }
    return matches;
}

// Naive O(n*m) reference used only to validate rabin_karp_search.
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
        {"the cat sat", " "},      // single-space pattern
    };
    for (const auto& c : cases) {
        assert(rabin_karp_search(c.first, c.second) ==
               naive_ref(c.first, c.second));
    }

    // Spot-check specific expected values.
    assert((rabin_karp_search("aaaa", "aa") ==
            std::vector<std::size_t>{0, 1, 2}));
    assert((rabin_karp_search("mississippi", "issi") ==
            std::vector<std::size_t>{1, 4}));
    assert(rabin_karp_search("abcdef", "gh").empty());

    // ---- Short demo --------------------------------------------------------
    const std::string text = "mississippi";
    const std::string pattern = "issi";
    std::cout << "Rabin-Karp string match demo\n";
    std::cout << "text    = \"" << text << "\"\n";
    std::cout << "pattern = \"" << pattern << "\"\n";
    std::cout << "matches at:";
    for (const std::size_t pos : rabin_karp_search(text, pattern)) {
        std::cout << ' ' << pos;
    }
    std::cout << '\n';
    std::cout << "All assertions passed.\n";
    return 0;
}
