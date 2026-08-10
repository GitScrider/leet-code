/*
 * Gas Station (Algorithm - Greedy)
 *
 * Problem:
 *   There are N gas stations arranged in a CIRCLE. Station i has gas[i] fuel,
 *   and it costs cost[i] fuel to drive from station i to station i+1 (indices
 *   wrap around, so station N-1 leads back to station 0). Starting with an empty
 *   tank at some station, you may travel only in the forward direction. Return
 *   the index of the UNIQUE starting station from which you can complete the full
 *   loop, or -1 if no such start exists. (When a solution exists it is unique for
 *   the classic LeetCode formulation.)
 *
 * Idea (the greedy choice and why it is safe):
 *   Let diff[i] = gas[i] - cost[i] be the NET fuel gained by crossing edge i.
 *     1) FEASIBILITY: the whole loop is completable iff sum(diff) >= 0. If the
 *        total gas is less than the total cost, no start can possibly finish, so
 *        we answer -1.
 *     2) LOCATING THE START (single pass): scan from index 0, accumulating a
 *        running tank = sum of diff over the stations visited since the last
 *        candidate start. Whenever tank drops BELOW ZERO at station i, none of the
 *        stations in the current window [start .. i] can be the true start, so we
 *        reset start = i + 1 and tank = 0.
 *   Why the reset is correct (the EXCHANGE / prefix argument):
 *     Suppose we began at 'start' and the tank first went negative when leaving
 *     station i. Then for every candidate s in [start .. i], the partial sum of
 *     diff from s up to i is also negative -- because start..i was the FIRST place
 *     the prefix sum from 'start' dipped below zero, so every intermediate prefix
 *     (from start up to any j < i) was non-negative; removing a non-negative head
 *     s>start can only make the s..i sum smaller, hence still negative. So no s in
 *     [start..i] survives to i, and we may safely skip directly to i+1. Combined
 *     with the feasibility check, the surviving 'start' must be the answer: it
 *     never dips negative on the tail, and the (negative) prefix before it is
 *     exactly compensated by the non-negative total when the loop wraps.
 *
 * Complexity:
 *   +----------------+----------------+
 *   |  Step          |  Time / Space  |
 *   +----------------+----------------+
 *   |  Single pass   |  O(N) / O(1)   |
 *   +----------------+----------------+
 *   No sorting is needed -- one linear scan settles both feasibility and start.
 *
 * Key points:
 *   - Two invariants: a GLOBAL total (feasibility) and a LOCAL running tank
 *     (candidate start). The global sum is what lets one pass suffice.
 *   - The reset is greedy because a failing window can never be "rescued" by
 *     starting later inside it -- see the prefix argument above.
 *   - Greedy is optimal here precisely because of the prefix-sum structure; if
 *     travel were bidirectional or costs depended on the tank, it would break.
 */

#include <vector>
#include <cassert>
#include <iostream>
#include <cstddef>

// Greedy single pass. Returns the unique starting index, or -1 if the circuit
// cannot be completed from any station.
int canCompleteCircuit(const std::vector<int>& gas, const std::vector<int>& cost) {
    assert(gas.size() == cost.size());
    const std::size_t n = gas.size();

    long long total = 0;   // sum of all diffs -> decides global feasibility
    long long tank = 0;    // running fuel since the current candidate start
    int start = 0;         // current candidate starting station

    for (std::size_t i = 0; i < n; ++i) {
        const long long diff = static_cast<long long>(gas[i]) - cost[i];
        total += diff;
        tank += diff;
        if (tank < 0) {
            // Everything in [start .. i] fails; the earliest possible start that
            // could still succeed is i+1. Reset the local tank accordingly.
            start = static_cast<int>(i) + 1;
            tank = 0;
        }
    }

    // If the whole loop's net fuel is negative, no start works.
    return (total >= 0) ? start : -1;
}

