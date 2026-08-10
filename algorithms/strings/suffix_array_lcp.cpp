/*
 * Suffix Array + LCP Array  (Algorithm - Strings)
 * -----------------------------------------------------------------------------
 * Problem : Sort all n suffixes of a string (the SUFFIX ARRAY, SA) and, for
 *           adjacent suffixes in that order, the length of their longest common
 *           prefix (the LCP ARRAY).
 *
 * Idea (SA): PREFIX DOUBLING. Sort suffixes by their first 2^k characters,
 *           doubling k each round. After round k every suffix has a rank based
 *           on its first 2^k chars. To compare by 2^(k+1) chars we use the pair
 *           ( rank[i], rank[i + 2^k] )  -- both already known -- so each round
 *           is a single comparison sort. About log2(n) rounds, each O(n log n)
 *           with std::sort, giving O(n log^2 n). (A radix sort per round would
 *           make it O(n log n); comparison sort is used here for readability.)
 *
 * Idea (LCP): KASAI'S ALGORITHM, O(n). Process suffixes in ORIGINAL order.
 *           If suffix i has LCP h with its predecessor in SA, then suffix i+1
 *           has LCP at least h-1 with ITS predecessor -- so h drops by at most
 *           one per step and never has to restart from zero. Total work O(n).
 *
 *   Complexity (n = |s|)
 *   +---------------------+---------------------+-------------------------+
 *   | Step                | This file           | Naive                   |
 *   +---------------------+---------------------+-------------------------+
 *   | Build suffix array  | O(n log^2 n)        | O(n^2 log n) (sort strs)|
 *   | Build LCP (Kasai)   | O(n)                | O(n^2)                  |
 *   | Space               | O(n)                | O(n)                    |
 *   +---------------------+---------------------+-------------------------+
 *
 * Complexity derivation (SA: rounds x comparison sort; LCP: amortized h-carry):
 *   SUFFIX ARRAY (prefix doubling). Round 0 assigns rank = first character in
 *   O(n). Each doubling round r (k = 2^r, r = 0, 1, 2, ...) performs:
 *       - one std::sort of n indices with an O(1) pair comparator -> c1*n*log2 n
 *       - one linear re-rank pass over the n suffixes             -> c2*n
 *   A round distinguishes suffixes by their first 2^(r+1) characters, so the loop
 *   stops once 2^(r+1) >= n, i.e. after R = ceil(log2 n) rounds (fewer if all ranks
 *   become distinct earlier). Summing the per-round cost over the R rounds:
 *
 *       T_SA(n) = SUM_{r=0}^{R-1} (c1*n*log2 n + c2*n)
 *               = R * (c1*n*log2 n + c2*n)
 *               = ceil(log2 n) * (c1*n*log2 n + c2*n)
 *               = O(log n * n log n) = O(n log^2 n)
 *
 *   The extra log factor over an ideal O(n log n) suffix array is exactly the
 *   R = log n rounds, each paying the comparison sort's own log n. (Radix-sorting
 *   the (rank[i], rank[i+k]) pairs per round would drop each sort to O(n) and give
 *   the O(n log n) build.)
 *
 *   LCP (Kasai), amortized. The outer loop runs n times. The carry variable h is
 *   incremented once per matched character in the while loop and decremented by at
 *   most 1 per outer iteration (the "next suffix keeps h-1" step). Since h >= 0
 *   always, starts at 0, and ends <= n:
 *
 *       (total increments) = (total decrements) + (final h)
 *                          <= n                 + n         = 2n
 *
 *   So the while body runs <= 2n times over the WHOLE run, not per i. Adding the
 *   O(n) inverse-rank pass and the n outer iterations:
 *
 *       T_LCP(n) = n (inverse rank) + n (outer loop) + 2n (matches) = O(n)
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f(n) = O(g)      iff  EXISTS c2, n0 :       f(n) <= c2*g(n)  for n >= n0
 *     f(n) = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f(n) = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   SUFFIX ARRAY is data-dependent in the ROUND COUNT R:
 *     WORST case (periodic / few distinct chars, e.g. "aaaa"): R = ceil(log2 n)
 *       rounds, each an n log n sort  =>  Theta(n log^2 n) (tight).
 *     BEST case (all ranks distinct after one doubling, e.g. all-distinct chars):
 *       R = 1 round, but still one full std::sort  =>  Theta(n log n) (tight).
 *     Over ALL inputs: O(n log^2 n) (from worst) and Omega(n log n) (from best);
 *     it is not a single Theta because best != worst. The Omega(n log n) here is
 *     the COMPARISON-SORT lower bound paid every round; it is NOT a lower bound for
 *     the suffix-array PROBLEM -- SA-IS / DC3 build in O(n) via radix (non-
 *     comparison) sorting, so n log^2 n reflects THIS method only.
 *   LCP (Kasai) is order-invariant: the amortized count obeys 2n <= f(n) <= 4n, so
 *     with g(n) = n it is squeezed both sides (c1 = 2, c2 = 4, n0 = 1) =>
 *     Theta(n) for best = average = worst.
 *
 * Key points:
 *   - Doubling reuses previous ranks: comparing 2^(k+1)-prefixes costs O(1) via
 *     the (rank[i], rank[i+2^k]) pair -- no character re-scanning.
 *   - Kasai's h-1 carry-over is what turns an O(n^2) LCP into O(n).
 *   - Application shown: number of DISTINCT substrings
 *        = n*(n+1)/2 - sum(LCP)
 *     (every substring is a prefix of some suffix; LCP removes the duplicates
 *      shared between adjacent suffixes).
 */

