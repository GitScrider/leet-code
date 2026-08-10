/*
 * Maximum-Sum Contiguous Subarray -- Kadane's Algorithm
 * (Algorithm - Dynamic Programming)
 *
 * Problem:
 *   Given an array of integers (which may be negative), find the contiguous,
 *   non-empty subarray with the largest sum, and report that sum together with
 *   the subarray's [start, end] index range.
 *
 * Idea:
 *   State:
 *     best_ending_here = the maximum sum of a subarray that ENDS exactly at the
 *                        current index i. (This is the DP state -- one value per
 *                        position, though we only keep the running one.)
 *   Recurrence:
 *     A subarray ending at i either extends the best subarray ending at i-1, or
 *     starts fresh at i. We take whichever is larger:
 *         best_ending_here(i) = max( a[i], best_ending_here(i-1) + a[i] )
 *     The global answer is the max over all i of best_ending_here(i):
 *         answer = max_i best_ending_here(i)
 *   Base case:
 *     best_ending_here(0) = a[0]   (a one-element subarray).
 *
 * Why it is correct / optimal substructure:
 *   The best subarray ending at i, if it has length > 1, must consist of the
 *   best subarray ending at i-1 followed by a[i] -- otherwise we could swap in a
 *   better prefix and improve it, a contradiction. So the optimal solution to
 *   the length-i problem is built from the optimal solution to the length-(i-1)
 *   problem: classic optimal substructure with a single overlapping subproblem
 *   reused at each step.
 *
 * All-negative arrays:
 *   Because we compare against a[i] alone (never against 0 / the empty subarray),
 *   the answer for an all-negative array is correctly its maximum single element.
 *
 * Complexity:
 *   +------------------------+-----------+-----------+
 *   |  Method                |   Time    |   Space   |
 *   +------------------------+-----------+-----------+
 *   |  Kadane (this file)    |   O(n)    |   O(1)    |
 *   |  Brute force reference |   O(n^2)  |   O(1)    |
 *   +------------------------+-----------+-----------+
 *   Kadane keeps only the running "best ending here" plus the global best and a
 *   few indices, so O(1) extra space.
 *
 * Key points:
 *   - This is DP with the table collapsed to a single rolling scalar; there is
 *     no benefit to a full array here.
 *   - To recover the RANGE, we remember where the current run started: whenever
 *     we "start fresh" (a[i] beats the extension) we move the tentative start to
 *     i, and we snapshot [start, i] whenever a new global maximum is found.
 */

#include <vector>
#include <cassert>
#include <iostream>
#include <cstddef>
#include <climits>

// Result: the best sum plus the inclusive index range [start, end] achieving it.
struct MaxSubarray {
    long long sum;
    std::size_t start;
    std::size_t end;
};

// Kadane's algorithm. Precondition: 'a' is non-empty.
MaxSubarray kadane(const std::vector<int>& a) {
    assert(!a.empty() && "kadane requires a non-empty array");

    // Initialize with the first element as a length-1 subarray.
    long long bestEndingHere = a[0];
    std::size_t curStart = 0;

    MaxSubarray best{a[0], 0, 0};

    for (std::size_t i = 1; i < a.size(); ++i) {
        const long long x = a[i];
        // Extend the previous run, or start a new one at i -- whichever is larger.
        if (bestEndingHere + x < x) {
            bestEndingHere = x;       // start fresh: the old prefix only hurt us
            curStart = i;
        } else {
            bestEndingHere += x;      // extend the existing run
        }
        // Snapshot whenever the running value sets a new global record.
        if (bestEndingHere > best.sum) {
            best.sum = bestEndingHere;
            best.start = curStart;
            best.end = i;
        }
    }
    return best;
}

// O(n^2) brute-force reference: try every [i, j] range explicitly.
MaxSubarray bruteForce(const std::vector<int>& a) {
    assert(!a.empty());
    MaxSubarray best{LLONG_MIN, 0, 0};
    for (std::size_t i = 0; i < a.size(); ++i) {
        long long running = 0;
        for (std::size_t j = i; j < a.size(); ++j) {
            running += a[j];
            if (running > best.sum) {
                best.sum = running;
                best.start = i;
                best.end = j;
            }
        }
    }
    return best;
}

int main() {
    // Fixed test arrays covering positive/negative mixes and edge shapes.
    const std::vector<std::vector<int>> tests = {
        {-2, 1, -3, 4, -1, 2, 1, -5, 4},   // classic; answer 6 from [3..6]
        {1, 2, 3, 4, 5},                    // all positive -> whole array
        {-8, -3, -6, -2, -5, -4},           // all negative -> max element -2
        {5},                                // single positive
        {-7},                               // single negative
        {0, 0, 0},                          // all zeros
        {3, -1, 4, -1, 5, -9, 2, 6},        // mixed
        {-1, -2, -3, -1},                   // all negative, best is a[0] = -1
    };

    for (const std::vector<int>& a : tests) {
        const MaxSubarray k = kadane(a);
        const MaxSubarray b = bruteForce(a);
        // The maximum SUM must match the brute-force reference exactly.
        assert(k.sum == b.sum);
        // The reported range must actually sum to that value (range validity).
        long long check = 0;
        for (std::size_t i = k.start; i <= k.end; ++i) check += a[i];
        assert(check == k.sum);
        assert(k.start <= k.end && k.end < a.size());
    }

    // Hardcoded spot-checks on the classic example.
    const std::vector<int> classic = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    const MaxSubarray c = kadane(classic);
    assert(c.sum == 6);
    assert(c.start == 3 && c.end == 6);   // subarray {4, -1, 2, 1}

    // All-negative array returns the single largest element.
    const std::vector<int> neg = {-8, -3, -6, -2, -5, -4};
    const MaxSubarray n = kadane(neg);
    assert(n.sum == -2);
    assert(n.start == 3 && n.end == 3);

    // Short demo output.
    std::cout << "Kadane on {-2,1,-3,4,-1,2,1,-5,4}: best sum = " << c.sum
              << " over indices [" << c.start << ", " << c.end << "]\n";
    std::cout << "Subarray:";
    for (std::size_t i = c.start; i <= c.end; ++i) std::cout << ' ' << classic[i];
    std::cout << "\nAll Kadane tests passed.\n";
    return 0;
}