// ------------------------ Brute-force verifier ------------------------
// For small inputs, try EVERY start and simulate the full loop. O(N^2). Used
// only to cross-check the greedy answer in the tests below.
int bruteForceStart(const std::vector<int>& gas, const std::vector<int>& cost) {
    const std::size_t n = gas.size();
    for (std::size_t s = 0; s < n; ++s) {
        long long tank = 0;
        bool ok = true;
        for (std::size_t step = 0; step < n; ++step) {
            const std::size_t i = (s + step) % n;
            tank += static_cast<long long>(gas[i]) - cost[i];
            if (tank < 0) { ok = false; break; }  // stranded before next station
        }
        if (ok) return static_cast<int>(s);
    }
    return -1;
}

int main() {
    // --- Solvable instance (classic LeetCode example) ---
    // gas  = [1, 2, 3, 4, 5]
    // cost = [3, 4, 5, 1, 2]
    // diff = [-2,-2,-2, 3, 3]  total = 0 -> feasible. Unique start is index 3.
    {
        const std::vector<int> gas  = {1, 2, 3, 4, 5};
        const std::vector<int> cost = {3, 4, 5, 1, 2};
        const int greedy = canCompleteCircuit(gas, cost);
        assert(greedy == 3);                       // known optimum
        assert(greedy == bruteForceStart(gas, cost));  // agrees with brute force
    }

    // --- Unsolvable instance ---
    // gas  = [2, 3, 4]
    // cost = [3, 4, 3]
    // total = (2-3)+(3-4)+(4-3) = -1 < 0 -> no start can finish. Expect -1.
    {
        const std::vector<int> gas  = {2, 3, 4};
        const std::vector<int> cost = {3, 4, 3};
        const int greedy = canCompleteCircuit(gas, cost);
        assert(greedy == -1);
        assert(greedy == bruteForceStart(gas, cost));
    }

    // --- A larger feasible instance, verified against brute force ---
    // gas  = [5, 1, 2, 3, 4]
    // cost = [4, 4, 1, 5, 1]
    // diff = [1,-3, 1,-2, 3]  total = 0 -> feasible. We let brute force decide the
    // expected start rather than hardcoding a possibly-wrong guess.
    {
        const std::vector<int> gas  = {5, 1, 2, 3, 4};
        const std::vector<int> cost = {4, 4, 1, 5, 1};
        const int greedy = canCompleteCircuit(gas, cost);
        assert(greedy == bruteForceStart(gas, cost));
    }

    // --- Single station: completable iff its own gas covers its own cost ---
    {
        assert(canCompleteCircuit({5}, {3}) == 0);   // 5 >= 3, loop of length 1
        assert(canCompleteCircuit({2}, {4}) == -1);  // cannot even leave
    }

    // --- Exhaustive cross-check against brute force on many small instances ---
    // Enumerate all gas/cost vectors of length 3 with entries in {0,1,2,3}.
    {
        const int R = 4;  // values 0..3
        for (int a = 0; a < R; ++a)
        for (int b = 0; b < R; ++b)
        for (int c = 0; c < R; ++c)
        for (int x = 0; x < R; ++x)
        for (int y = 0; y < R; ++y)
        for (int z = 0; z < R; ++z) {
            const std::vector<int> gas  = {a, b, c};
            const std::vector<int> cost = {x, y, z};
            const int greedy = canCompleteCircuit(gas, cost);
            const int brute  = bruteForceStart(gas, cost);
            // The greedy must always find a valid start whenever one exists, and
            // the completed loop it reports must itself be valid. (When multiple
            // starts happen to work in degenerate all-zero cases, both routines
            // pick the smallest feasible index, so they match exactly.)
            assert(greedy == brute);
        }
    }

    // ------------------------------ Demo ------------------------------
    {
        const std::vector<int> gas  = {1, 2, 3, 4, 5};
        const std::vector<int> cost = {3, 4, 5, 1, 2};
        const int start = canCompleteCircuit(gas, cost);
        std::cout << "Gas Station: starting index to complete the circuit = "
                  << start << '\n';
    }
    {
        const std::vector<int> gas  = {2, 3, 4};
        const std::vector<int> cost = {3, 4, 3};
        std::cout << "Gas Station: unsolvable instance returns = "
                  << canCompleteCircuit(gas, cost) << '\n';
    }

    std::cout << "All Gas Station tests passed.\n";
    return 0;
}
