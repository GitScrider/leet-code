/*
 * Longest Increasing Subsequence (LIS)
 * Category: Algorithm - Dynamic Programming
 *
 * Idea
 * ----
 * Given an array a[0..n-1], find the length of the longest strictly increasing
 * subsequence (elements keep their order, values strictly grow).
 *
 * (1) O(n^2) DP with reconstruction
 *   State:      dp[i] = length of the longest increasing subsequence that ENDS
 *                       exactly at index i.
 *   Recurrence: dp[i] = 1 + max{ dp[j] : j < i and a[j] < a[i] }   (0 if none)
 *   Base case:  dp[i] = 1 for every i (the element alone is a subsequence).
 *   Answer:     max over all dp[i]. A parent[] array lets us rebuild one LIS.
 *
 * (2) O(n log n) patience / binary-search method
 *   Maintain tails[k] = the smallest possible tail value of an increasing
 *   subsequence of length k+1 seen so far. tails is strictly increasing, so for
 *   each new value we binary-search (lower_bound, for STRICT increase) the first
 *   tail >= value: replace it, or append if value is larger than all tails. The
 *   final size of tails is the LIS length. (This yields length, not necessarily
 *   a contiguous witness, though one can be recovered with extra bookkeeping.)
 *
 * Why it is correct
 * -----------------
 * Optimal substructure: an LIS ending at i is some LIS ending at an earlier j
 * with a[j] < a[i], extended by a[i]; so dp[i] only depends on smaller-index
 * subproblems. Overlapping subproblems: dp[j] is reused by every later i that
 * can sit on top of it. For method (2), keeping the minimal tail per length is
 * greedy-optimal: a smaller tail can never reduce future extension options.
 *
 * Complexity
 * ----------
 *   +-------------------+--------------+-----------+
 *   | Approach          | Time         | Space     |
 *   +-------------------+--------------+-----------+
 *   | DP (n^2)          | O(n^2)       | O(n)      |
 *   | Patience (n log n)| O(n log n)   | O(n)      |
 *   +-------------------+--------------+-----------+
 *
 * Key points
 * ----------
 *  - Bottom-up DP is the intuitive version and reconstructs a concrete LIS.
 *  - The n log n method uses std::lower_bound for STRICT increase (use
 *    upper_bound instead if non-decreasing subsequences are wanted).
 *  - Both return the same length; we assert that below.
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

// (1) O(n^2) DP; fills `lis` with one longest increasing subsequence.
std::size_t lisDP(const std::vector<int>& a, std::vector<int>& lis) {
    lis.clear();
    const std::size_t n = a.size();
    if (n == 0) return 0;

    std::vector<std::size_t> dp(n, 1);   // dp[i] >= 1 always
    std::vector<std::size_t> parent(n);  // predecessor index in the best chain
    for (std::size_t i = 0; i < n; ++i) parent[i] = i;  // self => chain start

    std::size_t bestLen = 1;
    std::size_t bestEnd = 0;
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < i; ++j) {
            if (a[j] < a[i] && dp[j] + 1 > dp[i]) {
                dp[i] = dp[j] + 1;
                parent[i] = j;
            }
        }
        if (dp[i] > bestLen) {
            bestLen = dp[i];
            bestEnd = i;
        }
    }

    // Walk parents back from the best ending index to reconstruct the sequence.
    std::vector<int> rev;
    std::size_t cur = bestEnd;
    while (true) {
        rev.push_back(a[cur]);
        if (parent[cur] == cur) break;  // reached a chain start
        cur = parent[cur];
    }
    lis.assign(rev.rbegin(), rev.rend());
    return bestLen;
}

// (2) O(n log n) patience sorting; returns LIS length only.
std::size_t lisFast(const std::vector<int>& a) {
    std::vector<int> tails;  // tails[k] = min tail of an increasing subseq of len k+1
    for (const int x : a) {
        // First tail >= x. lower_bound => STRICT increase (duplicates not chained).
        auto it = std::lower_bound(tails.begin(), tails.end(), x);
        if (it == tails.end()) {
            tails.push_back(x);  // x extends the longest chain
        } else {
            *it = x;             // x gives a smaller tail for that length
        }
    }
    return tails.size();
}

// Brute-force reference: try every subset (in index order) and keep the longest
// strictly increasing one. Exponential; used only for tiny arrays.
std::size_t lisBrute(const std::vector<int>& a) {
    const std::size_t n = a.size();
    std::size_t best = 0;
    for (std::size_t mask = 0; mask < (std::size_t{1} << n); ++mask) {
        std::size_t len = 0;
        bool increasing = true;
        int prev = 0;
        bool havePrev = false;
        for (std::size_t bit = 0; bit < n; ++bit) {
            if (mask & (std::size_t{1} << bit)) {
                if (havePrev && !(prev < a[bit])) {
                    increasing = false;
                    break;
                }
                prev = a[bit];
                havePrev = true;
                ++len;
            }
        }
        if (increasing) best = std::max(best, len);
    }
    return best;
}

// Validate that `lis` is strictly increasing and a subsequence of `a`.
static bool isIncreasingSubsequence(const std::vector<int>& lis,
                                    const std::vector<int>& a) {
    for (std::size_t k = 1; k < lis.size(); ++k) {
        if (!(lis[k - 1] < lis[k])) return false;
    }
    std::size_t k = 0;
    for (std::size_t i = 0; i < a.size() && k < lis.size(); ++i) {
        if (a[i] == lis[k]) ++k;
    }
    return k == lis.size();
}

int main() {
    // Guidance case: [10,9,2,5,3,7,101,18] -> LIS length 4 (e.g. 2,5,7,101).
    {
        const std::vector<int> a = {10, 9, 2, 5, 3, 7, 101, 18};
        std::vector<int> lis;
        assert(lisDP(a, lis) == 4);
        assert(lisFast(a) == 4);
        assert(lis.size() == 4);
        assert(isIncreasingSubsequence(lis, a));
    }

    // Edge cases.
    {
        std::vector<int> lis;
        assert(lisDP({}, lis) == 0 && lis.empty());
        assert(lisFast({}) == 0);
        assert(lisDP({7}, lis) == 1 && lis == std::vector<int>{7});
        assert(lisFast({7}) == 1);
    }

    // Known cases.
    {
        std::vector<int> lis;
        assert(lisDP({0, 1, 0, 3, 2, 3}, lis) == 4);  // 0,1,2,3
        assert(lisFast({0, 1, 0, 3, 2, 3}) == 4);
        assert(lisDP({7, 7, 7, 7}, lis) == 1);        // strict => only 1
        assert(lisFast({7, 7, 7, 7}) == 1);
        assert(lisDP({1, 2, 3, 4, 5}, lis) == 5);
        assert(lisFast({5, 4, 3, 2, 1}) == 1);
    }

    // Cross-check DP, fast method, and brute force on many tiny arrays.
    {
        const std::vector<std::vector<int>> cases = {
            {}, {3}, {1, 2}, {2, 1}, {1, 3, 2}, {3, 1, 2, 0},
            {5, 1, 4, 2, 3}, {2, 2, 2}, {1, 5, 2, 4, 3, 6}, {9, 1, 3, 7, 5}};
        for (const auto& a : cases) {
            std::vector<int> lis;
            const std::size_t viaDp = lisDP(a, lis);
            const std::size_t viaFast = lisFast(a);
            const std::size_t viaBrute = lisBrute(a);
            assert(viaDp == viaBrute);
            assert(viaFast == viaBrute);
            assert(isIncreasingSubsequence(lis, a));
        }
    }

    // Short demo.
    {
        const std::vector<int> a = {10, 9, 2, 5, 3, 7, 101, 18};
        std::vector<int> lis;
        const std::size_t len = lisDP(a, lis);
        std::cout << "LIS length of [10,9,2,5,3,7,101,18]: " << len
                  << ", one LIS = [";
        for (std::size_t i = 0; i < lis.size(); ++i) {
            std::cout << lis[i] << (i + 1 < lis.size() ? ", " : "");
        }
        std::cout << "]\n";
        std::cout << "All LIS tests passed.\n";
    }
    return 0;
}
