/*
 * Subset Sum via Meet-in-the-Middle - Algorithm - NP-Complete
 * ===========================================================
 *
 * Decision problem (SUBSET-SUM):
 *   Given a multiset of non-negative integers a[0..n-1] and a target T,
 *   does there exist a subset S (subset of indices) with sum(a[i], i in S) == T?
 *
 * Complexity class:
 *   SUBSET-SUM is NP-complete. It is one of Karp's 21 problems; PARTITION,
 *   KNAPSACK and many others reduce to (or from) it. There is no known
 *   polynomial-time algorithm. The number T is given in binary, so an
 *   algorithm polynomial in n AND in the VALUE of T (see the DP note below)
 *   is only "pseudo-polynomial", not polynomial in the input SIZE.
 *
 * Algorithms implemented here (both EXACT):
 *   +--------------------------+-------------------------+--------------------+
 *   | Method                   | Time                    | Space              |
 *   +--------------------------+-------------------------+--------------------+
 *   | Brute force (reference)  | O(2^n * n)              | O(1)               |
 *   | Meet-in-the-middle       | O(2^(n/2) * n)          | O(2^(n/2))         |
 *   +--------------------------+-------------------------+--------------------+
 *   Meet-in-the-middle (Horowitz-Sahni, 1974) turns 2^n into 2 * 2^(n/2):
 *   e.g. n = 40 drops from ~1e12 down to ~1e6. Still EXPONENTIAL.
 *
 *   Pseudo-polynomial contrast: a boolean DP over reachable sums runs in
 *   O(n * T) time / O(T) space (see dynamic-programming/subset_sum_partition.cpp).
 *   That is great when T is small but blows up when T is a large binary number;
 *   meet-in-the-middle is the method of choice when n is small but T is huge.
 *
 * Complexity derivation (enumerate -> sort -> binary-search each half):
 *   BRUTE FORCE. The loop runs over every subset mask in [0, 2^n); each mask
 *   sums up to n items:
 *
 *       C_brute(n) = SUM_{mask=0}^{2^n - 1} ( n ) = n * 2^n = O(2^n * n),
 *
 *   with O(1) extra space (a running sum only).
 *
 *   MEET-IN-THE-MIDDLE. Split n into two halves of size ~n/2. Let M = 2^(n/2)
 *   be the number of subsets per half. The three phases:
 *     (1) enumerateHalf() on each half: M subsets, each summed in O(n/2) work
 *             2 * M * (n/2) = Theta(n * 2^(n/2))
 *     (2) sort one half's M sums (comparison sort)
 *             M * log2(M) = 2^(n/2) * (n/2) = Theta(n * 2^(n/2))
 *     (3) for each of the M left sums, one binary search over the M right sums
 *             M * log2(M) = 2^(n/2) * (n/2) = Theta(n * 2^(n/2))
 *   Adding the phases:
 *
 *       C_mitm(n) = Theta(n*2^(n/2)) + Theta(n*2^(n/2)) + Theta(n*2^(n/2))
 *                 = O(2^(n/2) * n)
 *
 *   Space: the two sum arrays hold 2 * M pairs -> O(2^(n/2)). This HALVES the
 *   exponent (2^n -> 2^(n/2)) but the cost is still exponential in n.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f = O(g)      iff  EXISTS c2, n0 :        f(n) <= c2*g(n)  for n >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   MITM: phases (1) and (2) run unconditionally regardless of the answer; the
 *     early return in phase (3) only shortens the final scan, which is already
 *     bounded by the enumeration/sort cost. Hence both directions hold:
 *         c1 * n*2^(n/2) <= C_mitm <= c2 * n*2^(n/2)  =>  Theta(2^(n/2) * n).
 *   The comparison-sort lower bound Omega(M log M) with M = 2^(n/2) equals
 *     2^(n/2) * (n/2) -- exactly the enumeration cost -- so the internal sort is
 *     asymptotically optimal and adds no extra order. That Omega(n log n)-style
 *     bound governs only that sort; the problem's exponential difficulty is
 *     combinatorial (NP-complete), not a sorting bound. The pseudo-polynomial
 *     DP alternative is Theta(n*T): polynomial in n and the VALUE T, but
 *     exponential in the bit-LENGTH of T, so not a true polynomial bound.
 *
 * Key points:
 *   - Split the n items into two halves; enumerate all 2^(n/2) subset sums of
 *     each half. Sort one half's sums, then for every sum s of the other half
 *     binary-search for (T - s). A hit means two half-subsets combine to T.
 *   - Subsets are encoded as bitmasks over std::uint32_t: bit i set == item i
 *     is included. This gives a compact witness we can reconstruct and verify.
 *   - Cross-checked against the 2^n brute force on many random instances, and
 *     every reported YES witness is re-summed to confirm it truly equals T.
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

// A (sum, mask) pair: the subset-sum value and the bitmask that produced it.
using SumMask = std::pair<std::int64_t, std::uint32_t>;

// Enumerate every subset sum of one half, tagging each with its bitmask.
// There are 2^k subsets; bit i of `mask` means "include half[i]".
static std::vector<SumMask> enumerateHalf(const std::vector<std::int64_t>& half) {
    const std::size_t k = half.size();
    // Guard the shift: 1u << k is undefined for k >= 32, and we also need
    // 2^k to fit comfortably in a uint32 loop counter.
    assert(k <= 30 && "half too large for a uint32 subset mask");
    const std::uint32_t subsets = 1u << k; // 2^k (k==0 -> 1, the empty subset)

    std::vector<SumMask> out;
    out.reserve(subsets);
    for (std::uint32_t mask = 0; mask < subsets; ++mask) {
        std::int64_t s = 0;
        for (std::size_t i = 0; i < k; ++i) {
            // (mask >> i) & 1u tests bit i without forming (1u << i) each time.
            if ((mask >> i) & 1u) s += half[i];
        }
        out.emplace_back(s, mask);
    }
    return out;
}

// Exact O(2^n * n) reference solver used only to validate meet-in-the-middle.
static bool subsetSumBrute(const std::vector<std::int64_t>& a, std::int64_t target) {
    const std::size_t n = a.size();
    assert(n <= 24 && "brute force is exponential; keep n small");
    const std::uint32_t subsets = 1u << n; // 2^n
    for (std::uint32_t mask = 0; mask < subsets; ++mask) {
        std::int64_t s = 0;
        for (std::size_t i = 0; i < n; ++i)
            if ((mask >> i) & 1u) s += a[i];
        if (s == target) return true;
    }
    return false;
}

// Meet-in-the-middle exact solver. Returns true iff some subset sums to target.
// If `witness` is non-null and the answer is YES, it is filled with the indices
// (into `a`) of one satisfying subset.
static bool subsetSumMITM(const std::vector<std::int64_t>& a, std::int64_t target,
                          std::vector<std::size_t>* witness = nullptr) {
    const std::size_t n = a.size();
    const std::size_t mid = n / 2;

    const std::vector<std::int64_t> left(a.begin(), a.begin() + static_cast<std::ptrdiff_t>(mid));
    const std::vector<std::int64_t> right(a.begin() + static_cast<std::ptrdiff_t>(mid), a.end());

    const std::vector<SumMask> leftSums = enumerateHalf(left);
    std::vector<SumMask> rightSums = enumerateHalf(right);

    // Sort the right half's sums so we can binary-search complements.
    std::sort(rightSums.begin(), rightSums.end(),
              [](const SumMask& p, const SumMask& q) { return p.first < q.first; });

    for (const SumMask& lp : leftSums) {
        // We need a right-subset summing to (target - lp.first). Inputs are
        // non-negative so this stays within int64 range for sane instances.
        const std::int64_t need = target - lp.first;

        // lower_bound with a projection comparator on the sum component.
        auto it = std::lower_bound(rightSums.begin(), rightSums.end(), need,
                                   [](const SumMask& p, std::int64_t val) { return p.first < val; });
        if (it != rightSums.end() && it->first == need) {
            if (witness) {
                witness->clear();
                const std::uint32_t ml = lp.second;
                for (std::size_t i = 0; i < mid; ++i)
                    if ((ml >> i) & 1u) witness->push_back(i);
                const std::uint32_t mr = it->second;
                for (std::size_t i = 0; i < right.size(); ++i)
                    if ((mr >> i) & 1u) witness->push_back(mid + i); // right indices are shifted
            }
            return true;
        }
    }
    return false;
}

// Sum a subset given by indices -- used to validate a returned witness.
static std::int64_t sumOfIndices(const std::vector<std::int64_t>& a,
                                 const std::vector<std::size_t>& idx) {
    std::int64_t s = 0;
    for (const std::size_t i : idx) {
        assert(i < a.size());
        s += a[i];
    }
    return s;
}

int main() {
    // --- Edge cases ---------------------------------------------------------
    // The empty set sums to 0: T = 0 is always reachable (empty subset).
    assert(subsetSumMITM({}, 0));
    assert(!subsetSumMITM({}, 5));          // no items -> only sum 0 is reachable

    // --- Known YES / NO instances ------------------------------------------
    const std::vector<std::int64_t> nums = {3, 34, 4, 12, 5, 2}; // total = 60
    std::vector<std::size_t> w;

    assert(subsetSumMITM(nums, 9, &w));     // 4 + 5 = 9
    assert(sumOfIndices(nums, w) == 9);     // witness really sums to the target

    assert(subsetSumMITM(nums, 60, &w));    // the whole set
    assert(sumOfIndices(nums, w) == 60);

    assert(!subsetSumMITM(nums, 1));        // 1 is below the smallest item (2)
    assert(!subsetSumMITM(nums, 61));       // above the total sum -> impossible

    // --- Randomized cross-check: MITM must agree with brute force ----------
    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> sizeDist(0, 14);
    std::uniform_int_distribution<int> valDist(0, 20);

    for (int trial = 0; trial < 4000; ++trial) {
        const std::size_t n = static_cast<std::size_t>(sizeDist(rng));
        std::vector<std::int64_t> a(n);
        std::int64_t total = 0;
        for (std::size_t i = 0; i < n; ++i) {
            a[i] = valDist(rng);
            total += a[i];
        }
        // Pick a target both inside and just outside the reachable range.
        std::uniform_int_distribution<int> tgtDist(-2, static_cast<int>(total) + 2);
        const std::int64_t target = tgtDist(rng);

        std::vector<std::size_t> wit;
        const bool mitm = subsetSumMITM(a, target, &wit);
        const bool brute = subsetSumBrute(a, target);
        assert(mitm == brute);              // exact solver agrees with reference
        if (mitm) assert(sumOfIndices(a, wit) == target); // and the witness is valid
    }

    // --- Short demo ---------------------------------------------------------
    std::cout << "Subset Sum (meet-in-the-middle)\n";
    std::cout << "items = {3, 34, 4, 12, 5, 2}, target = 9 -> ";
    if (subsetSumMITM(nums, 9, &w)) {
        std::cout << "YES  subset {";
        for (std::size_t j = 0; j < w.size(); ++j)
            std::cout << (j ? ", " : "") << nums[w[j]];
        std::cout << "}\n";
    } else {
        std::cout << "NO\n";
    }
    std::cout << "target = 61 -> " << (subsetSumMITM(nums, 61) ? "YES" : "NO") << "\n";
    std::cout << "All assertions passed.\n";
    return 0;
}
