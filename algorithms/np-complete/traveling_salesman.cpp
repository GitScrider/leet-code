/*
 * ============================================================================
 * Traveling Salesman Problem (TSP)
 * Category: Algorithm - NP-Hard (optimization) / NP-Complete (decision)
 *
 * DECISION PROBLEM:
 *   Given n cities, a distance dist[i][j] between each pair, and a bound B, is
 *   there a tour -- a cyclic order visiting every city exactly once and
 *   returning to the start -- of total length <= B? The OPTIMIZATION version
 *   asks for the shortest such tour.
 *
 * COMPLEXITY CLASS:
 *   The decision version is NP-complete; the optimization version is NP-hard.
 *   Hardness follows from a reduction from Hamiltonian Cycle: give existing
 *   edges weight 1 and non-edges weight 2 (or infinity) over n cities; a tour
 *   of length n exists iff the original graph has a Hamiltonian cycle. Since
 *   Hamiltonian Cycle is NP-complete, so is TSP's decision form.
 *
 * EXACT ALGORITHM -- Held-Karp dynamic programming over subsets:
 *   dp[mask][i] = minimum cost of a path that starts at city 0, visits exactly
 *                 the set of cities in `mask`, and currently ends at city i
 *                 (with 0 and i both in mask).
 *   Answer = min over i != 0 of dp[full][i] + dist[i][0]   (close the loop).
 *   +-----------------------+---------------------------+
 *   | Aspect                | Cost                      |
 *   +-----------------------+---------------------------+
 *   | Held-Karp time        | O(2^n * n^2)              |
 *   | Held-Karp space       | O(2^n * n)                |
 *   | Brute force (oracle)  | O(n!)                     |
 *   | Nearest-neighbor      | O(n^2)  (heuristic only)  |
 *   +-----------------------+---------------------------+
 *   Held-Karp beats the O(n!) permutation search by memoizing over the 2^n
 *   subsets of "already visited" cities instead of full orderings.
 *
 *   APPROXIMATION NOTE: nearest-neighbor is a fast heuristic with NO constant
 *   factor guarantee -- on metric instances it can be Theta(log n) times the
 *   optimum, and on non-metric ones arbitrarily bad. We therefore only assert
 *   the always-true fact that its tour is valid and its length >= the optimum,
 *   and we exhibit an instance where it is strictly suboptimal. (A constant
 *   ratio needs the triangle inequality plus a smarter method such as
 *   Christofides' 1.5-approximation.)
 *
 * KEY POINTS:
 *   - Subsets are bitmasks over uint32_t: bit i set == "city i is in the set".
 *     We only process masks containing bit 0, since every path starts at 0.
 *   - Iterating masks in increasing integer order guarantees every subset is
 *     finished before any superset that extends it (adding a city only sets
 *     more bits, i.e. increases the mask value).
 *   - INF is numeric_limits<int>::max()/4 so that INF + a distance cannot
 *     overflow; unreachable states are skipped, never added to.
 *   - The reconstructed tour is verified: a permutation of all cities whose
 *     recomputed length equals the DP optimum, cross-checked against brute force.
 * ============================================================================
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numeric>
#include <vector>

using Matrix = std::vector<std::vector<int>>;

// Guarded sentinel: adding any realistic distance to INF stays below overflow.
static const int INF = std::numeric_limits<int>::max() / 4;

// Length of a closed tour (visits tour[0..n-1] in order, then returns to
// tour[0]). Assumes `tour` is a permutation of 0..n-1.
static long long tourLength(const Matrix& dist, const std::vector<int>& tour) {
    long long total = 0;
    const std::size_t n = tour.size();
    for (std::size_t k = 0; k + 1 < n; ++k)
        total += dist[static_cast<std::size_t>(tour[k])]
                     [static_cast<std::size_t>(tour[k + 1])];
    if (!tour.empty())
        total += dist[static_cast<std::size_t>(tour[n - 1])]
                     [static_cast<std::size_t>(tour[0])];
    return total;
}

// Is `tour` a valid Hamiltonian ordering: length n, each city 0..n-1 once?
static bool isValidTour(const std::vector<int>& tour, int n) {
    if (static_cast<int>(tour.size()) != n) return false;
    std::vector<char> seen(static_cast<std::size_t>(n), 0);
    for (int city : tour) {
        if (city < 0 || city >= n) return false;
        if (seen[static_cast<std::size_t>(city)]) return false; // repeat
        seen[static_cast<std::size_t>(city)] = 1;
    }
    return true;
}

struct TspResult {
    long long cost;          // optimal tour length
    std::vector<int> tour;   // one optimal ordering, starting at city 0
};

// EXACT solver via Held-Karp DP over visited-set bitmasks.
static TspResult heldKarp(const Matrix& dist) {
    const int n = static_cast<int>(dist.size());
    if (n <= 1) return {0, std::vector<int>(static_cast<std::size_t>(n), 0)};

    const std::uint32_t full = (1u << n) - 1u; // all cities visited
    const std::size_t states = static_cast<std::size_t>(1u << n);

    // dp[mask][i]; parent[mask][i] = city visited just before i on the best path.
    std::vector<std::vector<int>> dp(
        states, std::vector<int>(static_cast<std::size_t>(n), INF));
    std::vector<std::vector<int>> parent(
        states, std::vector<int>(static_cast<std::size_t>(n), -1));

    dp[1][0] = 0; // start: only city 0 visited (mask == bit 0), path ends at 0

    for (std::uint32_t mask = 1; mask <= full; ++mask) {
        if (!(mask & 1u)) continue;              // every path must include city 0
        const std::size_t m = static_cast<std::size_t>(mask);
        for (int i = 0; i < n; ++i) {
            if (!(mask & (1u << i))) continue;    // i must be inside the set
            const int cur = dp[m][static_cast<std::size_t>(i)];
            if (cur >= INF) continue;             // unreachable -> never extend
            for (int j = 0; j < n; ++j) {
                if (mask & (1u << j)) continue;   // j already visited
                const std::uint32_t next = mask | (1u << j);
                const int cand = cur + dist[static_cast<std::size_t>(i)]
                                            [static_cast<std::size_t>(j)];
                int& slot = dp[static_cast<std::size_t>(next)]
                              [static_cast<std::size_t>(j)];
                if (cand < slot) {
                    slot = cand;
                    parent[static_cast<std::size_t>(next)]
                          [static_cast<std::size_t>(j)] = i;
                }
            }
        }
    }

    // Close the tour: return from the last city back to 0, pick the cheapest.
    int bestEnd = -1, bestCost = INF;
    for (int i = 1; i < n; ++i) {
        const int c = dp[static_cast<std::size_t>(full)][static_cast<std::size_t>(i)];
        if (c >= INF) continue;
        const int closed = c + dist[static_cast<std::size_t>(i)][0];
        if (closed < bestCost) {
            bestCost = closed;
            bestEnd = i;
        }
    }

    // Reconstruct the ordering by walking parent pointers back to city 0.
    std::vector<int> tour;
    std::uint32_t mask = full;
    int cur = bestEnd;
    while (cur != -1) {
        tour.push_back(cur);
        const int prev = parent[static_cast<std::size_t>(mask)]
                               [static_cast<std::size_t>(cur)];
        mask &= ~(1u << cur); // remove cur from the set as we step backwards
        cur = prev;
    }
    std::reverse(tour.begin(), tour.end()); // now starts at city 0
    return {static_cast<long long>(bestCost), tour};
}

// Brute-force oracle: try every ordering of cities 1..n-1 (city 0 fixed as the
// start). O(n!); used to certify Held-Karp on small inputs.
static long long bruteForceTSP(const Matrix& dist) {
    const int n = static_cast<int>(dist.size());
    if (n <= 1) return 0;
    std::vector<int> perm(static_cast<std::size_t>(n) - 1);
    std::iota(perm.begin(), perm.end(), 1); // {1, 2, ..., n-1}
    long long best = std::numeric_limits<long long>::max();
    do {
        std::vector<int> tour;
        tour.reserve(static_cast<std::size_t>(n));
        tour.push_back(0);
        tour.insert(tour.end(), perm.begin(), perm.end());
        best = std::min(best, tourLength(dist, tour));
    } while (std::next_permutation(perm.begin(), perm.end()));
    return best;
}

// Nearest-neighbor HEURISTIC: from city 0, repeatedly hop to the closest
// unvisited city. Fast but not optimal (see header note).
static std::vector<int> nearestNeighbor(const Matrix& dist) {
    const int n = static_cast<int>(dist.size());
    std::vector<char> visited(static_cast<std::size_t>(n), 0);
    std::vector<int> tour;
    if (n == 0) return tour;
    tour.reserve(static_cast<std::size_t>(n));
    int cur = 0;
    visited[0] = 1;
    tour.push_back(0);
    for (int step = 1; step < n; ++step) {
        int best = -1, bestDist = INF;
        for (int j = 0; j < n; ++j) {
            if (visited[static_cast<std::size_t>(j)]) continue;
            const int d = dist[static_cast<std::size_t>(cur)]
                              [static_cast<std::size_t>(j)];
            if (d < bestDist) {
                bestDist = d;
                best = j;
            }
        }
        visited[static_cast<std::size_t>(best)] = 1;
        tour.push_back(best);
        cur = best;
    }
    return tour;
}

int main() {
    // --- Known 4-city instance; optimal tour 0->1->3->2->0 has length 80. ---
    const Matrix dist = {
        {0, 10, 15, 20},
        {10, 0, 35, 25},
        {15, 35, 0, 30},
        {20, 25, 30, 0},
    };
    {
        const TspResult r = heldKarp(dist);
        assert(r.cost == 80);
        assert(isValidTour(r.tour, 4));            // valid permutation of cities
        assert(r.tour.front() == 0);               // starts at city 0
        assert(tourLength(dist, r.tour) == r.cost); // recomputed length matches
        assert(bruteForceTSP(dist) == 80);         // oracle agrees
    }

    // --- Edge cases. ---
    assert(heldKarp(Matrix{{0}}).cost == 0);       // single city: nothing to do
    {
        const Matrix two = {{0, 7}, {7, 0}};       // 2 cities: go and come back
        const TspResult r = heldKarp(two);
        assert(r.cost == 14 && isValidTour(r.tour, 2));
    }

    // --- Cross-check Held-Karp against brute force on several instances. ---
    {
        const std::vector<Matrix> suite = {
            {{0, 2, 9, 10}, {1, 0, 6, 4}, {15, 7, 0, 8}, {6, 3, 12, 0}}, // asym.
            {{0, 4, 8, 9, 12},
             {4, 0, 6, 8, 9},
             {8, 6, 0, 10, 11},
             {9, 8, 10, 0, 7},
             {12, 9, 11, 7, 0}},
            {{0, 3, 4, 2, 7, 3},
             {3, 0, 4, 6, 3, 4},
             {4, 4, 0, 5, 8, 5},
             {2, 6, 5, 0, 6, 4},
             {7, 3, 8, 6, 0, 4},
             {3, 4, 5, 4, 4, 0}},
        };
        for (const Matrix& m : suite) {
            const TspResult r = heldKarp(m);
            const int n = static_cast<int>(m.size());
            assert(isValidTour(r.tour, n));
            assert(tourLength(m, r.tour) == r.cost);
            assert(r.cost == bruteForceTSP(m));     // EXACT match with the oracle
        }
    }

    // --- Nearest-neighbor: valid tour and a valid UPPER BOUND (>= optimum). ---
    {
        const std::vector<int> nn = nearestNeighbor(dist);
        assert(isValidTour(nn, 4));
        const long long nnCost = tourLength(dist, nn);
        const long long opt = heldKarp(dist).cost;
        assert(nnCost >= opt);                      // never beats the optimum
    }

    // --- Demonstrate that nearest-neighbor can be STRICTLY suboptimal. ---
    // From 0 it greedily grabs the cheapest edge 0->1, but every edge LEAVING
    // city 1 is ruinously expensive (100), so the greedy walk is forced through
    // one. The optimum instead keeps city 1 for last, entering it cheaply and
    // closing on the cheap 1->0 edge. (Distances are directional / asymmetric.)
    {
        const Matrix trap = {
            {0, 1, 2, 3},
            {1, 0, 100, 100},
            {2, 1, 0, 1},
            {3, 1, 1, 0},
        };
        const long long nnCost = tourLength(trap, nearestNeighbor(trap));
        const long long opt = heldKarp(trap).cost;
        assert(opt == bruteForceTSP(trap));         // optimum is 5 ...
        assert(nnCost > opt);                       // ... but greedy pays 105
    }

    // --- Short demo. ---
    const TspResult demo = heldKarp(dist);
    std::cout << "Optimal TSP tour on the 4-city instance: ";
    for (std::size_t k = 0; k < demo.tour.size(); ++k)
        std::cout << demo.tour[k] << (k + 1 < demo.tour.size() ? " -> " : "");
    std::cout << " -> " << demo.tour.front() << "  (length " << demo.cost << ")\n";
    std::cout << "Nearest-neighbor length: "
              << tourLength(dist, nearestNeighbor(dist)) << " (>= optimum)\n";
    std::cout << "All TSP tests passed.\n";
    return 0;
}