#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <cassert>
#include <iostream>
#include <cstddef>
#include <cstdint>

// Suffix array via prefix doubling. Returns SA as suffix start indices.
std::vector<int> buildSuffixArray(const std::string& s) {
    const int n = static_cast<int>(s.size());
    std::vector<int> sa(n), rnk(n), tmp(n);
    if (n == 0) return sa;

    for (int i = 0; i < n; ++i) {
        sa[i]  = i;
        rnk[i] = static_cast<unsigned char>(s[i]);   // round 0: rank = first char
    }

    for (int k = 1; ; k <<= 1) {
        // Order by (rank[a], rank[a+k]); out-of-range second key is -1 (smallest).
        auto cmp = [&](int a, int b) {
            if (rnk[a] != rnk[b]) return rnk[a] < rnk[b];
            const int ra = (a + k < n) ? rnk[a + k] : -1;
            const int rb = (b + k < n) ? rnk[b + k] : -1;
            return ra < rb;
        };
        std::sort(sa.begin(), sa.end(), cmp);

        // Re-rank: equal adjacent suffixes share a rank, otherwise rank increases.
        tmp[sa[0]] = 0;
        for (int i = 1; i < n; ++i)
            tmp[sa[i]] = tmp[sa[i - 1]] + (cmp(sa[i - 1], sa[i]) ? 1 : 0);
        for (int i = 0; i < n; ++i) rnk[i] = tmp[i];

        if (rnk[sa[n - 1]] == n - 1) break;   // all ranks distinct -> fully sorted
    }
    return sa;
}

// LCP array via Kasai's algorithm. lcp[i] = LCP(SA[i-1], SA[i]); lcp[0] = 0.
std::vector<int> buildLCP(const std::string& s, const std::vector<int>& sa) {
    const int n = static_cast<int>(s.size());
    std::vector<int> rnk(n), lcp(n, 0);
    for (int i = 0; i < n; ++i) rnk[sa[i]] = i;   // inverse permutation of SA

    int h = 0;                                    // current LCP length, carried over
    for (int i = 0; i < n; ++i) {
        if (rnk[i] > 0) {
            const int j = sa[rnk[i] - 1];         // predecessor of suffix i in SA
            while (i + h < n && j + h < n && s[i + h] == s[j + h]) ++h;
            lcp[rnk[i]] = h;
            if (h > 0) --h;                        // next suffix keeps at least h-1
        } else {
            h = 0;                                 // suffix i is first in SA order
        }
    }
    return lcp;
}

// Count of distinct substrings = n*(n+1)/2 - sum(LCP).
std::int64_t countDistinctSubstrings(const std::string& s, const std::vector<int>& lcp) {
    const std::int64_t n = static_cast<std::int64_t>(s.size());
    std::int64_t total = n * (n + 1) / 2, sum = 0;
    for (int v : lcp) sum += v;
    return total - sum;
}

// ---- Naive references for cross-checking the two arrays. ----
static std::vector<int> naiveSuffixArray(const std::string& s) {
    const int n = static_cast<int>(s.size());
    std::vector<int> sa(n);
    for (int i = 0; i < n; ++i) sa[i] = i;
    std::sort(sa.begin(), sa.end(),
              [&](int a, int b) { return s.substr(a) < s.substr(b); });
    return sa;
}
static std::size_t naiveDistinctSubstrings(const std::string& s) {
    std::set<std::string> seen;
    for (std::size_t i = 0; i < s.size(); ++i)
        for (std::size_t len = 1; i + len <= s.size(); ++len)
            seen.insert(s.substr(i, len));
    return seen.size();
}

int main() {
    // Known result for "banana".
    const std::string banana = "banana";
    std::vector<int> sa  = buildSuffixArray(banana);
    std::vector<int> lcp = buildLCP(banana, sa);
    assert((sa  == std::vector<int>{5, 3, 1, 0, 4, 2}));   // a,ana,anana,banana,na,nana
    assert((lcp == std::vector<int>{0, 1, 3, 0, 0, 2}));
    assert(countDistinctSubstrings(banana, lcp) == 15);

    // Cross-check SA and distinct-substring count against naive on several strings.
    const char* samples[] = {
        "banana", "mississippi", "abracadabra", "aaaa", "abcabcabc",
        "a", "", "zzzzzy", "the quick brown fox"
    };
    for (const char* text : samples) {
        std::string s(text);
        std::vector<int> a  = buildSuffixArray(s);
        std::vector<int> l  = buildLCP(s, a);
        assert(a == naiveSuffixArray(s));
        assert(static_cast<std::size_t>(countDistinctSubstrings(s, l))
               == naiveDistinctSubstrings(s));
    }

    std::cout << "Suffix array demo for \"banana\"\n  SA :";
    for (int x : sa)  std::cout << ' ' << x;
    std::cout << "\n  LCP:";
    for (int x : lcp) std::cout << ' ' << x;
    std::cout << "\n  distinct substrings = "
              << countDistinctSubstrings(banana, lcp) << '\n';
    std::cout << "All suffix-array / LCP assertions passed.\n";
    return 0;
}
